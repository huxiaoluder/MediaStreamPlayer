//
//  APEncoder.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APEncoder_hpp
#define APEncoder_hpp

#include <string>
#include "MSCodecAsynProtocol.h"
#include "APCodecContext.hpp"

namespace MS {
    namespace APhard {
        
        typedef MSAsynEncoderProtocol<__CVBuffer>   APEncoderProtocol;
        typedef MSMedia<MSDecodeMedia,__CVBuffer>   APEncoderInputMedia;

        class APEncoder : public APEncoderProtocol {
            
            typedef function<void(const uint8_t &decodeData)> ThrowEncodeData;
            
            bool _isEncoding = false;
            
            const MSCodecID videoCodecID;
            const MSCodecID audioCodecID;
            
            APCodecContext *videoEncoderContext = nullptr;
            APCodecContext *audioEncoderContext = nullptr;
            
            APCodecContext * configureVideoEncoderContext(const APCodecContext &videoDecoderContext);
            APCodecContext * configureAudioEncoderContext(const APCodecContext &audioDecoderContext);
            
            const ThrowEncodeData throwEncodeVideo;
            const ThrowEncodeData throwEncodeAudio;
            
        public:
            void beginEncode();
            void encodeVideo(const APEncoderInputMedia &pixelData);
            void encodeAudio(const APEncoderInputMedia &sampleData);
            void endEncode();
            bool isEncoding();
            
            APEncoder(const MSCodecID videoCodecID,
                      const MSCodecID audioCodecID);
            ~APEncoder();
            
            bool configureEncoder(const APCodecContext * const videoDecoderContext,
                                  const APCodecContext * const audioDecoderContext);
        };
        
    }
}

#endif /* APEncoder_hpp */
