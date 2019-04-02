//
// MSUtil.cpp
// MediaStreamPlayer
// 
// Created by xiaoming on 2019/4/2.
// Copyright © 2019 freecoder. All rights reserved.
//
// Email: huxiaoluder@163.com
//

#include "MSUtil.hpp"

using namespace MS;

static inline bool
detectBitValue(const uint8_t * const dataRef, const size_t &startLocation) {
    return dataRef[startLocation / 8] & (0x80 >> (startLocation % 8));
}

static inline void
skipBits(size_t &starLocation, const size_t skipLen) {
    starLocation += skipLen;
}

static void
skipGolombBits(const uint8_t * const dataRef, size_t &startLocation, const int times) {
    for (int i = 0; i < times; i++) {
        // 指数
        int index = 0;
        while (!detectBitValue(dataRef, startLocation)) {
            index++;
            startLocation++;
        }
        startLocation += (index + 1);
    }
}

void
MS::reverse4Bytes(uint32_t &target) {
    uint32_t temp = 0;
    temp |= (target << 24);
    temp |= (target >> 24);
    temp |= (target & 0x0000FF00) << 8;
    temp |= (target & 0x00FF0000) >> 8;
    target = temp;
}

uint32_t
MS::getReverse4Bytes(const uint32_t target) {
    uint32_t temp = 0;
    temp |= (target << 24);
    temp |= (target >> 24);
    temp |= (target & 0x0000FF00) << 8;
    temp |= (target & 0x00FF0000) >> 8;
    return temp;
}

int
MS::getBitsValue(const uint8_t * const dataRef, size_t &startLocation, const int bitsCount) {
    int value = 0;
    for (int i = 0; i < bitsCount; i++) {
        value <<= 1;
        if (detectBitValue(dataRef, startLocation)) {
            value |= 1;
        }
        startLocation++;
    }
    return value;
}

void
MS::appendBitsValue(const uint32_t value, uint8_t * const dataRef, size_t &startLocation, const int bitsCount) {
    assert(bitsCount > 0 && bitsCount <= 32 && value >> bitsCount == 0);
    
    // 反转字节
    uint32_t writeValue = getReverse4Bytes(value);
    // 去除冗余 0 位, 右移高位补全, C 标准未定义, 但是大部分编译器, 正数右移都为补 0.
    while ((writeValue & 0x01) == 0) {
        writeValue >>= 1;
    }
    
    size_t offset = startLocation / 8;
    size_t modNum = startLocation % 8;
    
    uint8_t *writePtr = dataRef + offset;
    
    *(uint32_t *)writePtr |= writeValue << modNum;
    
    // 填充剩余数据
    uint32_t remainValue = writeValue >> 32 - modNum;
    if (remainValue) {
        writePtr[4] |= remainValue << (8 - modNum);
    }
    
    startLocation += bitsCount;
}

int
MS::ueGolomb(const uint8_t * const dataRef, size_t &startLocation) {
    // 指数
    int index = 0;
    while (!detectBitValue(dataRef, startLocation)) {
        index++;
        startLocation++;
    }
    
    // 跳过对称位 1
    startLocation++;
    
    // 去除指数的余值
    int value = getBitsValue(dataRef, startLocation, index);
    
    return (1 << index | value) - 1;
}

int
MS::seGolomb(const uint8_t * const dataRef, size_t &startLocation) {
    int ueValue = ueGolomb(dataRef, startLocation);
    double k = ueValue;
    // ceil函数求不小于给定实数的最小整数
    int newValue = ceil(k/2);
    return ueValue % 2 == 0 ? -newValue : newValue;
}

static void
skipH264ScalingList(const uint8_t * const spsRef, size_t &startLocation, size_t sizeOfScalingList) {
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
skipH265ScalingList(const uint8_t * const spsRef, size_t &startLocation) {
    for (int sizeId = 0; sizeId < 4; sizeId++) {
        for (int matrixId = 0; matrixId < (sizeId == 3 ? 2 : 6 ); matrixId++) {
            int scaling_list_pred_mode_flag = getBitsValue(spsRef, startLocation, 1);
            if (!scaling_list_pred_mode_flag) {
                skipGolombBits(spsRef, startLocation, 1);
            } else {
                int coefNum = std::min(64, (1 << (4 + (sizeId << 1))));
                if (sizeId > 1) {
                    skipGolombBits(spsRef, startLocation, 1);
                }
                for (int i = 0; i < coefNum; i++) {
                    skipGolombBits(spsRef, startLocation, 1);
                }
            }
        }
    }
}

static void
decode_h264_vui(const uint8_t * const spsRef, size_t &startLocation, int &frameRate) {
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
        frameRate = time_scale / (num_units_in_tick << 1);
    }
}

static void
decode_h265_vui(const uint8_t * const spsRef, size_t &startLocation, int &frameRate) {
    int aspect_ratio_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (aspect_ratio_info_present_flag) {
        int aspect_ratio_idc = getBitsValue(spsRef, startLocation, 8);
        if (aspect_ratio_idc == 255/* Extended_SAR = 255 */) {
            skipBits(startLocation, 32);
        }
    }
    
    int overscan_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (overscan_info_present_flag) {
        skipBits(startLocation, 1);
    }
    
    int video_signal_type_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (video_signal_type_present_flag) {
        skipBits(startLocation, 4);
        
        int colour_description_present_flag = getBitsValue(spsRef, startLocation, 1);
        if (colour_description_present_flag) {
            skipBits(startLocation, 24);
        }
    }
    
    int chroma_loc_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (chroma_loc_info_present_flag) {
        skipGolombBits(spsRef, startLocation, 2);
    }
    
    skipBits(startLocation, 3);
    
    int default_display_window_flag = getBitsValue(spsRef, startLocation, 1);
    if (default_display_window_flag) {
        skipGolombBits(spsRef, startLocation, 4);
    }
    
    int vui_timing_info_present_flag = getBitsValue(spsRef, startLocation, 1);
    if (vui_timing_info_present_flag) {
        int vui_num_units_in_tick = getBitsValue(spsRef, startLocation, 32);
        int vui_time_scale = getBitsValue(spsRef, startLocation, 32);
        frameRate = vui_time_scale / vui_num_units_in_tick;
    }
}

static void
profile_tier_level(const uint8_t * const spsRef, size_t &startLocation, int sps_max_sub_layers_minus1) {
    skipBits(startLocation, 96);
    
    int sub_layer_profile_present_flag[sps_max_sub_layers_minus1];
    int sub_layer_level_present_flag[sps_max_sub_layers_minus1];
    for (int i = 0; i < sps_max_sub_layers_minus1; i++) {
        sub_layer_profile_present_flag[i] = getBitsValue(spsRef, startLocation, 1);
        sub_layer_level_present_flag[i] = getBitsValue(spsRef, startLocation, 1);
    }
    
    if (sps_max_sub_layers_minus1 > 0) {
        skipBits(startLocation, 2 * (8 - sps_max_sub_layers_minus1));
    }
    
    for (int i = 0; i < sps_max_sub_layers_minus1; i++) {
        if (sub_layer_profile_present_flag[i]) {
            skipBits(startLocation, 88);
        }
        if (sub_layer_level_present_flag[i]) {
            skipBits(startLocation, 8);
        }
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

void
MS::decode_h264_sps(const uint8_t * const sourceSpsRef,
                    const size_t          sourceSpsSize,
                    MSVideoParameters     &videoParameter) {
    
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
                        skipH264ScalingList(realSps, startLocation, 16);
                    } else {
                        skipH264ScalingList(realSps, startLocation, 64);
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
    
    int pic_width_in_mbs_minus1         = ueGolomb(realSps, startLocation);
    int pic_height_in_map_units_minus1  = ueGolomb(realSps, startLocation);
    
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
        decode_h264_vui(realSps, startLocation, videoParameter.frameRate);
    }
    
    videoParameter.width  = (pic_width_in_mbs_minus1 + 1) * 16;
    // (主流的 1080p, 720p, 360p)按16字节对齐, 可能会产生8位的冗余长度, 需要去除
    videoParameter.height = (pic_height_in_map_units_minus1 + 1) * 16;
    if (videoParameter.height == 1088) {
        videoParameter.height = 1080;
    }
    
    delete [] realSps;
}

void
MS::decode_h265_sps(const uint8_t * const sourceSpsRef,
                    const size_t          sourceSpsSize,
                    MSVideoParameters     &videoParameter) {
    const uint8_t * const realSps = discardEmulationCode(sourceSpsRef, sourceSpsSize);
    
    size_t startLocation = 0;
    
    skipBits(startLocation, 20);
    
    int sps_max_sub_layers_minus1 = getBitsValue(realSps, startLocation, 3);
    
    skipBits(startLocation, 1);
    
    profile_tier_level(realSps, startLocation, sps_max_sub_layers_minus1);
    
    skipGolombBits(realSps, startLocation, 1);
    
    int chroma_format_idc = ueGolomb(realSps, startLocation);
    if (chroma_format_idc == 3) {
        skipBits(startLocation, 1);
    }
    
    int pic_width_in_luma_samples   = ueGolomb(realSps, startLocation);
    int pic_height_in_luma_samples  = ueGolomb(realSps, startLocation);
    
    int conformance_window_flag = getBitsValue(realSps, startLocation, 1);
    if (conformance_window_flag) {
        skipGolombBits(realSps, startLocation, 4);
    }
    
    skipGolombBits(realSps, startLocation, 2);
    
    int log2_max_pic_order_cnt_lsb_minus4 = ueGolomb(realSps, startLocation);
    
    int sps_sub_layer_ordering_info_present_flag = getBitsValue(realSps, startLocation, 1);
    for (int i = (sps_sub_layer_ordering_info_present_flag ? 0 : sps_max_sub_layers_minus1);
         i <= sps_max_sub_layers_minus1; i++) {
        skipGolombBits(realSps, startLocation, 3);
    }
    
    skipGolombBits(realSps, startLocation, 6);
    
    int scaling_list_enabled_flag = getBitsValue(realSps, startLocation, 1);
    if (scaling_list_enabled_flag) {
        int sps_scaling_list_data_present_flag = getBitsValue(realSps, startLocation, 1);
        if (sps_scaling_list_data_present_flag) {
            skipH265ScalingList(realSps, startLocation);
        }
    }
    
    skipBits(startLocation, 2);
    
    int pcm_enabled_flag = getBitsValue(realSps, startLocation, 1);
    if (pcm_enabled_flag) {
        skipBits(startLocation, 8);
        skipGolombBits(realSps, startLocation, 2);
        skipBits(startLocation, 1);
    }
    
    skipGolombBits(realSps, startLocation, 1);
    
    int long_term_ref_pics_present_flag = getBitsValue(realSps, startLocation, 1);
    if (long_term_ref_pics_present_flag) {
        int num_long_term_ref_pics_sps = ueGolomb(realSps, startLocation);
        for (int i = 0; i < num_long_term_ref_pics_sps; i++) {
            skipBits(startLocation, log2_max_pic_order_cnt_lsb_minus4 + 4 + 1);
        }
    }
    
    skipBits(startLocation, 2);
    
    int vui_parameters_present_flag = getBitsValue(realSps, startLocation, 1);
    if (vui_parameters_present_flag) {
        decode_h265_vui(realSps, startLocation, videoParameter.frameRate);
    }
    
    videoParameter.width    = pic_width_in_luma_samples;
    videoParameter.height   = pic_height_in_luma_samples;
    
    delete [] realSps;
}

void
MS::decode_aac_adts(const uint8_t * const sourceAdtsRef,
                    const size_t          sourceAdtsSize,
                    MSAudioParameters     &audioParameter) {
    size_t startLocation = 0;
    skipBits(startLocation, 16);
    audioParameter.profile          = getBitsValue(sourceAdtsRef, startLocation, 2);
    audioParameter.frequency.index  = getBitsValue(sourceAdtsRef, startLocation, 4);
    audioParameter.frequency.value  = adtsFrequencyList[audioParameter.frequency.index];
    skipBits(startLocation, 1);
    audioParameter.channels         = getBitsValue(sourceAdtsRef, startLocation, 3);
}

void
MS::insertFramerateToSps(const int framerate,
                         const uint8_t * const  inSps,
                         const size_t           inSize,
                         const uint8_t ** const outSps,
                         size_t * const         outSize) {
    const uint8_t * const realSps = discardEmulationCode(inSps, inSize);
    
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
                        skipH264ScalingList(realSps, startLocation, 16);
                    } else {
                        skipH264ScalingList(realSps, startLocation, 64);
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
    skipGolombBits(realSps, startLocation, 2);
    
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
        int aspect_ratio_info_present_flag = getBitsValue(realSps, startLocation, 1);
        if(aspect_ratio_info_present_flag) {
            int aspect_ratio_idc = getBitsValue(realSps, startLocation, 8);
            if(aspect_ratio_idc == 255/* Extended_SAR = 255 */) {
                skipBits(startLocation, 32);
            }
        }
        
        int overscan_info_present_flag = getBitsValue(realSps, startLocation, 1);
        if(overscan_info_present_flag) {
            skipBits(startLocation, 1);
        }
        
        int video_signal_type_present_flag = getBitsValue(realSps, startLocation, 1);
        if(video_signal_type_present_flag) {
            skipBits(startLocation, 4);
            int colour_description_present_flag = getBitsValue(realSps, startLocation, 1);
            if(colour_description_present_flag) {
                skipBits(startLocation, 24);
            }
        }
        
        int chroma_loc_info_present_flag = getBitsValue(realSps, startLocation, 1);
        if (chroma_loc_info_present_flag) {
            skipGolombBits(realSps, startLocation, 2);
        }
        
        *outSize = inSize + 2 * 4;
        uint8_t * const newSps = new uint8_t[*outSize]{0};
        *outSps = newSps;
        
        int timing_info_present_flag = getBitsValue(realSps, startLocation, 1);
        if(!timing_info_present_flag) {
            startLocation -= 1;
            appendBitsValue(1, newSps, startLocation, 1);
            
            size_t offset = startLocation / 8;
            size_t modBit = startLocation % 8;
            const uint8_t *tempPtr = realSps + offset;
            
            uint32_t num_units_in_tick = 1;
            uint32_t time_scale = (uint32_t)framerate << 1;
        }
    }
    
    delete [] realSps;
}
