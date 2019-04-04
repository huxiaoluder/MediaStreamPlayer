//
// MSUtil.hpp
// MediaStreamPlayer
// 
// Created by xiaoming on 2019/4/2.
// Copyright © 2019 freecoder. All rights reserved.
//
// Email: huxiaoluder@163.com
//

#ifndef MSUtil_hpp
#define MSUtil_hpp

#include <cstring>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace MS {
    
    static const int adtsFrequencyList[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350
    };
    
    struct MSVideoParameters {
        int width       = 0;
        int height      = 0;
        int frameRate   = 0;
    };
    
    struct MSAudioParameters {
        int profile     = 0; // == MPEG-4 Audio Object Types (APPLE ENUM: MPEG4ObjectID) - 1
        int channels    = 0;
        struct {
            int index   = 0;  // index of adtsFrequencyList
            int value   = 0; // value of index in adtsFrequencyList
        } frequency;
    };
    
    /**
     反转 4 字节数据源

     @param target [inout] 反转目标引用
     */
    void reverse4Bytes(uint32_t &target);
    
    /**
     反转 4 字节数据, 得到新的 4 字节数据

     @param target [in] 反转数据
     @return 反转结果
     */
    uint32_t getReverse4Bytes(const uint32_t target);
    
    /**
     按位取值

     @param dataRef [in] 数据源
     @param startLocation [inout] 读取位置
     @param bitsCount [in] 读取位数
     @return 读取值
     */
    int getBitsValue(const uint8_t * const dataRef, size_t &startLocation, const size_t bitsCount);
    
    /**
     按位写值(最多支持 32 bits), 数据源缓冲区需保证空间足够, 初始数据为 0.
     注: 不能用于在已有数据中间插入数据, 可能会出现插入数据错误, 除非你确定不会出现错误
     
     @param value [in] 写入数据
     @param dataRef [in] 写入数据源
     @param startLocation [inout] 写入位置
     @param bitsCount [in] 写入位数
     */
    void putBitsValue(const uint32_t value, uint8_t * const dataRef, size_t &startLocation, const size_t bitsCount);
    
    /**
     哥伦布编码取值(无符号)

     @param dataRef [in] 数据源
     @param startLocation [inout] 读取位置
     @return 读取值
     */
    int ueGolomb(const uint8_t * const dataRef, size_t &startLocation);
    
    /**
     哥伦布编码取值(有符号)

     @param dataRef [in] 数据源
     @param startLocation [inout] 读取位置
     @return 读取值
     */
    int seGolomb(const uint8_t * const dataRef, size_t &startLocation);
    
    /**
     解析 h264 sps 数据, 获取: width, height, framerate

     @param sourceSpsRef [in] 数据源
     @param sourceSpsSize [in] 数据大小
     @param videoParameter [inout] 解析结果
     */
    void decode_h264_sps(const uint8_t * const sourceSpsRef,
                         const size_t          sourceSpsSize,
                         MSVideoParameters     &videoParameter);
    
    /**
     解析 h265 sps 数据, 获取: width, height, framerate

     @param sourceSpsRef [in] 数据源
     @param sourceSpsSize [in] 数据大小
     @param videoParameter [inout] 解析结果
     */
    void decode_h265_sps(const uint8_t * const sourceSpsRef,
                         const size_t          sourceSpsSize,
                         MSVideoParameters     &videoParameter);
    
    /**
     解析 aac adts 数据, 获取: profile, channles, frequency

     @param sourceAdtsRef [in] 数据源
     @param sourceAdtsSize [in] 数据大小
     @param audioParameter [inout] 解析结果
     */
    void decode_aac_adts(const uint8_t * const sourceAdtsRef,
                         const size_t          sourceAdtsSize,
                         MSAudioParameters     &audioParameter);
    
    /**
     为 sps 插入帧率信息
     
     @param framerate [in] 帧率
     @param inSps [in] 旧 sps
     @param inSize [in] 旧 sps 大小
     @param outSps [out] 新 sps
     @param outSize [out] 新 sps 大小
     */
    void insertFramerateToSps(const int framerate,
                              const uint8_t * const  inSps,
                              const size_t           inSize,
                              const uint8_t ** const outSps,
                              size_t * const         outSize);
    
}

#endif /* MSUtil_hpp */
