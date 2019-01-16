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
#include <mutex>
#include "MSCodecSyncProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
      
        typedef MSSyncEncoderProtocol<AVFrame> FFEncoderProtocol;
        typedef MSMedia<isDecode,AVFrame>::MSEncoderInputMedia FFEncoderInputMedia;
        
        class FFEncoder : public FFEncoderProtocol {
            
            mutex fileWriteMutex;
            
            string filePath;
            
            bool _isEncoding = false;
            
            const MSCodecID videoCodecID;
            const MSCodecID audioCodecID;
            
            long long videoPts = 0;
            long long audioPts = 0;
            
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
            void encodeVideo(const FFEncoderInputMedia &pixelData);
            void encodeAudio(const FFEncoderInputMedia &sampleData);
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
