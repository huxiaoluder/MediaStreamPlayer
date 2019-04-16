//
//  MSViewController.m
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/5.
//  Copyright © 2018 freecoder. All rights reserved.
//

#import "MSViewController.h"
#import <iostream>
#import <MediaStreamPlayer.h>
#import <time.h>

#import "IotlibTool.h"
#import "DDOpenALAudioPlayer.h"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;
using namespace MS::APhard;


#define condition 1

@interface MSViewController ()<IotlibToolDelegate>
{
#if condition
    MSPlayer<AVFrame> *player;
#else
    MSPlayer<APFrame> *player;
#endif
    BOOL updateVideo;
    BOOL updateAudio;
}
@property (weak, nonatomic) IBOutlet UILabel *displayLabel;

@property (nonatomic, strong) APVideoRender *videoRender;
@property (nonatomic, strong) APAudioRender *audioRender;

@end

@implementation MSViewController

static int i;
//static int j;

- (void)enterForeground {
//    player->startPlayVideo();
}

- (void)enterBackground {
//    player->stopPlayVideo();
////    player->pausePlayVideo();
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    [self doSomeSetup];
    [self setupVideoRender];
    [self setupAudioRender];
    
    __weak typeof(MSViewController *) weakSelf = self;
    
#if condition
    auto decoder = new FFDecoder();
    auto encoder = new FFEncoder(MSCodecID_H264,MSCodecID_AAC);
    player = new MSPlayer<AVFrame>(decoder,encoder,
                                   [weakSelf](const MSMedia<MSDecodeMedia,AVFrame> &data) {
                                       if (data.frame) {
                                           [[weakSelf videoRender] displayAVFrame:*data.frame];
                                       }
                                   },
                                   [weakSelf](const MSMedia<MSDecodeMedia,AVFrame> &data) {
                                       if (data.frame) {
                                           [[weakSelf audioRender] updateChannels:data.frame->channels
                                                                        frequency:data.frame->sample_rate];
                                           [[weakSelf audioRender] displayAVFrame:*data.frame];
//                                           [[DDOpenALAudioPlayer sharePalyer] openAudioFromQueue:data.frame->data[0]
//                                                                                        dataSize:data.frame->linesize[0]
//                                                                                      samplerate:8000
//                                                                                        channels:1
//                                                                                             bit:16];
                                        }
                                   });
#else
    auto decoder = new APDecoder();
    auto encoder = new APEncoder(MSCodecID_H264,MSCodecID_AAC);
    player = new MSPlayer<APFrame>(decoder,encoder,
                                   [weakSelf](const MSMedia<MSDecodeMedia,APFrame> &data) {
                                       if (data.frame) {
                                           [[weakSelf videoRender] displayAPFrame:*data.frame];
                                       }
                                   },
                                   [weakSelf](const MSMedia<MSDecodeMedia,APFrame> &data) {
                                       if (data.frame) {
                                           [[weakSelf audioRender] updateChannels:data.frame->audioParameters.channels
                                                                        frequency:data.frame->audioParameters.frequency.value];
                                           [[weakSelf audioRender] displayAPFrame:*data.frame];
                                       }
                                   });
#endif
    
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
            auto data = new MSMedia<MSEncodeMedia>((uint8_t *)data_ptr,dataLen,headerMedia->is_key_frame,MSCodecID_H264);
            player->pushVideoStreamData(data);
        } else if (headerMedia->stream_type == e_stream_type_H265) {
            auto data = new MSMedia<MSEncodeMedia>((uint8_t *)data_ptr,dataLen,headerMedia->is_key_frame,MSCodecID_H265);
            player->pushVideoStreamData(data);
        }
    }
    
    if (updateAudio) {
        if (headerMedia->stream_type == e_stream_type_AAC) {
//            printf("--------------datalen: %d\n",dataLen);
            auto data = new MSMedia<MSEncodeMedia>((uint8_t *)data_ptr,dataLen,headerMedia->is_key_frame,MSCodecID_AAC);
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
#if condition
    FFEncoder &encoder = (FFEncoder &)player->reEncoder();
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
#else
    
    APEncoder &encoder = (APEncoder &)player->reEncoder();
    APDecoder &decoder = (APDecoder &)player->asynDecoder();
    if (encoder.isEncoding()) {
        player->stopReEncode();
    } else {
        NSString *videoPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject] stringByAppendingPathComponent:[NSString stringWithFormat:@"test.mp4"]];
        bool ret = encoder.configureEncoder(videoPath.UTF8String,
                                            decoder.getCurrentVideoParameters(),
                                            decoder.getCurrentAudioParameters());
        if (ret) {
            player->startReEncode();
        }
    }
    
#endif
}




- (IBAction)back:(UIButton *)sender {
    [self dismissViewControllerAnimated:true completion:^{
        
    }];
}


- (void)dealloc {
    printf("----delloc\n");
    delete player;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}





- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if ([touches.allObjects.firstObject locationInView:self.view].y > [touches.allObjects.firstObject previousLocationInView:self.view].y) {
        CGRect rect = self.videoRender.drawRect;
        rect.size.width += 5;
        rect.size.height = rect.size.width / 16 * 9;
        [self.videoRender updateDrawRect:rect];
    } else if ([touches.allObjects.firstObject locationInView:self.view].y < [touches.allObjects.firstObject previousLocationInView:self.view].y) {
        CGRect rect = self.videoRender.drawRect;
        rect.size.width -= 5;
        rect.size.height = rect.size.width / 16 * 9;
        [self.videoRender updateDrawRect:rect];
    }
}

- (void)motionEnded:(UIEventSubtype)motion withEvent:(UIEvent *)event {
    UIImage *image = self.videoRender.snapshot;
    NSLog(@"%@", image);
}



//----------------------
- (void)doSomeSetup {
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(enterBackground)
                                                 name:UIApplicationDidEnterBackgroundNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(enterForeground)
                                                 name:UIApplicationWillEnterForegroundNotification
                                               object:nil];
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
    
    // 38l(h265, aac): IOTSHMK038L0000419790
    // 私模(h264, aac): IOTSHMK000S0008EDA1FCDD
    // 摇头机(h264, alaw): IOTSHMKP00300004F0716
    [[IotlibTool shareIotlibTool] startConnectWithDeviceId:@"IOTSHMKP00300004F0716"
                                                  callback:^(e_trans_conn_state status,
                                                             int connectId)
     {
         printf("------------connectId: %d\n",connectId);
     }];
}

- (void)setupVideoRender {
    
    CGFloat width   = UIScreen.mainScreen.bounds.size.width;
    CGFloat height  = UIScreen.mainScreen.bounds.size.width * 9 / 16;
    CGFloat yVtx    = UIScreen.mainScreen.bounds.size.height - 73 - height;
    CGRect rect = CGRectMake(0, yVtx, width, height);
    NSLock *lock = [NSLock new];
    self.videoRender = [APVideoRender renderTo:self.view drawRect:rect syncLock:lock];
}

- (void)setupAudioRender {
    self.audioRender = [APAudioRender renderWithChannels:1 frequency:8000];
}

@end
