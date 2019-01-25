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
        OSStatus status;
        
//        assert(*(uint32_t *)data.naluData == 0x01000000);
        uint32_t *tempRef = (uint32_t *)data.naluParts().idrRef() - 1;
        size_t tempSize = data.naluParts().idrSize()+4;
        // 替换开始码为数据长度(小端存储)
        *tempRef &= 0;
        *tempRef |= (tempSize << 24);
        *tempRef |= (tempSize >> 24);
        *tempRef |= (tempSize & 0x0000FF00) << 8;
        *tempRef |= (tempSize & 0x00FF0000) >> 8;
        
        CMBlockBufferRef blockBuffer = nullptr;
        status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                    tempRef,
                                                    tempSize,
                                                    blockAllocator, // 传入 nullptr, 将使用默认分配器 kCFAllocatorDefault.
                                                    nullptr, // 该结构参数将自定义内存分配和释放, 如果不为 nullptr, blockAllocator 参数将被忽略
                                                    0,
                                                    tempSize,
                                                    bufferFlags, // 传入 NULL 则该函数不会对传入数据重新分配空间.
                                                    &blockBuffer);
        if (status != kCMBlockBufferNoErr) {
            ErrorLocationLog("call CMBlockBufferCreateWithMemoryBlock fail");
            delete videoData;
            return;
        }
        
        CMSampleBufferRef sampleBuffer = nullptr;
        size_t sampleSizeArray[] = {tempSize};
        status = CMSampleBufferCreateReady(kCFAllocatorDefault,
                                           blockBuffer,
                                           decoderContext->videoFmtDescription,
                                           1, 0, nullptr,
                                           sizeof(sampleSizeArray) / sizeof(size_t),
                                           sampleSizeArray,
                                           &sampleBuffer);
        if (status != noErr) {
            ErrorLocationLog("call CMSampleBufferCreateReady fail");
            CFRelease(blockBuffer);
            delete videoData;
            return;
        }
        
        // VTDecodeInfoFlags infoFlagsOut = NULL;
        status = VTDecompressionSessionDecodeFrame(decoderContext->videoDecodeSession,
                                                   sampleBuffer,
                                                   decodeFlags, // 传入 NULL 则该函数会阻塞到回调函数返回后才返回.
                                                   (void *)videoData, // 附带参数, 会透传到回调函数
                                                   nullptr);
        if (status != noErr) {
            ErrorLocationLog("call VTDecompressionSessionDecodeFrame fail");
            delete videoData;
        }
        CFRelease(sampleBuffer);
        CFRelease(blockBuffer);
    } else {
        delete videoData;
    }
}

void
APDecoder::decodeAudio(const MSMedia<isEncode> * const audioData) {
    
}

APDecoder::APDecoder(const VTDecodeFrameFlags decodeFlags)
:APDecoderProtocol((void *)decompressionOutputCallback),
decodeFlags(decodeFlags),
bufferFlags(initBufferFlags()),
blockAllocator(initBlockAllocator()) {

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
                                            sourceData.naluParts(), *this);
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
        
        const APAsynDataProvider &dataProvider = *(APAsynDataProvider *)decompressionOutputRefCon;
        
        microseconds timeInterval = microseconds(presentationDuration.value * 1000000L / presentationDuration.timescale);
        
        dataProvider.launchVideoFrameData(new APDecoderOutputMeida(retainBuffer,
                                                                   timeInterval,
                                                                   (MSMedia<isEncode> *)sourceFrameRefCon,
                                                                   CVBufferRelease,
                                                                   CVBufferRetain));
        printf("=================解码成功=================\n");
    } else {
        delete (MSMedia<isEncode> *)sourceFrameRefCon;
        printf("=================解码失败=================\n");
    }
};

