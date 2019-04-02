//
//  MSNaluParts.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/1/17.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSNaluParts.hpp"
#include <cassert>

using namespace MS;

static size_t
nextSeparatorOffset(const uint8_t * MSNonnull const lastPtr) {
    const uint8_t *nextPtr = lastPtr;
    while (true) {
        if (*nextPtr++ == 0x01) {
            if (*(uint32_t *)(nextPtr - 4) == 0x01000000) {
                return nextPtr - lastPtr - 4;
            }
            if (*(uint32_t *)(nextPtr - 3) << 8 == 0x01000000) {
                return nextPtr - lastPtr - 3;
            }
        }
    }
    return 0;
}

void
MSNaluParts::initH264NaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize) {
    const uint8_t *ptr = nalUnit;
    while (true) {
        if (*ptr++ == 0x01) {
            switch (*ptr & 0x1F) {
                case 1: { // 非关键帧数据,不划分片段 (SLICE)
                    _slcRef = ptr;
                    _slcSize = naluSize - (ptr - nalUnit);
                }   return;
//                case 2: { // 非关键帧数据划分片段A部分 (DPA)
//                    _dpaRef = ptr;
//                    _dpaSize = nextSeparatorOffset(ptr);
//                    ptr += _dpaSize;
//                }   break;
//                case 3: { // 非关键帧数据划分片段B部分 (DPB)
//                    _dpbRef = ptr;
//                    _dpbSize = nextSeparatorOffset(ptr);
//                    ptr += _dpbSize;
//                }   break;
//                case 4: { // 非关键帧数据划分片段C部分 (DPC)
//                    _dpbRef = ptr;
//                    _dpbSize = naluSize - (ptr - nalUnit);
//                }   return;
                case 5: { // 关键帧数据 (IDR)
                    _idrRef = ptr;
                    _idrSize = naluSize - (ptr - nalUnit);
                }   return;
                case 6: { // 补充增强信息（SEI）
                    _seiRef = ptr;
                    _seiSize = nextSeparatorOffset(ptr);
                    ptr += _seiSize;
                }   break;
                case 7: { // 序列参数集（SPS）
                    _spsRef = ptr;
                    _spsSize = nextSeparatorOffset(ptr);
                    ptr += _spsSize;
                }   break;
                case 8: { // 图像参数集（PPS）
                    _ppsRef = ptr;
                    _ppsSize = nextSeparatorOffset(ptr);
                    ptr += _ppsSize;
                }   break;
                default:break;
            }
        }
    }
}

void
MSNaluParts::initH265NaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize) {
    const uint8_t *ptr = nalUnit;
    while (true) {
        if (*ptr++ == 0x01) {
            switch (*ptr >> 1) {
                case 1: { // 非关键帧数据,不划分片段 (SLICE)
                    _slcRef = ptr;
                    _slcSize = naluSize - (ptr - nalUnit);
                }   return;
                case 19: { // 关键帧数据 (IDR)
                    _idrRef = ptr;
                    _idrSize = naluSize - (ptr - nalUnit);
                }   return;
                case 32: { // 视频参数集 (VPS)
                    _vpsRef = ptr;
                    _vpsSize = nextSeparatorOffset(ptr);
                    ptr += _vpsSize;
                }
                case 33: { // 序列参数集（SPS）
                    _spsRef = ptr;
                    _spsSize = nextSeparatorOffset(ptr);
                    ptr += _spsSize;
                }   break;
                case 34: { // 图像参数集（PPS）
                    _ppsRef = ptr;
                    _ppsSize = nextSeparatorOffset(ptr);
                    ptr += _ppsSize;
                }   break;
                case 39: { // 补充增强信息（SEI）
                    _seiRef = ptr;
                    _seiSize = nextSeparatorOffset(ptr);
                    ptr += _seiSize;
                }   break;
                default:break;
            }
        }
    }
}

void
MSNaluParts::initAACNaluParts(const uint8_t * MSNonnull const nalUnit,  const size_t naluSize) {
    size_t startLocation = 15;
    int protectionAbsent = getBitsValue(nalUnit, startLocation, 1);
    
    _adtsRef    = nalUnit;
    _adtsSize   = protectionAbsent == 1 ? 7 : 9;
    _dataRef    = _adtsRef + _adtsSize;
    _dataSize   = naluSize - _adtsSize;
}



MSNaluParts::MSNaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize, const MSCodecID codecID) {
    if (codecID == MSCodecID_H264) {
        initH264NaluParts(nalUnit, naluSize);
    } else if (codecID == MSCodecID_H265) {
        initH265NaluParts(nalUnit, naluSize);
    } else if (codecID == MSCodecID_AAC) {
        initAACNaluParts(nalUnit, naluSize);
    }
}

const uint8_t *
MSNaluParts::vpsRef() const {
    return _vpsRef;
}

const uint8_t *
MSNaluParts::spsRef() const {
    return _spsRef;
}

const uint8_t *
MSNaluParts::ppsRef() const {
    return _ppsRef;
}

const uint8_t *
MSNaluParts::seiRef() const {
    return _seiRef;
}

const uint8_t *
MSNaluParts::idrRef() const {
    return _idrRef;
}

const uint8_t *
MSNaluParts::slcRef() const {
    return _slcRef;
}

/**
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
 */

size_t
MSNaluParts::vpsSize() const {
    return _vpsSize;
}

size_t
MSNaluParts::spsSize() const {
    return _spsSize;
}

size_t
MSNaluParts::ppsSize() const {
    return _ppsSize;
}

size_t
MSNaluParts::seiSize() const {
    return _seiSize;
}

size_t
MSNaluParts::idrSize() const {
    return _idrSize;
}

size_t
MSNaluParts::slcSize() const {
    return _slcSize;
}

/**
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
 */

const uint8_t * MSNullable
MSNaluParts::adtsRef() const {
    return _adtsRef;
}

size_t
MSNaluParts::adtsSize() const {
    return _adtsSize;
}

const uint8_t * MSNonnull
MSNaluParts::dataRef() const {
    return _dataRef;
}

size_t
MSNaluParts::dataSize() const {
    return _dataSize;
}

const MSVideoParameters *
MSNaluParts::parseH264Sps() const {
    MSVideoParameters *videoParameters = new MSVideoParameters;
    decode_h264_sps(_spsRef, _spsSize, *videoParameters);
    return videoParameters;
}

const MSVideoParameters *
MSNaluParts::parseH265Sps() const {
    MSVideoParameters *videoParameters = new MSVideoParameters;
    decode_h265_sps(_spsRef, _spsSize, *videoParameters);
    return videoParameters;
}

const MSAudioParameters *
MSNaluParts::parseAacAdts() const {
    MSAudioParameters *audioParameters = new MSAudioParameters;
    decode_aac_adts(_adtsRef, _adtsSize, *audioParameters);
    return audioParameters;
}

