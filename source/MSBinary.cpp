//
//  MSBinary.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSBinary.hpp"

using namespace MS;

MSBinary::MSBinary(const uint8_t * MSNonnull const bytes,
                   const size_t size,
                   const bool allowAllocate)
:bytes(new uint8_t[size]), size(size), allowAllocate(allowAllocate) {
    if (allowAllocate) {
        memcpy(this->bytes, bytes, size);
    }
}

MSBinary::MSBinary(const MSBinary &data)
:bytes(new uint8_t[data.size]), size(data.size), allowAllocate(true) {
    memcpy(bytes, data.bytes, size);
}

MSBinary::~MSBinary() {
    if (allowAllocate) {
        delete [] bytes;
    }
}
