//
//  MSCodecAsynProtocol.h
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/1/2.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSCodecAsynProtocol_h
#define MSCodecAsynProtocol_h

#include "MSMedia.hpp"

namespace MS {
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSEncoderProtocol {
    public:
        virtual ~MSEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const MSMedia<MSDecodeMedia,T> &pixelData) = 0;
        virtual void encodeAudio(const MSMedia<MSDecodeMedia,T> &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}

#endif /* MSCodecAsynProtocol_h */
