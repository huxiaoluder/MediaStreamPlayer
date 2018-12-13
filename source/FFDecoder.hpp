//
//  MSDecoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSDecoder_hpp
#define MSDecoder_hpp

#include <map>
#include "MSCodecProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
       
        typedef MSDecoderProtocol<AVFrame> FFDecoderProtocol;
        
        class FFDecoder : public FFDecoderProtocol {
            std::map<MSCodecID,FFCodecContext *> decoderContexts;
            
            const FFCodecContext & getDecoderContext(MSCodecID codecID);
            
            MSDecoderOutputData * const decodeData(const MSMediaData<isEncode> &data);
        public:
            MSDecoderOutputData * const decodeVideo(const MSMediaData<isEncode> &videoData);
            MSDecoderOutputData * const decodeAudio(const MSMediaData<isEncode> &audioData);
            FFDecoder();
            ~FFDecoder();
        };
        
    }   
}

#endif /* MSDecoder_hpp */
