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
            
            bool _isEncoding = false;
            
            const MSCodecID videoCodecID;
            const MSCodecID audioCodecID;

            AVFormatContext *outputFormatContext = nullptr;
            FFCodecContext  *videoEncoderContext = nullptr;
            FFCodecContext  *audioEncoderContext = nullptr;
            
            AVStream *videoStream = nullptr;
            AVStream *audioStream = nullptr;
            
            AVFormatContext * configureOutputFormatContext();
            FFCodecContext  * configureVideoEncoderContext(const FFCodecContext &videoDecoderContext);
            FFCodecContext  * configureAudioEncoderContext(const FFCodecContext &audioDecoderContext);
            
            void releaseEncoderConfiguration();
            
            void encodeData(AVFrame * const frame,
                            AVStream * const outStream,
                            AVCodecContext * const encoderContext);
            
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
                                  const FFCodecContext * const videoDecoderContext,
                                  const FFCodecContext * const audioDecoderContext);
        };
        
    }
}

#endif /* MSEncoder_hpp */
