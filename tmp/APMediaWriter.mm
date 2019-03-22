//
// APMediaWriter.m
// MediaStreamPlayer
// 
// Created by xiaoming on 2019/3/21.
// Copyright © 2019 freecoder. All rights reserved.
//
// Email: huxiaoluder@163.com
//

#import "APMediaWriter.h"
#include <queue>
#include <pthread.h>
#include <AVFoundation/AVFoundation.h>

using namespace std;

@interface APMediaWriter ()

@property (nonatomic, assign) queue<CVPixelBufferRef> *bufferQueue;
@property (nonatomic, assign) pthread_mutex_t *mutex;
@property (nonatomic, assign) BOOL isInit;
@property (nonatomic, strong) AVAssetWriter *assetWriter;
@property (nonatomic, strong) AVAssetWriterInput *videoInput;

@end

@implementation APMediaWriter

- (instancetype)init
{
    self = [super init];
    if (self) {
        _bufferQueue = new queue<CVPixelBufferRef>;
        _mutex = new pthread_mutex_t;
        pthread_mutex_init(_mutex, nullptr);
    }
    return self;
}

- (void)initWithPixelBuffer:(CVPixelBufferRef)pixelBuffer {
    
    NSError *error;
    NSString *videoPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject] stringByAppendingPathComponent:[NSString stringWithFormat:@"test.mov"]];
    [[NSFileManager defaultManager] removeItemAtPath:videoPath error:nil];
    
    _assetWriter = [AVAssetWriter assetWriterWithURL:[NSURL fileURLWithPath:videoPath]
                                            fileType:AVFileTypeQuickTimeMovie
                                               error:&error];
    NSDictionary *properties = @{
                                 AVVideoExpectedSourceFrameRateKey : @20,
                                 AVVideoMaxKeyFrameIntervalKey : @1,
                                 AVVideoAverageBitRateKey : @10500000,
                                 AVVideoProfileLevelKey : AVVideoProfileLevelH264HighAutoLevel
                                 };
    
    NSDictionary *outputSetting = @{
                                    AVVideoCodecKey     : AVVideoCodecH264,
                                    AVVideoWidthKey     : @(CVPixelBufferGetWidth(pixelBuffer)),
                                    AVVideoHeightKey    : @(CVPixelBufferGetHeight(pixelBuffer)),
                                    AVVideoCompressionPropertiesKey : properties
                                    };

    
    BOOL ret = [_assetWriter canApplyOutputSettings:outputSetting forMediaType:AVMediaTypeVideo];
    if (!ret) {
        ErrorLocationLog("start fail");
        return;
    }
    
    _videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo
                                                     outputSettings:outputSetting];
    
    if ([_assetWriter canAddInput:_videoInput]) {
        [_assetWriter addInput:_videoInput];
        _videoInput.expectsMediaDataInRealTime = YES;
    }
    
    ret = [_assetWriter startWriting];
    if (!ret) {
        ErrorLocationLog("start fail");
        return;
    }
    
    [_assetWriter startSessionAtSourceTime:kCMTimeZero];
    
    _isInit = true;
    
    __weak typeof(APMediaWriter *) weakSelf = self;
    
    dispatch_queue_t writeQ = dispatch_queue_create("cn.xiaoming.write", nullptr);

    [_videoInput requestMediaDataWhenReadyOnQueue:writeQ usingBlock:^{
        OSStatus status = noErr;
        
        AVAssetWriterInput *videoInput = weakSelf.videoInput;
        
        while ([videoInput isReadyForMoreMediaData]) {
            if (weakSelf.bufferQueue->size() == 0) {
                continue;
            }
            @autoreleasepool {
                pthread_mutex_lock(weakSelf.mutex);
                CVPixelBufferRef imageBuffer = weakSelf.bufferQueue->front();
                weakSelf.bufferQueue->pop();
                pthread_mutex_unlock(weakSelf.mutex);
                
                CMSampleBufferRef sampleBuffer;
                
                static CMSampleTimingInfo sampleTiming{0};
                
                CMVideoFormatDescriptionRef desc;
                CMVideoFormatDescriptionCreateForImageBuffer(kCFAllocatorDefault, imageBuffer, &desc);
                
                status = CMSampleBufferCreateReadyWithImageBuffer(kCFAllocatorDefault,
                                                                  imageBuffer,
                                                                  desc,
                                                                  &sampleTiming,
                                                                  &sampleBuffer);
                if (status) {
                    OSStatusErrorLocationLog("call CMSampleBufferCreateReadyWithImageBuffer fail", status);
                }
                
                BOOL state = [videoInput appendSampleBuffer:sampleBuffer];
                if (!state) {
                    ErrorLocationLog("fail write---------");
                    NSLog(@"%@", weakSelf.assetWriter.error);
                }
                CFRelease(sampleBuffer);
                CFRelease(desc);
                
                sampleTiming.presentationTimeStamp.value += 1;
                
                CVPixelBufferRelease(imageBuffer);
            }
        }
    }];
    
}

- (void)writeVideoBuffer:(CVPixelBufferRef)pixelBuffer {
    if (!_isInit) {
        [self initWithPixelBuffer:pixelBuffer];
        if (!_isInit) {
            return;
        }
    }
    
    pthread_mutex_lock(_mutex);
    _bufferQueue->push(CVPixelBufferRetain(pixelBuffer));
    pthread_mutex_unlock(_mutex);
}

- (void)dealloc {
    if (_bufferQueue) {
        delete _bufferQueue;
    }
    if (_mutex) {
        pthread_mutex_destroy(_mutex);
        delete _mutex;
    }
}

@end
