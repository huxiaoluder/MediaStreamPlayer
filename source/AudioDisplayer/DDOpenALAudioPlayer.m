//
//  DDOpenALAudioPlayer.m
//  SmartHome
//
//  Created by mysoul on 2018/8/14.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import "DDOpenALAudioPlayer.h"
#import <OpenAL/al.h>
#import <OpenAL/alc.h>
#import <OpenAL/oalMacOSX_OALExtensions.h>

@interface DDOpenALAudioPlayer ()
{
    ALCcontext *mContext;
    ALCdevice *mDevice;
    ALuint outSourceId;
    ALuint buff;
}

@end

@implementation DDOpenALAudioPlayer
static DDOpenALAudioPlayer *_player;

+ (id)sharePalyer {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        if (_player == nil) {
            _player = [[DDOpenALAudioPlayer alloc] init];
            [_player initOpenAL];
        }
    });
    return _player;
}

// 初始化openAL
- (void)initOpenAL {
    mDevice=alcOpenDevice(NULL);
    if (mDevice) {
        mContext = alcCreateContext(mDevice, NULL);
        alcMakeContextCurrent(mContext);
    }
    
    alGenSources(1, &outSourceId);
    alSpeedOfSound(1.0);
    alDopplerVelocity(1.0);
    alDopplerFactor(1.0);
    alSourcef(outSourceId, AL_PITCH, 1.0f);
    alSourcef(outSourceId, AL_GAIN, 1.0f);
    alSourcei(outSourceId, AL_LOOPING, AL_FALSE);
    alSourcef(outSourceId, AL_SOURCE_TYPE, AL_STREAMING);
}

// 播放回调
- (void)openAudioFromQueue:(uint8_t *)data dataSize:(size_t)dataSize samplerate:(int)samplerate channels:(int)channels bit:(int)bit {
    NSCondition* ticketCondition= [[NSCondition alloc] init];
    [ticketCondition lock];
    
    if (!mContext) {
        [self initOpenAL];
    }
    
    ALuint bufferID = 0;
    alGenBuffers(1, &bufferID);
    NSData * tmpData = [NSData dataWithBytes:data length:dataSize];
    int aSampleRate,aBit,aChannel;
    aSampleRate = samplerate;
    aBit = bit;
    aChannel = channels;
    ALenum format = 0;
    if (aBit == 8) {
        if (aChannel == 1)
            format = AL_FORMAT_MONO8;
        else if(aChannel == 2)
            format = AL_FORMAT_STEREO8;
        else if( alIsExtensionPresent( "AL_EXT_MCFORMATS" ) )
        {
            if( aChannel == 4 )
            {
                format = alGetEnumValue( "AL_FORMAT_QUAD8" );
            }
            if( aChannel == 6 )
            {
                format = alGetEnumValue( "AL_FORMAT_51CHN8" );
            }
        }
    }else if( aBit == 16 ){
        if( aChannel == 1 )
        {
            format = AL_FORMAT_MONO16;
        }
        if( aChannel == 2 )
        {
            format = AL_FORMAT_STEREO16;
        }
        if( alIsExtensionPresent( "AL_EXT_MCFORMATS" ) )
        {
            if( aChannel == 4 )
            {
                format = alGetEnumValue( "AL_FORMAT_QUAD16" );
            }
            if( aChannel == 6 )
            {
                format = alGetEnumValue( "AL_FORMAT_51CHN16" );
            }
        }
    }
    alBufferData(bufferID, format, (char*)[tmpData bytes], (ALsizei)[tmpData length],aSampleRate);
    alSourceQueueBuffers(outSourceId, 1, &bufferID);
    [self updataQueueBuffer];
    
    ALint stateVaue;
    alGetSourcei(outSourceId, AL_SOURCE_STATE, &stateVaue);
    
    [ticketCondition unlock];
    ticketCondition = nil;
    
}

- (BOOL)updataQueueBuffer {
    ALint stateVaue;
    int processed, queued;
    
    alGetSourcei(outSourceId, AL_BUFFERS_PROCESSED, &processed);
    alGetSourcei(outSourceId, AL_BUFFERS_QUEUED, &queued);
    
    alGetSourcei(outSourceId, AL_SOURCE_STATE, &stateVaue);
    
    if (stateVaue == AL_STOPPED ||
        stateVaue == AL_PAUSED ||
        stateVaue == AL_INITIAL) {
        [self playSound];
    } else if (stateVaue == AL_PLAYING && queued < 1){
        [self pauseSound];
    } else if(stateVaue == 4116){
        return NO;
    }
    while(processed--) {
        alSourceUnqueueBuffers(outSourceId, 1, &buff);
        alDeleteBuffers(1, &buff);
    }
    return YES;
}

- (void)playSound {
    alSourcePlay(outSourceId);
}

- (void)pauseSound {
    ALint  state;
    alGetSourcei(outSourceId, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING) {
        alSourcePause(outSourceId);
    }
}

- (void)stopSound {
    alSourcePause(outSourceId);
    alSourceStop(outSourceId);
    [self cleanUpOpenAL];
}

- (void)cleanUpOpenAL {
    int processed;
    
    alGetSourcei(outSourceId, AL_BUFFERS_PROCESSED, &processed);
    while(processed-- > 0) {
        alDeleteBuffers(1, &buff);
    }
}

@end
