//
//  MSEncoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFEncoder.hpp"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;

void
FFEncoder::beginEncodeToFile(const string filePath) {
    
}

void
FFEncoder::encodeVideo(const MSEncoderInputData &pixelData) {
    
    
}

void
FFEncoder::encodeAudio(const MSEncoderInputData &sampleData) {
    
    
}

void
FFEncoder::endEncode() {
    
}

bool
FFEncoder::isEncoding() {
    return _isEncoding;
}

FFEncoder::FFEncoder(const AVCodecID vedioCodecID,
                     const AVCodecID audioCodecID)
:vedioCodecID(vedioCodecID), audioCodecID(audioCodecID) {
    
}

FFEncoder::~FFEncoder() {
    // 释放编码器
    
}
