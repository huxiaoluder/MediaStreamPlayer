//
//  MSTimer.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/22.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSTimer.hpp"
#include <sys/time.h>

using namespace MS;

using namespace this_thread;

MSTimer<MSTimerForOther>::MSTimer(const microseconds delayTime,
                 const microseconds timeInterval,
                 const TaskType task)
:delayTime(delayTime), timeInterval(timeInterval), task(task) {
    if (timeInterval.count() == 0) {
        this->timeInterval = microseconds(1000000LL);
    }
}

MSTimer<MSTimerForOther>::~MSTimer() {
    stop();
}

void
MSTimer<MSTimerForOther>::start() {
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
MSTimer<MSTimerForOther>::pause() {
    assert(isRunning);
    isPausing = true;
}

void
MSTimer<MSTimerForOther>::rePlay() {
    assert(isRunning);
    isPausing = false;
}

void
MSTimer<MSTimerForOther>::stop() {
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

bool
MSTimer<MSTimerForOther>::isActivity() {
    return isRunning;
}

microseconds
MSTimer<MSTimerForOther>::getTimeInterval() {
    return this->timeInterval;
}

MSTimer<MSTimerForOther> &
MSTimer<MSTimerForOther>::updateTask(const TaskType task) {
    this->task = task;
    return *this;
}

MSTimer<MSTimerForOther> &
MSTimer<MSTimerForOther>::updateDelayTime(const microseconds delayTime) {
    this->delayTime = delayTime;
    return *this;
}

MSTimer<MSTimerForOther> &
MSTimer<MSTimerForOther>::updateTimeInterval(const microseconds timeInterval) {
    this->timeInterval = (timeInterval.count() ?
                          timeInterval :
                          microseconds(1000000LL));
    return *this;
}


MSTimer<MSTimerForApple>::MSTimer(const microseconds delayTime,
                                  const microseconds timeInterval,
                                  const TaskType task)
:delayTime(delayTime), timeInterval(timeInterval), task(task) {
    if (timeInterval.count() == 0) {
        this->timeInterval = microseconds(1000000LL);
    }
}

MSTimer<MSTimerForApple>::~MSTimer() {
    stop();
}

void
MSTimer<MSTimerForApple>::start() {
    assert(isRunning == false && task != nullptr);
    isRunning = true;
    timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(0, 0));
    dispatch_source_set_timer(timer, DISPATCH_TIME_NOW, timeInterval.count() * NSEC_PER_USEC, 0);
    dispatch_source_set_event_handler(timer, ^{
        task();
    });
    sleep_for(delayTime);
    dispatch_resume(timer);
}

void
MSTimer<MSTimerForApple>::pause() {
    assert(isRunning);
    isPausing = true;
    dispatch_suspend(timer);
}

void
MSTimer<MSTimerForApple>::rePlay() {
    assert(isRunning);
    isPausing = false;
    dispatch_resume(timer);
}

void
MSTimer<MSTimerForApple>::stop() {
    isRunning = false;
    isPausing = false;
    if (timer) {
        dispatch_source_cancel(timer);
        dispatch_release(timer);
        timer = NULL;
    }
}

bool
MSTimer<MSTimerForApple>::isActivity() {
    return isRunning;
}

microseconds
MSTimer<MSTimerForApple>::getTimeInterval() {
    return this->timeInterval;
}

MSTimer<MSTimerForApple> &
MSTimer<MSTimerForApple>::updateTask(const TaskType task) {
    this->task = task;
    return *this;
}

MSTimer<MSTimerForApple> &
MSTimer<MSTimerForApple>::updateDelayTime(const microseconds delayTime) {
    this->delayTime = delayTime;
    return *this;
}

MSTimer<MSTimerForApple> &
MSTimer<MSTimerForApple>::updateTimeInterval(const microseconds timeInterval) {
    if (timeInterval != this->timeInterval) {    
        this->timeInterval = (timeInterval.count() ?
                              timeInterval :
                              microseconds(1000000LL));
        dispatch_source_set_timer(timer, DISPATCH_TIME_NOW, timeInterval.count() * NSEC_PER_USEC, 0);
    }
    return *this;
}
