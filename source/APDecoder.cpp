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

const APDecoderProtocol::MSDecoderOutputData *
APDecoder::decodeVideo(const MSMediaData<isEncode> &videoData) {
   
    return nullptr;
}

const APDecoderProtocol::MSDecoderOutputData *
APDecoder::decodeAudio(const MSMediaData<isEncode> &audioData) {
    return nullptr;
}

APDecoder::APDecoder() {

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

