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
#include "MSType.hpp"
#include "MSCodecProtocol.h"
#include "MSCodecContext.hpp"

namespace MS {
    
    class MSDecoder : public MSDecoderProtocol {
        std::map<MSCodecID, MSCodecContext *> *decoderMap = new std::map<MSCodecID, MSCodecContext *>();
        
    public:
        MSData * const decodeVideo(const MSData &videoData);
        MSData * const decodeAudio(const MSData &audioData);
        MSDecoder();
        ~MSDecoder();
    };
    
}

#endif /* MSDecoder_hpp */
