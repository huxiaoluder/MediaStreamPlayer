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

void MSPlayer::defaultInit() {
    decodeThread = thread([&](){
        MSData *sourceData = nullptr;
        MSData *frameData = nullptr;
        while (decodeState) {
            while (sourceDataQueue->empty() || frameDataQueue->size() > 20) {
                unique_lock<mutex> lock(conditionMutex);
                condition.wait(lock);
                if (!decodeState) break;
            }
            if (!decodeState) break;
            printf("解码\n");
            sourceData = sourceDataQueue->front();
            while (!sourceMutex.try_lock());
            sourceDataQueue->pop();
            sourceMutex.unlock();
            frameData = decoder->decode(*sourceData);
            if (frameData) {
                while (!frameMutex.try_lock());
                frameDataQueue->push(frameData);
                frameMutex.unlock();
            }
            delete sourceData;
        }
    });
    
    timer->updateTask([&](){
        if (!frameDataQueue->empty()) {
            MSData *frameData = nullptr;
            MSData *encodeData = nullptr;
            frameData = frameDataQueue->front();
            while (!frameMutex.try_lock());
            frameDataQueue->pop();
            frameMutex.unlock();
            if (frameDataQueue->size() < 5) {
                condition.notify_one();
            }
            timer->updateTimeInterval(frameData->timeInterval);
            throwDecodeData(*frameData);
            if (encodeState) {
                encodeData = encoder->encode(*frameData);
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
    delete sourceDataQueue;
    delete frameDataQueue;
    delete timer;
}

void MSPlayer::clearAllData() {
    MSData *data = nullptr;
    while (!sourceDataQueue->empty()) {
        data = sourceDataQueue->front();
        sourceDataQueue->pop();
        delete data;
    }
    while (!frameDataQueue->empty()) {
        data = frameDataQueue->front();
        frameDataQueue->pop();
        delete data;
    }
}

void MSPlayer::pushSourceData(MSData *sourceData) {
    while (!sourceMutex.try_lock());
    sourceDataQueue->push(sourceData);
    sourceMutex.unlock();
}

void MSPlayer::startPlay(const ThrowData throwDecodeData) {
    stopPlay();
    assert(throwDecodeData);
    this->throwDecodeData = throwDecodeData;
    timer->start();
}

void MSPlayer::pausePlay() {
    timer->pause();
}

void MSPlayer::continuePlay() {
    assert(throwDecodeData);
    timer->_continue();
}

void MSPlayer::stopPlay() {
    timer->stop();
    encodeState = false;
    clearAllData();
}

void MSPlayer::startReEncode(const ThrowData throwEncodeData) {
    assert(throwEncodeData);
    this->throwEncodeData = throwEncodeData;
    encodeState = true;
}

void MSPlayer::pauseReEncode() {
    encodeState = false;
}

void MSPlayer::continueReEncode() {
    assert(throwEncodeData);
    encodeState = true;
}

void MSPlayer::stopReEncode() {
    encodeState = false;
}
