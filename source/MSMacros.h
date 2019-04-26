//
//  MSMacros.h
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/1/16.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSMacros_h
#define MSMacros_h

#include <cstdio>
#include <cassert>

#define intervale(rate) microseconds(1000000LL / rate)

// aac 每 1024 帧数据(s16 大小: 2048 Byte), 开始编码 1 pack
#define AacPacketFrameNum 1024

#define AlawPacketFrameNum 160

#define MaxPixelBufferSize 100

#define MaxSampleBufferSize 40

namespace MS {
    extern bool EnableDebugLog;
}

#define ErrorLocationLog(reason) \
if (MS::EnableDebugLog) {\
    printf("\n"\
    "-----------------------------ERROR-----------------------------\n"\
    "| filepath:    %s\n"\
    "| linenumber:  %d\n"\
    "| funcname:    %s\n"\
    "| reson:       %s\n"\
    "---------------------------------------------------------------\n",__FILE__,__LINE__,__func__,reason);\
}

// parse OSStatus meaning
#define OSStatus2Str(status) \
char errStr[5] = {char(status >> 24), char(status >> 16), char(status >> 8), char(status) , 0x00}

#define OSStatusErrorLocationLog(reason,status) \
if (MS::EnableDebugLog) {\
    OSStatus2Str(status);\
    printf("\n"\
    "-----------------------------ERROR-----------------------------\n"\
    "| filepath:    %s\n"\
    "| linenumber:  %d\n"\
    "| funcname:    %s\n"\
    "| reson:       %s\n"\
    "| status:      %d --> %s\n"\
    "---------------------------------------------------------------\n",__FILE__,__LINE__,__func__,reason,(int)status,errStr);\
}

#ifdef __APPLE__

#define MSNullable  _Nullable
#define MSNonnull   _Nonnull
#define TimerPlatform MSTimerForApple

#else

#define MSNullable
#define MSNonnull
#define TimerPlatform MSTimerForOther

#endif

#endif /* MSMacros_h */
