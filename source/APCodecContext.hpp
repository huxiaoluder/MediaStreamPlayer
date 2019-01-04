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
#include "MSCodecAsynProtocol.h"

namespace MS {
    namespace APhard {
        
        typedef UInt32  APCodecID;
        typedef bool    IsVideoCodec;
        typedef tuple<APCodecID,IsVideoCodec> APCodecInfo;
        typedef MSAsynDecoderProtocol<__CVBuffer> APOutputDataSender;

        enum APCodecType {
            APCodecDecoder,
            APCodecEncoder,
        };
        
        struct APCodecContext {
            const APCodecType codecType;
            const MSCodecID codecID;
            const APOutputDataSender &asynDataSender;
            
            AudioConverterRef           const _Nullable audioConvert;
            VTCompressionSessionRef     const _Nullable videoEncodeSession;
            VTDecompressionSessionRef   const _Nullable videoDecodeSession;
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           const APOutputDataSender &asynDataSender);
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           const MSBinaryData &spsData,
                           const MSBinaryData &ppsData,
                           const APOutputDataSender &asynDataSender);
            
            ~APCodecContext();
            
            static APCodecInfo getAPCodecInfo(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (APCodecContext &)obj
            APCodecContext(const APCodecContext &codecContext);
            
            AudioConverterRef           _Nullable   initAudioConvert();
            VTCompressionSessionRef     _Nullable   initVideoEncodeSession();
            VTDecompressionSessionRef   _Nullable   initVideoDecodeSession(const MSBinaryData &spsData, const MSBinaryData &ppsData);
        };

    }
}
    
#endif /* APCodecContext_hpp */
