//
//  MSViewController.m
//  ios_example
//
//  Created by 胡校明 on 2018/12/5.
//  Copyright © 2018 freecoder. All rights reserved.
//

#import "MSViewController.h"
#import <iostream>
#import <MSPlayer.hpp>
#import <FFDecoder.hpp>
#import <FFEncoder.hpp>
#import <APDecoder.hpp>
#import "IotlibTool.h"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;
using namespace MS::APhard;

@interface MSViewController ()<IotlibToolDelegate>
{
    MSPlayer<AVFrame> *player;
    BOOL updateVideo;
    BOOL updateAudio;
}

@end

@implementation MSViewController

static int i;
//static int j;

- (void)viewDidLoad {
    [super viewDidLoad];
    auto decoder = new FFDecoder();
    auto encoder = new FFEncoder(MSCodecID_H264,MSCodecID_AAC);
    player = new MSPlayer<AVFrame>(decoder,encoder,
                                   [&](const MSMedia<isDecode,AVFrame> &data) {
//                                       if (data.content) {
//                                           cout
//                                           << "video 播放: "
//                                           << data.content->timeInterval.count()
//                                           << "----- "
//                                           << i++
//                                           << endl;
//                                       } else {
//                                           cout
//                                           << "video 播放: ----- 没有资源"
//                                           << endl;
//                                       }
                                   },
                                   [&](const MSMedia<isDecode,AVFrame> &data) {
//                                       if (data.content) {
//                                           cout
//                                           << "audio 播放: "
//                                           << data.content->timeInterval.count()
//                                           << "----- "
//                                           << j++
//                                           << endl;
//                                       } else {
//                                           cout
//                                           << "audio 播放: ----- 没有资源"
//                                           << endl;
//                                       }
                                   });
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(connectsuccess:)
                                                 name:CONNECTSUCCESS
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(connectfail:)
                                                 name:CONNECTFAIL
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(connectfail:)
                                                 name:CONNECTSTOPPED
                                               object:nil];
    [IotlibTool shareIotlibTool].delegate = self;

    [[IotlibTool shareIotlibTool] startConnectWithDeviceId:@"IOTSHMK000S00004EDA785C"
                                                  callback:^(e_trans_conn_state status,
                                                             int connectId)
     {
         printf("------------connectId: %d\n",connectId);
     }];
}

- (void)connectfail:(NSNotification *)notify {
    printf("------------conn fail\n");
}

- (void)connectsuccess:(NSNotification *)notify {
    printf("------------conn success\n");
    NSDictionary *dic = (NSDictionary *)notify.object;
    int connID = [dic[@"connID"] intValue];
    [[IotlibTool shareIotlibTool] startVideoWithConnectId:connID
                                                     chno:1
                                             videoQuality:500
                                                 callback:^(int status)
    {
        printf("video: -----------------status: %d\n", status);
    }];
    
    [[IotlibTool shareIotlibTool] startAudioWithChannelID:connID Callback:^(int status,
                                                                            uint32_t audio_codec,
                                                                            uint32_t rate,
                                                                            uint32_t bit,
                                                                            uint32_t track) {
        printf("audio: -----------------status: %d\n", status);
    }];
}

- (void)onMediaDataWithConnId:(uint32_t)connId
                  headerMedia:(header_media_t *)headerMedia
                      dataPtr:(const char *)data_ptr
                      dataLen:(uint32_t)dataLen {
//    printf("--------------datalen: %d\n",dataLen);
    if (updateVideo) {
        if (headerMedia->stream_type == e_stream_type_H264) {
            auto data = new MSMedia<isEncode>((uint8_t *)data_ptr,dataLen,headerMedia->is_key_frame,MSCodecID_H264);
            player->pushVideoStreamData(data);
        }
    }
    
    if (updateAudio) {
        if (headerMedia->stream_type == e_stream_type_AAC) {
            auto data = new MSMedia<isEncode>((uint8_t *)data_ptr,dataLen,headerMedia->is_key_frame,MSCodecID_AAC);
            player->pushAudioStreamData(data);
        }
    }
}



- (IBAction)playVideo:(UIButton *)sender {
    i = 0;
    player->startPlayVideo();
}

- (IBAction)pauseVideo:(UIButton *)sender {
    player->pausePlayVideo();
}

- (IBAction)continueVideo:(UIButton *)sender {
    player->continuePlayVideo();
}

- (IBAction)stopVideo:(UIButton *)sender {
    player->stopPlayVideo();
}

- (IBAction)updateVideo:(UIButton *)sender {
    updateVideo = !updateVideo;
}



- (IBAction)playAudio:(UIButton *)sender {
    player->startPlayAudio();
}

- (IBAction)pauseAudio:(UIButton *)sender {
    player->pausePlayAudio();
}

- (IBAction)continueAudio:(UIButton *)sender {
    player->continuePlayAudio();
}

- (IBAction)stopAudio:(UIButton *)sender {
    player->stopPlayAudio();
}

- (IBAction)updateAudio:(UIButton *)sender {
    updateAudio = !updateAudio;
}



- (IBAction)encodeMedia:(UIButton *)sender {
    FFEncoder &encoder = (FFEncoder &)player->syncEncoder();
    FFDecoder &decoder = (FFDecoder &)player->syncDecoder();
    if (encoder.isEncoding()) {
        player->stopReEncode();
    } else {
        NSString *videoPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject] stringByAppendingPathComponent:[NSString stringWithFormat:@"test.mp4"]];
        bool ret = encoder.configureEncoder(videoPath.UTF8String,
                                            decoder.findDecoderContext(MSCodecID_H264),
                                            decoder.findDecoderContext(MSCodecID_AAC));
        if (ret) {
            player->startReEncode();
        }
    }
}




- (IBAction)back:(UIButton *)sender {
    [self dismissViewControllerAnimated:true completion:^{
        
    }];
}


- (void)dealloc {
    printf("----delloc\n");
    delete player;
}

@end
