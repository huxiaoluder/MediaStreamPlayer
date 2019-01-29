//
//  MSNaluParts.cpp
//  ios_example
//
//  Created by 胡校明 on 2019/1/17.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSNaluParts.hpp"
#include <cmath>

using namespace MS;

static uint32_t
Ue(const uint8_t * const pBuff, const uint32_t nLen, uint32_t &nStartBit) {
    //计算0bit的个数
    uint32_t nZeroNum = 0;
    while (nStartBit < nLen * 8) {
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) { //&:按位与，%取余
            break;
        }
        nZeroNum++;
        nStartBit++;
    }
    nStartBit ++;
    
    //计算结果
    uint32_t dwRet = 0;
    for (uint32_t i=0; i<nZeroNum; i++) {
        dwRet <<= 1;
        if (pBuff[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            dwRet += 1;
        }
        nStartBit++;
    }
    return (1 << nZeroNum) - 1 + dwRet;
}


static uint32_t
Se(const uint8_t * const pBuff, const uint32_t nLen, uint32_t &nStartBit) {
    uint32_t UeVal=Ue(pBuff,nLen,nStartBit);
    double k=UeVal;
    uint32_t nValue=ceil(k/2);//ceil函数：ceil函数的作用是求不小于给定实数的最小整数。ceil(2)=ceil(1.2)=cei(1.5)=2.00
    if (UeVal % 2==0)
        nValue=-nValue;
    return nValue;
}


static uint32_t
u(const uint32_t BitCount, const uint8_t * const buf, uint32_t &nStartBit) {
    uint32_t dwRet = 0;
    for (uint32_t i=0; i<BitCount; i++) {
        dwRet <<= 1;
        if (buf[nStartBit / 8] & (0x80 >> (nStartBit % 8))) {
            dwRet += 1;
        }
        nStartBit++;
    }
    return dwRet;
}

/**
 * H264的NAL起始码防竞争机制
 *
 * @param buf SPS数据内容
 *
 * @无返回值
 */
//static void
//de_emulation_prevention(uint8_t *buf, uint32_t *buf_size) {
//    int i=0,j=0;
//    uint8_t* tmp_ptr=NULL;
//    unsigned int tmp_buf_size=0;
//    int val=0;
//
//    tmp_ptr=buf;
//    tmp_buf_size=*buf_size;
//    for(i=0;i<(tmp_buf_size-2);i++) {
//        //check for 0x000003
//        val=(tmp_ptr[i]^0x00) +(tmp_ptr[i+1]^0x00)+(tmp_ptr[i+2]^0x03);
//        if(val==0) {
//            //kick out 0x03
//            for(j=i+2;j<tmp_buf_size-1;j++)
//                tmp_ptr[j]=tmp_ptr[j+1];
//
//            //and so we should devrease bufsize
//            (*buf_size)--;
//        }
//    }
//}

/**
 * 解码SPS,获取视频图像宽、高信息
 *
 * @param buf SPS数据内容
 * @param nLen SPS数据的长度
 * @param width 图像宽度
 * @param height 图像高度
 * @成功则返回1 , 失败则返回0
 */
static void
h264_decode_sps(const uint8_t * const buf, const uint32_t nLen, int &width, int &height, int &fps) {
    uint32_t StartBit=0;
    fps=0;
    // H264的NAL起始码防竞争机制 (需要的自行打开注释)
//    uint8_t *temBuf = const_cast<uint8_t *>(buf);
//    de_emulation_prevention(temBuf,&nLen);
    
    //uint32_t forbidden_zero_bit=
    u(1,buf,StartBit);
    //uint32_t nal_ref_idc=
    u(2,buf,StartBit);
    uint32_t nal_unit_type=u(5,buf,StartBit);
    if(nal_unit_type==7) {
        int profile_idc=u(8,buf,StartBit);
        //uint32_t constraint_set0_flag=
        u(1,buf,StartBit);//(buf[1] & 0x80)>>7;
        //uint32_t constraint_set1_flag=
        u(1,buf,StartBit);//(buf[1] & 0x40)>>6;
        //uint32_t constraint_set2_flag=
        u(1,buf,StartBit);//(buf[1] & 0x20)>>5;
        //uint32_t constraint_set3_flag=
        u(1,buf,StartBit);//(buf[1] & 0x10)>>4;
        //uint32_t reserved_zero_4bits=
        u(4,buf,StartBit);
        //uint32_t level_idc=
        u(8,buf,StartBit);
        //uint32_t seq_parameter_set_id=
        Ue(buf,nLen,StartBit);
        
        if(profile_idc == 100 || profile_idc == 110 ||
           profile_idc == 122 || profile_idc == 144 ) {
            uint32_t chroma_format_idc=Ue(buf,nLen,StartBit);
            if(chroma_format_idc == 3) {
                //uint32_t residual_colour_transform_flag=
                u(1,buf,StartBit);
            }
            //uint32_t bit_depth_luma_minus8=
            Ue(buf,nLen,StartBit);
            //uint32_t bit_depth_chroma_minus8=
            Ue(buf,nLen,StartBit);
            //uint32_t qpprime_y_zero_transform_bypass_flag=
            u(1,buf,StartBit);
            uint32_t seq_scaling_matrix_present_flag=u(1,buf,StartBit);
            
            uint32_t seq_scaling_list_present_flag[8];
            if(seq_scaling_matrix_present_flag) {
                for(uint32_t i = 0; i < 8; i++ ) {
                    seq_scaling_list_present_flag[i]=u(1,buf,StartBit);
                }
            }
        }
        //uint32_t log2_max_frame_num_minus4=
        Ue(buf,nLen,StartBit);
        uint32_t pic_order_cnt_type=Ue(buf,nLen,StartBit);
        if( pic_order_cnt_type == 0 ) {
            //uint32_t log2_max_pic_order_cnt_lsb_minus4=
            Ue(buf,nLen,StartBit);
        } else if( pic_order_cnt_type == 1 ) {
            //uint32_t delta_pic_order_always_zero_flag=
            u(1,buf,StartBit);
            //uint32_t offset_for_non_ref_pic=
            Se(buf,nLen,StartBit);
            //uint32_t offset_for_top_to_bottom_field=
            Se(buf,nLen,StartBit);
            uint32_t num_ref_frames_in_pic_order_cnt_cycle=Ue(buf,nLen,StartBit);
            
            uint32_t *offset_for_ref_frame=new uint32_t[num_ref_frames_in_pic_order_cnt_cycle];
            for( uint32_t i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++ )
                offset_for_ref_frame[i]=Se(buf,nLen,StartBit);
            delete [] offset_for_ref_frame;
        }
        //uint32_t num_ref_frames=
        Ue(buf,nLen,StartBit);
        //uint32_t gaps_in_frame_num_value_allowed_flag=
        u(1,buf,StartBit);
        uint32_t pic_width_in_mbs_minus1=Ue(buf,nLen,StartBit);
        uint32_t pic_height_in_map_units_minus1=Ue(buf,nLen,StartBit);
        
        width=(pic_width_in_mbs_minus1+1)*16;
        height=(pic_height_in_map_units_minus1+1)*16;
        
        uint32_t frame_mbs_only_flag=u(1,buf,StartBit);
        if(!frame_mbs_only_flag)
            //uint32_t mb_adaptive_frame_field_flag=
            u(1,buf,StartBit);
        
        //uint32_t direct_8x8_inference_flag=
        u(1,buf,StartBit);
        uint32_t frame_cropping_flag=u(1,buf,StartBit);
        if(frame_cropping_flag) {
            //uint32_t frame_crop_left_offset=
            Ue(buf,nLen,StartBit);
            //uint32_t frame_crop_right_offset=
            Ue(buf,nLen,StartBit);
            //uint32_t frame_crop_top_offset=
            Ue(buf,nLen,StartBit);
            //uint32_t frame_crop_bottom_offset=
            Ue(buf,nLen,StartBit);
        }
        uint32_t vui_parameter_present_flag=u(1,buf,StartBit);
        if(vui_parameter_present_flag) {
            uint32_t aspect_ratio_info_present_flag=u(1,buf,StartBit);
            if(aspect_ratio_info_present_flag) {
                uint32_t aspect_ratio_idc=u(8,buf,StartBit);
                if(aspect_ratio_idc==255) {
                    //uint32_t sar_width=
                    u(16,buf,StartBit);
                    //uint32_t sar_height=
                    u(16,buf,StartBit);
                }
            }
            uint32_t overscan_info_present_flag=u(1,buf,StartBit);
            if(overscan_info_present_flag)
                //uint32_t overscan_appropriate_flagu=
                u(1,buf,StartBit);
            uint32_t video_signal_type_present_flag=u(1,buf,StartBit);
            if(video_signal_type_present_flag) {
                //uint32_t video_format=
                u(3,buf,StartBit);
                //uint32_t video_full_range_flag=
                u(1,buf,StartBit);
                uint32_t colour_description_present_flag=u(1,buf,StartBit);
                if(colour_description_present_flag) {
                    //uint32_t colour_primaries=
                    u(8,buf,StartBit);
                    //uint32_t transfer_characteristics=
                    u(8,buf,StartBit);
                    //uint32_t matrix_coefficients=
                    u(8,buf,StartBit);
                }
            }
            uint32_t chroma_loc_info_present_flag=u(1,buf,StartBit);
            if(chroma_loc_info_present_flag) {
                //uint32_t chroma_sample_loc_type_top_field=
                Ue(buf,nLen,StartBit);
                //uint32_t chroma_sample_loc_type_bottom_field=
                Ue(buf,nLen,StartBit);
            }
            uint32_t timing_info_present_flag=u(1,buf,StartBit);
            if(timing_info_present_flag) {
                uint32_t num_units_in_tick=u(32,buf,StartBit);
                uint32_t time_scale=u(32,buf,StartBit);
                fps=time_scale/(2*num_units_in_tick);
            }
        }
    }
}

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

MSVideoParameters
MSNaluParts::videoParameter = MSVideoParameters();

void
MSNaluParts::parseSps(SpsType spsType) const {
    if (spsType) { // SpsTypeH264
        h264_decode_sps(_spsRef,
                        (uint32_t)_spsSize,
                        videoParameter.width,
                        videoParameter.height,
                        videoParameter.frameRate);
    } else {
        
    }
}
