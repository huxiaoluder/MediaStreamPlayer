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
       
        typedef MSSyncDecoderProtocol<AVFrame>  FFDecoderProtocol;
        typedef MSMedia<MSDecodeMedia,AVFrame>  FFDecoderOutputMedia;
        
        class FFDecoder : public FFDecoderProtocol {
            std::map<MSCodecID,FFCodecContext *> decoderContexts;
            
            const FFCodecContext & getDecoderContext(const MSCodecID codecID);
            
            const FFDecoderOutputMedia * MSNullable decodeData(const MSMedia<MSEncodeMedia> * MSNonnull const mediaData);
        public:
            const FFDecoderOutputMedia * MSNullable decodeVideo(const MSMedia<MSEncodeMedia> * MSNonnull const videoData);
            const FFDecoderOutputMedia * MSNullable decodeAudio(const MSMedia<MSEncodeMedia> * MSNonnull const audioData);
            FFDecoder();
            ~FFDecoder();
            
            const FFCodecContext * MSNullable findDecoderContext(MSCodecID codecID);
        };
        
    }   
}

#endif /* MSDecoder_hpp */
