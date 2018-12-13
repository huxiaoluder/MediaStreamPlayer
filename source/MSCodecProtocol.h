//
//  MSCodecProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecProtocol_h
#define MSCodecProtocol_h

#include "MSMediaData.hpp"

#define intervale(rate) microseconds(1000000LL/rate)

namespace MS {
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSDecoderProtocol {
    public:
        typedef MSMediaData<isDecode,T> MSDecoderOutputData;
        typedef MSContent<isDecode,T> MSDecoderOutputContent;
        
        virtual ~MSDecoderProtocol() {};
        virtual MSDecoderOutputData * const decodeVideo(const MSMediaData<isEncode> &videoData) = 0;
        virtual MSDecoderOutputData * const decodeAudio(const MSMediaData<isEncode> &audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSEncoderProtocol {
    public:
        typedef MSMediaData<isDecode,T> MSEncoderInputData;
        typedef MSContent<isDecode,T> MSEncoderInputContent;
        
        virtual ~MSEncoderProtocol() {};
        virtual void beginEncodeToFile(const string filePath) = 0;
        virtual void encodeVideo(const MSEncoderInputData &pixelData) = 0;
        virtual void encodeAudio(const MSEncoderInputData &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}


#endif /* MSCodecProtocol_h */
