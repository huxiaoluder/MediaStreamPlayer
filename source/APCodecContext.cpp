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
                               const APAsynDataSender &asynDataSender)
:codecType(codecType),
codecID(codecID),
asynDataSender(asynDataSender),
audioConvert(nullptr),
videoDecodeSession(nullptr),
videoEncodeSession(nullptr) {
    
}

APCodecContext::APCodecContext(const APCodecType codecType,
                               const MSCodecID codecID,
                               const MSNaluParts &naluParts,
                               const APAsynDataSender &asynDataSender)
:codecType(codecType),
codecID(codecID),
asynDataSender(asynDataSender),
audioConvert(initAudioConvert()),
videoDecodeSession(initVideoDecodeSession(naluParts)),
videoEncodeSession(initVideoEncodeSession()) {
    
}

APCodecContext::~APCodecContext() {
    
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
APCodecContext::initVideoDecodeSession(const MSNaluParts &naluParts) {
    APCodecInfo codecInfo = getAPCodecInfo(codecID);
    IsVideoCodec isVideoCodec   = get<1>(codecInfo);
    
    CMFormatDescriptionRef      formatDescription   = nullptr;
    VTDecompressionSessionRef   videoDecoderSession = nullptr;
    
    if (codecType == APCodecDecoder && isVideoCodec) {
        OSStatus status;
        
        // MSBinary, 此处直接使用 sps, pps 内存引用, 未拼接 NALUnitHeaderLength.(待测试)
        const uint8_t *datas[] = {naluParts.spsRef(),  naluParts.ppsRef()};
        const size_t lengths[] = {naluParts.spsSize(), naluParts.ppsSize()};
        
        if (codecID == MSCodecID_H264) {
            status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                         sizeof(datas) / sizeof(uint8_t *),
                                                                         datas, lengths, 0,
                                                                         &formatDescription);
        } else {
            if (__builtin_available(iOS 11.0, *)) {
                status = CMVideoFormatDescriptionCreateFromHEVCParameterSets(kCFAllocatorDefault,
                                                                             sizeof(datas) / sizeof(uint8_t *),
                                                                             datas, lengths, 0, nullptr,
                                                                             &formatDescription);
            } else {
                return nullptr;
            }
        }
    
        if (status != noErr) {
            ErrorLocationLog("fail to instance CMFormatDescriptionRef");
            return nullptr;
        }
        
        VTDecompressionOutputCallbackRecord outputCallback;
        outputCallback.decompressionOutputCallback = (VTDecompressionOutputCallback)asynDataSender.asynCallBack();
        outputCallback.decompressionOutputRefCon = (void *)&asynDataSender;

        status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                              formatDescription,
                                              nullptr,
                                              nullptr,
                                              &outputCallback,
                                              &videoDecoderSession);
        
        if (status != noErr) {
            ErrorLocationLog("fail to instance VTDecompressionSessionRef");
            return nullptr;
        }
    }
    return videoDecoderSession;
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
