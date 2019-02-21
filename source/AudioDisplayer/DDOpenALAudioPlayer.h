//
//  DDOpenALAudioPlayer.h
//  SmartHome
//
//  Created by mysoul on 2018/8/14.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DDOpenALAudioPlayer : NSObject
+(id)sharePalyer;

/**
 *  播放
 *
 *  @param data       数据
 *  @param dataSize   长度
 *  @param samplerate 采样率
 *  @param channels   通道
 *  @param bit        位数
 */
-(void)openAudioFromQueue:(uint8_t *)data dataSize:(size_t)dataSize samplerate:(int)samplerate channels:(int)channels bit:(int)bit;

/**
 *  停止播放
 */
-(void)stopSound;
@end
