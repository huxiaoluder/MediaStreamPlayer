//
//  MSCodecSyncProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecSyncProtocol_h
#define MSCodecSyncProtocol_h

#include "MSMediaData.hpp"

namespace MS {
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSSyncDecoderProtocol {
    public:
        virtual ~MSSyncDecoderProtocol() {};
        virtual const typename MSMediaData<isDecode,T>::MSDecoderOutputData * decodeVideo(const MSMediaData<isEncode> &videoData) = 0;
        virtual const typename MSMediaData<isDecode,T>::MSDecoderOutputData * decodeAudio(const MSMediaData<isEncode> &audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSSyncEncoderProtocol {
    public:
        virtual ~MSSyncEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const typename MSMediaData<isDecode,T>::MSEncoderInputData &pixelData) = 0;
        virtual void encodeAudio(const typename MSMediaData<isDecode,T>::MSEncoderInputData &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}


#endif /* MSCodecSyncProtocol_h */
