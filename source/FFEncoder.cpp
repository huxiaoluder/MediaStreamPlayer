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

MSMediaData<isEncode> * const
FFEncoder::encodeVideo(const MSInputData &pixelData) {
    
    return nullptr;
}

MSMediaData<isEncode> * const
FFEncoder::encodeAudio(const MSInputData &sampleData) {
    
    return nullptr;
}

FFEncoder::FFEncoder(const AVCodecID vedioCodecID, const AVCodecID audioCodecID)
:vedioCodecID(vedioCodecID), audioCodecID(audioCodecID) {
    
}

FFEncoder::~FFEncoder() {
    // 释放编码器
    
}
