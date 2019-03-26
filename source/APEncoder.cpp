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

void
APEncoder::beginEncode() {
    
}

void
APEncoder::encodeVideo(const APEncoderInputMedia &pixelData) {

}

void
APEncoder::encodeAudio(const APEncoderInputMedia &sampleData) {
    
}

void
APEncoder::endEncode() {
    
}

bool
APEncoder::isEncoding() {
    return false;
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
    
    videoEncoderSession = configureVideoEncoderSession(*videoParameters);
    
    audioEncoderConvert = configureAudioEncoderConvert(*audioParameters);
    
    return true;
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
        OSStatusErrorLocationLog("fail to instance VTCompressionSessionRef",status);
    }
    
    // 编码器实时编码输出(在线编码, 要求实时性, 离线编码不要求实时性, 设为 false, 可以保证更好的编码效果)
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_RealTime, kCFBooleanTrue);
    
    // 编码器配置级别
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_ProfileLevel, kVTProfileLevel_H264_High_AutoLevel);
    
    // 编码 GOP 大小
    int frameInterval = 100;
    CFNumberRef frameIntervalRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &frameInterval);
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_MaxKeyFrameInterval, frameIntervalRef);
    
    // 编码期望帧率
    int framerate = videoParameters.frameRate;
    CFNumberRef fpsRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &framerate);
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_ExpectedFrameRate, fpsRef);
    
    // 码率最大值，单位: bits per second
    int bitRateLimit = videoParameters.width * videoParameters.height * 3 * 4;
    CFNumberRef bitRateLimitRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bitRateLimit);
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_DataRateLimits, bitRateLimitRef);
    
    // 码率平均值，单位: bytes per second
    int bitRate = bitRateLimit * 8;
    CFNumberRef bitRateRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bitRate);
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_AverageBitRate, bitRateRef);
    
    return videoEncoderSession;
}

AudioConverterRef
APEncoder::configureAudioEncoderConvert(const MSAudioParameters &audioParameters) {
    AudioConverterRef audioEncoderConvert = nullptr;
    return audioEncoderConvert;
}

void
APEncoder::compressionOutputCallback(void * MSNullable outputCallbackRefCon,
                                     void * MSNullable sourceFrameRefCon,
                                     OSStatus status,
                                     VTEncodeInfoFlags infoFlags,
                                     CMSampleBufferRef MSNullable sampleBuffer) {
    
}
