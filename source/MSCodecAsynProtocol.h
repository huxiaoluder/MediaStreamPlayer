//
//  MSCodecAsynProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/1/2.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSCodecAsynProtocol_h
#define MSCodecAsynProtocol_h

#include "MSAsynDataProtocol.h"
#include "MSMediaData.hpp"

namespace MS {
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSAsynDecoderProtocol {
        // asyn call back function pointer, please convert to designated type for used
        const void * const _asynCallBack;
        MSAsynDataProtocol<T> &_asynDataReceiver;
    public:
        MSAsynDecoderProtocol(MSAsynDataProtocol<T> &asynDataHandle,const void * const asynCallBack)
        :_asynDataReceiver(asynDataHandle), _asynCallBack(asynCallBack) {};
        
        const void * asynCallBack() const { return _asynCallBack; };
        
        MSAsynDataProtocol<T> & asynDataReceiver() const { return _asynDataReceiver; };
        
        virtual ~MSAsynDecoderProtocol() {};
        virtual void decodeVideo(const MSMediaData<isEncode> &videoData) = 0;
        virtual void decodeAudio(const MSMediaData<isEncode> &audioData) = 0;
    };
    
    template <typename T,
    typename = typename enable_if<!is_pointer<T>::value,T>::type>
    class MSAsynEncoderProtocol {
    public:
        virtual ~MSAsynEncoderProtocol() {};
        virtual void beginEncode() = 0;
        virtual void encodeVideo(const typename MSMediaData<isDecode,T>::MSEncoderInputData &pixelData) = 0;
        virtual void encodeAudio(const typename MSMediaData<isDecode,T>::MSEncoderInputData &sampleData) = 0;
        virtual void endEncode() = 0;
        virtual bool isEncoding() = 0;
    };
    
}

#endif /* MSCodecAsynProtocol_h */
