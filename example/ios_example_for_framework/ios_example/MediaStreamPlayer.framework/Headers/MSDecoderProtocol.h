//
//  MSDecoderProtocol.h
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSDecoderProtocol_h
#define MSDecoderProtocol_h

#include "MSMedia.hpp"
#include "MSAsynDataReceiver.h"

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
        
        // if unnecessary, not allow be called by user
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
    
}


#endif /* MSDecoderProtocol_h */
