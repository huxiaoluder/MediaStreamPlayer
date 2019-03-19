//
//  APAudioRender.h
//  ios_example
//
//  Created by 胡校明 on 2019/3/18.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include <AudioUnit/AudioUnit.h>
#import "FFCodecContext.hpp"
#import "APCodecContext.hpp"

using namespace MS;
using namespace MS::APhard;

NS_ASSUME_NONNULL_BEGIN

@interface APAudioRender : NSObject

@property (readonly) int currentChannels;
@property (readonly) int currentFrequency;

/**
 AudioUnit 渲染器
 不支持多路音频复合播放(不同于音乐 APP, p2p 摄像头使用场景稀少, 强行复合多路音频, 用户将无法分辨视频声音)

 @param channels 音频通道数
 @param frequency 音频采样率
 @return APAudioRender
 */
+ (instancetype)renderWithChannels:(int)channels frequency:(int)frequency;

/**
 更新渲染器配置

 @param channels 音频通道数
 @param frequency 音频采样率
 */
- (void)updateChannels:(int)channels frequency:(int)frequency;

/**
 渲染 FFmpeg 软解音频数据(LinearPCM)
 
 @param frame AVFrame
 */
- (void)displayAVFrame:(AVFrame &)frame;

/**
 渲染 AudioToolBox 解码音频数据(LinearPCM)
 
 @param frame APFrame
 */
- (void)displayAPFrame:(APFrame &)frame;

@end

NS_ASSUME_NONNULL_END
