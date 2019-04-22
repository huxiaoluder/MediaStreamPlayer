//
//  MSTimer.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/22.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSTimer_hpp
#define MSTimer_hpp

#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace MS {
    
    enum MSTimerPlatform {
        MSTimerForApple,
        MSTimerForOther
    };

    typedef std::function<void()> TaskType;
    
    using namespace std;
    
    using namespace chrono;
    
    template <MSTimerPlatform platform>
    class MSTimer {
        
    };
    
    /**
     定时器
     
     这里使用标准库函数 sleep_for(...) 函数来定时,
     标准库因为有平台差异, 定时误差较大,
     以 50000ms 为例, 误差达到了 3200ms 左右,
     因此推荐使用平台相关的定时器.
     */
    template<>
    class MSTimer<MSTimerForOther> {
        thread timerThread;
        
        thread taskThread;
        
        mutex mtx;
        
        condition_variable condition;
        
        bool isRunning = false;
        
        bool isPausing = false;
        
        microseconds delayTime;
        
        microseconds timeInterval;
        
        microseconds deviation;
        
        TaskType task;
    public:
        MSTimer(const microseconds delayTime,
                const microseconds timeInterval,
                const TaskType task);
        
        ~MSTimer();

        void start();
        
        void pause();
        
        void rePlay();
        
        void stop();
        
        bool isActivity();
        
        microseconds getTimeInterval();
        
        MSTimer & updateTask(const TaskType task);
        
        MSTimer & updateDelayTime(const microseconds delayTime);
        
        MSTimer & updateTimeInterval(const microseconds timeInterval);
    };
    
    
    template <>
    class MSTimer<MSTimerForApple> {
        dispatch_source_t timer = NULL;
        
        bool isRunning = false;
        
        bool isPausing = false;
        
        microseconds delayTime;
        
        microseconds timeInterval;
        
        microseconds deviation;
        
        TaskType task;
    public:
        MSTimer(const microseconds delayTime,
                const microseconds timeInterval,
                const TaskType task);
        
        ~MSTimer();
        
        void start();
        
        void pause();
        
        void rePlay();
        
        void stop();
        
        bool isActivity();
        
        microseconds getTimeInterval();
        
        MSTimer & updateTask(const TaskType task);
        
        MSTimer & updateDelayTime(const microseconds delayTime);
        
        MSTimer & updateTimeInterval(const microseconds timeInterval);
    };
    
}

#endif /* MSTimer_hpp */
