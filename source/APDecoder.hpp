//
//  APDecoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APDecoder_hpp
#define APDecoder_hpp

#include "MSCodecProtocol.h"
#include "MSMediaData.hpp"
#include "APCodecContext.hpp"

namespace MS {
    namespace APhard {
    
        typedef MSDecoderProtocol<__CVBuffer> APDecoderProtocol;
        
        class APDecoder : public APDecoderProtocol {
            
        public:
            const MSDecoderOutputData * decodeVideo(const MSMediaData<isEncode> &videoData);
            const MSDecoderOutputData * decodeAudio(const MSMediaData<isEncode> &audioData);
            APDecoder();
            ~APDecoder();
        };
        
    }
}


#endif /* APDecoder_hpp */
