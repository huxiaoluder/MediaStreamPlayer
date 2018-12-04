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
#include "MSMediaData"
#include "MSCodecProtocol.h"
#include "MSCodecContext.hpp"

namespace MS {

    typedef MSEncoderProtocol<AVFrame *> FFEncoderProtocol;
    
    class FFEncoder : public FFEncoderProtocol {
        std::map<MSCodecID,MSCodecContext *> encoderContexts;
        
    public:
        MSMediaData<isEncode> * const encodeVideo(const inputType &pixelData);
        MSMediaData<isEncode> * const encodeAudio(const inputType &sampleData);
        FFEncoder();
        ~FFEncoder();
    };
    
}

#endif /* MSEncoder_hpp */
