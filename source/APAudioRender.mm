//
//  APAudioRender.m
//  ios_example
//
//  Created by 胡校明 on 2019/3/18.
//  Copyright © 2019 freecoder. All rights reserved.
//

#import "APAudioRender.h"
#import <pthread.h>
#include <queue>
#include <unistd.h>

@interface APAudioRender ()
{
    AudioUnit remoteIOUnit;
    int channels;
    int frequency;
}

@property (nonatomic, strong) NSPipe *pipe;
@property (nonatomic, assign) BOOL isStarted;

@end

@implementation APAudioRender

OSStatus
renderCallback(void * inRefCon,
               AudioUnitRenderActionFlags * ioActionFlags,
               const AudioTimeStamp * inTimeStamp,
               UInt32 inBusNumber,
               UInt32 inNumberFrames,
               AudioBufferList * MSNullable ioData) {
    APAudioRender *render = (__bridge APAudioRender *)inRefCon;

    for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
        @autoreleasepool {
            AudioBuffer &ioBuffer = ioData->mBuffers[i];
            NSData *data = [render.pipe.fileHandleForReading readDataOfLength:ioBuffer.mDataByteSize];
            if (data.length) {
                memcpy(ioBuffer.mData, data.bytes, data.length);
            } else {
                memset(ioBuffer.mData, 0, ioBuffer.mDataByteSize);
            }
        }
    }
    return noErr;
}

+ (instancetype)renderWithChannels:(int)channels frequency:(int)frequency {
    APAudioRender *render = [APAudioRender new];
    [render updateChannels:channels frequency:frequency];
    return render;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _pipe = [NSPipe pipe];
        
        const AudioComponentDescription componentDescription {
            .componentType = kAudioUnitType_Output,
            .componentSubType = kAudioUnitSubType_RemoteIO,
            .componentManufacturer = kAudioUnitManufacturer_Apple,
            .componentFlags = 0,
            .componentFlagsMask = 0,
        };
        
        AudioComponent audioComponent = AudioComponentFindNext(nullptr, &componentDescription);
        
        AudioComponentInstanceNew(audioComponent, &remoteIOUnit);
    }
    return self;
}

- (void)updateChannels:(int)channels frequency:(int)frequency {
    if ((self->channels ^ channels) |
        (self->frequency ^ frequency)) {
        self->channels = channels;
        self->frequency = frequency;
        [self setupAudioUnit];
    }
}

- (void)setupAudioUnit {

    OSStatus status = noErr;
    
    if (_isStarted) {
        status = AudioOutputUnitStop(remoteIOUnit);
        if (status) {
            OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
        }
        
        status = AudioUnitReset(remoteIOUnit, kAudioUnitScope_Global, 0);
        if (status) {
            OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
        }
        
        status = AudioUnitUninitialize(remoteIOUnit);
        if (status) {
            OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
        }
        _isStarted = false;
    }
    
    // 输出设备为 0, 输入设备为 1.
    UInt32 inElement = 0;
    
    UInt32 inData = 1;
    status = AudioUnitSetProperty(remoteIOUnit,
                                  kAudioOutputUnitProperty_EnableIO,
                                  kAudioUnitScope_Output,
                                  inElement,
                                  &inData,
                                  sizeof(inData));
    if (status) {
        OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
    }
    
    AURenderCallbackStruct callBack {
        .inputProc = renderCallback,
        .inputProcRefCon = (__bridge void *)self,
    };
    status = AudioUnitSetProperty(remoteIOUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input,
                                  inElement,
                                  &callBack,
                                  sizeof(callBack));
    if (status) {
        OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
    }

    AudioStreamBasicDescription inAsbd {
        .mSampleRate        = (Float64)frequency,
        .mFormatID          = kAudioFormatLinearPCM,
        .mFormatFlags       = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
        .mBytesPerPacket    = 1 * 2 * (UInt32)channels,
        .mFramesPerPacket   = 1, // 只支持 1 pack 1 frame, 否则报错(code: -50)
        .mBytesPerFrame     = 2 * (UInt32)channels,
        .mChannelsPerFrame  = (UInt32)channels,
        .mBitsPerChannel    = 16,
        .mReserved          = 0
    };
    status = AudioUnitSetProperty(remoteIOUnit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  inElement,
                                  &inAsbd,
                                  sizeof(inAsbd));
    if (status) {
        OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
    }

    status = AudioUnitInitialize(remoteIOUnit);
    if (status) {
        OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
    }
    
    status = AudioOutputUnitStart(remoteIOUnit);
    if (status) {
        OSStatusErrorLocationLog("call AudioUnitSetProperty fail", status);
    }
    
    assert(status == noErr);
    
    _isStarted = true;
}

- (void)displayAVFrame:(AVFrame &)frame {
   [_pipe.fileHandleForWriting writeData:[NSData dataWithBytes:frame.data[0] length:frame.linesize[0]]];
}

- (void)displayAPFrame:(APFrame &)frame {
    [_pipe.fileHandleForWriting writeData:[NSData dataWithBytes:frame.audio->mData length:frame.audio->mDataByteSize]];
}

- (void)dealloc {
    if (remoteIOUnit) {
        AudioOutputUnitStop(remoteIOUnit);
        AudioUnitReset(remoteIOUnit, kAudioUnitScope_Global, 0);
        AudioUnitUninitialize(remoteIOUnit);
        AudioComponentInstanceDispose(remoteIOUnit);
    }
}

@end
