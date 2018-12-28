//
//  MediaStreamPlayer.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MediaStreamPlayer_h
#define MediaStreamPlayer_h

#include "MSCodecProtocol.h"
#include "MSMediaData.hpp"
#include "MSBinaryData"
#include "MSTimer.hpp"
#include "MSPlayer.hpp"

// FFmpeg 跨平台软解/软编
#include "FFCodecContext.hpp"
#include "FFDecoder.hpp"
#include "FFEncoder.hpp"

#ifdef __APPLE__
// ios 硬解/硬编
#elif
// android
#endif

#endif /* MediaStreamPlayer_h */
