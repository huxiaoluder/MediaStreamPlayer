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
        typedef MSAsynDataProtocol<__CVBuffer>      APAsynDataProtocol;
        typedef MSMediaData<isDecode,__CVBuffer>::MSDecoderOutputData       APDecoderOutputData;
        typedef MSMediaData<isDecode,__CVBuffer>::MSDecoderOutputContent    APDecoderOutputContent;
        
        class APDecoder : public APDecoderProtocol {
            std::map<MSCodecID,APCodecContext *> decoderContexts;
            
            const APCodecContext & getDecoderContext(const MSCodecID codecID,
                                                     const MSBinaryData &spsData,
                                                     const MSBinaryData &ppsData);
            
        public:
            void decodeVideo(const MSMediaData<isEncode> &videoData);
            void decodeAudio(const MSMediaData<isEncode> &audioData);
            
            APDecoder(MSAsynDataProtocol<__CVBuffer> &asynDataHandle);
            ~APDecoder();
            
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


#endif /* APDecoder_hpp */
