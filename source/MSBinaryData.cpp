//
//  MSBinaryData.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSBinaryData.hpp"

using namespace MS;

MSBinaryData::MSBinaryData(const uint8_t * const bytes, const size_t size)
:bytes(new uint8_t[size]), size(size) {
    memcpy(this->bytes, bytes, size);
}

MSBinaryData::MSBinaryData(const MSBinaryData &data)
:bytes(new uint8_t[data.size]), size(data.size) {
    memcpy(bytes, data.bytes, size);
}

MSBinaryData::~MSBinaryData() {
    delete [] bytes;
}
