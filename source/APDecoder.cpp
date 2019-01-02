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
//   VTDecompressionSessionDecodeFrame(<#VTDecompressionSessionRef  _Nonnull session#>, <#CMSampleBufferRef  _Nonnull sampleBuffer#>, <#VTDecodeFrameFlags decodeFlags#>, <#void * _Nullable sourceFrameRefCon#>, <#VTDecodeInfoFlags * _Nullable infoFlagsOut#>)
    
}

void
APDecoder::decodeAudio(const MSMediaData<isEncode> &audioData) {
    
}

APDecoder::APDecoder(MSAsynDataProtocol<__CVBuffer> &asynDataHandle)
:APDecoderProtocol(asynDataHandle, decompressionOutputCallback) {

}

APDecoder::~APDecoder() {
    
}

void createDecompressionSession() {
    //创建VTDecompressionSession
//    VTDecompressionSessionRef _decompressionSession = NULL;
//    VTDecompressionOutputCallbackRecord callBackRecord;
//    callBackRecord.decompressionOutputCallback = decompressionSessionDecodeFrameCallback;
//    
//    callBackRecord.decompressionOutputRefCon = (__bridge void *)self;
//    
//    NSDictionary* destinationPixelBufferAttributes = @{
//        (id)kCVPixelBufferPixelFormatTypeKey : [NSNumber numberWithInt:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange],
//        //硬解必须是 kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange
//        //或者是kCVPixelFormatType_420YpCbCr8Planar
//        //因为iOS是  nv12  其他是nv21
//        (id)kCVPixelBufferWidthKey : [NSNumber numberWithInt:h264outputHeight*2],
//        (id)kCVPixelBufferHeightKey : [NSNumber numberWithInt:h264outputWidth*2],
//        //这里宽高和编码反的
//        (id)kCVPixelBufferOpenGLCompatibilityKey : [NSNumber numberWithBool:YES]
//    };
//    OSStatus status = VTDecompressionSessionCreate(kCFAllocatorDefault,
//                                                   _formatDesc,
//                                                   NULL,
//                                                   (__bridge CFDictionaryRef)destinationPixelBufferAttributes,
//                                                   &callBackRecord,
//                                                   &_decompressionSession);
//    
//    if (status == noErr) {
////        NSLog(@"Video Decompression Session 创建成功!");
//    }else{
////        NSLog(@"Video Decompression Session 创建失败，错误码： %d",(int)status);
//    }
}

void
APDecoder::decompressionOutputCallback(void * _Nullable decompressionOutputRefCon,
                                       void * _Nullable sourceFrameRefCon,
                                       OSStatus status,
                                       VTDecodeInfoFlags infoFlags,
                                       CVImageBufferRef _Nullable imageBuffer,
                                       CMTime presentationTimeStamp,
                                       CMTime presentationDuration) {
    APOutputDataSender &sender = *(APDecoderProtocol *)decompressionOutputRefCon;
    CVBufferRetain(imageBuffer);
    microseconds timeInterval = microseconds(presentationDuration.value * 1000000L / presentationDuration.timescale);
    
    APDecoderOutputContent *content = new APDecoderOutputContent(imageBuffer,
                                                                 timeInterval,
                                                                 CVBufferRelease,
                                                                 CVBufferRetain);
    
    sender.asynDataReceiver().MSAsynDataProtocol::asynPushAudioFrameData(new APDecoderOutputData(content));
};

