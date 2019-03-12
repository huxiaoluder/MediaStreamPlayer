//
//  MediaStreamPlayer.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MediaStreamPlayer_h
#define MediaStreamPlayer_h

// MSPlayer 播放器组件
#include "MSCodecSyncProtocol.h"
#include "MSCodecAsynProtocol.h"
#include "MSAsynDataReceiver.h"
#include "MSPlayer.hpp"
#include "MSTimer.hpp"
#include "MSMedia.hpp"
#include "MSNaluParts.hpp"
#include "MSBinary.hpp"

// OpenGLES 渲染 YUV 定制化封装
#include "MSOpenGLES.hpp"
#include "MSGLHanlder.hpp"

// FFmpeg 跨平台软解/软编
#include "FFCodecContext.hpp"
#include "FFDecoder.hpp"
#include "FFEncoder.hpp"

#ifdef __APPLE__
// ios 硬解/硬编
#include "APCodecContext.hpp"
#include "APDecoder.hpp"
#include "APEncoder.hpp"
#include "APVideoRender.h"
#include "APAudioRender.hpp"
#elif
// android
#endif

#endif /* MediaStreamPlayer_h */
