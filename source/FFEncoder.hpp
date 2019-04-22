//
//  MSEncoder.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSEncoder_hpp
#define MSEncoder_hpp

#include <string>
#include <mutex>
#include "MSEncoderProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
      
        typedef MSEncoderProtocol<AVFrame>  FFEncoderProtocol;
        typedef MSMedia<MSDecodeMedia,AVFrame>  FFEncoderInputMedia;
        
        class FFEncoder : public FFEncoderProtocol {
            
            mutex fileWriteMutex;
            
            string filePath;
            
            bool _isEncoding = false;
            
            const MSCodecID videoCodecID;
            const MSCodecID audioCodecID;
            
            long long videoPts;
            long long audioPts;
            
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
            
            // 暂不支持自定义编码方式(因为还没有将 MP4 muxer 拆解出来)
//            FFEncoder(const MSCodecID videoCodecID,
//                      const MSCodecID audioCodecID);
            FFEncoder();
            ~FFEncoder();
            
            bool configureEncoder(const string &muxingfilePath,
                                  const FFCodecContext * const videoDecoderContext,
                                  const FFCodecContext * const audioDecoderContext);
        };
        
    }
}

#endif /* MSEncoder_hpp */
