//
//  IotlibTool.h
//  Audio_Demo
//
//  Created by mysoul on 2018/2/6.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import <Foundation/Foundation.h>

#define NCWeakObj(obj) __weak typeof(obj) obj##Weak = obj

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#import "header_def.h"
#import "iot_dev_sdk.h"
#import "iotsdk_if.h"
#pragma clang pop

#define CONNECTING          @"connecting"
#define CONNECTFAIL         @"connect_fail"
#define CONNECTSTOPPED      @"connect_stopped"
#define CONNECTSUCCESS      @"connect_success"
#define CONNECTDISMISS      @"connect_dismiss"
#define CONNECTNONETWORK    @"connect_noNetwork"
#define RECONNECTSUCCESS    @"reconnect_success"

typedef NS_ENUM(NSUInteger, RomUpdateMode) {
    RomUpdateBySystem = 1,
    RomUpdateByUser,
};

typedef void(^ConnectStatusChanged)(e_trans_conn_state status, int connectId);
typedef void(^SuccessCallback)(id sender);
typedef void(^FailureCallback)(NSString * error);

typedef enum : NSUInteger {
    connecting,
    connectFail,
    connectSuccess,
    connectStopped,
    connectDismiss,
    connectNoNetwork,
} ConnectStatus;

@protocol IotlibToolDelegate <NSObject>

@optional
/**
 开启音视频的数据代理

 @param connId id
 @param headerMedia 媒体头
 @param data_ptr 数据
 @param dataLen 数据长度
 */
- (void)onMediaDataWithConnId:(uint32_t)connId
                  headerMedia:(header_media_t *)headerMedia
                      dataPtr:(const char *)data_ptr
                      dataLen:(uint32_t)dataLen;


@end


@interface IotlibTool : NSObject

@property(nonatomic , assign)int conn_id;

/**
 设备的id
 */
@property (nonatomic,copy) NSString * deviceId;

/**
 是否需要获取当前设备的时区
 */
@property (nonatomic,assign) BOOL isFirst;

/**
 是否是云存储
 */
@property (nonatomic,assign) BOOL isClound;

/**
 当前设备所处的时区
 */
@property (nonatomic,copy) NSString * devicTimeZone;

/**
 tf时区
 */
@property (nonatomic,copy) NSString * tfTimeZone;

/**
 是否重置解码器
 */
@property (nonatomic,assign) BOOL isResetDcode;

/**
 老的时区
 */
@property (nonatomic,copy) NSString * oldTimeZone;

/**
 快进的速度
 */
@property (nonatomic,assign) NSInteger cloundQucikSpeed;

@property(nonatomic , weak)id<IotlibToolDelegate> delegate;

/**
 是否重置音频的解码
 */
@property (nonatomic,assign) BOOL isResetAudioDecode;

/**
 当天所有设备的视频
 */
@property (nonatomic,strong) NSMutableArray * recodeVideoArr;

/**
 单例

 @return self
 */
+ (instancetype)shareIotlibTool;

/**
 更换sdk用户

 @param userName 用户名
 @param pass 密码
 */
- (void)iotSdkSetUserName:(NSString *)userName pass:(NSString *)pass;

/**
 初始化sdk
 */
- (void)initIotSdkWithUuid:(NSString *)uuid;


#pragma mark - 获取视频流量传输
/**
 获取视频传输流量

 @param connId 连接id
 @return 速度
 */
- (int)getWholeSpeedWithConnId:(int)connId;

#pragma mark - settting断线重连
/**
 断线重连

 @param deviceId 设备id
 */
- (void)reconnectWhenSettingConnectBrokenWithDeviceId:(NSString *)deviceId callback:(ConnectStatusChanged)connectStatus;

/**
 获取当前连接id（本地的）

 @param deviceId 设备id
 @return 连接id
 */
- (void)getCurrentConnectIdWithDeviceId:(NSString *)deviceId callback:(ConnectStatusChanged)connectStatus;


#pragma mark - 音视频连接开启相关操作

/**
 获取连接id

 @param deviceId 设备id
 */
- (int)getConnIdWithDeviceId:(NSString *)deviceId;

/**
 连接服务

 @param connectStatus 连接状态回调  status状态
 */
- (void)startConnectWithDeviceId:(NSString *)deviceId callback:(ConnectStatusChanged)connectStatus;


/**
 开启视频

 @param connectId 连接id
 @param chno nvr/ipc      0、nvr    1、ipc
 @param videoQuality 视频质量
 @param startCallback 回调
 */
- (void)startVideoWithConnectId:(int)connectId chno:(int)chno videoQuality:(int)videoQuality callback:(void (^)(int status))startCallback;

/**
 开启音频连接

 @param startCallback 回调
 */
- (void)startAudioWithChannelID:(uint32_t )channelId Callback:(void (^)(int status,
                                         uint32_t audio_codec,
                                         uint32_t rate,
                                         uint32_t bit,
                                         uint32_t track))startCallback;


/**
 开启语音对话

 @param startCallback 成功回调
 */
- (void)startTalkWithChannelID:(uint32_t )channelId Callback:(void (^)(int status,
                                    uint32_t audio_codec,          //音频格式
                                    uint32_t rate,                 //采样率
                                    uint32_t bit))startCallback;   //位声

/**
 发送语音
 
 @param payload_ptr 音频数据
 @param payload_len 音频数据长度
 */
- (void)writeAudioDataWithPayloadPtr:(const char *)payload_ptr
                       payloadPtrLen:(int)payload_len
                          streamType:(e_stream_type)stream_type;

/**
 获取连接状态（通过deviceId）

 @return 连接状态
 */
- (e_trans_conn_state)getConnectStatusWithDeviceId:(NSString *)devId;

/**
 获取连接状态（通过connectId）
 
 @return 连接状态
 */
- (e_trans_conn_state)getConnectStatusWithConnId:(int)connId;

/**
 获取连接id
 
 @param devId 设备id
 @return 连接id
 */
- (int)getConnectIdWithDeviceId:(NSString *)devId;

/**
 中断连接
 */
- (void)stopConnect;

/**
 终端设备连接

 @param connectId 连接id
 */
- (void)stopConnectWithConnectId:(int)connectId complete:(void (^)(BOOL success))complete;

/**
 停止播放视频
 */
//- (int)stopVideoWithConnectId:(int)connectId complete:(SuccessCallback)complete;
- (int)stopVideoWithConnectId:(int)connectId chno:(int )chno complete:(SuccessCallback)complete;

/**
 停止播放音频
 */
//- (int)stopAudioWithComplete:(SuccessCallback)complete;
- (int)stopAudioWithChannelID:(uint32_t )channelId Complete:(SuccessCallback)complete;

/**
 停止对话
 */
//- (int)stopTalkbackWithComplete:(SuccessCallback)complete;
- (int)stopTalkbackWithChannelID:(uint32_t )channelId Complete:(SuccessCallback)complete;

@end
