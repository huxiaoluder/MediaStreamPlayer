//
//  APCodecContext.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "APCodecContext.hpp"

using namespace MS;
using namespace MS::APhard;

APFrame::APFrame(__CVBuffer  * MSNonnull const video, const MSVideoParameters &videoParameters)
:video(video), videoParameters(videoParameters) {
    
}

APFrame::APFrame(AudioBuffer * MSNonnull const audio, const MSAudioParameters &audioParameters)
:audio(audio), audioParameters(audioParameters) {
    
}

void
APFrame::freeVideoFrame(const APFrame * MSNonnull const frame) {
    CVPixelBufferRelease(frame->video);
    delete frame;
}

void
APFrame::freeAudioFrame(const APFrame * MSNonnull const frame) {
    free(frame->audio->mData);
    delete frame->audio;
    delete frame;
}

APFrame * MSNonnull
APFrame::copyVideoFrame(const APFrame * MSNonnull const frame) {
    return new APFrame(CVPixelBufferRetain(frame->video), frame->videoParameters);
}

APFrame * MSNonnull
APFrame::copyAudioFrame(const APFrame * MSNonnull const frame) {
    AudioBuffer *audio = new AudioBuffer();
    audio->mData = malloc(frame->audio->mDataByteSize);
    audio->mDataByteSize = frame->audio->mDataByteSize;
    audio->mNumberChannels = frame->audio->mNumberChannels;
    memcpy(audio->mData, frame->audio->mData, audio->mDataByteSize);
    return new APFrame(audio, frame->audioParameters);
}


APCodecContext::APCodecContext(const MSCodecID codecID,
                               const MSAudioParameters &audioParameters,
                               const APAsynDataProvider &asynDataProvider)
:codecID(codecID),
asynDataProvider(asynDataProvider),
audioDecoderConvert(initAudioConvert(audioParameters)) {
    
}

APCodecContext::APCodecContext(const MSCodecID codecID,
                               const bool isColorFullRange,
                               const MSNaluParts &naluParts,
                               const APAsynDataProvider &asynDataProvider)
:codecID(codecID),
asynDataProvider(asynDataProvider),
videoFmtDescription(initVideoFmtDescription(naluParts)),
videoDecoderSession(initVideoDecoderSession(isColorFullRange)) {
    
}

APCodecContext::~APCodecContext() {
    if (videoDecoderSession) {
        VTDecompressionSessionInvalidate(videoDecoderSession);
    }
    if (audioDecoderConvert) {
        AudioConverterDispose(audioDecoderConvert);
    }
    if (videoFmtDescription) {
        CFRelease(videoFmtDescription);
    }
}

CMVideoFormatDescriptionRef
APCodecContext::initVideoFmtDescription(const MSNaluParts &naluParts) {
    
    CMVideoFormatDescriptionRef videoFmtDescription = nullptr;

    OSStatus status = 0;
    
    if (codecID == MSCodecID_H264) {
        // MSNaluParts, 此处直接使用 sps, pps 内存引用, 不需要使用 NALUnitHeaderLength 替换开始码.
        const uint8_t *datas[] = {naluParts.spsRef(),  naluParts.ppsRef()};
        const size_t lengths[] = {naluParts.spsSize(), naluParts.ppsSize()};
        
        status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                     sizeof(datas)/sizeof(uint8_t *),
                                                                     datas, lengths, 4,
                                                                     &videoFmtDescription);
    } else if (codecID == MSCodecID_HEVC) {
        if (__builtin_available(iOS 11.0, *)) {
            
            const uint8_t *datas[] = {naluParts.vpsRef(),  naluParts.spsRef(),  naluParts.ppsRef()};
            const size_t lengths[] = {naluParts.vpsSize(), naluParts.spsSize(), naluParts.ppsSize()};
            
            status = CMVideoFormatDescriptionCreateFromHEVCParameterSets(kCFAllocatorDefault,
                                                                         sizeof(datas)/sizeof(uint8_t *),
                                                                         datas, lengths, 4, nullptr,
                                                                         &videoFmtDescription);
        } else {
            ErrorLocationLog("current iOS version is not 11.0+, not support the hevc");
        }
    } else {
        ErrorLocationLog("not support the codecID");
    }
    
    if (status) {
        OSStatusErrorLocationLog("fail to instance CMFormatDescriptionRef",status);
    }
    
    return videoFmtDescription;
}

AudioConverterRef
APCodecContext::initAudioConvert(const MSAudioParameters &audioParameters) {

    AudioFormatID audioFormatID = getAPCodecId(codecID);
    
    AudioConverterRef audioConverter = nullptr;
    
    OSStatus status = 0;
    
    AudioStreamBasicDescription sourceFormat = {
        .mSampleRate        = (Float64)audioParameters.frequency.value,
        .mFormatID          = audioFormatID,
        .mFormatFlags       = (UInt32)audioParameters.profile + 1,//kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger,
        .mBytesPerPacket    = 0,
        .mFramesPerPacket   = 1024, // AAC: 1 packet 1024 frame
        .mBytesPerFrame     = 0,
        .mChannelsPerFrame  = (UInt32)audioParameters.channels,
        .mBitsPerChannel    = 0,
        .mReserved          = 0
    };
    
    AudioStreamBasicDescription destinationFormat = {
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
        
    /**
     // 获取对应类型的解码器描述, 经测试 iphone 中 AAC 编解码只有一种类型 'appl'
     // kAppleSoftwareAudioCodecManufacturer = 'appl' => 软编解码
     // kAppleHardwareAudioCodecManufacturer = 'aphw' => 硬编解码
     UInt32 decodersSize;
     status = AudioFormatGetPropertyInfo(kAudioFormatProperty_Decoders,
                                         sizeof(audioFormatID),
                                         &audioFormatID,
                                         &decodersSize);
     if (status != noErr) {
         OSStatusErrorLocationLog("call AudioFormatGetPropertyInfo fail!",status);
     }
    
     int decodersCount = decodersSize / sizeof(AudioClassDescription);
     AudioClassDescription decoders[decodersCount];
     status = AudioFormatGetProperty(kAudioFormatProperty_Decoders,
                                     sizeof(audioFormatID),
                                     &audioFormatID,
                                     &decodersSize,
                                     decoders);
     if (status != noErr) {
         OSStatusErrorLocationLog("call AudioFormatGetProperty fail!",status);
     }
     
     for (int i = 0; i < decodersCount; i++) {
         OSType mType = decoders[i].mType;
         OSType mSubType = decoders[i].mSubType;
         OSType mManufacturer = decoders[i].mManufacturer;
     }
     */
    
    status = AudioConverterNew(&sourceFormat, &destinationFormat, &audioConverter);
    if (status) {
        OSStatusErrorLocationLog("instance audio converter fail!",status);
    }

    return audioConverter;
}

VTDecompressionSessionRef
APCodecContext::initVideoDecoderSession(const bool isColorFullRange) {
    
    VTDecompressionSessionRef videoDecoderSession = nullptr;
    
    assert(videoFmtDescription);
    
    OSStatus status = noErr;
    
    VTDecompressionOutputCallbackRecord outputCallback;
    outputCallback.decompressionOutputCallback = (VTDecompressionOutputCallback)asynDataProvider.asynCallBack();
    outputCallback.decompressionOutputRefCon = (void *)&asynDataProvider;
    
    /* 指定输出数据格式为 yuv420p '420f', 方便 yuv 数据分离, 提供给着色器 */
    SInt32 pixFmtNum = (isColorFullRange ?
                        kCVPixelFormatType_420YpCbCr8PlanarFullRange :
                        kCVPixelFormatType_420YpCbCr8Planar);
    CFNumberRef pixFmtType = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixFmtNum);
    
    const void * keys[] = { (void *)kCVPixelBufferPixelFormatTypeKey };
    const void * values[] = { (void *)pixFmtType };
    
    CFDictionaryRef dstBufferAttr = CFDictionaryCreate(kCFAllocatorDefault,
                                                       keys,
                                                       values,
                                                       sizeof(keys)/sizeof(void *),
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);
    
    status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                          videoFmtDescription,
                                          nullptr,
                                          dstBufferAttr,
                                          &outputCallback,
                                          &videoDecoderSession);
    CFRelease(dstBufferAttr);
    CFRelease(pixFmtType);
    
    if (status) {
        OSStatusErrorLocationLog("fail to instance VTDecompressionSessionRef",status);
    }
    
    return videoDecoderSession;
}

void
APCodecContext::setVideoFmtDescription(const MSNaluParts &naluParts) {
    if (videoFmtDescription) {
        CFRelease(videoFmtDescription);
    }
    videoFmtDescription = initVideoFmtDescription(naluParts);
    
    bool ret = VTDecompressionSessionCanAcceptFormatDescription(videoDecoderSession, videoFmtDescription);
    if (!ret) {
        ErrorLocationLog("call VTDecompressionSessionCanAcceptFormatDescription fail");
    }
}

CMVideoFormatDescriptionRef
APCodecContext::getVideoFmtDescription() const {
    return videoFmtDescription;
}

APCodecID
APCodecContext::getAPCodecId(const MSCodecID codecID) {
    APCodecID codec_id;
    switch (codecID) {
        case MSCodecID_H264:    codec_id = kCMVideoCodecType_H264;  break;
        case MSCodecID_H265:    codec_id = kCMVideoCodecType_HEVC;  break;
        case MSCodecID_AAC:     codec_id = kAudioFormatMPEG4AAC;    break;
        case MSCodecID_OPUS:    codec_id = kAudioFormatOpus;        break;
        case MSCodecID_ALAW:    codec_id = kAudioFormatALaw;        break;
    }
    return codec_id;
}
