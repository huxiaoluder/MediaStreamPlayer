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
#import "IotlibTool.h"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;

@interface MSViewController ()<IotlibToolDelegate>
{
    MSPlayer<AVFrame> *player;
    BOOL updateData;
}

@end

@implementation MSViewController

static int i;

- (void)viewDidLoad {
    [super viewDidLoad];
    auto decoder = new FFDecoder();
    auto encoder = new FFEncoder(AV_CODEC_ID_H264,AV_CODEC_ID_AAC);
    player = new MSPlayer<AVFrame>(decoder,encoder,
                                   [&](const MSMediaData<isDecode,AVFrame> &data) {
                                       if (data.content) {
                                           cout
                                           << "播放: "
                                           << data.content->timeInterval.count()
                                           << "----- "
                                           << i++
                                           << endl;
                                       } else {
                                           cout
                                           << "播放: ----- 没有资源"
                                           << endl;
                                       }
                                   },nullptr);
    
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
        printf("-----------------status: %d", status);
    }];
}

- (void)onMediaDataWithConnId:(uint32_t)connId
                  headerMedia:(header_media_t *)headerMedia
                      dataPtr:(const char *)data_ptr
                      dataLen:(uint32_t)dataLen {
//    printf("--------------datalen: %d\n",dataLen);
    if (updateData) {
        if (headerMedia->stream_type == e_stream_type_H264) {
            uint8_t *datap = new uint8_t[dataLen]{0};
            memcpy(datap, data_ptr, dataLen);
            auto content = new MSContent<isEncode>(datap,dataLen,MSCodecID_H264);
            MSMediaData<isEncode> *data = new MSMediaData<isEncode>(content);
            player->pushVideoData(data);
        }
    }
}

- (IBAction)play:(UIButton *)sender {
    i = 0;
    player->startPlayVideo();
}

- (IBAction)pause:(UIButton *)sender {
    player->pausePlayVideo();
}

- (IBAction)_continue:(UIButton *)sender {
    player->continuePlayVideo();
}

- (IBAction)stop:(UIButton *)sender {
    player->stopPlayVideo();
}

- (IBAction)update:(UIButton *)sender {
    updateData = !updateData;
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
