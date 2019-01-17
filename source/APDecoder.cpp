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
APDecoder::decodeVideo(const MSMedia<isEncode> * const videoData) {
    const MSMedia<isEncode> &data = *videoData;
    APCodecContext * const decoderContext = getDecoderContext(data.codecID, data);
    
    if (decoderContext) {
        CMBlockBufferRef blockBuffer = nullptr;
        CMSampleBufferRef sampleBuffer = nullptr;
        
        size_t sampleSizeArray[] = {data.naluSize};
        
        CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                           data.nalUnit,
                                           data.naluSize,
                                           blockAllocator, // 传入 nullptr, 将使用默认分配器 kCFAllocatorDefault.
                                           nullptr, // 该结构参数将自定义内存分配和释放, 如果不为 nullptr, blockAllocator 参数将被忽略
                                           0,
                                           data.naluSize,
                                           bufferFlags, // 传入 NULL 则该函数不会对传入数据重新分配空间.
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
                                          decodeFlags, // 传入 NULL 则该函数会阻塞到回调函数返回后才返回.
                                          (void *)videoData, // 附带参数, 会透传到回调函数
                                          &infoFlagsOut);
    } else {
        ErrorLocationLog("not surport The codec");
        delete videoData;
    }
}

void
APDecoder::decodeAudio(const MSMedia<isEncode> * const audioData) {
    
}

APDecoder::APDecoder(const MSAsynDataReceiver<__CVBuffer> &asynDataHandle,
                     const VTDecodeFrameFlags decodeFlags)
:APDecoderProtocol(asynDataHandle, (void *)decompressionOutputCallback),
decodeFlags(decodeFlags), bufferFlags(initBufferFlags()), blockAllocator(initBlockAllocator()) {

}

APDecoder::~APDecoder() {
    
}

CMBlockBufferFlags
APDecoder::initBufferFlags() {
    if (decodeFlags) {
        return kCMBlockBufferAssureMemoryNowFlag;
    }
    return NULL;
}

CFAllocatorRef
APDecoder::initBlockAllocator() {
    if (bufferFlags) {
        return kCFAllocatorDefault;
    }
    return kCFAllocatorNull;
}

APCodecContext *
APDecoder::getDecoderContext(const MSCodecID codecID,
                             const MSMedia<isEncode> &sourceData) {
    APCodecContext *decoderContext = decoderContexts[codecID];
    if (!decoderContext && sourceData.isKeyFrame) {
        
        decoderContext = new APCodecContext(APCodecDecoder, codecID,
                                            *sourceData.naluParts(), *this);
        decoderContexts[codecID] = decoderContext;
    }
    return decoderContext;
}

void
APDecoder::decompressionOutputCallback(void * MSNullable decompressionOutputRefCon,
                                       void * MSNullable sourceFrameRefCon,
                                       OSStatus status,
                                       VTDecodeInfoFlags infoFlags,
                                       CVImageBufferRef MSNullable imageBuffer,
                                       CMTime presentationTimeStamp,
                                       CMTime presentationDuration) {
    if (status == noErr && imageBuffer) {
        CVBufferRef retainBuffer = CVBufferRetain(imageBuffer);
        
        APAsynDataReceiver &receiver = *(APAsynDataReceiver *)decompressionOutputRefCon;
        
        microseconds timeInterval = microseconds(presentationDuration.value * 1000000L / presentationDuration.timescale);
        
        receiver.asynPushVideoFrameData(new APDecoderOutputMeida(retainBuffer,
                                                                 timeInterval,
                                                                 (MSMedia<isEncode> *)sourceFrameRefCon,
                                                                 CVBufferRelease,
                                                                 CVBufferRetain));
    } else {
        delete (MSMedia<isEncode> *)sourceFrameRefCon;
    }
};

