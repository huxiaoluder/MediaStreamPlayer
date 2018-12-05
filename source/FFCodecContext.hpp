//
//  MSCodecSupport.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecContext_hpp
#define MSCodecContext_hpp

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

#include "MSMediaData"

#define ErrorLocationLog \
        printf("Error:\nfile: %s,\nline: %d,\nfunc: %s",__FILE__,__LINE__,__func__)

namespace MS {
    namespace FFmpeg {
        
        void av_frame_free(AVFrame * const frame);
        
        enum FFCodecType {
            FFCodecDecoder,
            FFCodecEncoder,
        };
        
        struct FFCodecContext {
            const FFCodecType codecType;
            
            const MSCodecID codecID;
            
            AVCodecContext * const codec_ctx;
            
            AVFormatContext * const fmt_ctx;
            
            FFCodecContext(const FFCodecType codecType, const MSCodecID codecID);
            
            ~FFCodecContext();
            
        private:
            AVCodecID const getAVCodecId();
            
            AVCodecContext * const initCodecContext();
            
            AVFormatContext * const initFormatContex();
        };
        
    }
}

#endif /* MSCodecSupport_hpp */
