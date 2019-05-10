//
//  APDecoder.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "APDecoder.hpp"

using namespace MS;
using namespace MS::APhard;

void
APDecoder::decodeVideo(const MSMedia<MSEncodeMedia> * const videoData) {
    const MSMedia<MSEncodeMedia> &data = *videoData;
    ReGetContext: APCodecContext * const decoderContext = getVideoDecoderContext(data);
    
    if (decoderContext) {
        OSStatus status = noErr;
        
        assert(*(uint32_t *)data.naluData == 0x01000000);
        
        const MSNaluParts &naluParts = data.getNaluParts();
        
        // 帧数据引用
        uint32_t *tempRef = (uint32_t *)naluParts.dataRef();
        // 帧数据长度
        size_t tempSize = naluParts.dataSize();
        // 包含开始码
        tempRef -= 1;
        
        // atom 型: 替换开始码为数据长度(小端存储)
        *tempRef = getReverse4Bytes((uint32_t)tempSize);
        
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
            OSStatusErrorLocationLog("call CMBlockBufferCreateWithMemoryBlock fail",status);
            delete videoData;
            return;
        }
       
        CMSampleBufferRef sampleBuffer = nullptr;
        size_t sampleSizeArray[] = {tempSize};
        
        /*
         // 注: 时间信息附带参数(对解码器并没有影响), 这里因为自定义透传信息 APVideoAttachment 中附带了有效数据, 所以不用传输
         CMSampleTimingInfo sampleTimingArray[] = {{
            .presentationTimeStamp = {
                .value = 1,
                .timescale = videoParametersMap[this]->frameRate,
                .flags = kCMTimeFlags_Valid,
                .epoch = 0,
            },
            .duration = NULL,
            .decodeTimeStamp = NULL
         }};
         */
        
        // 报: kVTVideoDecoderMalfunctionErr -12911,
        // API 中(CM_NULLABLE formatDescription)  并不准确. --------------------------------¬
        status = CMSampleBufferCreateReady(kCFAllocatorDefault,                      //   ¦
                                           blockBuffer,                              //   ↓
                                           decoderContext->getVideoFmtDescription(), // 必须传(用于描述blockBuffer),否则回调报错,且会卡死解码函数
                                           sizeof(sampleSizeArray) / sizeof(size_t),
                                           0, // sizeof(sampleTimingArray) / sizeof(CMSampleTimingInfo),
                                           nullptr, // sampleTimingArray,
                                           sizeof(sampleSizeArray) / sizeof(size_t),
                                           sampleSizeArray,
                                           &sampleBuffer);
        if (status) {
            OSStatusErrorLocationLog("call CMSampleBufferCreateReady fail",status);
            CFRelease(blockBuffer);
            delete videoData;
            return;
        }

        videoAttachment.videoSource = videoData;
        videoAttachment.videoParameters = videoParametersMap[this];
        
        // VTDecodeInfoFlags infoFlagsOut = NULL;
        status = VTDecompressionSessionDecodeFrame(decoderContext->videoDecoderSession,
                                                   sampleBuffer,
                                                   decodeFlags, // 传入 NULL 则该函数会阻塞到回调函数返回后才返回.(API 注释有误 0 为同步)
                                                   &videoAttachment, // 附带参数, 会透传到回调函数
                                                   nullptr);
        CFRelease(sampleBuffer);
        CFRelease(blockBuffer);
        if (status != noErr) {
            OSStatusErrorLocationLog("call VTDecompressionSessionDecodeFrame fail",status);
            if (status == kVTInvalidSessionErr || // APP进入后台时, 系统会重置解码器
                status == kVTFormatDescriptionChangeNotSupportedErr) { // sps, pps 变化跨度太大
                delete decoderContexts[videoData->codecID];
                decoderContexts.erase(videoData->codecID);
            }
            goto ReGetContext;
        }
    } else {
        delete videoData;
    }
}

void
APDecoder::decodeAudio(const MSMedia<MSEncodeMedia> * const audioData) {
    const MSMedia<MSEncodeMedia> &data = *audioData;
    APCodecContext * const decoderContext = getAudioDecoderContext(data);
    
    if (decoderContext) {
        OSStatus status = noErr;
        
        const MSAudioParameters &audioParameters = *audioParametersMap[this];
        
        AudioConverterComplexInputDataProc decompressionInputProc = nullptr;
        UInt32 outPacktNumber = 0;
        switch (audioData->codecID) {
            case MSCodecID_AAC: {
                outPacktNumber = AacPacketFrameNum;
                decompressionInputProc = decompressionAacConverterInputProc;
            }   break;
            case MSCodecID_ALAW: {
                outPacktNumber = AlawPacketFrameNum;
                decompressionInputProc = decompressionAlawConverterInputProc;
            }   break;
            case MSCodecID_OPUS: {
                outPacktNumber = 0;
                decompressionInputProc = decompressionOpusConverterInputProc;
            }   break;
            default: {
                delete audioData;
            }   return;
        }
        
        UInt32 mDataByteSize = outPacktNumber * 2 * (UInt32)audioParameters.channels;
        AudioBufferList outBufferList {
            .mNumberBuffers = 1,
            .mBuffers[0] = {
                .mNumberChannels = (UInt32)audioParameters.channels,
                .mDataByteSize = mDataByteSize,
                .mData = malloc(mDataByteSize)
            }
        };
        
        audioAttachment.audioSource = audioData;
        audioAttachment.audioParameters = &audioParameters;
        
//        static AudioStreamPacketDescription outAspDesc[1024];
        status = AudioConverterFillComplexBuffer(decoderContext->audioDecoderConvert,
                                                 decompressionInputProc,
                                                 &audioAttachment,
                                                 &outPacktNumber,
                                                 &outBufferList,
                                                 nullptr); //outAspDesc
        if (status != noErr) {
            OSStatusErrorLocationLog("call AudioConverterFillComplexBuffer fail",status);
            delete audioData;
            return;
        }
        
        AudioBuffer *audioBuffer = new AudioBuffer();
        *audioBuffer = outBufferList.mBuffers[0];
        
        APFrame *frame = new APFrame(audioBuffer, audioParameters);
        
        MSTimeInterval timeInterval{(int)outPacktNumber, audioParametersMap[this]->frequency.value};
        
        launchAudioFrameData(new APDecoderOutputMeida(frame,
                                                      timeInterval,
                                                      audioData,
                                                      APFrame::freeAudioFrame,
                                                      APFrame::copyAudioFrame));
    } else {
        delete audioData;
    }
}

map<APDecoder *, const MSVideoParameters *>
APDecoder::videoParametersMap;

map<APDecoder *, const MSAudioParameters *>
APDecoder::audioParametersMap;

APDecoder::APDecoder(const VTDecodeFrameFlags decodeFlags)
:APDecoderProtocol((void *)decompressionOutputCallback),
decodeFlags(decodeFlags),
bufferFlags(initBufferFlags()),
blockAllocator(initBlockAllocator()) {

}

APDecoder::~APDecoder() {
    auto mediaParameters = videoParametersMap[this];
    if (mediaParameters) {
        videoParametersMap.erase(this);
        delete mediaParameters;
    }
    for (auto element : decoderContexts) {
        delete element.second;
    }
    decoderContexts.clear();
}

const MSVideoParameters *
APDecoder::getCurrentVideoParameters() {
    return videoParametersMap[this];
}

const MSAudioParameters *
APDecoder::getCurrentAudioParameters() {
    return audioParametersMap[this];
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
APDecoder::getVideoDecoderContext(const MSMedia<MSEncodeMedia> &sourceData) {
    MSCodecID codecID = sourceData.codecID;
    APCodecContext *decoderContext = decoderContexts[codecID];
    
    if (sourceData.isKeyFrame) {
        const MSNaluParts &naluParts = sourceData.getNaluParts();
        // 解析 sps 基本信息(实时更新宽高帧率色域)
        const MSVideoParameters *videoParameter = videoParametersMap[this];
        if (videoParameter) {
            delete videoParameter;
        }
        if (codecID == MSCodecID_H264) {
            videoParameter = naluParts.parseH264Sps();
        } else {
            videoParameter = naluParts.parseH265Sps();
        }
        videoParametersMap[this] = videoParameter;
        
        if (decoderContext) {
            // 检测当前解码器配置是否能解码该数据帧(sps, pps 被改变)
            bool ret = decoderContext->canAcceptNewFormatDescription(naluParts);
            /** 重点(大坑, 系统 Bug):
             某些机型在解码前, 检测到参数变化过大时, 不能够继续让解码器解码该数据, 若继续解码, 报错: -12916 后,
             会导致当前解码器无法释放, 因为在销毁解码器时:
             VTDecompressionSessionWaitForAsynchronousFrames() 和 VTDecompressionSessionInvalidate()
             两个函数内部都会锁死, 所以应该直接销毁当前解码器并初始化i新的解码器
             */                                                // ⏬
            if (!ret) { // sps, pps 变化跨度太大𝗅                    ⏬
                delete decoderContext;                         // ⏬
                decoderContext = new APCodecContext(codecID, videoParameter->isColorFullRange, naluParts, *this);
                decoderContexts[codecID] = decoderContext;
            }
        } else {
            decoderContext = new APCodecContext(codecID, videoParameter->isColorFullRange, naluParts, *this);
            decoderContexts[codecID] = decoderContext;
        }
    }
    return decoderContext;
}

APCodecContext *
APDecoder::getAudioDecoderContext(const MSMedia<MSEncodeMedia> &sourceData) {
    MSCodecID codecID = sourceData.codecID;
    APCodecContext *decoderContext = decoderContexts[codecID];
    
    if (!decoderContext) {
        const MSNaluParts &naluParts = sourceData.getNaluParts();
        // 音频一般不会动态更改编码配置, 暂时以首次的 adts 数据为准
        const MSAudioParameters *audioParameters = nullptr;
        switch (codecID) {
            case MSCodecID_AAC: {
                 audioParameters = naluParts.parseAacAdts();
            }   break;
            case MSCodecID_ALAW: {
                auto audioParam = new MSAudioParameters;
                audioParam->profile     = 1;
                audioParam->channels    = 1;
                audioParam->frequency   = {11, 8000};
                audioParameters = audioParam;
            }   break;
            case MSCodecID_OPUS: {
                //TODO: MSCodecID_OPUS
            }   return nullptr;
            default: return nullptr;
        }
        audioParametersMap[this] = audioParameters;
        decoderContext = new APCodecContext(codecID, *audioParameters, *this);
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
        const APAsynDataProvider &dataProvider = *(APAsynDataProvider *)decompressionOutputRefCon;
        const APVideoAttachment  &attachment = *(APVideoAttachment *)sourceFrameRefCon;

        APFrame *frame = new APFrame(CVPixelBufferRetain(imageBuffer), *attachment.videoParameters);
        
        MSTimeInterval timeInterval{1, attachment.videoParameters->frameRate};
        
        dataProvider.launchVideoFrameData(new APDecoderOutputMeida(frame,
                                                                   timeInterval,
                                                                   attachment.videoSource,
                                                                   APFrame::freeVideoFrame,
                                                                   APFrame::copyVideoFrame));
        
    } else {
        OSStatusErrorLocationLog("APDecoder decode video error!", status);
    }
};

OSStatus
APDecoder::decompressionAacConverterInputProc(AudioConverterRef MSNonnull inAudioConverter,
                                              UInt32 * MSNonnull ioNumberDataPackets,
                                              AudioBufferList * MSNonnull ioData,
                                              AudioStreamPacketDescription * MSNullable * MSNullable outDataPacketDescription,
                                              void * MSNullable inUserData) {
    const APAudioAttachment &audioAttachment = *(APAudioAttachment *)inUserData;
    const MSNaluParts &naluParts = audioAttachment.audioSource->getNaluParts();
    
    ioData->mNumberBuffers = 1;
    ioData->mBuffers->mData = (void *)naluParts.dataRef();
    ioData->mBuffers->mDataByteSize = (UInt32)naluParts.dataSize();
    ioData->mBuffers->mNumberChannels = (UInt32)audioAttachment.audioParameters->channels;
    
    static AudioStreamPacketDescription aspDesc;
    aspDesc.mStartOffset = 0;
    aspDesc.mVariableFramesInPacket = 0;
    aspDesc.mDataByteSize = (UInt32)naluParts.dataSize();
    *outDataPacketDescription = &aspDesc;
    
    return noErr;
}

OSStatus
APDecoder::decompressionAlawConverterInputProc(AudioConverterRef MSNonnull inAudioConverter,
                                               UInt32 * MSNonnull ioNumberDataPackets,
                                               AudioBufferList * MSNonnull ioData,
                                               AudioStreamPacketDescription * MSNullable * MSNullable outDataPacketDescription,
                                               void * MSNullable inUserData) {
    const APAudioAttachment &audioAttachment = *(APAudioAttachment *)inUserData;
    auto audioSource = audioAttachment.audioSource;
    
    ioData->mNumberBuffers = 1;
    ioData->mBuffers->mData = (void *)audioSource->naluData;
    ioData->mBuffers->mDataByteSize = (UInt32)audioSource->naluSize;
    ioData->mBuffers->mNumberChannels = (UInt32)audioAttachment.audioParameters->channels;
    
    return noErr;
}


OSStatus
APDecoder::decompressionOpusConverterInputProc(AudioConverterRef MSNonnull inAudioConverter,
                                               UInt32 * MSNonnull ioNumberDataPackets,
                                               AudioBufferList * MSNonnull ioData,
                                               AudioStreamPacketDescription * MSNullable * MSNullable outDataPacketDescription,
                                               void * MSNullable inUserData) {
    const APAudioAttachment &audioAttachment = *(APAudioAttachment *)inUserData;
    const MSNaluParts &naluParts = audioAttachment.audioSource->getNaluParts();
    
    ioData->mNumberBuffers = 1;
    ioData->mBuffers->mData = (void *)naluParts.dataRef();
    ioData->mBuffers->mDataByteSize = (UInt32)naluParts.dataSize();
    ioData->mBuffers->mNumberChannels = (UInt32)audioAttachment.audioParameters->channels;
    
    return noErr;
}
