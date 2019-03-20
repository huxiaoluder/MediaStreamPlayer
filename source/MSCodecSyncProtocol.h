//
//  MSCodecSyncProtocol.h
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/14.
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
        // videoData free by heir
        virtual const MSMedia<MSDecodeMedia,T> * MSNullable decodeVideo(const MSMedia<MSEncodeMedia> * MSNonnull const videoData) = 0;
        // audioData free by heir
        virtual const MSMedia<MSDecodeMedia,T> * MSNullable decodeAudio(const MSMedia<MSEncodeMedia> * MSNonnull const audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSSyncEncoderProtocol {
    public:
        virtual ~MSSyncEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const MSMedia<MSDecodeMedia,T> &pixelData) = 0;
        virtual void encodeAudio(const MSMedia<MSDecodeMedia,T> &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}


#endif /* MSCodecSyncProtocol_h */
