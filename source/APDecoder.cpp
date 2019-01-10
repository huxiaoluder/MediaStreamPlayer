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
    MSBinaryData spsData(nullptr,0);
    MSBinaryData ppsData(nullptr,0);

    const APCodecContext &decoderContext = getDecoderContext(videoData.content->codecID,spsData,ppsData);
    
    CMSampleBufferRef sampleBuffer = nullptr;
    
    size_t sampleSizeArray[] = {videoData.content->size};
    
//    CMSampleBufferCreate(nullptr,
//                         CMBlockBufferRef,
//                         true,
//                         <#CMSampleBufferMakeDataReadyCallback  _Nullable makeDataReadyCallback#>,
//                         <#void * _Nullable makeDataReadyRefcon#>,
//                         <#CMFormatDescriptionRef  _Nullable formatDescription#>,
//                         <#CMItemCount numSamples#>,
//                         <#CMItemCount numSampleTimingEntries#>,
//                         <#const CMSampleTimingInfo * _Nullable sampleTimingArray#>,
//                         CMItemCount numSampleSizeEntries,
//                         sampleSizeArray,
//                         &sampleBuffer);
    
   VTDecompressionSessionDecodeFrame(decoderContext.videoDecodeSession,
                                     sampleBuffer,
                                     kVTDecodeFrame_EnableAsynchronousDecompression,
                                     nullptr,
                                     nullptr);
    
}

void
APDecoder::decodeAudio(const MSMediaData<isEncode> &audioData) {
    
}

APDecoder::APDecoder(MSAsynDataProtocol<__CVBuffer> &asynDataHandle)
:APDecoderProtocol(asynDataHandle, (void *)decompressionOutputCallback) {

}

APDecoder::~APDecoder() {
    
}

const APCodecContext &
APDecoder::getDecoderContext(const MSCodecID codecID,
                             const MSBinaryData &spsData,
                             const MSBinaryData &ppsData) {
    APCodecContext *decoderContext = decoderContexts[codecID];
    if (!decoderContext) {
        decoderContext = new APCodecContext(APCodecDecoder, codecID,
                                            spsData, ppsData, *this);
        decoderContexts[codecID] = decoderContext;
    }
    return *decoderContext;
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

