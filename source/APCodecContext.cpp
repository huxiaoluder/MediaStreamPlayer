//
//  APCodecContext.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "APCodecContext.hpp"

using namespace MS;
using namespace MS::APhard;

APCodecContext::APCodecContext(const APCodecType codecType,
                               const MSCodecID codecID,
                               const APAsynDataProvider &asynDataProvider)
:codecType(codecType),
codecID(codecID),
asynDataProvider(asynDataProvider),
audioConvert(nullptr),
videoDecodeSession(nullptr),
videoEncodeSession(nullptr) {
    
}

APCodecContext::APCodecContext(const APCodecType codecType,
                               const MSCodecID codecID,
                               const MSNaluParts &naluParts,
                               const APAsynDataProvider &asynDataProvider)
:codecType(codecType),
codecID(codecID),
asynDataProvider(asynDataProvider),
_videoFmtDescription(initVideoFmtDescription(naluParts)),
audioConvert(initAudioConvert()),
videoDecodeSession(initVideoDecodeSession()),
videoEncodeSession(initVideoEncodeSession()) {
    
}

APCodecContext::~APCodecContext() {
    if (videoDecodeSession) {
        VTDecompressionSessionInvalidate(videoDecodeSession);
    }
    if (videoEncodeSession) {
        VTCompressionSessionInvalidate(videoEncodeSession);
    }
    if (audioConvert) {
        AudioConverterDispose(audioConvert);
    }
    if (_videoFmtDescription) {
        CFRelease(_videoFmtDescription);
    }
}

CMVideoFormatDescriptionRef
APCodecContext::initVideoFmtDescription(const MSNaluParts &naluParts) {
    APCodecInfo codecInfo = getAPCodecInfo(codecID);
    IsVideoCodec isVideoCodec = get<1>(codecInfo);
    
    CMVideoFormatDescriptionRef videoFmtDescription = nullptr;
    
    if (codecType == APCodecDecoder && isVideoCodec) {
        OSStatus status = 0;
        
        // MSNaluParts, 此处直接使用 sps, pps 内存引用, 未拼接 NALUnitHeaderLength.(待测试)
        const uint8_t *datas[] = {naluParts.spsRef(),  naluParts.ppsRef()};
        const size_t lengths[] = {naluParts.spsSize(), naluParts.ppsSize()};
        
        if (codecID == MSCodecID_H264) {
            status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                         sizeof(datas)/sizeof(uint8_t *),
                                                                         datas, lengths, 4,
                                                                         &videoFmtDescription);
        } else if (codecID == MSCodecID_HEVC) {
            if (__builtin_available(iOS 11.0, *)) {
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

        if (status != noErr) {
            ErrorLocationLog("fail to instance CMFormatDescriptionRef");
        }
    }
    return videoFmtDescription;
}

AudioConverterRef
APCodecContext::initAudioConvert() {
    return nullptr;
}

VTCompressionSessionRef
APCodecContext::initVideoEncodeSession() {
    return nullptr;
}

VTDecompressionSessionRef
APCodecContext::initVideoDecodeSession() {
    APCodecInfo codecInfo = getAPCodecInfo(codecID);
    IsVideoCodec isVideoCodec = get<1>(codecInfo);
    
    VTDecompressionSessionRef videoDecoderSession = nullptr;
    
    if (codecType == APCodecDecoder && isVideoCodec) {
        assert(_videoFmtDescription);
        
        OSStatus status = 0;

        VTDecompressionOutputCallbackRecord outputCallback;
        outputCallback.decompressionOutputCallback = (VTDecompressionOutputCallback)asynDataProvider.asynCallBack();
        outputCallback.decompressionOutputRefCon = (void *)&asynDataProvider;

        SInt32 pixFmtNum = kCVPixelFormatType_420YpCbCr8BiPlanarFullRange;
        CFNumberRef pixFmtType = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixFmtNum);
        
        const void * keys[] = {(void *)kCVPixelBufferPixelFormatTypeKey};
        const void * values[] = {(void *)pixFmtType};
        
        CFDictionaryRef dstBufferAttr = CFDictionaryCreate(kCFAllocatorDefault,
                                                           keys, values, sizeof(keys)/sizeof(void *),
                                                           &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
#warning dstBufferAttr
        status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                              _videoFmtDescription,
                                              nullptr,
                                              nullptr,
                                              &outputCallback,
                                              &videoDecoderSession);
        CFRelease(dstBufferAttr);
        CFRelease(pixFmtType);
        
        if (status != noErr) {
            ErrorLocationLog("fail to instance VTDecompressionSessionRef");
        }
    }
    return videoDecoderSession;
}

void
APCodecContext::setVideoFmtDescription(const MSNaluParts &naluParts) {
    if (_videoFmtDescription) {
        CFRelease(_videoFmtDescription);
    }
    _videoFmtDescription = initVideoFmtDescription(naluParts);
    
    bool ret = VTDecompressionSessionCanAcceptFormatDescription(videoDecodeSession, _videoFmtDescription);
    if (!ret) {
        ErrorLocationLog("call VTDecompressionSessionCanAcceptFormatDescription fail");
    }
}

CMVideoFormatDescriptionRef
APCodecContext::videoFmtDescription() const {
    return _videoFmtDescription;
}

APCodecInfo
APCodecContext::getAPCodecInfo(const MSCodecID codecID) {
    APCodecID codec_id;
    switch (codecID) {
        case MSCodecID_H264:    codec_id = kCMVideoCodecType_H264;  break;
        case MSCodecID_H265:    codec_id = kCMVideoCodecType_HEVC;  break;
        case MSCodecID_AAC:     codec_id = kAudioFormatMPEG4AAC;    break;
        case MSCodecID_OPUS:    codec_id = kAudioFormatOpus;        break;
        case MSCodecID_ALAW:    codec_id = kAudioFormatALaw;        break;
    }
    return APCodecInfo(codec_id, codecID <= MSCodecID_H265);
}
