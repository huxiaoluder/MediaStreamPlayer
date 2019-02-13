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
    APCodecContext * const decoderContext = getVideoDecoderContext(data);
    
    if (decoderContext) {
        OSStatus status;
        
        assert(*(uint32_t *)data.naluData == 0x01000000);
        
        // 帧数据引用
        uint32_t *tempRef = (uint32_t *)(data.getNaluParts().slcRef() ?
                                         data.getNaluParts().slcRef() :
                                         data.getNaluParts().idrRef());
        // 帧数据长度
        size_t tempSize = (data.getNaluParts().slcSize() ?
                           data.getNaluParts().slcSize() :
                           data.getNaluParts().idrSize());
        // 包含开始码
        tempRef -= 1;
        
        // atom 型: 替换开始码为数据长度(小端存储)
        *tempRef &= 0;
        *tempRef |= (tempSize << 24);
        *tempRef |= (tempSize >> 24);
        *tempRef |= (tempSize & 0x0000FF00) << 8;
        *tempRef |= (tempSize & 0x00FF0000) >> 8;
        
        // atom 型: nalu 数据长度
        tempSize += 4;
        
        CMBlockBufferRef blockBuffer = nullptr;
        status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                    tempRef,
                                                    tempSize,
                                                    blockAllocator, // 传入 nullptr, 将使用默认分配器 kCFAllocatorDefault.
                                                    nullptr, // 该结构参数将自定义内存的分配和释放, 如果不为 nullptr, blockAllocator 参数将被忽略
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
        
        CMSampleTimingInfo sampleTimingArray[] = {{
            .duration = {
                .value = 1000000LL / MSNaluParts::videoParameter.frameRate,
                .timescale = 1000000,
                .flags = kCMTimeFlags_Valid,
                .epoch = 0,
            },
            .presentationTimeStamp = NULL,
            .decodeTimeStamp = NULL
        }};
        
        status = CMSampleBufferCreateReady(kCFAllocatorDefault,
                                           blockBuffer,
                                           decoderContext->getVideoFmtDescription(), // 必须传,否则回调报错,且会卡死解码函数(kVTVideoDecoderMalfunctionErr -12911, 文档并不准确)
                                           sizeof(sampleSizeArray) / sizeof(size_t),
                                           sizeof(sampleTimingArray) / sizeof(CMSampleTimingInfo),
                                           sampleTimingArray,
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
    const MSMedia<isEncode> &data = *audioData;
    APCodecContext * const decoderContext = getVideoDecoderContext(data);
    
    if (decoderContext) {
        OSStatus status;
        decoderContext->audioConverter;
    } else {
        delete audioData;
    }
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
APDecoder::getVideoDecoderContext(const MSMedia<isEncode> &sourceData) {
    MSCodecID codecID = sourceData.codecID;
    APCodecContext *decoderContext = decoderContexts[codecID];
    
    if (sourceData.isKeyFrame) {
        if (decoderContext) {
            // 实时更新解码器配置(用新的 sps, pps)
            decoderContext->setVideoFmtDescription(sourceData.getNaluParts());
        } else {
            decoderContext = new APCodecContext(APCodecDecoder, codecID, sourceData.getNaluParts(), *this);
            decoderContexts[codecID] = decoderContext;
        }
        // 解析基本信息(实时更新宽高帧率)
        sourceData.getNaluParts().parseSps(codecID == MSCodecID_H264 ?
                                        SpsTypeH264 :
                                        SpsTypeH265);
    }
    return decoderContext;
}

APCodecContext *
APDecoder::getAudioDecoderContext(const MSMedia<isEncode> &sourceData) {
    MSCodecID codecID = sourceData.codecID;
    APCodecContext *decoderContext = decoderContexts[codecID];
    
    if (!decoderContext) {
        decoderContext = new APCodecContext(APCodecDecoder, codecID, *this);
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
        
        microseconds timeInterval = microseconds(presentationDuration.value);
        
        dataProvider.launchVideoFrameData(new APDecoderOutputMeida(retainBuffer,
                                                                   timeInterval,
                                                                   (MSMedia<isEncode> *)sourceFrameRefCon,
                                                                   CVBufferRelease,
                                                                   CVBufferRetain));
        
    } else {
        ErrorLocationLog("APDecoder decode video error!");
    }
};

