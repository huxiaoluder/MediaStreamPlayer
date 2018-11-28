//
//  main.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include <unistd.h>
#include <csignal>
#include <ctime>
#include <MSPlayer.hpp>
#include <iostream>
#include <thread>
#include <future>
#include <chrono>
//#include <mutex>
//#include <shared_mutex>
#include <condition_variable>
#include <semaphore.h>
#include <csignal>
#include <sys/time.h>
#include <MSTimer.hpp>

using namespace MS;

MSPlayer *player;

int i = 0;

void test(){
    player = new MSPlayer();
    sleep(5);
    
    player->startPlay([](MSData &data){
        cout << "播放: " << data.timeInterval.count() << "-----" << i++ << endl;
    });
    
    for (int i = 0; i < 20; i++) {
        MSData *data = new MSData((uint8_t *)0x001,0,intervale(1));
        player->pushVideoData(data);
    }
    
    sleep(20);
    
    delete player;
}

int main(int argc, const char * argv[]) {
    
    test();
//    static int i = 0;
    while (true) {
//        printf("++++++++++\n");
        
        this_thread::sleep_for(seconds(1));
    }

//    this_thread::sleep_for(hours(24));
    return 0;
}


