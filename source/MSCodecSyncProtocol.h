//
//  MSCodecSyncProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSCodecSyncProtocol_h
#define MSCodecSyncProtocol_h

#include "MSMedia.hpp"

namespace MS {
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSSyncDecoderProtocol {
    public:
        virtual ~MSSyncDecoderProtocol() {};
        virtual const typename MSMedia<isDecode,T>::MSDecoderOutputMedia * MSNullable decodeVideo(const MSMedia<isEncode> * MSNonnull const videoData) = 0;
        virtual const typename MSMedia<isDecode,T>::MSDecoderOutputMedia * MSNullable decodeAudio(const MSMedia<isEncode> * MSNonnull const audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSSyncEncoderProtocol {
    public:
        virtual ~MSSyncEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const typename MSMedia<isDecode,T>::MSEncoderInputMedia &pixelData) = 0;
        virtual void encodeAudio(const typename MSMedia<isDecode,T>::MSEncoderInputMedia &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}


#endif /* MSCodecSyncProtocol_h */
