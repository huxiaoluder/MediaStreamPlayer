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

#include "MSPlayer.hpp"
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
            
            AudioConverterRef           const _Nullable audioConvert;
            VTCompressionSessionRef     const _Nullable videoEncodeSession;
            VTDecompressionSessionRef   const _Nullable videoDecodeSession;
            
            MSPlayer<__CVBuffer> &player;
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           MSPlayer<__CVBuffer> &player);
            
            APCodecContext(const APCodecType codecType,
                           const MSCodecID codecID,
                           const MSBinaryData &spsData,
                           const MSBinaryData &ppsData,
                           MSPlayer<__CVBuffer> &player);
            
            ~APCodecContext();
            
            static APCodecInfo getAPCodecInfo(const MSCodecID codecID);
        private:
            // Note: don't allow copy with (APCodecContext &)obj
            APCodecContext(const APCodecContext &codecContext);
            
            AudioConverterRef           _Nullable   initAudioConvert();
            VTCompressionSessionRef     _Nullable   initVideoEncodeSession();
            VTDecompressionSessionRef   _Nullable   initVideoDecodeSession(const MSBinaryData &spsData, const MSBinaryData &ppsData);
            
            static void decompressionOutputCallback(void * _Nullable decompressionOutputRefCon,
                                                    void * _Nullable sourceFrameRefCon,
                                                    OSStatus status,
                                                    VTDecodeInfoFlags infoFlags,
                                                    CVImageBufferRef _Nullable imageBuffer,
                                                    CMTime presentationTimeStamp,
                                                    CMTime presentationDuration);
        };

    }
}
    
#endif /* APCodecContext_hpp */
