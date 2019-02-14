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
        
        // asyn data receiver
        // the default value set by MSAsynDataReceiver(player) when MSAsynDecoderProtocol(decoder) pass for MSAsynDataReceiver(player)
        MSAsynDataReceiver<T> * MSNonnull _dataReceiver;
    public:
        MSAsynDecoderProtocol(const void * MSNonnull const asynCallBack)
        :_asynCallBack(asynCallBack) {};
        
        const void * MSNonnull asynCallBack() const {
            return _asynCallBack;
        };
        
        // if unnecessary, not allow be called bu user 
        // default be called by MSAsynDataReceiver(player) inner when MSAsynDecoderProtocol(decoder) pass for MSAsynDataReceiver(player)
        void setDataReceiver(MSAsynDataReceiver<T> * MSNonnull dataReceiver) {
            _dataReceiver = dataReceiver;
        };
        
        void launchVideoFrameData(const MSMedia<MSDecodeMedia,T> * const MSNonnull frameData) const {
            assert(_dataReceiver);
            _dataReceiver->asynPushVideoFrameData(frameData);
        }
        
        void launchAudioFrameData(const MSMedia<MSDecodeMedia,T> * const MSNonnull frameData) const {
            assert(_dataReceiver);
            _dataReceiver->asynPushAudioFrameData(frameData);
        }
        
        virtual ~MSAsynDecoderProtocol() {};
        // videoData free by user
        virtual void decodeVideo(const MSMedia<MSEncodeMedia> * MSNonnull const videoData) = 0;
        // audioData free by user
        virtual void decodeAudio(const MSMedia<MSEncodeMedia> * MSNonnull const audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSAsynEncoderProtocol {
    public:
        virtual ~MSAsynEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const MSMedia<MSDecodeMedia,T> &pixelData) = 0;
        virtual void encodeAudio(const MSMedia<MSDecodeMedia,T> &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}

#endif /* MSCodecAsynProtocol_h */
