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
#include "MSMediaData"
#include "MSCodecProtocol.h"
#include "MSCodecContext.hpp"

namespace MS {
    
    typedef MSDecoderProtocol<AVFrame *> FFDecoderProtocol;
    
    class FFDecoder : public FFDecoderProtocol {
        std::map<MSCodecID,MSCodecContext *> decoderContexts;
        
        const MSCodecContext & getDecoderContext(MSCodecID codecID);
    public:
        outputType * const decodeVideo(const MSMediaData<isEncode> &videoData);
        outputType * const decodeAudio(const MSMediaData<isEncode> &audioData);
        FFDecoder();
        ~FFDecoder();
    };
    
}

#endif /* MSDecoder_hpp */
