//
//  APDecoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APDecoder_hpp
#define APDecoder_hpp

#include <map>
#include "MSCodecAsynProtocol.h"
#include "APCodecContext.hpp"

namespace MS {
    namespace APhard {
        
        typedef MSAsynDecoderProtocol<__CVBuffer>   APDecoderProtocol;
        typedef MSAsynDataReceiver<__CVBuffer>      APAsynDataReceiver;
        typedef MSMedia<isDecode,__CVBuffer>        APDecoderOutputMeida;
        
        class APDecoder : public APDecoderProtocol {
            std::map<MSCodecID,APCodecContext *> decoderContexts;
            
            const VTDecodeFrameFlags decodeFlags;
            
            const CMBlockBufferFlags bufferFlags;
            
            const CFAllocatorRef MSNonnull blockAllocator;
            
            /**
             be related by decodeFlags,
             if decodeFlags == NULL, decoder must realloc memery for source data, avert double free crash.
             
             because decodeFlags != NULL, the decoder may process frames asynchronously,
             but source data has be freed already, so need copy a new memery.
             */
            CMBlockBufferFlags initBufferFlags();
            
            /**
             be related by bufferFlags,
             if bufferFlags == NULL, decoder do not realloc memery for source data, so needn't the allocator.
             */
            CFAllocatorRef MSNonnull initBlockAllocator();
            
            APCodecContext * MSNullable getDecoderContext(const MSCodecID codecID,
                                                          const MSMedia<isEncode> &sourceData);
            
        public:
            void decodeVideo(const MSMedia<isEncode> * MSNonnull const videoData);
            void decodeAudio(const MSMedia<isEncode> * MSNonnull const audioData);
            
            /**
             if decodeFlags == NULL, the decoder still decode source data synchronously.
             if decodeFlags != NULL, the decoder will decode source data asynchronously.
             */
            APDecoder(const VTDecodeFrameFlags decodeFlags = NULL);
            ~APDecoder();
            
            static void decompressionOutputCallback(void * MSNullable decompressionOutputRefCon,
                                                    void * MSNullable sourceFrameRefCon,
                                                    OSStatus status,
                                                    VTDecodeInfoFlags infoFlags,
                                                    CVImageBufferRef MSNullable imageBuffer,
                                                    CMTime presentationTimeStamp,
                                                    CMTime presentationDuration);
        };
        
    }
}


#endif /* APDecoder_hpp */
