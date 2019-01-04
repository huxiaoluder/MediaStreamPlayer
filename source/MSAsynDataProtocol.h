//
//  MSAsynDataProtocol.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/1/2.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSAsynDataProtocol_h
#define MSAsynDataProtocol_h

#include "MSMediaData.hpp"

namespace MS {
   
    template <typename T>
    class MSAsynDataProtocol {
    public:
        virtual ~MSAsynDataProtocol() {};
        virtual void asynPushVideoFrameData(const MSMediaData<isDecode, T> * const frameData) = 0;
        virtual void asynPushAudioFrameData(const MSMediaData<isDecode, T> * const frameData) = 0;
    };
    
}


#endif /* MSAsynDataProtocol_h */
