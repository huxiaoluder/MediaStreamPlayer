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
#include "MSCodecProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
      
        typedef MSEncoderProtocol<AVFrame> FFEncoderProtocol;
        
        class FFEncoder : public FFEncoderProtocol {
            const AVCodecID vedioCodecID;
            const AVCodecID audioCodecID;
            
        public:
            MSMediaData<isEncode> * const encodeVideo(const MSInputData &pixelData);
            MSMediaData<isEncode> * const encodeAudio(const MSInputData &sampleData);
            FFEncoder(const AVCodecID vedioCodecID, const AVCodecID audioCodecID);
            ~FFEncoder();
        };
        
    }
}

#endif /* MSEncoder_hpp */
