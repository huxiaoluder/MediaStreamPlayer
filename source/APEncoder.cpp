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
    } else {
        int ret = avformat_write_header(outputFormatContext, nullptr);
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
            return;
        }
        isWriteHeader = true;
    }
    
    _isEncoding = true;
}

void
APEncoder::encodeVideo(const APEncoderInputMedia &pixelData) {
    assert(_isEncoding);
    
    if (videoEncoderSession) {
        /**
         @Note: 编码初始的非 I 帧, 再直接写入源编码数据, MP4 无法识别后续源编码帧(可能是参数有不同, 待解决)
         */
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
    assert(_isEncoding);
    
    if (audioEncoderConvert && isWriteHeader) {
        
        OSStatus status = noErr;
        
        APFrame &audioFrame = *sampleData.frame;
        
        AVPacket packet;
        av_init_packet(&packet);
        
        // AAC 不要参考帧, 可以不走重编吗, 这里也可以支持 AAC 重编
        if (sampleData.packt->codecID != MSCodecID_AAC) {
            // 构造输入数据
            AudioBuffer *inInputData = audioFrame.audio;
            UInt32 remainSize = 0;
            UInt32 usedSize = 0;
            if (inInputData->mDataByteSize < AacPacketFrameNum * 2) {
                usedSize = std::min(AacPacketFrameNum * 2 - pcmInBuffer->mDataByteSize, inInputData->mDataByteSize);
                remainSize = inInputData->mDataByteSize - usedSize;
                
                pcmInBuffer->mDataByteSize += usedSize;
                memcpy((uint8_t *)pcmInBuffer->mData + pcmInBuffer->mDataByteSize,
                       inInputData->mData,
                       usedSize);
                
                if (pcmInBuffer->mDataByteSize < AacPacketFrameNum * 2) {
                    return;
                }
                pcmInBuffer->mNumberChannels = inInputData->mNumberChannels;
                inInputData = pcmInBuffer;
            }
            
            UInt32 outPacktNumber = 1;
            aacOutBuffer->mBuffers[0].mDataByteSize = AacPacketFrameNum * 2;
            
            //        static AudioStreamPacketDescription outAspDesc[1];
            status = AudioConverterFillComplexBuffer(audioEncoderConvert,
                                                     compressionConverterInputProc,
                                                     inInputData,
                                                     &outPacktNumber,
                                                     aacOutBuffer,
                                                     nullptr); //outAspDesc
            if (status != noErr) {
                OSStatusErrorLocationLog("call AudioConverterFillComplexBuffer fail",status);
                return;
            }
            
            // G771 转 AAC 处理剩余数据
            if (inInputData == pcmInBuffer) {
                inInputData->mDataByteSize = remainSize;
                if (remainSize) {
                    memcpy(inInputData->mData,
                           (uint8_t *)audioFrame.audio->mData + usedSize,
                           remainSize);
                }
            }
            
            const AudioBuffer &outAudioBuffer = aacOutBuffer->mBuffers[0];
            packet.data = (uint8_t *)outAudioBuffer.mData;
            packet.size = outAudioBuffer.mDataByteSize;
        } else {
            packet.data = sampleData.packt->naluData + 7;
            packet.size = (int)sampleData.packt->naluSize - 7;
        }
        
        audioPts += AacPacketFrameNum;
        packet.stream_index = audioStream->index;
        packet.pts = audioPts;
        packet.dts = audioPts;
        
        AVRational time_base{1, audioFrame.audioParameters.frequency.value};
        av_packet_rescale_ts(&packet, time_base, audioStream->time_base);
        
        while (!fileWriteMutex.try_lock());
        if (_isEncoding) {
            int ret = av_interleaved_write_frame(outputFormatContext, &packet);
            fileWriteMutex.unlock();
            if (ret < 0) {
                ErrorLocationLog(av_err2str(ret));
            }
        }
    }
}

void
APEncoder::endEncode() {
    while (!fileWriteMutex.try_lock());
    
    _isEncoding = false;
    
    avio_flush(outputFormatContext->pb);
    
    int ret = av_write_trailer(outputFormatContext);
    
    fileWriteMutex.unlock();
    
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
    }
    releaseEncoderConfiguration();
}

bool
APEncoder::isEncoding() {
    return _isEncoding;
}

//APEncoder::APEncoder(const MSCodecID videoCodecID,
//                     const MSCodecID audioCodecID)
//:videoCodecID(videoCodecID),
//audioCodecID(audioCodecID),
//aacOutBuffer(new AudioBufferList),
//pcmInBuffer(new AudioBuffer){
//    memset(aacOutBuffer, 0, sizeof(AudioBufferList));
//    aacOutBuffer->mNumberBuffers = 1;
//    aacOutBuffer->mBuffers[0].mData = malloc(AacPacketFrameNum * 2);
//
//    memset(pcmInBuffer, 0, sizeof(AudioBuffer));
//    pcmInBuffer->mData = malloc(AacPacketFrameNum * 2);
//}

APEncoder::APEncoder()
:videoCodecID(MSCodecID_H264),
audioCodecID(MSCodecID_AAC),
aacOutBuffer(new AudioBufferList),
pcmInBuffer(new AudioBuffer){
    memset(aacOutBuffer, 0, sizeof(AudioBufferList));
    aacOutBuffer->mNumberBuffers = 1;
    aacOutBuffer->mBuffers[0].mData = malloc(AacPacketFrameNum * 2);
    
    memset(pcmInBuffer, 0, sizeof(AudioBuffer));
    pcmInBuffer->mData = malloc(AacPacketFrameNum * 2);
}

APEncoder::~APEncoder() {
    if (_isEncoding) {
        endEncode();
    } else {
        releaseEncoderConfiguration();
    }

    free(aacOutBuffer->mBuffers[0].mData);
    delete aacOutBuffer;
    
    free(pcmInBuffer->mData);
    delete pcmInBuffer;
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
    
    return videoEncoderSession;
}

AudioConverterRef
APEncoder::configureAudioEncoderConvert(const MSAudioParameters &audioParameters) {
    
    OSStatus status = noErr;
    
    AudioConverterRef audioEncoderConvert = nullptr;
    
    AudioStreamBasicDescription sourceFormat = {
        .mSampleRate        = (Float64)audioParameters.frequency.value,
        .mFormatID          = kAudioFormatLinearPCM,
        .mFormatFlags       = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
        .mBytesPerPacket    = 1 * 2 * (UInt32)audioParameters.channels,
        .mFramesPerPacket   = 1, // PCM: 1 packet 1 frame, 否则报错(code: -50)
        .mBytesPerFrame     = 2 * (UInt32)audioParameters.channels,
        .mChannelsPerFrame  = (UInt32)audioParameters.channels,
        .mBitsPerChannel    = 16,
        .mReserved          = 0
    };
    
    AudioStreamBasicDescription destinationFormat = {
        .mSampleRate        = (Float64)audioParameters.frequency.value,
        .mFormatID          = kAudioFormatMPEG4AAC,
        .mFormatFlags       = (UInt32)audioParameters.profile + 1,
        .mBytesPerPacket    = 0,
        .mFramesPerPacket   = AacPacketFrameNum, // AAC: 1 packet 1024 frame
        .mBytesPerFrame     = 0,
        .mChannelsPerFrame  = (UInt32)audioParameters.channels,
        .mBitsPerChannel    = 0,
        .mReserved          = 0
    };
    
    status = AudioConverterNew(&sourceFormat, &destinationFormat, &audioEncoderConvert);
    if (status) {
        OSStatusErrorLocationLog("fail to instance AudioConverterRef", status);
    }
    
    audioStream = avformat_new_stream(outputFormatContext, nullptr);
    audioStream->time_base = {1, audioParameters.frequency.value};
    AVCodecParameters &codecpar = *audioStream->codecpar;
    
    MSAdtsForMp4 adts;
    adts.initialize();
    // audioParameters.profile 是 ADTS 中的 profile 需要转换到 mp4 中(真实的 MPEG-4 Audio Object Type).
    adts.profile = audioParameters.profile + 1;
    adts.frequencyIndex = audioParameters.frequency.index;
    adts.channelConfiguration = audioParameters.channels;
    
    MSBinary *adtsBinary = adts.getBigEndianBinary();
    
    codecpar.codec_type = AVMEDIA_TYPE_AUDIO;
    codecpar.codec_id = FFmpeg::FFCodecContext::getAVCodecId(audioCodecID);
    codecpar.codec_tag = 0;
    // mp4 中不用参考位域限制, profile == ObjectType(小坑!!!)
    codecpar.extradata = (uint8_t *)av_malloc(adtsBinary->size);
    codecpar.extradata_size = (int)adtsBinary->size;
    codecpar.format = AV_SAMPLE_FMT_S16;
    codecpar.profile = audioParameters.profile;
    codecpar.bit_rate = audioParameters.frequency.value * 2 * 8;
    codecpar.channel_layout = 4;
    codecpar.channels = audioParameters.channels;
    codecpar.sample_rate = audioParameters.frequency.value;
    codecpar.frame_size = AacPacketFrameNum;
    codecpar.initial_padding = AacPacketFrameNum * 2;
    
    memcpy(codecpar.extradata, adtsBinary->bytes, adtsBinary->size);
    delete adtsBinary;
    
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
        avio_closep(&outputFormatContext->pb);
        avformat_free_context(outputFormatContext);
        outputFormatContext = nullptr;
    }
    if (videoStream) {
        videoStream = nullptr;
    }
    if (audioStream) {
        audioStream = nullptr;
    }
    isWriteHeader = false;
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
        
        CMTime duration = CMSampleBufferGetDuration(sampleBuffer);
        
        APEncoder &encoder = *(APEncoder *)outputCallbackRefCon;
    
        bool isKeyFrame = CMSampleBufferIsKeyFrame(sampleBuffer);
        // 视频异步编码, 获取 sps, pps会延迟, 所以优先处理 video header 信息, 再进行 audio 编码
        if (encoder.isWriteHeader == false) {
            if (!isKeyFrame) { return; }
            AVStream &videoStream = *avformat_new_stream(encoder.outputFormatContext, nullptr);
            videoStream.time_base = {1, duration.timescale};
            AVCodecParameters &codecpar = *videoStream.codecpar;
            
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
                // MP4 封装不需要帧率, 时间信息只跟 MP4 元数据相关, 由 pack 写入时决定.
                // 如需要进行 Nalu 网络数据发送, 且需要帧率信息, 需要自己添加(大坑一个!!!), 硬编码出来的数据中, sps 里不含 framerate.
                // insertFramerateToSps(20, spsData, spsLen, &newSpsData, &newSpsLen);

                codecpar.codec_type = AVMEDIA_TYPE_VIDEO;
                codecpar.codec_id = FFmpeg::FFCodecContext::getAVCodecId(encoder.videoCodecID);
                codecpar.codec_tag = 0;
                codecpar.extradata_size = (int)spsLen + 4 * 2 + (int)ppsLen;
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
                memcpy(codecpar.extradata + offset, spsData, spsLen);
                offset += spsLen;
                memcpy(codecpar.extradata + offset, separator, 4);
                offset += 4;
                memcpy(codecpar.extradata + offset, ppsData, ppsLen);
            
                int ret = avformat_write_header(encoder.outputFormatContext, nullptr);
                if (ret < 0) {
                    ErrorLocationLog(av_err2str(ret));
                    return;
                }
                
                encoder.videoStream = &videoStream;
                encoder.isWriteHeader = true;
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
        packet.stream_index = encoder.videoStream->index;
        packet.pts = encoder.videoPts;
        packet.dts = encoder.videoPts;
        AVRational time_base{1, duration.timescale}; // @NOTE: 注意时间戳转换, 否则影响 MP4 元数据信息, 导致播放器不识别
        av_packet_rescale_ts(&packet, time_base, encoder.videoStream->time_base);
        if (isKeyFrame) {
            packet.flags = AV_PKT_FLAG_KEY;
        }
        
        while (!encoder.fileWriteMutex.try_lock());
        if (encoder._isEncoding) {
            int ret = av_interleaved_write_frame(encoder.outputFormatContext, &packet);
            encoder.fileWriteMutex.unlock();
            if (ret < 0) {
                ErrorLocationLog(av_err2str(ret));
            }
        }
    }
}

OSStatus
APEncoder::compressionConverterInputProc(AudioConverterRef MSNonnull inAudioConverter,
                                         UInt32 * MSNonnull ioNumberDataPackets,
                                         AudioBufferList * MSNonnull ioData,
                                         AudioStreamPacketDescription * MSNullable * MSNullable outDataPacketDescription,
                                         void * MSNullable inUserData) {
    *ioNumberDataPackets = 1;
    
    const AudioBuffer &inAudioBuffer = *(AudioBuffer *)inUserData;
    ioData->mNumberBuffers = 1;
    ioData->mBuffers[0] = inAudioBuffer;
    
    return noErr;
}
