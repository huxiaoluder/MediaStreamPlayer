//
//  MSViewController.m
//  ios_example
//
//  Created by 胡校明 on 2018/12/5.
//  Copyright © 2018 freecoder. All rights reserved.
//

#import "MSViewController.h"
#import <iostream>
#import <MediaStreamPlayer.h>

#import "IotlibTool.h"
#import "NVGLPlayView.h"
#import "DDOpenAlAudioPlayer.h"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;
using namespace MS::APhard;

@interface MSViewController ()<IotlibToolDelegate>
{
//    MSPlayer<AVFrame> *player;
    MSPlayer<APFrame> *player;
    BOOL updateVideo;
    BOOL updateAudio;
}
@property (weak, nonatomic) IBOutlet UILabel *displayLabel;
@property (weak, nonatomic) IBOutlet NVGLPlayView *displayView;

@property (nonatomic, strong) APVideoRender *videoRender;

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
//    [self setupRenderView];
    [self setupVideoRender];
    
    __weak typeof(MSViewController *) weakSelf = self;
    
//    auto decoder = new FFDecoder();
//    auto encoder = new FFEncoder(MSCodecID_H264,MSCodecID_AAC);
//    player = new MSPlayer<AVFrame>(decoder,encoder,
//                                   [weakSelf](const MSMedia<MSDecodeMedia,AVFrame> &data) {
//                                       if (data.frame) {
//                                           [[weakSelf videoRender] displayAVFrame:*data.frame];
//                                       }
//                                   },
//                                   [weakSelf](const MSMedia<MSDecodeMedia,AVFrame> &data) {
//                                       if (data.frame) {
//                                            AVFrame &audio = *data.frame;
//                                            [[DDOpenALAudioPlayer sharePalyer] openAudioFromQueue:audio.data[0]
//                                                                                         dataSize:audio.linesize[0]
//                                                                                       samplerate:audio.sample_rate
//                                                                                         channels:audio.channels
//                                                                                              bit:16];
//                                        }
//                                   });
    
    auto decoder = new APDecoder();
    //    auto encoder = new APEncoder(MSCodecID_H264,MSCodecID_AAC);
    player = new MSPlayer<APFrame>(decoder,nullptr,
                                   [weakSelf](const MSMedia<MSDecodeMedia,APFrame> &data) {
                                       if (data.frame) {
                                           [[weakSelf videoRender] displayAPFrame:*data.frame];
                                       }
                                   },
                                   [weakSelf](const MSMedia<MSDecodeMedia,APFrame> &data) {
                                       if (data.frame) {
                                           AudioBuffer &audio = *data.frame->audio;
                                           [[DDOpenALAudioPlayer sharePalyer] openAudioFromQueue:(uint8_t *)audio.mData
                                                                                        dataSize:audio.mDataByteSize
                                                                                      samplerate:8000
                                                                                        channels:audio.mNumberChannels
                                                                                             bit:16];
                                       }
                                   });
    
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
        } else {
//            printf("**********video buffer empty!\n");
        }
    }
    
    if (updateAudio) {
        if (headerMedia->stream_type == e_stream_type_AAC) {
            auto data = new MSMedia<MSEncodeMedia>((uint8_t *)data_ptr,dataLen,headerMedia->is_key_frame,MSCodecID_AAC);
            player->pushAudioStreamData(data);
        } else {
//            printf("++++++++++audio buffer empty!\n");
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
    [[NSNotificationCenter defaultCenter] removeObserver:self];
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
    
    // IOTSHMK000S00004EDA785C
    [[IotlibTool shareIotlibTool] startConnectWithDeviceId:@"IOTSHMK000S0008EDA1FCDD"
                                                  callback:^(e_trans_conn_state status,
                                                             int connectId)
     {
         printf("------------connectId: %d\n",connectId);
     }];
}

- (void)setupRenderView {
    self.displayView.transform = CGAffineTransformRotate(CGAffineTransformIdentity, M_PI);
    self.displayView.layer.transform = CATransform3DRotate(CATransform3DIdentity, M_PI, UIScreen.mainScreen.bounds.size.width/2, 0, 0);
    [self.displayView setupGL];
}

- (void)setupVideoRender {
    
    CGFloat width   = UIScreen.mainScreen.bounds.size.width;
    CGFloat height  = UIScreen.mainScreen.bounds.size.width * 9 / 16;
    CGFloat yVtx    = UIScreen.mainScreen.bounds.size.height - 73 - height;
    CGRect rect = CGRectMake(0, yVtx, width, height);
    NSLock *lock = [NSLock new];
    self.videoRender = [APVideoRender renderTo:self.view drawRect:rect syncLock:lock];
}

@end
