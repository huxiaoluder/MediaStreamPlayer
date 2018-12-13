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
#include <string>
#include "MSCodecProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
      
        typedef MSEncoderProtocol<AVFrame> FFEncoderProtocol;
        
        class FFEncoder : public FFEncoderProtocol {
            string filePath;
            bool _isEncoding;
            const AVCodecID vedioCodecID;
            const AVCodecID audioCodecID;
            
        public:
            void beginEncodeToFile(const string filePath);
            void encodeVideo(const MSEncoderInputData &pixelData);
            void encodeAudio(const MSEncoderInputData &sampleData);
            void endEncode();
            bool isEncoding();
            FFEncoder(const AVCodecID vedioCodecID,
                      const AVCodecID audioCodecID);
            ~FFEncoder();
        };
        
    }
}

#endif /* MSEncoder_hpp */
