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

#include "MSBinary.hpp"

namespace MS {
    
    static const int adtsFrequencyList[] = {
        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350
    };
    
    /**
     Reference: https://wiki.multimedia.cx/index.php?title=ADTS
     AAAAAAAA AAAABCCD EEFFFFGH HHIJKLMM MMMMMMMM MMMOOOOO OOOOOOPP (QQQQQQQQ QQQQQQQQ)
     Header consists of 7 or 9 bytes (without or with CRC).
     
     @syncword  all bits must be 1
     @version  0 for MPEG-4, 1 for MPEG-2
     @layer always 0
     @protectionAbsent  Warning, set to 1 if there is no CRC and 0 if there is CRC
     @profile  the MPEG-4 Audio Object Type minus 1
     @frequencyIndex  15 is forbidden
     @privateBit  guaranteed never to be used by MPEG, set to 0 when encoding, ignore when decoding
     @channelConfiguration  in the case of 0, the channel configuration is sent via an inband PCE
     @originality  set to 0 when encoding, ignore when decoding
     @home  set to 0 when encoding, ignore when decoding
     @copyrightIdBit  the next bit of a centrally registered copyright identifier, set to 0 when encoding, ignore when decoding
     @copyrightIdStart  signals that this frame's copyright id bit is the first bit of the copyright id, set to 0 when encoding, ignore when decoding
     @frameLength  this value must include 7 or 9 bytes of header length: FrameLength = (ProtectionAbsent == 1 ? 7 : 9) + size(AACFrame)
     @bufferFullness
     @numberOfFrames  (RDBs) in ADTS frame minus 1, for maximum compatibility always use 1 AAC frame per ADTS frame
     @CRC  if protection absent is 0
     */
#pragma pack(push, 1)
    struct MSAdtsForAAC {
        unsigned int CRC                 :16;
        unsigned int numberOfFrames      :2;
        unsigned int bufferFullness      :11;
        unsigned int frameLength         :13;
        unsigned int copyrightIdStart    :1;
        unsigned int copyrightIdBit      :1;
        unsigned int home                :1;
        unsigned int originality         :1;
        unsigned int channelConfiguration:3;
        unsigned int privateBit          :1;
        unsigned int frequencyIndex      :4;
        unsigned int profile             :2; //@Note: adts 中因为位域原因, profile == ObjectType - 1
        unsigned int protectionAbsent    :1;
        unsigned int layer               :2;
        unsigned int version             :1;
        unsigned int syncword            :12;
        
        MSAdtsForAAC & initialize();
        
        /**
         获取二进制数据流

         @return 二进制流, free by caller
         */
        MSBinary * getBigEndianBinary();
    };
    
    struct MSAdtsForMp4 {
        unsigned int reserve             :3;
        unsigned int channelConfiguration:4;
        unsigned int frequencyIndex      :4;
        unsigned int profile             :5; //@Note: mp4 中不用参考位域限制, profile == ObjectType
        
        MSAdtsForMp4 & initialize();
        
        /**
         获取二进制数据流
         
         @return 二进制流, free by caller
         */
        MSBinary * getBigEndianBinary();
    };
#pragma pack(pop)
    
    struct MSVideoParameters {
        int width       = 0;
        int height      = 0;
        int frameRate   = 0;
        int isColorFullRange = 0;
    };
    
    struct MSAudioParameters {
        int profile     = 0; // == MPEG-4 Audio Object Types (APPLE ENUM: MPEG4ObjectID) - 1
        int channels    = 0;
        struct {
            int index   = 0; // index of adtsFrequencyList
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
     解析 h265 sps 数据, 获取: width, height, framerate, isColorFullRange

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
                              const uint8_t *  const inSps,
                              const size_t           inSize,
                              const uint8_t ** const outSps,
                              size_t * const         outSize);
    
}

#endif /* MSUtil_hpp */
