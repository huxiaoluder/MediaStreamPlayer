//
//  MSAsynDataReceiver.h
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/1/2.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSAsynDataReceiver_h
#define MSAsynDataReceiver_h

#include "MSMedia.hpp"

namespace MS {
   
    template <typename T>
    class MSAsynDataReceiver {
    public:
        virtual ~MSAsynDataReceiver() {};
        virtual void asynPushVideoFrameData(const MSMedia<MSDecodeMedia, T> * const MSNonnull frameData) = 0;
        virtual void asynPushAudioFrameData(const MSMedia<MSDecodeMedia, T> * const MSNonnull frameData) = 0;
    };
    
}


#endif /* MSAsynDataReceiver_h */
