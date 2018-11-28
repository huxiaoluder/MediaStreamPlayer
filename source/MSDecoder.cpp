//
//  MSDecoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSDecoder.hpp"

using namespace MS;

MSData * const
MSDecoder::decodeVideo(const MSData &videoData) {
    MSData *data = new MSData(videoData);
    return data;
}

MSData * const
MSDecoder::decodeAudio(const MSData &audioData) {
    MSData *data = new MSData(audioData);
    return data;
}

MSDecoder::MSDecoder() {
    
}

MSDecoder::~MSDecoder() {
    // 释放解码器
    for (auto element : *decoderMap) {
        delete element.second;
    }
    delete decoderMap;
}
