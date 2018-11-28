//
//  MSEncoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSEncoder.hpp"

using namespace MS;

using namespace std;

MSData * const
MSEncoder::encodeVideo(const MSData &pixelData) {
    MSData *data = new MSData(pixelData);
    return data;
}

MSData * const
MSEncoder::encodeAudio(const MSData &sampleData) {
    MSData *data = new MSData(sampleData);
    return data;
}

MSEncoder::MSEncoder() {
    
}

MSEncoder::~MSEncoder() {
    // 释放编码器
    for (auto element : *encoderMap) {
        delete element.second;
    }
    delete encoderMap;
}
