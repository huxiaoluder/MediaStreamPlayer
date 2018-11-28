//
//  MSEncoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSEncoder_hpp
#define MSEncoder_hpp

#include <map>
#include "MSType.hpp"
#include "MSCodecProtocol.h"
#include "MSCodecContext.hpp"

namespace MS {
    
    class MSEncoder : public MSEncoderProtocol {
        std::map<MSCodecID,MSCodecContext *> *encoderMap = new std::map<MSCodecID, MSCodecContext *>();
        
    public:
        MSData * const encodeVideo(const MSData &pixelData);
        MSData * const encodeAudio(const MSData &sampleData);
        MSEncoder();
        ~MSEncoder();
    };
    
}

#endif /* MSEncoder_hpp */
