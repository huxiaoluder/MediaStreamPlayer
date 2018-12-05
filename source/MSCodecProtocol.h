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

#define intervale(rate) microseconds(1000000LL/rate)

namespace MS {
    
    template <typename T>
    class MSDecoderProtocol {
    public:
        typedef MSMediaData<isDecode,T> MSOutputData;
        typedef MSContent<isDecode,T> MSOutputContent;
        
        virtual ~MSDecoderProtocol() {};
        virtual MSOutputData * const decodeVideo(const MSMediaData<isEncode> &videoData) = 0;
        virtual MSOutputData * const decodeAudio(const MSMediaData<isEncode> &audioData) = 0;
    };
    
    template <typename T>
    class MSEncoderProtocol {
    public:
        typedef MSMediaData<isDecode,T> MSInputData;
        typedef MSContent<isDecode,T> MSInputContent;
        
        virtual ~MSEncoderProtocol() {};
        virtual MSMediaData<isEncode> * const encodeVideo(const MSInputData &pixelData) = 0;
        virtual MSMediaData<isEncode> * const encodeAudio(const MSInputData &sampleData) = 0;
    };
    
}


#endif /* MSCodecProtocol_h */
