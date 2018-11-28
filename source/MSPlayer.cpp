//
//  MSPlayer.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSPlayer.hpp"
#include "MSDecoder.hpp"
#include "MSEncoder.hpp"

using namespace MS;

using namespace this_thread;

MSData * const
MSPlayer::nullData = new MSData(nullptr,0,microseconds(0));

MSPlayer::MSPlayer()
:decoder(new MSDecoder()), encoder(new MSEncoder()) {
    defaultInit();
}

MSPlayer::MSPlayer(MSDecoderProtocol * const decoder,
                   MSEncoderProtocol * const encoder)
:decoder(decoder), encoder(encoder) {
    defaultInit();
}

void
MSPlayer::defaultInit() {
    decodeThread = thread([&](){
        MSData *sourceData = nullptr;
        MSData *frameData = nullptr;
        while (decodeState) {
            while (videoQueue->empty() || pixelQueue->size() > 20) {
                unique_lock<mutex> lock(conditionMutex);
                condition.wait(lock);
                if (!decodeState) break;
            }
            if (!decodeState) break;
            sourceData = videoQueue->front();
            while (!videoMutex.try_lock());
            videoQueue->pop();
            videoMutex.unlock();
            frameData = decoder->decodeVideo(*sourceData);
            if (frameData) {
                while (!pixelMutex.try_lock());
                pixelQueue->push(frameData);
                pixelMutex.unlock();
            }
            delete sourceData;
        }
    });
    
    timer->updateTask([&](){
        if (!pixelQueue->empty()) {
            MSData *frameData = nullptr;
            MSData *encodeData = nullptr;
            frameData = pixelQueue->front();
            while (!pixelMutex.try_lock());
            pixelQueue->pop();
            pixelMutex.unlock();
            if (pixelQueue->size() < 5) {
                condition.notify_one();
            }
            timer->updateTimeInterval(frameData->timeInterval);
            throwDecodeData(*frameData);
            if (encodeState) {
                encodeData = encoder->encodeVideo(*frameData);
                if (encodeData) {
                    throwEncodeData(*encodeData);
                    delete encodeData;
                }
            }
            delete frameData;
        } else {
            throwDecodeData(*nullData);
            condition.notify_one();
        }
    });
}

MSPlayer::~MSPlayer() {
    stopPlay();
    decodeState = false;
    condition.notify_one();
    if (decodeThread.joinable()) {
        decodeThread.join();
    }
    delete decoder;
    delete encoder;
    delete videoQueue;
    delete pixelQueue;
    delete timer;
}

void
MSPlayer::clearAllData() {
    MSData *data = nullptr;
    while (!videoQueue->empty()) {
        data = videoQueue->front();
        videoQueue->pop();
        delete data;
    }
    while (!pixelQueue->empty()) {
        data = pixelQueue->front();
        pixelQueue->pop();
        delete data;
    }
}

void
MSPlayer::startPlay(const ThrowData throwDecodeData) {
    stopPlay();
    assert(throwDecodeData);
    this->throwDecodeData = throwDecodeData;
    timer->start();
}

void
MSPlayer::pausePlay() {
    timer->pause();
}

void
MSPlayer::continuePlay() {
    assert(throwDecodeData);
    timer->_continue();
}

void
MSPlayer::stopPlay() {
    timer->stop();
    encodeState = false;
    clearAllData();
}

void
MSPlayer::startReEncode(const ThrowData throwEncodeData) {
    assert(throwEncodeData);
    this->throwEncodeData = throwEncodeData;
    encodeState = true;
}

void
MSPlayer::pauseReEncode() {
    encodeState = false;
}

void
MSPlayer::continueReEncode() {
    assert(throwEncodeData);
    encodeState = true;
}

void
MSPlayer::stopReEncode() {
    encodeState = false;
}

void
MSPlayer::pushVideoData(MSData *videoData) {
    while (!videoMutex.try_lock());
    videoQueue->push(videoData);
    videoMutex.unlock();
}

void
MSPlayer::pushAudioData(MSData *audioData) {
    while (!audioMutex.try_lock());
    audioQueue->push(audioData);
    audioMutex.unlock();
}
