//
//  MSCodecProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecProtocol_h
#define MSCodecProtocol_h

#include "MSMediaData"

namespace MS {
    
    template <typename T>
    class MSDecoderProtocol {
    public:
        typedef MSMediaData<isDecode,T> outputType;
        virtual ~MSDecoderProtocol() {};
        
        virtual outputType * const decodeVideo(const MSMediaData<isEncode> &videoData) = 0;
        virtual outputType * const decodeAudio(const MSMediaData<isEncode> &audioData) = 0;
    };
    
    template <typename T>
    class MSEncoderProtocol {
    public:
        typedef MSMediaData<isDecode,T> inputType;
        virtual ~MSEncoderProtocol() {};
        
        virtual MSMediaData<isEncode> * const encodeVideo(const inputType &pixelData) = 0;
        virtual MSMediaData<isEncode> * const encodeAudio(const inputType &sampleData) = 0;
    };
    
}


#endif /* MSCodecProtocol_h */
