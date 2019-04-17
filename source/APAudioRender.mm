//
//  APAudioRender.mm
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/3/18.
//  Copyright © 2019 freecoder. All rights reserved.
//

#import "APAudioRender.h"

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
    NSPipe *pipe = (__bridge NSPipe *)inRefCon;

    for (UInt32 i = 0; i < ioData->mNumberBuffers; i++) {
        @autoreleasepool {
            AudioBuffer &ioBuffer = ioData->mBuffers[i];
            // readDataOfLength 管道数据不够 length 时, 会阻塞当前前程, 导致暂停播放, 直到管道写入新数据足够 length 才恢复继续执行
            NSData *data = [pipe.fileHandleForReading readDataOfLength:ioBuffer.mDataByteSize];
            printf("+++++++++++++\n");
            // 管道保证了数据来源确定性, 如果其他方式读取数据, 数据不够时, 应该 memset 为 0, 提供静音播放
            memcpy(ioBuffer.mData, data.bytes, data.length);
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
    
    AURenderCallbackStruct callBackStruct {
        .inputProc = renderCallback,
        .inputProcRefCon = (__bridge void *)_pipe,
    };
    status = AudioUnitSetProperty(remoteIOUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input,
                                  inElement,
                                  &callBackStruct,
                                  sizeof(callBackStruct));
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
    if (_isStarted) {
        [_pipe.fileHandleForWriting writeData:[NSData dataWithBytes:frame.data[0] length:frame.linesize[0]]];
    }
}

- (void)displayAPFrame:(APFrame &)frame {
    if (_isStarted) {
        [_pipe.fileHandleForWriting writeData:[NSData dataWithBytes:frame.audio->mData length:frame.audio->mDataByteSize]];
    }
}

- (void)dealloc {
    _isStarted = false;
    // 先关闭管道, 保证线程不会阻塞
    [_pipe.fileHandleForWriting closeFile];
    [_pipe.fileHandleForReading closeFile];
    AudioOutputUnitStop(remoteIOUnit);
    AudioUnitReset(remoteIOUnit, kAudioUnitScope_Global, 0);
    AudioUnitUninitialize(remoteIOUnit);
    AudioComponentInstanceDispose(remoteIOUnit);
}

@end
