//
//  APCodecContext.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
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
#include "MSCodecAsynProtocol.h"

namespace MS {
    namespace APhard {
        
        typedef UInt32  APCodecID;
        typedef bool    IsVideoCodec;
        typedef tuple<APCodecID,IsVideoCodec> APCodecInfo;
        typedef MSAsynDecoderProtocol<__CVBuffer> APAsynDataProvider;

        enum APCodecType {
            APCodecDecoder,
            APCodecEncoder,
        };
        
        struct APCodecContext {
            const APCodecType codecType;
            const MSCodecID codecID;
            const APAsynDataProvider &asynDataProvider;
        private:
            CMVideoFormatDescriptionRef MSNullable videoFmtDescription;
        public:
            AudioConverterRef           const MSNullable audioConverter;
            VTDecompressionSessionRef   const MSNullable videoDecodeSession;
            VTCompressionSessionRef     const MSNullable videoEncodeSession;
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           const MSAudioParameters &audioParameters,
                           const APAsynDataProvider &asynDataProvider);
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           const MSNaluParts &naluParts,
                           const APAsynDataProvider &asynDataProvider);
            
            ~APCodecContext();
            
            void setVideoFmtDescription(const MSNaluParts &naluParts);
            
            CMVideoFormatDescriptionRef MSNullable getVideoFmtDescription() const;
            
            static APCodecInfo getAPCodecInfo(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (APCodecContext &)obj
            APCodecContext(const APCodecContext &codecContext);
            
            CMVideoFormatDescriptionRef MSNullable  initVideoFmtDescription(const MSNaluParts &naluParts);
            AudioConverterRef           MSNullable  initAudioConvert(const MSAudioParameters &audioParameters);
            VTCompressionSessionRef     MSNullable  initVideoEncodeSession();
            VTDecompressionSessionRef   MSNullable  initVideoDecodeSession();
        };

    }
}
    
#endif /* APCodecContext_hpp */
