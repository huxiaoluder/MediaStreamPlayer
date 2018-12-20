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
            
            const MSCodecID videoCodecID;
            const MSCodecID audioCodecID;

            AVFormatContext *outputFormatContext;
            FFCodecContext  *videoEncoderContext;
            FFCodecContext  *audioEncoderContext;
            
            AVFormatContext * configureOutputFormatContext();
            FFCodecContext  * configureVideoEncoderContext(const FFCodecContext &videoDecoderContext);
            FFCodecContext  * configureAudioEncoderContext(const FFCodecContext &audioDecoderContext);
            
        public:
            void beginEncode();
            void encodeVideo(const MSEncoderInputData &pixelData);
            void encodeAudio(const MSEncoderInputData &sampleData);
            void endEncode();
            bool isEncoding();
            
            FFEncoder(const MSCodecID videoCodecID,
                      const MSCodecID audioCodecID);
            ~FFEncoder();
            
            bool configureEncoder(const string muxingfilePath,
                                  const FFCodecContext &videoDecoderContext,
                                  const FFCodecContext &audioDecoderContext);
        };
        
    }
}

#endif /* MSEncoder_hpp */
