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
        kVTCompressionPropertyKey_ExpectedFrameRate,    // 编码期望帧率
        // 以下两值相同 ==> 恒定码率编码 CBR
        kVTCompressionPropertyKey_DataRateLimits,       // 码率最大值，单位: bytes per second
        kVTCompressionPropertyKey_AverageBitRate        // 码率平均值，单位: bits per second
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

void
APEncoder::beginEncode() {
    videoPts = 0;
    audioPts = 0;

    if (videoEncoderSession) {
        VTCompressionSessionPrepareToEncodeFrames(videoEncoderSession);
    }
    
    _isEncoding = true;
}

void
APEncoder::encodeVideo(const APEncoderInputMedia &pixelData) {
    assert(_isEncoding);
    
    if (videoEncoderSession) {
        
        CMTime presentationTimeStamp {
            .value = (int)videoPts,
            .timescale = pixelData.frame->videoParameters.frameRate,
            .flags = kCMTimeFlags_Valid,
            .epoch = 0
        };

        OSStatus status = VTCompressionSessionEncodeFrame(videoEncoderSession,
                                                          CVPixelBufferRetain(pixelData.frame->video),
                                                          presentationTimeStamp,
                                                          kCMTimeInvalid,
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
    
    if (audioParameters) {
        audioEncoderConvert = configureAudioEncoderConvert(*audioParameters);
        if (!audioParameters) {
            releaseEncoderConfiguration();
            return false;
        }
    }
    
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
        using namespace FFmpeg;

        FFCodecContext *videoEncoderContext = new FFCodecContext(FFCodecEncoder,videoCodecID);
        AVCodecContext &encoderContext = *videoEncoderContext->codec_ctx;

        // 编码器参数配置
        AVDictionary *dict = nullptr;
        if (outputFormatContext->oformat->video_codec == AV_CODEC_ID_H264) {
            av_dict_set(&dict, "preset", "medium", 0);
            av_dict_set(&dict, "tune", "zerolatency", 0);

            encoderContext.profile  = FF_PROFILE_H264_HIGH;
        }

        encoderContext.pix_fmt      = AV_PIX_FMT_YUVJ420P;
        encoderContext.width        = videoParameters.width;
        encoderContext.height       = videoParameters.height;
        encoderContext.bit_rate     = videoParameters.width * videoParameters.height * 3 * 2;
        encoderContext.gop_size     = 100;
        encoderContext.max_b_frames = 0;
        encoderContext.time_base    = { 1, 20};
        encoderContext.sample_aspect_ratio = { 16, 9};
        if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
            encoderContext.flags   |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        int ret = avcodec_open2(&encoderContext, videoEncoderContext->codec, &dict);
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
            delete videoEncoderContext;
            return nullptr;
        }

        av_dict_free(&dict);

        AVStream &outStream = *avformat_new_stream(outputFormatContext, videoEncoderContext->codec);

        ret = avcodec_parameters_from_context(outStream.codecpar, &encoderContext);
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
            delete videoEncoderContext;
            return nullptr;
        }
        outStream.time_base = encoderContext.time_base;

        videoStream = &outStream;
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
    CVPixelBufferRelease((CVPixelBufferRef)sourceFrameRefCon);

    if (status) {
        OSStatusErrorLocationLog("APEncode frame fail",status);
        return;
    }
    if (CMSampleBufferDataIsReady(sampleBuffer)) {
        
        APEncoder &encoder = *(APEncoder *)outputCallbackRefCon;
        
        bool isKeyFrame = CMSampleBufferIsKeyFrame(sampleBuffer);
        if (isKeyFrame && !encoder.audioStream) {
            encoder.audioStream = (AVStream *)0x111;
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
//                AVStream &outStream = *encoder.videoStream;//*avformat_new_stream(encoder.outputFormatContext, nullptr);
        
//                outStream.codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
//                outStream.codecpar->codec_id = FFmpeg::FFCodecContext::getAVCodecId(encoder.videoCodecID);
//                outStream.codecpar->codec_tag = 0;
//                outStream.codecpar->format = AV_PIX_FMT_YUVJ420P;
//                outStream.codecpar->bit_rate = 1920 * 1080 * 3 * 2 * 8;
//                outStream.codecpar->width = 1920;
//                outStream.codecpar->height = 1080;
//                outStream.codecpar->sample_aspect_ratio = {16, 9};
//                size_t extradataLen = 4 + 4 + spsLen + ppsLen;
//                outStream.codecpar->extradata = (uint8_t *)av_malloc(extradataLen);
//                outStream.codecpar->extradata_size = (int)extradataLen;
//
//                uint8_t separator[] = {0x00,0x00,0x00,0x01};
//                int offset = 0;
//                memcpy(outStream.codecpar->extradata+offset, separator, 4);
//                offset += 4;
//                memcpy(outStream.codecpar->extradata+offset, spsData, spsLen);
//                offset += spsLen;
//                memcpy(outStream.codecpar->extradata+offset, separator, 4);
//                offset += 4;
//                memcpy(outStream.codecpar->extradata+offset, separator, ppsLen);
                
//                CMTime presentTime = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
                
//                outStream.time_base = { 1, presentTime.timescale };
//                outStream.r_frame_rate = { presentTime.timescale, 1 };
                
//                encoder.videoStream = &outStream;
                
                int ret = avio_open(&encoder.outputFormatContext->pb, encoder.filePath.c_str(), AVIO_FLAG_READ_WRITE);
                if (ret < 0) {
                    ErrorLocationLog(av_err2str(ret));
                    encoder.releaseEncoderConfiguration();
                    return;
                }
                av_dump_format(encoder.outputFormatContext, 0, encoder.filePath.c_str(), 1);
                ret = avformat_write_header(encoder.outputFormatContext, nullptr);
                if (ret < 0) {
                    ErrorLocationLog(av_err2str(ret));
                    encoder.releaseEncoderConfiguration();
                    return;
                }
            }
        }

        CMBlockBufferRef blockBuffer = CMSampleBufferGetDataBuffer(sampleBuffer);
        size_t dataLen;
        char * dataPtr;
        CMBlockBufferGetDataPointer(blockBuffer, 0, nullptr, &dataLen, &dataPtr);
//        printf("_______________ success encode: %zu\n", dataLen);
        AVPacket packet;
        av_init_packet(&packet);
        
        uint8_t * const newData = new uint8_t[dataLen];
        memcpy(newData, dataPtr, dataLen);
        
        uint32_t len = *(uint32_t *)newData;
        len = CFSwapInt32BigToHost(len);
        *(uint32_t *)newData = 0x01000000;
        if (len < dataLen - encoder.nalUnitHeaderLen) {
            len += encoder.nalUnitHeaderLen;
            *(uint32_t *)(newData + len) = 0x01000000;
        }
        
        packet.data = newData;
        packet.size = (int)dataLen;
        packet.pts  = encoder.videoPts++;
        packet.stream_index = encoder.videoStream->index;
        
        int ret = av_interleaved_write_frame(encoder.outputFormatContext, &packet);
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
        }
        
        delete [] newData;
        
        if (encoder.videoPts == 200) {
            avio_flush(encoder.outputFormatContext->pb);
            
            int ret = av_write_trailer(encoder.outputFormatContext);
            if (ret < 0) {
                ErrorLocationLog(av_err2str(ret));
            }
            exit(0);
        }
         
    }
}
