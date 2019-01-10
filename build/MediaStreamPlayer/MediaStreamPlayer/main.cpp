//
//  main.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include <unistd.h>
#include <iostream>
#include <chrono>
//#include <MSPlayer.hpp>
//#include <FFDecoder.hpp>
//#include <FFEncoder.hpp>
#include <VideoToolbox/VideoToolbox.h>

//using namespace MS;
//using namespace MS::FFmpeg;

//MSPlayer<AVFrame> *player = nullptr;

//int i = 0;


//void test(){
//
//    FFDecoder *decoder = new FFDecoder();
//    FFEncoder *encoder = new FFEncoder(AV_CODEC_ID_H264,AV_CODEC_ID_AAC);
//    player = new MSPlayer<AVFrame>(decoder,encoder,
//                                   [](const MSMediaData<isDecode,AVFrame> &data) {
//                                       if (data.content) {
//                                           cout << "播放: " << data.content->timeInterval.count() << "-----" << i++ << endl;
//                                       } else {
//                                           cout << "没有资源" << endl;
//                                       }
//                                   },nullptr);
//
//    player->startPlayVideo();
//
//    for (int i = 0; i < 30; i++) {
//        auto content = new MSContent<isEncode>(nullptr,0,MSCodecID_H264);
//        MSMediaData<isEncode> *data = new MSMediaData<isEncode>(content);
//        player->pushVideoData(data);
//    }
//
//    sleep(40);
//
//    delete player;
//}

typedef void(^Test)(int a);

int main(int argc, const char * argv[]) {
    
    Test test;
    
    test(10);
    
    OSStatus status = VTDecompressionSessionDecodeFrameWithOutputHandler((VTDecompressionSessionRef)0x11,
                                                                         (CMSampleBufferRef)0x12,
                                                                         NULL,
                                                                         nullptr,
                                                                         ^(OSStatus status,
                                                                           VTDecodeInfoFlags infoFlags,
                                                                           CVImageBufferRef  _Nullable imageBuffer,
                                                                           CMTime presentationTimeStamp,
                                                                           CMTime presentationDuration)
                                                                         {
                                                                             printf("");
                                                                             
                                                                         });
    
    

    return 0;
}



