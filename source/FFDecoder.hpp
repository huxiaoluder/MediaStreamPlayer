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
            
            const MSDecoderOutputData * decodeData(const MSMediaData<isEncode> &data);
        public:
            const MSDecoderOutputData * decodeVideo(const MSMediaData<isEncode> &videoData);
            const MSDecoderOutputData * decodeAudio(const MSMediaData<isEncode> &audioData);
            FFDecoder();
            ~FFDecoder();
            
            const FFCodecContext * findDecoderContext(MSCodecID codecID);
        };
        
    }   
}

#endif /* MSDecoder_hpp */
