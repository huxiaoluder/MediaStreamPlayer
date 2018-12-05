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
using namespace MS::FFmpeg;

MSPlayer<AVFrame> *player = nullptr;

int i = 0;

void test(){

    auto decoder = new FFDecoder();
    auto encoder = new FFEncoder(AV_CODEC_ID_H264,AV_CODEC_ID_AAC);
    player = new MSPlayer<AVFrame>(decoder,encoder);
    sleep(10);

    player->startPlay([](const MSMediaData<isDecode,AVFrame> &data){
        cout << "播放: " << data.content->timeInterval.count() << "-----" << i++ << endl;
    });

    for (int i = 0; i < 30; i++) {
        auto content = new MSContent<isEncode>(nullptr,0,MSCodecID_H264);
        MSMediaData<isEncode> *data = new MSMediaData<isEncode>(content);
        player->pushVideoData(data);
    }

    sleep(40);

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



