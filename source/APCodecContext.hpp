//
//  APCodecContext.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APCodecContext_hpp
#define APCodecContext_hpp

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
extern "C" {
#include <MediaToolbox/MediaToolbox.h>
#include <VideoToolbox/VideoToolbox.h>
#include <AudioToolbox/AudioToolbox.h>
}
#pragma clang diagnostic pop

#include "MSMedia.hpp"
#include "MSBinary.hpp"
#include "MSDecoderProtocol.h"

namespace MS {
    namespace APhard {
        
        struct APFrame {
            union {
                __CVBuffer  * MSNonnull const video;
                AudioBuffer * MSNonnull const audio;
            };
            
            union {
                MSVideoParameters const videoParameters;
                MSAudioParameters const audioParameters;
            };
            
            APFrame(__CVBuffer  * MSNonnull const video, const MSVideoParameters &videoParameters);

            APFrame(AudioBuffer * MSNonnull const audio, const MSAudioParameters &audioParameters);
            
            static void freeVideoFrame(const APFrame * MSNonnull const frame);
            
            static void freeAudioFrame(const APFrame * MSNonnull const frame);
            
            static APFrame * MSNonnull copyVideoFrame(const APFrame * MSNonnull const frame);
            
            static APFrame * MSNonnull copyAudioFrame(const APFrame * MSNonnull const frame);
        };
        
        typedef UInt32 APCodecID;
        
        typedef MSAsynDecoderProtocol<APFrame>  APAsynDataProvider;
        
        /*
         用于封装 APPLE 音视频解码器, 因为 APPLE 的密封性, 这里不对编码器进行封装
         APEncoder 中自行对 APPLE 编码器进行初始化和封装
         */
        struct APCodecContext {
            
            const MSCodecID     codecID;
            
            const APAsynDataProvider &asynDataProvider;
            
        private:
            CMVideoFormatDescriptionRef MSNullable videoFmtDescription;
            
        public:
            union {            
                AudioConverterRef         const MSNonnull audioDecoderConvert;
                VTDecompressionSessionRef const MSNonnull videoDecoderSession;
            };
            
            /*
             音频解码环境构造器
             */
            APCodecContext(const MSCodecID codecID,
                           const MSAudioParameters &audioParameters,
                           const APAsynDataProvider &asynDataProvider);
            
            /*
             视频解码环境构造器
             */
            APCodecContext(const MSCodecID codecID,
                           const MSNaluParts &naluParts,
                           const APAsynDataProvider &asynDataProvider);
            
            ~APCodecContext();
            
            void setVideoFmtDescription(const MSNaluParts &naluParts);
            
            CMVideoFormatDescriptionRef MSNullable getVideoFmtDescription() const;
            
            static APCodecID getAPCodecId(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (APCodecContext &)obj
            APCodecContext(const APCodecContext &codecContext);
            
            CMVideoFormatDescriptionRef MSNullable  initVideoFmtDescription(const MSNaluParts &naluParts);
            AudioConverterRef           MSNullable  initAudioConvert(const MSAudioParameters &audioParameters);
            VTDecompressionSessionRef   MSNullable  initVideoDecoderSession();
        };

    }
}
    
#endif /* APCodecContext_hpp */
