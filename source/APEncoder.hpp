//
//  APEncoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APEncoder_hpp
#define APEncoder_hpp

#include "MSCodecSyncProtocol.h"
#include "APCodecContext.hpp"

namespace MS {
    namespace APhard {
        
        typedef MSSyncEncoderProtocol<__CVBuffer> APEncoderProtocol;
        typedef MSMediaData<isDecode,__CVBuffer>::MSEncoderInputData    APEncoderInputData;
        typedef MSMediaData<isDecode,__CVBuffer>::MSEncoderInputContent APEncoderInputContent;

        class APEncoder : public APEncoderProtocol {
    
        public:
            void beginEncode();
            void encodeVideo(const APEncoderInputData &pixelData);
            void encodeAudio(const APEncoderInputData &sampleData);
            void endEncode();
            bool isEncoding();
            
            APEncoder(const MSCodecID videoCodecID,
                      const MSCodecID audioCodecID);
            ~APEncoder();
        };
        
    }
}

#endif /* APEncoder_hpp */
