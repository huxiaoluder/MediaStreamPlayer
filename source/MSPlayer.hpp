//
//  MSPlayer
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
#include "MSTimer.hpp"
#include "MSCodecProtocol.h"

namespace MS {
    
    using namespace std;
    
    using namespace chrono;

#pragma mark - MSPlayer<T>(declaration)
    template <typename T>
    class MSPlayer {
        typedef function<void(const MSMediaData<isDecode,T> &decodeData)> ThrowDecodeData;
        
        typedef function<void(const MSMediaData<isEncode> &decodeData, bool isEnd)> ThrowEncodeData;
        
        MSDecoderProtocol<T> * const decoder;
        
        MSEncoderProtocol<T> * const encoder;
        
        MSTimer * const videoTimer = new MSTimer(microseconds(0),intervale(1),nullptr);
        
        MSTimer * const audioTimer = new MSTimer(microseconds(0),intervale(1),nullptr);
        
        thread videoDecodeThread;
        
        thread audioDecodeThread;
        
        condition_variable videoThreadCondition;
        
        condition_variable audioThreadCondition;
        
        mutex videoConditionMutex;
        
        mutex audioConditionMutex;
        
        mutex videoMutex;
        
        mutex pixelMutex;
        
        mutex audioMutex;
        
        mutex sampleMutex;
        
        queue<MSMediaData<isEncode> *> videoQueue;
        
        queue<MSMediaData<isEncode> *> audioQueue;
        
        queue<MSMediaData<isDecode,T> *> pixelQueue;
        
        queue<MSMediaData<isDecode,T> *> sampleQueue;
        
        bool isDecoding = true;
        
        bool isEncoding = false;
        
        const ThrowDecodeData throwDecodeVideo;
        
        const ThrowDecodeData throwDecodeAudio;
        
        const ThrowEncodeData throwEncodeData;
        
        void clearAllVideo();
        
        void clearAllAudio();
    public:
        MSPlayer(MSDecoderProtocol<T> * const decoder,
                 MSEncoderProtocol<T> * const encoder,
                 const ThrowDecodeData throwDecodeVideo,
                 const ThrowDecodeData throwDecodeAudio,
                 const ThrowEncodeData throwEncodeData);
        
        ~MSPlayer();
        
        void startPlayVideo();
        
        void pausePlayVideo();
        
        void continuePlayVideo();
        
        void stopPlayVideo();
        
        void startPlayAudio();

        void pausePlayAudio();

        void continuePlayAudio();

        void stopPlayAudio();
        
        void startReEncode();
        
        void pauseReEncode();
        
        void continueReEncode();
        
        void stopReEncode();
        
        void pushVideoData(MSMediaData<isEncode> *VideoData);
        
        void pushAudioData(MSMediaData<isEncode> *audioData);
    };
    
#pragma mark - MSPlayer<T>(implementation)
    template <typename T>
    MSPlayer<T>::MSPlayer(MSDecoderProtocol<T> * const decoder,
                          MSEncoderProtocol<T> * const encoder,
                          const ThrowDecodeData throwDecodeVideo,
                          const ThrowDecodeData throwDecodeAudio,
                          const ThrowEncodeData throwEncodeData)
    :decoder(decoder), encoder(encoder),
    throwDecodeVideo(throwDecodeVideo),
    throwDecodeAudio(throwDecodeAudio),
    throwEncodeData(throwEncodeData) {
        
        videoDecodeThread = thread([this](){
            MSMediaData<isEncode> *sourceData = nullptr;
            MSMediaData<isDecode,T> *frameData = nullptr;
            while (isDecoding) {
                while (videoQueue.empty() || pixelQueue.size() > 20) {
                    unique_lock<mutex> lock(videoConditionMutex);
                    videoThreadCondition.wait(lock);
                    if (!isDecoding) break;
                }
                if (!isDecoding) break;
                sourceData = videoQueue.front();
                while (!videoMutex.try_lock());
                videoQueue.pop();
                videoMutex.unlock();
                frameData = this->decoder->decodeVideo(*sourceData);
                if (frameData) {
                    while (!pixelMutex.try_lock());
                    pixelQueue.push(frameData);
                    pixelMutex.unlock();
                }
                delete sourceData;
            }
        });
        
        audioDecodeThread = thread([this](){
            MSMediaData<isEncode> *sourceData = nullptr;
            MSMediaData<isDecode,T> *frameData = nullptr;
            while (isDecoding) {
                while (audioQueue.empty() || sampleQueue.size() > 20) {
                    unique_lock<mutex> lock(audioConditionMutex);
                    audioThreadCondition.wait(lock);
                    if (!isDecoding) break;
                }
                if (!isDecoding) break;
                sourceData = audioQueue.front();
                while (!audioMutex.try_lock());
                audioQueue.pop();
                audioMutex.unlock();
                frameData = this->decoder->decodeAudio(*sourceData);
                if (frameData) {
                    while (!sampleMutex.try_lock());
                    sampleQueue.push(frameData);
                    sampleMutex.unlock();
                }
                delete sourceData;
            }
        });
        
        videoTimer->updateTask([this](){
            if (!pixelQueue.empty()) {
                MSMediaData<isDecode,T> *frameData = nullptr;
                MSMediaData<isEncode> *encodeData = nullptr;
                frameData = pixelQueue.front();
                while (!pixelMutex.try_lock());
                pixelQueue.pop();
                pixelMutex.unlock();
                if (pixelQueue.size() < 5) {
                    videoThreadCondition.notify_one();
                }
                videoTimer->updateTimeInterval(frameData->content->timeInterval);
                this->throwDecodeVideo(*frameData);
                if (isEncoding) {
                    encodeData = this->encoder->encodeVideo(*frameData);
                    if (encodeData) {
                        this->throwEncodeData(*encodeData,false);
                        delete encodeData;
                    }
                }
                delete frameData;
            } else {
                this->throwDecodeVideo(MSMediaData<isDecode,T>::defaultNullData);
                videoThreadCondition.notify_one();
            }
        });
        
        audioTimer->updateTask([this](){
            if (!sampleQueue.empty()) {
                MSMediaData<isDecode,T> *frameData = nullptr;
                MSMediaData<isEncode> *encodeData = nullptr;
                frameData = sampleQueue.front();
                while (!sampleMutex.try_lock());
                sampleQueue.pop();
                sampleMutex.unlock();
                if (sampleQueue.size() < 5) {
                    audioThreadCondition.notify_one();
                }
                audioTimer->updateTimeInterval(frameData->content->timeInterval);
                this->throwDecodeAudio(*frameData);
                if (isEncoding) {
                    encodeData = this->encoder->encodeAudio(*frameData);
                    if (encodeData) {
                        this->throwEncodeData(*encodeData,false);
                        delete encodeData;
                    }
                }
                delete frameData;
            } else {
                this->throwDecodeAudio(MSMediaData<isDecode,T>::defaultNullData);
                audioThreadCondition.notify_one();
            }
        });
    }
    
    template <typename T>
    MSPlayer<T>::~MSPlayer() {
        stopPlayVideo();
        stopPlayAudio();
        isDecoding = false;
        videoThreadCondition.notify_one();
        if (videoDecodeThread.joinable()) {
            videoDecodeThread.join();
        }
        audioThreadCondition.notify_one();
        if (audioDecodeThread.joinable()) {
            audioDecodeThread.join();
        }
        delete decoder;
        delete encoder;
        delete videoTimer;
    }
    
    template <typename T>
    void MSPlayer<T>::clearAllVideo() {
        MSMediaData<isEncode> *encodeData = nullptr;
        MSMediaData<isDecode,T> *decodeData = nullptr;
        while (!videoQueue.empty()) {
            encodeData = videoQueue.front();
            videoQueue.pop();
            delete encodeData;
        }
        while (!pixelQueue.empty()) {
            decodeData = pixelQueue.front();
            pixelQueue.pop();
            delete decodeData;
        }
    }
    
    template <typename T>
    void MSPlayer<T>::clearAllAudio() {
        MSMediaData<isEncode> *encodeData = nullptr;
        MSMediaData<isDecode,T> *decodeData = nullptr;
        while (!audioQueue.empty()) {
            encodeData = audioQueue.front();
            audioQueue.pop();
            delete encodeData;
        }
        while (!sampleQueue.empty()) {
            decodeData = sampleQueue.front();
            sampleQueue.pop();
            delete decodeData;
        }
    }
    
    template <typename T>
    void MSPlayer<T>::startPlayVideo() {
        assert(videoTimer->isValid() == false);
        videoTimer->start();
    }
    
    template <typename T>
    void MSPlayer<T>::pausePlayVideo() {
        videoTimer->pause();
    }
    
    template <typename T>
    void MSPlayer<T>::continuePlayVideo() {
        videoTimer->_continue();
    }
    
    template <typename T>
    void MSPlayer<T>::stopPlayVideo() {
        videoTimer->stop();
        isEncoding = false;
        clearAllVideo();
    }
    
    template <typename T>
    void MSPlayer<T>::startPlayAudio() {
        assert(audioTimer->isValid() == false);
        audioTimer->start();
    }
    
    template <typename T>
    void MSPlayer<T>::pausePlayAudio() {
        audioTimer->pause();
    }
    
    template <typename T>
    void MSPlayer<T>::continuePlayAudio() {
        audioTimer->_continue();
    }
    
    template <typename T>
    void MSPlayer<T>::stopPlayAudio() {
        audioTimer->stop();
        isEncoding = false;
        clearAllAudio();
    }
    
    template <typename T>
    void MSPlayer<T>::startReEncode() {
        assert(isEncoding == false);
        isEncoding = true;
    }
    
    template <typename T>
    void MSPlayer<T>::pauseReEncode() {
        isEncoding = false;
    }
    
    template <typename T>
    void MSPlayer<T>::continueReEncode() {
        isEncoding = true;
    }
    
    template <typename T>
    void MSPlayer<T>::stopReEncode() {
        isEncoding = false;
        throwEncodeData(MSMediaData<isEncode>::defaultNullData,true);
        throwEncodeData = nullptr;
    }
    
    template <typename T>
    void MSPlayer<T>::pushVideoData(MSMediaData<isEncode> *VideoData) {
        if (videoTimer->isValid()) {
            while (!videoMutex.try_lock());
            videoQueue.push(VideoData);
            videoMutex.unlock();
        } else {
            delete VideoData;
        }
    }
    
    template <typename T>
    void MSPlayer<T>::pushAudioData(MSMediaData<isEncode> *audioData) {
        if (videoTimer->isValid()) {
            while (!audioMutex.try_lock());
            audioQueue.push(audioData);
            audioMutex.unlock();
        } else {
            delete audioData;
        }
    }
    
}

#endif /* MSPlayer_hpp */
