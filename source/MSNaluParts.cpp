//
//  MSNaluParts.cpp
//  ios_example
//
//  Created by 胡校明 on 2019/1/17.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSNaluParts.hpp"

using namespace MS;

static size_t
nextSeparatorOffset(const uint8_t * MSNonnull const lastPtr) {
    const uint8_t *nextPtr = lastPtr;
    while (true) {
        if (*nextPtr++ == 0x01) {
            if (*(uint32_t *)((uint64_t)nextPtr - 4) == 0x01000000) {
                return (uint64_t)nextPtr - (uint64_t)lastPtr - 4;
            }
            if (*(uint32_t *)((uint64_t)nextPtr - 3) << 8 == 0x01000000) {
                return (uint64_t)nextPtr - (uint64_t)lastPtr - 3;
            }
        }
    }
    return 0;
}

MSNaluParts::MSNaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize) {
    const uint8_t *ptr = nalUnit;
    while (true) {
        if (*ptr++ == 0x01) {
            switch (*ptr & 0x1F) {
                case 0x01: { // 非关键帧数据,不划分片段 (SLICE)
                    _slcRef = ptr;
                    _slcSize = naluSize - (ptr - nalUnit);
                }   return;
                case 0x02: { // 非关键帧数据划分片段A部分 (DPA)
                    _dpaRef = ptr;
                    _dpaSize = nextSeparatorOffset(ptr);
                    ptr += _dpaSize;
                }   break;
                case 0x03: { // 非关键帧数据划分片段B部分 (DPB)
                    _dpbRef = ptr;
                    _dpbSize = nextSeparatorOffset(ptr);
                    ptr += _dpbSize;
                }   break;
                case 0x04: { // 非关键帧数据划分片段C部分 (DPC)
                    _dpbRef = ptr;
                    _dpbSize = nextSeparatorOffset(ptr);
                    ptr += _dpbSize;
                }   break;
                case 0x05: { // 关键帧数据 (IDR)
                    _idrRef = ptr;
                    _idrSize = naluSize - (ptr - nalUnit);
                }   return;
                case 0x06: { // 补充增强信息（SEI）
                    _seiRef = ptr;
                    _seiSize = nextSeparatorOffset(ptr);
                    ptr += _seiSize;
                }   break;
                case 0x07: { // 序列参数集（SPS）
                    _spsRef = ptr;
                    _spsSize = nextSeparatorOffset(ptr);
                    ptr += _spsSize;
                }   break;
                case 0x08: { // 图像参数集（PPS）
                    _ppsRef = ptr;
                    _ppsSize = nextSeparatorOffset(ptr);
                    ptr += _ppsSize;
                }   break;
                default:break;
            }
        }
    }
}

const uint8_t *
MSNaluParts::slcRef() const {
    return _slcRef;
}

const uint8_t *
MSNaluParts::dpaRef() const {
    return _dpaRef;
}

const uint8_t *
MSNaluParts::dpbRef() const {
    return _dpbRef;
}

const uint8_t *
MSNaluParts::dpcRef() const {
    return _dpcRef;
}

const uint8_t *
MSNaluParts::idrRef() const {
    return _idrRef;
}

const uint8_t *
MSNaluParts::seiRef() const {
    return _seiRef;
}

const uint8_t *
MSNaluParts::spsRef() const {
    return _spsRef;
}

const uint8_t *
MSNaluParts::ppsRef() const {
    return _ppsRef;
}

size_t
MSNaluParts::slcSize() const {
    return _slcSize;
}

size_t
MSNaluParts::dpaSize() const {
    return _dpaSize;
}

size_t
MSNaluParts::dpbSize() const {
    return _dpbSize;
}

size_t
MSNaluParts::dpcSize() const {
    return _dpcSize;
}

size_t
MSNaluParts::idrSize() const {
    return _idrSize;
}

size_t
MSNaluParts::seiSize() const {
    return _seiSize;
}

size_t
MSNaluParts::spsSize() const {
    return _spsSize;
}

size_t
MSNaluParts::ppsSize() const {
    return _ppsSize;
}
