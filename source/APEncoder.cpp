//
//  APEncoder.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "APEncoder.hpp"

using namespace MS;
using namespace MS::APhard;

/**
 设置编码会话配置

 @param videoEncoderSession 编码会话
 @param videoParameters 视频信息参数
 */
static void
setupVideoSessionProperties(const VTCompressionSessionRef videoEncoderSession,
                            const MSVideoParameters &videoParameters) {
    // GOP size
    int frameInterval = 100;
    CFNumberRef frameIntervalRef = CFNumberCreate(kCFAllocatorDefault,
                                                  kCFNumberIntType,
                                                  &frameInterval);
    // 帧率
    int framerate = videoParameters.frameRate;
    CFNumberRef framerateRef = CFNumberCreate(kCFAllocatorDefault,
                                              kCFNumberIntType,
                                              &framerate);
    // 码率最大值
    int bitRateLimit = videoParameters.width * videoParameters.height * 3 * 2;
    CFNumberRef bitRateLimitRef = CFNumberCreate(kCFAllocatorDefault,
                                                 kCFNumberIntType,
                                                 &bitRateLimit);
    // 码率平均值
    int bitRateAvera = bitRateLimit * 8;
    CFNumberRef bitRateAveraRef = CFNumberCreate(kCFAllocatorDefault,
                                                 kCFNumberIntType,
                                                 &bitRateAvera);
    const void *keys[] = {
        // 编码器实时编码输出(在线编码, 要求实时性, 离线编码不要求实时性, 设为 false, 可以保证更好的编码效果)
        kVTCompressionPropertyKey_RealTime,
        kVTCompressionPropertyKey_ProfileLevel,         // 编码器配置级别
        kVTCompressionPropertyKey_MaxKeyFrameInterval,  // 编码 GOP 大小
        // 注: 并没有卵用(只用于提示编码器初始化内部配置, 可以不配置), 生成的 SPS 中不会带有帧率信息
        // 注: 实际帧速率将取决于帧持续时间，并且可能会有所不同。
        kVTCompressionPropertyKey_ExpectedFrameRate,    // 编码期望帧率
        // 以下两值相同 ==> 恒定码率编码 CBR
        kVTCompressionPropertyKey_DataRateLimits,       // 码率最大值，单位: bytes per second
        kVTCompressionPropertyKey_AverageBitRate        // 码率平均值，单位: bits  per second
    };
    const void *values[] = {
        kCFBooleanTrue,
        kVTProfileLevel_H264_High_AutoLevel,
        frameIntervalRef,
        framerateRef,
        bitRateLimitRef,
        bitRateAveraRef
    };
    CFDictionaryRef properties = CFDictionaryCreate(kCFAllocatorDefault,
                                                    keys,
                                                    values,
                                                    sizeof(keys) / sizeof(void *),
                                                    &kCFTypeDictionaryKeyCallBacks,
                                                    &kCFTypeDictionaryValueCallBacks);
    OSStatus status = VTSessionSetProperties(videoEncoderSession, properties);
    if (status) {
        OSStatusErrorLocationLog("fail to set properties", status);
    }
    CFRelease(frameIntervalRef);
    CFRelease(framerateRef);
    CFRelease(bitRateLimitRef);
    CFRelease(bitRateAveraRef);
    CFRelease(properties);
}

/**
 获取 CMSampleBufferRef 是否是 I-frame
 
 @param sampleBuffer sampleBuffer
 @return 是否为关键帧
 */
static inline bool
CMSampleBufferIsKeyFrame(CMSampleBufferRef const MSNonnull sampleBuffer) {
    /*
     返回值描述: one dictionary per sample in the CMSampleBuffer ==> 返回值长度长度最大为 1
     注意: 1. 第二个参数为真, 不论 buffer 中是否有附件, 都返回一个含有对应 sampleBuffer 个数个 CFMutableDictionaryRef 的 CFArrayref,
     其中 CFMutableDictionaryRef 中内容可能为空(不含附件时)
     2. 反之, 若 buffer 中没有附件, 则返回 null.
     */
    CFArrayRef attachments = CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
    
    if (attachments) {
        CFMutableDictionaryRef attachment = (CFMutableDictionaryRef)CFArrayGetValueAtIndex(attachments, 0);
        return CFDictionaryGetValue(attachment, kCMSampleAttachmentKey_DependsOnOthers) == kCFBooleanFalse;
    }
    return false;
}

/**
 获取 CMSampleBufferRef 的数据编码信息
 
 @param sampleBuffer    sampleBuffer
 @param dataTypeNameOut 数据编码格式名, need not free
 @param spsDataOut  sps 拷贝指针, need not free
 @param ppsDataOut  pps 拷贝指针, need not free
 @param spsLenOut   sps 长度
 @param ppsLenOut   pps 长度
 @param nalUnitHeaderLenOut 编码数据头长度(数据头为 视频编码数据的长度)
 @return 是否获取成功
 */
static inline bool
CMSampleBufferGetEncodeInfo(CMSampleBufferRef const MSNonnull sampleBuffer,
                            CFStringRef * const MSNullable dataTypeNameOut,
                            const uint8_t * * const MSNullable spsDataOut,
                            const uint8_t * * const MSNullable ppsDataOut,
                            size_t * const MSNullable spsLenOut,
                            size_t * const MSNullable ppsLenOut,
                            int * const MSNullable nalUnitHeaderLenOut) {
    CMFormatDescriptionRef fmtDesc = CMSampleBufferGetFormatDescription(sampleBuffer);
    
    if (dataTypeNameOut) {
        CFDictionaryRef extensions = CMFormatDescriptionGetExtensions(fmtDesc);
        *dataTypeNameOut = (CFStringRef)CFDictionaryGetValue(extensions, kCMFormatDescriptionExtension_FormatName);
    }
    
    
    size_t  parameterSetCount;
    OSStatus status = CMVideoFormatDescriptionGetH264ParameterSetAtIndex(fmtDesc,
                                                                         NULL, // out 参数: 指针 和 长度 都为空时, 该参数被忽略
                                                                         nullptr,
                                                                         nullptr,
                                                                         &parameterSetCount,    // fmtDesc 含有的 parameter 总数
                                                                         nalUnitHeaderLenOut);  // nalUnit 数据头长度
    if (status) {
        OSStatusErrorLocationLog("call CMVideoFormatDescriptionGetH264ParameterSetAtIndex fail",status);
        return false;
    }
    if (parameterSetCount < 2) { // h264 数据至少含有 sps, pps
        ErrorLocationLog("fail to get video parameterSet");
        return false;
    }
    // 获取 sps
    CMVideoFormatDescriptionGetH264ParameterSetAtIndex(fmtDesc, 0,
                                                       spsDataOut,
                                                       spsLenOut,
                                                       nullptr,
                                                       nullptr);
    // 获取 pps
    CMVideoFormatDescriptionGetH264ParameterSetAtIndex(fmtDesc, 1,
                                                       ppsDataOut,
                                                       ppsLenOut,
                                                       nullptr,
                                                       nullptr);
    return true;
}
static FILE *file = nullptr;
void
APEncoder::beginEncode() {
    videoPts = 0;
    audioPts = 0;

    if (videoEncoderSession) {
        VTCompressionSessionPrepareToEncodeFrames(videoEncoderSession);
    }
    file = fopen(filePath.c_str(), "wb");
    _isEncoding = true;
}

void
APEncoder::encodeVideo(const APEncoderInputMedia &pixelData) {
    assert(_isEncoding);
    
    if (videoEncoderSession) {
        
        CMTime presentationTimeStamp { // 帧显示时间戳 pts
            .value = (int)videoPts,
            .timescale = 0,
            .flags = kCMTimeFlags_Valid,
            .epoch = 0
        };
        
        // 实际控制帧率的属性, 附带在 buffer 的时间信息中, 对编码器无用, 可以传 kCMTimeInvalid
        CMTime duration { // 帧播放持续时间
            .value = 1,
            .timescale = pixelData.frame->videoParameters.frameRate,
            .flags = kCMTimeFlags_Valid,
            .epoch = 0
        };

        OSStatus status = VTCompressionSessionEncodeFrame(videoEncoderSession,
                                                          CVPixelBufferRetain(pixelData.frame->video),
                                                          presentationTimeStamp,
                                                          duration,
                                                          nullptr,
                                                          pixelData.frame->video,
                                                          nullptr);
        if (status) {
            OSStatusErrorLocationLog("call VTCompressionSessionEncodeFrame fail",status);
        }
    }
}

void
APEncoder::encodeAudio(const APEncoderInputMedia &sampleData) {
    
}

void
APEncoder::endEncode() {
    _isEncoding = false;
    releaseEncoderConfiguration();
}

bool
APEncoder::isEncoding() {
    return _isEncoding;
}

APEncoder::APEncoder(const MSCodecID videoCodecID,
                     const MSCodecID audioCodecID)
:videoCodecID(videoCodecID), audioCodecID(audioCodecID) {
    
}

APEncoder::~APEncoder() {
    
}

bool
APEncoder::configureEncoder(const string &muxingfilePath,
                            const MSVideoParameters * MSNullable const videoParameters,
                            const MSAudioParameters * MSNullable const audioParameters) {
    filePath = muxingfilePath;
    
    outputFormatContext = configureOutputFormatContext();
    if (!outputFormatContext) {
        releaseEncoderConfiguration();
        return false;
    }
    
    if (videoParameters) {
        videoEncoderSession = configureVideoEncoderSession(*videoParameters);
        if (!videoEncoderSession) {
            releaseEncoderConfiguration();
            return false;
        }
    }
    
//    if (audioParameters) {
//        audioEncoderConvert = configureAudioEncoderConvert(*audioParameters);
//        if (!audioParameters) {
//            releaseEncoderConfiguration();
//            return false;
//        }
//    }
    
    return true;
}

AVFormatContext *
APEncoder::configureOutputFormatContext() {
    AVFormatContext *outputFormatContext = nullptr;
    int ret = avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, filePath.c_str());
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        return nullptr;
    }
    
    outputFormatContext->oformat->video_codec = FFmpeg::FFCodecContext::getAVCodecId(videoCodecID);
    outputFormatContext->oformat->audio_codec = FFmpeg::FFCodecContext::getAVCodecId(audioCodecID);
    
    return outputFormatContext;
}

VTCompressionSessionRef
APEncoder::configureVideoEncoderSession(const MSVideoParameters &videoParameters) {
    
    OSStatus status = noErr;
    
    VTCompressionSessionRef videoEncoderSession = nullptr;
    
    status = VTCompressionSessionCreate(kCFAllocatorDefault,
                                        videoParameters.width,
                                        videoParameters.height,
                                        kCMVideoCodecType_H264,
                                        nullptr,
                                        nullptr,
                                        kCFAllocatorDefault,
                                        compressionOutputCallback,
                                        this,
                                        &videoEncoderSession);
    if (status) {
        OSStatusErrorLocationLog("fail to instance VTCompressionSessionRef", status);
    }
    
    if (videoEncoderSession) {
        setupVideoSessionProperties(videoEncoderSession, videoParameters);
    }
    
    {
//        using namespace FFmpeg;
//
//        FFCodecContext *videoEncoderContext = new FFCodecContext(FFCodecEncoder,videoCodecID);
//        AVCodecContext &encoderContext = *videoEncoderContext->codec_ctx;
//
//        // 编码器参数配置
//        AVDictionary *dict = nullptr;
//        if (outputFormatContext->oformat->video_codec == AV_CODEC_ID_H264) {
//            av_dict_set(&dict, "preset", "medium", 0);
//            av_dict_set(&dict, "tune", "zerolatency", 0);
//
//            encoderContext.profile  = FF_PROFILE_H264_HIGH;
//        }
//
//        encoderContext.pix_fmt      = AV_PIX_FMT_YUVJ420P;
//        encoderContext.width        = videoParameters.width;
//        encoderContext.height       = videoParameters.height;
//        encoderContext.bit_rate     = videoParameters.width * videoParameters.height * 3 * 2;
//        encoderContext.gop_size     = 100;
//        encoderContext.max_b_frames = 0;
//        encoderContext.time_base    = { 1, 20 };
//        encoderContext.sample_aspect_ratio = { 16, 9 };
//        if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
//            encoderContext.flags   |= AV_CODEC_FLAG_GLOBAL_HEADER;
//        }
//
//        av_dict_free(&dict);
//
//        AVStream &outStream = *avformat_new_stream(outputFormatContext, videoEncoderContext->codec);
//
//        int ret = avcodec_parameters_from_context(outStream.codecpar, &encoderContext);
//        if (ret < 0) {
//            ErrorLocationLog(av_err2str(ret));
//            delete videoEncoderContext;
//            return nullptr;
//        }
//        delete videoEncoderContext;
//
//        outStream.time_base = encoderContext.time_base;
//
//        videoStream = &outStream;
    }
    
    return videoEncoderSession;
}

AudioConverterRef
APEncoder::configureAudioEncoderConvert(const MSAudioParameters &audioParameters) {
    AudioConverterRef audioEncoderConvert = nullptr;
    return audioEncoderConvert;
}

void
APEncoder::releaseEncoderConfiguration() {
    if (videoEncoderSession) {
        VTCompressionSessionCompleteFrames(videoEncoderSession, kCMTimeInvalid);
        VTCompressionSessionInvalidate(videoEncoderSession);
        videoEncoderSession = nullptr;
    }
    if (audioEncoderConvert) {
        AudioConverterDispose(audioEncoderConvert);
        audioEncoderConvert = nullptr;
    }
    if (outputFormatContext) {
        avformat_free_context(outputFormatContext);
        outputFormatContext = nullptr;
    }
}

static inline bool
detectBitValue(const uint8_t * const dataRef, const size_t &startLocation) {
    return dataRef[startLocation / 8] & (0x80 >> (startLocation % 8));
}

static inline void
skipBits(size_t &starLocation, const size_t skipLen) {
    starLocation += skipLen;
}

static void
skipGolombBits(const uint8_t * const dataRef, size_t &startLocation, const int times) {
    for (int i = 0; i < times; i++) {
        // 指数
        int index = 0;
        while (!detectBitValue(dataRef, startLocation)) {
            index++;
            startLocation++;
        }
        startLocation += (index + 1);
    }
}

static int
getBitsValue(const uint8_t * const dataRef, size_t &startLocation, const int bitsCount) {
    int value = 0;
    for (int i = 0; i < bitsCount; i++) {
        value <<= 1;
        if (detectBitValue(dataRef, startLocation)) {
            value |= 1;
        }
        startLocation++;
    }
    return value;
}

static int
ueGolomb(const uint8_t * const dataRef, size_t &startLocation) {
    // 指数
    int index = 0;
    while (!detectBitValue(dataRef, startLocation)) {
        index++;
        startLocation++;
    }
    
    // 跳过对称位 1
    startLocation++;
    
    // 去除指数的余值
    int value = getBitsValue(dataRef, startLocation, index);
    
    return (1 << index | value) - 1;
}

static int
seGolomb(const uint8_t * const dataRef, size_t &startLocation) {
    int ueValue = ueGolomb(dataRef, startLocation);
    double k = ueValue;
    // ceil函数求不小于给定实数的最小整数
    int newValue = ceil(k/2);
    return ueValue % 2 == 0 ? -newValue : newValue;
}

static void
skipH264ScalingList(const uint8_t * const spsRef, size_t &startLocation, size_t sizeOfScalingList) {
    int lastScale = 8;
    int nextScale = 8;
    int delta_scale;
    for (int i = 0; i < sizeOfScalingList; i++) {
        if (nextScale != 0) {
            delta_scale = seGolomb(spsRef, startLocation);
            nextScale = (lastScale + delta_scale + 256) % 256;
        }
        lastScale = nextScale == 0 ? lastScale : nextScale;
    }
}

static const uint8_t *
discardEmulationCode(const uint8_t * const sourceSpsRef, const size_t sourceSpsSize) {
    uint8_t * const realSps = new uint8_t[sourceSpsSize]{0};
    
    uint8_t * tempStart = realSps;
    int lastStartIdx = 0;
    size_t tempSize;
    
    for (int i = 3; i < sourceSpsSize; i++) {
        if (sourceSpsRef[i] == 0x03 &&
            *(uint32_t *)(sourceSpsRef + i - 2) << 8 == 0x03000000) {
            tempSize = i - lastStartIdx;
            memcpy(tempStart, sourceSpsRef+lastStartIdx, tempSize);
            tempStart += tempSize;
            lastStartIdx = i + 1;
            i += 2;
        }
    }
    
    tempSize = sourceSpsSize - lastStartIdx;
    memcpy(tempStart, sourceSpsRef+lastStartIdx, tempSize);
    return realSps;
}

static void
decode_h264_vui(const uint8_t * const spsRef, size_t &startLocation, int &frameRate) {
    int aspect_ratio_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(aspect_ratio_info_present_flag) {
        int aspect_ratio_idc = getBitsValue(spsRef, startLocation, 8);
        if(aspect_ratio_idc == 255/* Extended_SAR = 255 */) {
            skipBits(startLocation, 32);
        }
    }
    int overscan_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(overscan_info_present_flag) {
        skipBits(startLocation, 1);
    }
    int video_signal_type_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(video_signal_type_present_flag) {
        skipBits(startLocation, 4);
        int colour_description_present_flag = getBitsValue(spsRef, startLocation, 1);
        if(colour_description_present_flag) {
            skipBits(startLocation, 24);
        }
    }
    int chroma_loc_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (chroma_loc_info_present_flag) {
        skipGolombBits(spsRef, startLocation, 2);
    }
    int timing_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(timing_info_present_flag) {
        int num_units_in_tick = getBitsValue(spsRef, startLocation, 32);
        int time_scale = getBitsValue(spsRef, startLocation, 32);
        frameRate = time_scale / (2 * num_units_in_tick);
    }
}

static void
decode_h264_sps(const uint8_t * const sourceSpsRef, const size_t sourceSpsSize, MSVideoParameters &videoParameter) {
    
    const uint8_t * const realSps = discardEmulationCode(sourceSpsRef, sourceSpsSize);
    
    size_t startLocation = 0;
    
    skipBits(startLocation, 8);
    
    int profile_idc = getBitsValue(realSps, startLocation, 8);
    skipBits(startLocation, 16);
    skipGolombBits(realSps, startLocation, 1);
    
    if (profile_idc == 100 ||
        profile_idc == 110 ||
        profile_idc == 122 ||
        profile_idc == 144) {
        
        int chroma_format_idc = ueGolomb(realSps, startLocation);
        if(chroma_format_idc == 3) {
            skipBits(startLocation, 1);
        }
        skipGolombBits(realSps, startLocation, 2);
        skipBits(startLocation, 1);
        
        int seq_scaling_matrix_present_flag = getBitsValue(realSps, startLocation, 1);
        if(seq_scaling_matrix_present_flag) {
            for (int i = 0; i < 8; i++) {
                if (getBitsValue(realSps, startLocation, 1)) {
                    if (i < 6) {
                        skipH264ScalingList(realSps, startLocation, 16);
                    } else {
                        skipH264ScalingList(realSps, startLocation, 64);
                    }
                }
            }
        }
    }
    skipGolombBits(realSps, startLocation, 1);
    
    int pic_order_cnt_type = ueGolomb(realSps, startLocation);
    if(pic_order_cnt_type == 0) {
        skipGolombBits(realSps, startLocation, 1);
    } else if (pic_order_cnt_type == 1) {
        skipBits(startLocation, 1);
        skipGolombBits(realSps, startLocation, 2);
        skipGolombBits(realSps, startLocation, ueGolomb(realSps, startLocation));
    }
    skipGolombBits(realSps, startLocation, 1);
    skipBits(startLocation, 1);
    
    int pic_width_in_mbs_minus1         = ueGolomb(realSps, startLocation);
    int pic_height_in_map_units_minus1  = ueGolomb(realSps, startLocation);
    
    int frame_mbs_only_flag = getBitsValue(realSps, startLocation, 1);
    if(!frame_mbs_only_flag) {
        skipBits(startLocation, 1);
    }
    skipBits(startLocation, 1);
    
    int frame_cropping_flag = getBitsValue(realSps, startLocation, 1);
    if(frame_cropping_flag) {
        skipGolombBits(realSps, startLocation, 4);
    }
    
    int vui_parameters_present_flag = getBitsValue(realSps, startLocation, 1);
    if(vui_parameters_present_flag) {
        decode_h264_vui(realSps, startLocation, videoParameter.frameRate);
    }
    
    videoParameter.width  = (pic_width_in_mbs_minus1 + 1) * 16;
    // (主流的 1080p, 720p, 360p)按16字节对齐, 可能会产生8位的冗余长度, 需要去除
    videoParameter.height = (pic_height_in_map_units_minus1 + 1) * 16;
    if (videoParameter.height == 1088) {
        videoParameter.height = 1080;
    }
    
    delete [] realSps;
}
void
APEncoder::compressionOutputCallback(void * MSNullable outputCallbackRefCon,
                                     void * MSNullable sourceFrameRefCon,
                                     OSStatus status,
                                     VTEncodeInfoFlags infoFlags,
                                     CMSampleBufferRef MSNullable sampleBuffer) {
    CVPixelBufferRelease((CVPixelBufferRef)sourceFrameRefCon);

    if (status) {
        OSStatusErrorLocationLog("APEncode frame fail",status);
        return;
    }

    uint8_t separator[] = {0x00,0x00,0x00,0x01};
    
    if (CMSampleBufferDataIsReady(sampleBuffer)) {
        
        APEncoder &encoder = *(APEncoder *)outputCallbackRefCon;
    
        bool isKeyFrame = CMSampleBufferIsKeyFrame(sampleBuffer);
        if (isKeyFrame) {
            
            const uint8_t *spsData;
            const uint8_t *ppsData;
            size_t spsLen;
            size_t ppsLen;
            bool ret = CMSampleBufferGetEncodeInfo(sampleBuffer,
                                                   nullptr,
                                                   &spsData,
                                                   &ppsData,
                                                   &spsLen,
                                                   &ppsLen,
                                                   &encoder.nalUnitHeaderLen);
            if (ret) {
                fwrite(separator, sizeof(separator), 1, file);
                fwrite(spsData, spsLen, 1, file);
                fwrite(separator, sizeof(separator), 1, file);
                fwrite(ppsData, ppsLen, 1, file);
                MSVideoParameters vp;
                decode_h264_sps(spsData, spsLen, vp);
                printf("+++++++++++++ %p\n", spsData);
            }
        }

        CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
        size_t dataLen;
        char * dataPtr;
        CMBlockBufferGetDataPointer(blockBuffer, 0, nullptr, &dataLen, &dataPtr);
        
        uint32_t len = 0;
        do {
            len = CFSwapInt32BigToHost(*(uint32_t *)dataPtr);
            fwrite(separator, sizeof(separator), 1, file);
            
            dataPtr += 4;
            fwrite(dataPtr, len, 1, file);
            
            dataPtr += len;
            dataLen -= (4 + len);
        } while (dataLen);
        
        printf("------------- %lld\n", encoder.videoPts);
        
        if (encoder.videoPts++ == 399) {
            fclose(file);
            exit(0);
        }
    }
}
