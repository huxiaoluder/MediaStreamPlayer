//
//  APDecoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "APDecoder.hpp"

using namespace MS;
using namespace MS::APhard;

void
APDecoder::decodeVideo(const MSMediaData<isEncode> &videoData) {
    APCodecContext * const decoderContext = getDecoderContext(videoData.content->codecID,videoData);
    
    if (decoderContext) {
        CMBlockBufferRef blockBuffer = nullptr;
        CMSampleBufferRef sampleBuffer = nullptr;
        
        size_t sampleSizeArray[] = {videoData.content->size};
        
#warning 每次 CMBlockBuffer 自动生成了流数据副本,不影响外界释放(反复拷贝、释放, 会损耗性能, 待优化策略: 创建内存缓冲池)
        CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                           videoData.content->packt,
                                           videoData.content->size,
                                           kCFAllocatorDefault,
                                           nullptr, 0,
                                           videoData.content->size,
                                           kCMBlockBufferAssureMemoryNowFlag,
                                           &blockBuffer);
        
        CMSampleBufferCreateReady(kCFAllocatorDefault,
                                  blockBuffer,
                                  nullptr, 1, 0, nullptr,
                                  sizeof(sampleSizeArray) / sizeof(size_t),
                                  sampleSizeArray,
                                  &sampleBuffer);
        
        VTDecodeInfoFlags infoFlagsOut = NULL;
        
        VTDecompressionSessionDecodeFrame(decoderContext->videoDecodeSession,
                                          sampleBuffer,
                                          kVTDecodeFrame_EnableAsynchronousDecompression,
                                          nullptr,
                                          &infoFlagsOut);
    }
}

void
APDecoder::decodeAudio(const MSMediaData<isEncode> &audioData) {
    
}

APDecoder::APDecoder(MSAsynDataProtocol<__CVBuffer> &asynDataHandle)
:APDecoderProtocol(asynDataHandle, (void *)decompressionOutputCallback) {

}

APDecoder::~APDecoder() {
    
}

APCodecContext *
APDecoder::getDecoderContext(const MSCodecID codecID,
                             const MSMediaData<isEncode> &sourceData) {
    APCodecContext *decoderContext = decoderContexts[codecID];
    if (!decoderContext && sourceData.content->isKeyFrame) {
        
        
        
        MSBinaryData spsData(nullptr,0);
        MSBinaryData ppsData(nullptr,0);
        decoderContext = new APCodecContext(APCodecDecoder, codecID,
                                            spsData, ppsData, *this);
        decoderContexts[codecID] = decoderContext;
    }
    return decoderContext;
}

void
APDecoder::decompressionOutputCallback(void * _Nullable decompressionOutputRefCon,
                                       void * _Nullable sourceFrameRefCon,
                                       OSStatus status,
                                       VTDecodeInfoFlags infoFlags,
                                       CVImageBufferRef _Nullable imageBuffer,
                                       CMTime presentationTimeStamp,
                                       CMTime presentationDuration) {
    CVBufferRef retainBuffer = CVBufferRetain(imageBuffer);

    APAsynDataProtocol &receiver = *(APAsynDataProtocol *)decompressionOutputRefCon;

    microseconds timeInterval = microseconds(presentationDuration.value * 1000000L / presentationDuration.timescale);
    
    APDecoderOutputContent *content = new APDecoderOutputContent(retainBuffer,
                                                                 timeInterval,
                                                                 CVBufferRelease,
                                                                 CVBufferRetain);

    receiver.asynPushVideoFrameData(new APDecoderOutputData(content));
};

