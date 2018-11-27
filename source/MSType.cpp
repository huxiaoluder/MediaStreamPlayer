//
//  MSType.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/15.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSType.hpp"

using namespace MS;

MSData::MSData(uint8_t * const bytes, size_t const len, std::chrono::microseconds const timeInterval)
:bytes(new uint8_t[len]()), len(len), timeInterval(timeInterval) {
    memcpy(this->bytes, bytes, len);
}

MSData::MSData(const MSData& data)
:bytes(new uint8_t[data.len]()), len(data.len), timeInterval(data.timeInterval) {
    memcpy(bytes, data.bytes, len);
}

MSData::~MSData() {
    delete []bytes;
}

MSData *
MSData::copy() {
    return new MSData(*this);
}
