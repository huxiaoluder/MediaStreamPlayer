//
//  MSCodecAsynProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/1/2.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSCodecAsynProtocol_h
#define MSCodecAsynProtocol_h

#include "MSAsynDataReceiver.h"
#include "MSMedia.hpp"

namespace MS {
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSAsynDecoderProtocol {
        // asyn call back function pointer, please convert to designated type for used
        const void * MSNonnull const _asynCallBack;
        const MSAsynDataReceiver<T> &asynDataReceiver;
    public:
        MSAsynDecoderProtocol(const MSAsynDataReceiver<T> &asynDataReceiver,
                              const void * MSNonnull const asynCallBack)
        :asynDataReceiver(asynDataReceiver), _asynCallBack(asynCallBack) {};
        
        const void * MSNonnull asynCallBack() const { return _asynCallBack; };
        
        void launchVideoFrameData(const MSMedia<isDecode,T> * const MSNonnull frameData) const {
            MSAsynDataReceiver<T> &receiver = const_cast<MSAsynDataReceiver<T> &>(asynDataReceiver);
            receiver.asynPushVideoFrameData(frameData);
        }
        
        void launchAudioFrameData(const MSMedia<isDecode,T> * const MSNonnull frameData) const {
            MSAsynDataReceiver<T> &receiver = const_cast<MSAsynDataReceiver<T> &>(asynDataReceiver);
            receiver.asynPushAudioFrameData(frameData);
        }
        
        virtual ~MSAsynDecoderProtocol() {};
        // videoData free by user
        virtual void decodeVideo(const MSMedia<isEncode> * MSNonnull const videoData) = 0;
        // audioData free by user
        virtual void decodeAudio(const MSMedia<isEncode> * MSNonnull const audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSAsynEncoderProtocol {
    public:
        virtual ~MSAsynEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const MSMedia<isDecode,T> &pixelData) = 0;
        virtual void encodeAudio(const MSMedia<isDecode,T> &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}

#endif /* MSCodecAsynProtocol_h */
