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
#include <Accelerate/Accelerate.h>
#pragma clang diagnostic pop

#include "MSMedia.hpp"
#include "MSBinary.hpp"
#include "MSDecoderProtocol.h"

namespace MS {
    namespace APhard {
        
        /**
         重新排列 CGImage 中的 ARGB 顺序

         @param image 源图片
         @param bitmapInfo 排列方式
         @return 新图片(free by caller)
         */
        CGImageRef MSNonnull CGImageCreateByPermuteARGBBitmap(CGImageRef MSNonnull const image,
                                                              CGBitmapInfo const bitmapInfo);
        
        struct APYUV420PTexture {
            vImage_Buffer Yp;
            vImage_Buffer Cb;
            vImage_Buffer Cr;
            
            /**
             以 ITU_R_709_2 标准, 转换 ARGB 像素为 full range 420Yp_Cb_Cr
             */
            APYUV420PTexture(CGImageRef MSNonnull image);
            
            ~APYUV420PTexture();
        };
        
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
            
            const MSCodecID codecID;
            
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
                           const bool isColorFullRange,
                           const MSNaluParts &naluParts,
                           const APAsynDataProvider &asynDataProvider);
            
            ~APCodecContext();
            
            /**
             当前解码器是否支持解码新的视频数据(参数变动太大, 将解码失败, 需要重新初始化新的解码器)
             */
            bool canAcceptNewFormatDescription(const MSNaluParts &naluParts);
            
            CMVideoFormatDescriptionRef MSNullable getVideoFmtDescription() const;
            
            static APCodecID getAPCodecId(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (APCodecContext &)obj
            APCodecContext(const APCodecContext &codecContext);
            
            CMVideoFormatDescriptionRef MSNullable  initVideoFmtDescription(const MSNaluParts &naluParts);
            AudioConverterRef           MSNullable  initAudioConvert(const MSAudioParameters &audioParameters);
            VTDecompressionSessionRef   MSNullable  initVideoDecoderSession(const bool isColorFullRange);
        };

    }
}
    
#endif /* APCodecContext_hpp */
