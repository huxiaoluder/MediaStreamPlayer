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

#include "MSMediaData.hpp"
#include "MSBinaryData.hpp"

namespace MS {
    namespace APhard {
        
        typedef UInt32  APCodecID;
        typedef bool    IsVideoCodec;
        typedef tuple<APCodecID,IsVideoCodec> APCodecInfo;

        enum APCodecType {
            APCodecDecoder,
            APCodecEncoder,
        };
        
        struct APCodecContext {
            const APCodecType codecType;
            const MSCodecID codecID;
            
            AudioConverterRef           const audioConvert;
            VTDecompressionSessionRef   const videoDecodeSession;
            VTCompressionSessionRef     const videoEncodeSession;
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID);
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           const MSBinaryData &spsData,
                           const MSBinaryData &ppsData);
            
            ~APCodecContext();
            
            static APCodecInfo getAPCodecInfo(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (APCodecContext &)obj
            APCodecContext(const APCodecContext &codecContext);
            
            AudioConverterRef           initAudioConvert();
            VTDecompressionSessionRef   initVideoDecodeSession(const MSBinaryData &spsData, const MSBinaryData &ppsData);
            VTCompressionSessionRef     initVideoEncodeSession();
        };

    }
}
    
#endif /* APCodecContext_hpp */
