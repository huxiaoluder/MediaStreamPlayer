//
//  main.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include <unistd.h>
#include <iostream>
#include <MSPlayer>
#include <FFDecoder.hpp>
#include <FFEncoder.hpp>

using namespace MS;

MSPlayer<AVFrame *> *player;

int i = 0;

void test(){

    
    auto decoder = new FFDecoder();
    auto encoder = new FFEncoder();
    player = new MSPlayer<AVFrame *>(decoder,encoder);
    sleep(5);

//    player->startPlay([](const MSMediaData<isDecodeData,AVFrame *> &data){
//        cout << "播放: " << data.content.timeInterval.count() << "-----" << i++ << endl;
//    });
//
//    for (int i = 0; i < 20; i++) {
//        MSContentData<isEncodeData> content((uint8_t*)0x10,0,MSCodecID_H264);
//        MSMediaData<isEncodeData> *data = new MSMediaData<isEncodeData>(content);
//        player->pushVideoData(data);
//    }

    sleep(20);

    delete player;
}

int main(int argc, const char * argv[]) {
    
    test();
    int *i = new int(10);
    auto &ii = *i;
    ii = 20;
    printf("%p\n", i);
    printf("%p\n", &ii);
    printf("%d\n",*i);
    while (true) {
        this_thread::sleep_for(seconds(1));
    }
    return 0;
}



