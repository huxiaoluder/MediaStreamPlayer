//
//  MSDecoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSDecoder_hpp
#define MSDecoder_hpp

#include <map>
#include "MSCodecSyncProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
       
        typedef MSSyncDecoderProtocol<AVFrame> FFDecoderProtocol;
        typedef MSMediaData<isDecode,AVFrame>::MSDecoderOutputData      FFDecoderOutputData;
        typedef MSMediaData<isDecode,AVFrame>::MSDecoderOutputContent   FFDecoderOutputContent;
        
        class FFDecoder : public FFDecoderProtocol {
            std::map<MSCodecID,FFCodecContext *> decoderContexts;
            
            const FFCodecContext & getDecoderContext(const MSCodecID codecID);
            
            const FFDecoderOutputData * decodeData(const MSMediaData<isEncode> &data);
        public:
            const FFDecoderOutputData * decodeVideo(const MSMediaData<isEncode> &videoData);
            const FFDecoderOutputData * decodeAudio(const MSMediaData<isEncode> &audioData);
            FFDecoder();
            ~FFDecoder();
            
            const FFCodecContext * findDecoderContext(MSCodecID codecID);
        };
        
    }   
}

#endif /* MSDecoder_hpp */
