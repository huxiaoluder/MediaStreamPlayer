//
//  APEncoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APEncoder_hpp
#define APEncoder_hpp

#include "MSCodecProtocol.h"
#include "APCodecContext.hpp"

namespace MS {
    namespace APhard {
        
        typedef MSEncoderProtocol<__CVBuffer> APEncoderProtocol;

        class APEncoder : public APEncoderProtocol {
    
        public:
            void beginEncode();
            void encodeVideo(const MSEncoderInputData &pixelData);
            void encodeAudio(const MSEncoderInputData &sampleData);
            void endEncode();
            bool isEncoding();
            
            APEncoder(const MSCodecID videoCodecID,
                      const MSCodecID audioCodecID);
            ~APEncoder();
        };
        
    }
}

#endif /* APEncoder_hpp */
