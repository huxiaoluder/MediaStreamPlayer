//
//  MSBinary.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSBinary.hpp"

using namespace MS;

MSBinary::MSBinary(const uint8_t * MSNonnull const bytes, const size_t size)
:bytes(new uint8_t[size]), size(size) {
    memcpy(this->bytes, bytes, size);
}

MSBinary::MSBinary(const MSBinary &data)
:bytes(new uint8_t[data.size]), size(data.size) {
    memcpy(bytes, data.bytes, size);
}

MSBinary::~MSBinary() {
    delete [] bytes;
}
