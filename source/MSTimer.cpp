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

MSTimer::MSTimer(microseconds delayTime, microseconds timeInterval, TaskType task)
:delayTime(delayTime), timeInterval(timeInterval), task(task) {

}

MSTimer::~MSTimer() {
    stop();
    if (timerThread.joinable()) {
        timerThread.join();
    }
    if (taskThread.joinable()) {
        taskThread.join();
    }
}

void MSTimer::start() {
    assert(isRun == false && task != nullptr);
    isRun = true;
    timerThread = thread([&]() {
        sleep_for(delayTime);
        taskThread = thread([&](){
            while (isRun) {
                task();
                unique_lock<mutex> lock(mtx);
                condition.wait(lock);
            }
        });
        condition.notify_one();
        while (isRun) {
            sleep_for(timeInterval);
            if (!isPause) {
                condition.notify_one();
            }
        }
    });
}

void MSTimer::pause() {
    isPause = true;
}

void MSTimer::_continue() {
    isPause = false;
}

void MSTimer::stop() {
    isRun = false;
    condition.notify_one();
}

MSTimer& MSTimer::updateTask(TaskType task) {
    this->task = task;
    return *this;
}

MSTimer& MSTimer::updateTimeInterval(microseconds timeInterval) {
    this->timeInterval = timeInterval;
    return *this;
}
