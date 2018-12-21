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

#include "MSMediaData.hpp"

#define ErrorLocationLog \
        printf( "-----------------------ERROR----------------------\n"\
                "| filename:    %-s\n"\
                "| linenumber:  %d,\n"\
                "| funcname:    %s\n"\
                "--------------------------------------------------\n",__FILE__,__LINE__,__func__)

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
            
            AVCodec * const codec;
            
            AVCodecContext * const codec_ctx;
            
            /**
             * if codecType == FFCodecEncoder
             * set codec_ctx params by user
             * codec open by user
             */
            FFCodecContext(const FFCodecType codecType, const MSCodecID codecID);
            
            ~FFCodecContext();
            
            static AVCodecID getAVCodecId(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (FFCodecContext &)obj
            FFCodecContext(const FFCodecContext &codecContext);
            
            AVCodec * initCodec();
            
            AVCodecContext * initCodecContext();
            
            AVFormatContext * initFormatContex();
        };
        
    }
}

#endif /* MSCodecSupport_hpp */
