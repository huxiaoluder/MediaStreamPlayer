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
        kVTCompressionPropertyKey_AllowFrameReordering, // 是否允许打乱顺序(是否允许编码 B 帧)
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
        kCFBooleanFalse,
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

void
APEncoder::beginEncode() {
    videoPts = 0;
    audioPts = 0;

    int ret = avio_open(&outputFormatContext->pb, filePath.c_str(), AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        return;
    }
    
    if (videoEncoderSession) {
        VTCompressionSessionPrepareToEncodeFrames(videoEncoderSession);
    }
    
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

void
APEncoder::compressionOutputCallback(void * MSNullable outputCallbackRefCon,
                                     void * MSNullable sourceFrameRefCon,
                                     OSStatus status,
                                     VTEncodeInfoFlags infoFlags,
                                     CMSampleBufferRef MSNullable sampleBuffer) {
    int width = (int)CVPixelBufferGetWidth((CVPixelBufferRef)sourceFrameRefCon);
    int height = (int)CVPixelBufferGetHeight((CVPixelBufferRef)sourceFrameRefCon);
    CVPixelBufferRelease((CVPixelBufferRef)sourceFrameRefCon);

    if (status) {
        OSStatusErrorLocationLog("APEncode frame fail",status);
        return;
    }

    uint8_t separator[] = {0x00,0x00,0x00,0x01};
    
    if (CMSampleBufferDataIsReady(sampleBuffer)) {
        
        APEncoder &encoder = *(APEncoder *)outputCallbackRefCon;
    
        bool isKeyFrame = CMSampleBufferIsKeyFrame(sampleBuffer);
        if (encoder.videoStream == nullptr) {
            if (!isKeyFrame) { return; }
            encoder.videoStream = avformat_new_stream(encoder.outputFormatContext, nullptr);
            encoder.videoStream->time_base = {1, 20};
            AVCodecParameters &codecpar = *encoder.videoStream->codecpar;
            
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
                const uint8_t *newSpsData;
                size_t newSpsLen;
                // 硬编码出来的数据中, sps 里不含 framerate, 需要自己添加(大坑一个!!!)
                insertFramerateToSps(20, spsData, spsLen, &newSpsData, &newSpsLen);

                codecpar.codec_type = AVMEDIA_TYPE_VIDEO;
                codecpar.codec_id = FFmpeg::FFCodecContext::getAVCodecId(encoder.videoCodecID);
                codecpar.codec_tag = 0;
                codecpar.extradata_size = (int)newSpsLen + 4 * 2 + (int)ppsLen;
                codecpar.extradata = (uint8_t *)av_malloc(codecpar.extradata_size);
                codecpar.format = AV_PIX_FMT_YUVJ420P;
                codecpar.bit_rate = width * height * 3 * 2 * 8;
                codecpar.profile = FF_PROFILE_H264_HIGH;
                codecpar.width = width;
                codecpar.height = height;
                codecpar.color_range = AVCOL_RANGE_JPEG;
                
                int offset = 0;
                memcpy(codecpar.extradata + offset, separator, 4);
                offset += 4;
                memcpy(codecpar.extradata + offset, newSpsData, newSpsLen);
                offset += newSpsLen;
                memcpy(codecpar.extradata + offset, separator, 4);
                offset += 4;
                memcpy(codecpar.extradata + offset, ppsData, ppsLen);
                
                delete [] newSpsData;
                
                av_dump_format(encoder.outputFormatContext, 0, encoder.filePath.c_str(), 1);
                
                int ret = avformat_write_header(encoder.outputFormatContext, nullptr);
                if (ret < 0) {
                    ErrorLocationLog(av_err2str(ret));
                    return;
                }
            }
        }

        CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
        size_t dataLen;
        char * dataPtr;
        CMBlockBufferGetDataPointer(blockBuffer, 0, nullptr, &dataLen, &dataPtr);
        
        uint32_t len = 0;
        uint32_t offset = 0;
        do {
            len = getReverse4Bytes(*(uint32_t *)(dataPtr + offset));
            memcpy(dataPtr + offset, separator, 4);
            offset = offset + 4 + len;
        } while (offset < dataLen);
        
        encoder.videoPts += 1;
        
        AVPacket packet;
        av_init_packet(&packet);
        packet.data = (uint8_t *)dataPtr;
        packet.size = (int)dataLen;
        packet.pts = encoder.videoPts;
        packet.dts = encoder.videoPts;
        AVRational time_base{1,20};
        av_packet_rescale_ts(&packet, time_base, encoder.videoStream->time_base);
        if (isKeyFrame) {
            packet.flags = AV_PKT_FLAG_KEY;
        }
        
        int ret = av_interleaved_write_frame(encoder.outputFormatContext, &packet);
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
        }
        printf("------------- %lld\n", encoder.videoPts);
        
        if (encoder.videoPts == 400) {
            avio_flush(encoder.outputFormatContext->pb);
            int ret = av_write_trailer(encoder.outputFormatContext);
            avio_closep(&encoder.outputFormatContext->pb);
            avformat_free_context(encoder.outputFormatContext);
            exit(0);
        }
    }
}
