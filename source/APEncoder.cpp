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
    if (videoEncoderSession) {
        VTCompressionSessionPrepareToEncodeFrames(videoEncoderSession);
    }
    _isEncoding = true;
}

void
APEncoder::encodeVideo(const APEncoderInputMedia &pixelData) {

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
    
    // 恒定码率编码 CBR
    // 码率最大值，单位: bytes per second
    int bitRateLimit = videoParameters.width * videoParameters.height * 3 * 2;
    CFNumberRef bitRateLimitRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bitRateLimit);
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_DataRateLimits, bitRateLimitRef);
    
    // 码率平均值，单位: bits per second
    int bitRateAvera = bitRateLimit * 8;
    CFNumberRef bitRateRef = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &bitRateAvera);
    VTSessionSetProperty(videoEncoderSession, kVTCompressionPropertyKey_AverageBitRate, bitRateRef);
    
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
    
}
