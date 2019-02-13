//
//  MSNaluParts.cpp
//  ios_example
//
//  Created by 胡校明 on 2019/1/17.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSNaluParts.hpp"
#include <cassert>
#include <cmath>

using namespace MS;

static inline bool
detectBitValue(const uint8_t * const spsRef, const size_t &startLocation) {
    return spsRef[startLocation / 8] & (0x80 >> (startLocation % 8));
}

static inline void
skipBits(size_t &starLocation, const size_t skipLen) {
    starLocation += skipLen;
}

static void
skipGolombBits(const uint8_t * const spsRef, size_t &startLocation, const int times) {
    for (int i = 0; i < times; i++) {
        // 指数
        int index = 0;
        while (!detectBitValue(spsRef, startLocation)) {
            index++;
            startLocation++;
        }
        startLocation += (index + 1);
    }
}

static int
getBitsValue(const uint8_t * const spsRef, size_t &startLocation, const int bitsCount) {
    int value = 0;
    for (int i = 0; i < bitsCount; i++) {
        value <<= 1;
        if (detectBitValue(spsRef, startLocation)) {
            value |= 1;
        }
        startLocation++;
    }
    return value;
}

static int
ueGolomb(const uint8_t * const spsRef, size_t &startLocation) {
    // 指数
    int index = 0;
    while (!detectBitValue(spsRef, startLocation)) {
        index++;
        startLocation++;
    }
    
    // 跳过对称位 1
    startLocation++;
    
    // 去除指数的余值
    int value = getBitsValue(spsRef, startLocation, index);
    
    return (1 << index | value) - 1;
}

static int
seGolomb(const uint8_t * const spsRef, size_t &startLocation) {
    int ueValue = ueGolomb(spsRef, startLocation);
    double k = ueValue;
    // ceil函数求不小于给定实数的最小整数
    int newValue = ceil(k/2);
    return ueValue % 2 == 0 ? -newValue : newValue;
}

static void
skipScalingList(const uint8_t * const spsRef, size_t &startLocation, size_t sizeOfScalingList) {
    int lastScale = 8;
    int nextScale = 8;
    int delta_scale;
    for (int i = 0; i < sizeOfScalingList; i++) {
        if (nextScale != 0) {
            delta_scale = seGolomb(spsRef, startLocation);
            nextScale = (lastScale + delta_scale + 256) % 256;
        }
        lastScale = nextScale == 0 ? lastScale : nextScale;
    }
}

static void
decode_vui_parameters(const uint8_t * const spsRef, size_t &startLocation, int &frameRate) {
    int aspect_ratio_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(aspect_ratio_info_present_flag) {
        int aspect_ratio_idc = getBitsValue(spsRef, startLocation, 8);
        if(aspect_ratio_idc == 255/* Extended_SAR = 255 */) {
            skipBits(startLocation, 32);
        }
    }
    int overscan_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(overscan_info_present_flag) {
        skipBits(startLocation, 1);
    }
    int video_signal_type_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(video_signal_type_present_flag) {
        skipBits(startLocation, 4);
        int colour_description_present_flag = getBitsValue(spsRef, startLocation, 1);
        if(colour_description_present_flag) {
            skipBits(startLocation, 24);
        }
    }
    int chroma_loc_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (chroma_loc_info_present_flag) {
        skipGolombBits(spsRef, startLocation, 2);
    }
    int timing_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if(timing_info_present_flag) {
        int num_units_in_tick = getBitsValue(spsRef, startLocation, 32);
        int time_scale = getBitsValue(spsRef, startLocation, 32);
        frameRate = time_scale / (2 * num_units_in_tick);
    }
}

static const uint8_t *
discardEmulationCode(const uint8_t * const sourceSpsRef, const size_t sourceSpsSize) {
    uint8_t * const realSps = new uint8_t[sourceSpsSize]{0};
    
    uint8_t * tempStart = realSps;
    int lastStartIdx = 0;
    size_t tempSize;
    
    for (int i = 3; i < sourceSpsSize; i++) {
        if (sourceSpsRef[i] == 0x03 &&
            *(uint32_t *)(sourceSpsRef + i - 2) << 8 == 0x03000000) {
            tempSize = i - lastStartIdx;
            memcpy(tempStart, sourceSpsRef+lastStartIdx, tempSize);
            tempStart += tempSize;
            lastStartIdx = i + 1;
            i += 2;
        }
    }
    
    tempSize = sourceSpsSize - lastStartIdx;
    memcpy(tempStart, sourceSpsRef+lastStartIdx, tempSize);
    return realSps;
}

static void
decode_h264_sps(const uint8_t * const sourceSpsRef, const size_t sourceSpsSize, MSVideoParameters &videoParameter) {
    
    const uint8_t * const realSps = discardEmulationCode(sourceSpsRef, sourceSpsSize);
    
    size_t startLocation = 0;
    
    skipBits(startLocation, 8);
    
    int profile_idc = getBitsValue(realSps, startLocation, 8);
    skipBits(startLocation, 16);
    skipGolombBits(realSps, startLocation, 1);
    
    if (profile_idc == 100 ||
        profile_idc == 110 ||
        profile_idc == 122 ||
        profile_idc == 144) {
        
        int chroma_format_idc = ueGolomb(realSps, startLocation);
        if(chroma_format_idc == 3) {
            skipBits(startLocation, 1);
        }
        skipGolombBits(realSps, startLocation, 2);
        skipBits(startLocation, 1);
        
        int seq_scaling_matrix_present_flag = getBitsValue(realSps, startLocation, 1);
        if(seq_scaling_matrix_present_flag) {
            for (int i = 0; i < 8; i++) {
                if (getBitsValue(realSps, startLocation, 1)) {
                    if (i < 6) {
                        skipScalingList(realSps, startLocation, 16);
                    } else {
                        skipScalingList(realSps, startLocation, 64);
                    }
                }
            }
        }
    }
    skipGolombBits(realSps, startLocation, 1);
    
    int pic_order_cnt_type = ueGolomb(realSps, startLocation);
    if(pic_order_cnt_type == 0) {
        skipGolombBits(realSps, startLocation, 1);
    } else if (pic_order_cnt_type == 1) {
        skipBits(startLocation, 1);
        skipGolombBits(realSps, startLocation, 2);
        skipGolombBits(realSps, startLocation, ueGolomb(realSps, startLocation));
    }
    skipGolombBits(realSps, startLocation, 1);
    skipBits(startLocation, 1);
    
    int pic_width_in_mbs_minus1 = ueGolomb(realSps, startLocation);
    int pic_height_in_map_units_minus1 = ueGolomb(realSps, startLocation);
    
    int frame_mbs_only_flag = getBitsValue(realSps, startLocation, 1);
    if(!frame_mbs_only_flag) {
        skipBits(startLocation, 1);
    }
    skipBits(startLocation, 1);
    
    int frame_cropping_flag = getBitsValue(realSps, startLocation, 1);
    if(frame_cropping_flag) {
        skipGolombBits(realSps, startLocation, 4);
    }
    
    int vui_parameters_present_flag = getBitsValue(realSps, startLocation, 1);
    if(vui_parameters_present_flag) {
        decode_vui_parameters(realSps, startLocation, videoParameter.frameRate);
    }
    
    videoParameter.width    = (pic_width_in_mbs_minus1 + 1) * 16;
    // (主流的 1080p, 720p, 360p)按16字节对齐, 可能会产生8位的冗余长度, 需要去除
    videoParameter.height   = (pic_height_in_map_units_minus1 + 1) * 16 ^ 8;
    
    delete [] realSps;
}

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
                    _dpbSize = naluSize - (ptr - nalUnit);
                }   return;
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

MSVideoParameters
MSNaluParts::videoParameter = MSVideoParameters();

void
MSNaluParts::parseSps(SpsType spsType) const {
    if (spsType) { // SpsTypeH264
        decode_h264_sps(_spsRef, _spsSize, videoParameter);
    } else {
        
    }
}
