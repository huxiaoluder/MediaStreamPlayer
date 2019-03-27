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
#include "FFCodecContext.hpp"

namespace MS {
    namespace APhard {
        
        typedef MSAsynEncoderProtocol<APFrame>   APEncoderProtocol;
        typedef MSMedia<MSDecodeMedia,APFrame>   APEncoderInputMedia;

        class APEncoder : public APEncoderProtocol {
            
            typedef function<void(const uint8_t * MSNonnull const decodeData)> ThrowEncodeData;
            
            string filePath;
            
            bool _isEncoding = false;
            
            const MSCodecID videoCodecID;
            const MSCodecID audioCodecID;
            
            AVFormatContext       * MSNullable outputFormatContext = nullptr;
            VTCompressionSessionRef MSNullable videoEncoderSession = nullptr;
            AudioConverterRef       MSNullable audioEncoderConvert = nullptr;
            
            AVFormatContext      *  MSNullable configureOutputFormatContext();
            VTCompressionSessionRef MSNullable configureVideoEncoderSession(const MSVideoParameters &videoParameters);
            AudioConverterRef       MSNullable configureAudioEncoderConvert(const MSAudioParameters &audioParameters);
            
            void releaseEncoderConfiguration();
            
            const ThrowEncodeData throwEncodeVideo;
            const ThrowEncodeData throwEncodeAudio;
            
            static void compressionOutputCallback(void * MSNullable outputCallbackRefCon,
                                                  void * MSNullable sourceFrameRefCon,
                                                  OSStatus status,
                                                  VTEncodeInfoFlags infoFlags,
                                                  CMSampleBufferRef MSNullable sampleBuffer);
            
        public:
            void beginEncode();
            void encodeVideo(const APEncoderInputMedia &pixelData);
            void encodeAudio(const APEncoderInputMedia &sampleData);
            void endEncode();
            bool isEncoding();
            
            APEncoder(const MSCodecID videoCodecID,
                      const MSCodecID audioCodecID);
            ~APEncoder();
            
            bool configureEncoder(const string &muxingfilePath,
                                  const MSVideoParameters * MSNullable const videoParameters,
                                  const MSAudioParameters * MSNullable const audioParameters);
        };
        
    }
}

#endif /* APEncoder_hpp */
