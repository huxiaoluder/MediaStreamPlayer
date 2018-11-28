//
//  MSCodecSupport.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecContext_hpp
#define MSCodecContext_hpp

#include "MSType.hpp"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}
#pragma clang diagnostic pop

namespace MS {
    
    enum MSCodecType {
        MSCodecDecoder,
        MSCodecEncoder,
    };
    
    struct MSCodecContext {
        MSCodecType const codecType;
        
        MSCodecID const codecID;
        
        AVCodecContext * const codec_ctx;
        
        AVFormatContext * const fmt_ctx;
        
        MSCodecContext(MSCodecType codecType, MSCodecID codecID);
        
        ~MSCodecContext();
    
    private:
        AVCodecID const getAVCodecId();
        
        AVCodecContext * const initCodecContext();
        
        AVFormatContext * const initFormatContex();
    };
    
}

#endif /* MSCodecSupport_hpp */
