//
//  MSPlayer.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSPlayer_hpp
#define MSPlayer_hpp

#include <string>
#include <queue>
#include <cassert>
#include "MSType.hpp"
#include "MSTimer.hpp"
#include "MSCodecProtocol.h"

namespace MS {
    
    using namespace std;
    
    using namespace chrono;
    
    class MSPlayer {
        static MSData * const nullData;
        
        thread decodeThread;
        
        condition_variable condition;
        
        mutex conditionMutex;
        
        mutex videoMutex;
        
        mutex pixelMutex;
        
        mutex audioMutex;
        
        mutex sampleMutex;
        
        MSDecoderProtocol * const decoder;
        
        MSEncoderProtocol * const encoder;
        
        MSTimer * const timer = new MSTimer(microseconds(0),intervale(25),nullptr);
        
        queue<MSData *> * const videoQueue = new queue<MSData *>();
        
        queue<MSData *> * const pixelQueue = new queue<MSData *>();
        
        queue<MSData *> * const audioQueue = new queue<MSData *>();
        
        queue<MSData *> * const sampleQueue = new queue<MSData *>();
        
        bool decodeState = true;
        
        bool encodeState = false;
        
        ThrowData throwDecodeData;
        
        ThrowData throwEncodeData;
        
        void defaultInit();

        void clearAllData();
    public:
        MSPlayer();
        
        MSPlayer(MSDecoderProtocol * const decoder,
                 MSEncoderProtocol * const encoder);
        
        ~MSPlayer();
        
        void startPlay(const ThrowData throwDecodeData);
        
        void pausePlay();
        
        void continuePlay();
        
        void stopPlay();
        
        void startReEncode(const ThrowData throwEncodeData);
        
        void pauseReEncode();
        
        void continueReEncode();
        
        void stopReEncode();
        
        void pushVideoData(MSData *videoData);
        
        void pushAudioData(MSData *audioData);
    };
    
}

#endif /* MSPlayer_hpp */
