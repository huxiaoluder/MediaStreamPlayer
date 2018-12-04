//
//  MSEncoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFEncoder.hpp"

using namespace MS;

using namespace std;

MSMediaData<isEncode> * const
FFEncoder::encodeVideo(const inputType &pixelData) {
    
    return nullptr;
}

MSMediaData<isEncode> * const
FFEncoder::encodeAudio(const inputType &sampleData) {
    
    return nullptr;
}

FFEncoder::FFEncoder() {
    
}

FFEncoder::~FFEncoder() {
    // 释放编码器
    for (auto element : encoderContexts) {
        delete element.second;
    }
    encoderContexts.clear();
}
