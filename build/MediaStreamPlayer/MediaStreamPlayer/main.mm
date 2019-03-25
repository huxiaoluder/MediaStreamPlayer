//
//  main.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include <unistd.h>
#include "MSTimer.hpp"
#include <sys/time.h>

#include <Foundation/Foundation.h>

typedef void(^Test)(int a);

int main(int argc, const char * argv[]) {
//    static int *var = new int[1024];
//    static int i = 0;
//    static timeval time0;
//    static timeval time1;
//    gettimeofday(&time0, nullptr);
//    dispatch_source_t timer0 = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_global_queue(0, 0));
//    dispatch_source_set_timer(timer0, DISPATCH_TIME_NOW, 50 * USEC_PER_SEC, 0);
//    dispatch_source_set_event_handler(timer0, ^{
//        gettimeofday(&time1, nullptr);
////        printf("--- %d\n", time1.tv_usec - time0.tv_usec);
//        var[i++] = time1.tv_usec - time0.tv_usec;
//        time0 = time1;
//        if (i == 1024 - 1) {
//            long long cout = 0;
//            long long sum = 0;
//            for (int i = 0; i < 1024; i++) {
//                if (var[i] > 40000) {
//                    cout++;
//                    sum += var[i];
//                }
//            }
//            printf(".................. %lld\n", sum / cout);
//            exit(0);
//        }
//    });
//    dispatch_resume(timer0);
    
    int *var = new int[1024];
    static int i = 0;
    timeval time0;
    timeval time1;
    gettimeofday(&time0, nullptr);
    MS::MSTimer timer(MS::microseconds(0), MS::microseconds(50000), [&](){
        gettimeofday(&time1, nullptr);
//        printf("^^^^^^^^^^^^ %d\n", time1.tv_usec - time0.tv_usec);
        var[i++] = time1.tv_usec - time0.tv_usec;
        time0 = time1;
        if (i == 1024 - 1) {
            long long cout = 0;
            long long sum = 0;
            for (int i = 0; i < 1024; i++) {
                if (var[i] > 40000) {
                    cout++;
                    sum += var[i];
                }
            }
            printf(".................. %lld\n", sum / cout);
            exit(0);
        }
    });
    timer.start();
    pause();

    return 0;
}



