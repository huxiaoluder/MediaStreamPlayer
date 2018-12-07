//
//  MSTimer.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/22.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSTimer.hpp"

using namespace MS;

using namespace this_thread;

MSTimer::MSTimer(const microseconds delayTime,
                 const microseconds timeInterval,
                 const TaskType task)
:delayTime(delayTime), timeInterval(timeInterval), task(task) {

}

MSTimer::~MSTimer() {
    stop();
}

void
MSTimer::start() {
    assert(isRunning == false && task != nullptr);
    isRunning = true;
    timerThread = thread([&]() {
        sleep_for(delayTime);
        taskThread = thread([&](){
            while (isRunning) {
                task();
                unique_lock<mutex> lock(mtx);
                condition.wait(lock);
            }
        });
        condition.notify_one();
        while (isRunning) {
            sleep_for(timeInterval);
            if (!isPausing) {
                condition.notify_one();
            }
        }
    });
}

void
MSTimer::pause() {
    assert(isRunning);
    isPausing = true;
}

void
MSTimer::_continue() {
    assert(isRunning);
    isPausing = false;
}

void
MSTimer::stop() {
    isRunning = false;
    isPausing = false;
    condition.notify_one();
    if (timerThread.joinable()) {
        timerThread.join();
    }
    if (taskThread.joinable()) {
        taskThread.join();
    }
}

const bool
MSTimer::isValid() {
    return isRunning;
}

MSTimer &
MSTimer::updateTask(const TaskType task) {
    this->task = task;
    return *this;
}

MSTimer &
MSTimer::updateTimeInterval(const microseconds timeInterval) {
    this->timeInterval = timeInterval;
    return *this;
}
