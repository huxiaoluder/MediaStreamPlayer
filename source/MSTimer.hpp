//
//  MSTimer.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/22.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSTimer_hpp
#define MSTimer_hpp

#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>

#define intervale(rate) microseconds(1000000LL/rate)

namespace MS {

    typedef std::function<void()> TaskType;
    
    using namespace std;
    
    using namespace chrono;
    
    class MSTimer {
        thread timerThread;
        
        thread taskThread;
        
        mutex mtx;
        
        condition_variable condition;
        
        bool isRun = false;
        
        bool isPause = false;
        
        const microseconds delayTime;
        
        microseconds timeInterval;
        
        TaskType task;
    public:
        MSTimer(const microseconds delayTime,
                const microseconds timeInterval,
                const TaskType task);
        
        ~MSTimer();

        void start();
        
        void pause();
        
        void _continue();
        
        void stop();
        
        MSTimer & updateTask(const TaskType task);
        
        MSTimer & updateTimeInterval(const microseconds timeInterval);
    };
    
}

#endif /* MSTimer_hpp */