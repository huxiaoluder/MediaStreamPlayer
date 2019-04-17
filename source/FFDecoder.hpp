//
//  MSDecoder.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSDecoder_hpp
#define MSDecoder_hpp

#include <map>
#include "MSDecoderProtocol.h"
#include "FFCodecContext.hpp"

namespace MS {
    namespace FFmpeg {
       
        typedef MSSyncDecoderProtocol<AVFrame>  FFDecoderProtocol;
        typedef MSMedia<MSDecodeMedia,AVFrame>  FFDecoderOutputMedia;
        
        class FFDecoder : public FFDecoderProtocol {
            MSCodecID currentVedioCodecID = MSCodecID_None;
            
            MSCodecID currentAudioCodecID = MSCodecID_None;
            
            std::map<MSCodecID,FFCodecContext *> decoderContexts;
            
            const FFCodecContext & getDecoderContext(const MSCodecID codecID);
            
            const FFDecoderOutputMedia * MSNullable decodeData(const MSMedia<MSEncodeMedia> * MSNonnull const mediaData);
        public:
            const FFDecoderOutputMedia * MSNullable decodeVideo(const MSMedia<MSEncodeMedia> * MSNonnull const videoData);
            const FFDecoderOutputMedia * MSNullable decodeAudio(const MSMedia<MSEncodeMedia> * MSNonnull const audioData);
            FFDecoder();
            ~FFDecoder();
            
            const FFCodecContext * MSNullable findDecoderContext(MSCodecID codecID);
            
            const FFCodecContext * MSNullable getCurrentVideoContext();
            
            const FFCodecContext * MSNullable getCurrentAudioContext();
        };
        
    }   
}

#endif /* MSDecoder_hpp */
