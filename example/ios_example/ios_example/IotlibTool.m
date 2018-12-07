//
//  IotlibTool.m
//  Audio_Demo
//
//  Created by mysoul on 2018/2/6.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import "IotlibTool.h"

int g_conn_id;
e_app_state glAppState;
e_trans_conn_state glTransState;
static IotlibTool * iotlibTool;
SuccessCallback nvrOnlineStatusCallback;
ConnectStatusChanged statusChanged;


NSMutableDictionary * taskDict;
NSMutableDictionary * idDict;
NSMutableDictionary * reTaskDict;

bool isReconnect = false;

@interface IotlibTool()

@end

@implementation IotlibTool

+ (instancetype)shareIotlibTool{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        iotlibTool = [[IotlibTool alloc] init];
        taskDict = [NSMutableDictionary dictionary];
        reTaskDict = [NSMutableDictionary dictionary];
        idDict = [NSMutableDictionary dictionary];
    });
    return iotlibTool;
}

void logCallback(const char* log_content, int log_len){
//    DmLog(@"%@",[NSString stringWithCString:log_content encoding:NSUTF8StringEncoding]);
}

//初始化sdk
- (void)initIotSdkWithUuid:(NSString *)uuid{
    const char* arg_uuid      = [uuid UTF8String];
    
    iotsdk_init(true);
//#ifdef DEBUG
//    iotsdk_set_proto(1);
//#endif
    
    iotsdk_set_cb_server_ip_change(on_dns_server_change);
    iotsdk_set_cb_app_state_change(on_app_state_change);
    iotsdk_set_cb_on_media(on_media_data);
    iotsdk_set_cb_trans_state_change(on_transport_conn_state_change);
    iotsdk_user_set_wait_timeo(3*1000);
    iotsdk_set_log_cb(logCallback);
    
//    iotsdk_set_log_level(e_log_error);

//#ifdef IsDebugModel     //pch里面打开注释
    iotsdk_add_server_ip_addr("120.79.178.33", 5009);
//#else
//    iotsdk_add_server_ip_addr("120.77.150.206", 5009);
//    iotsdk_add_server_ip_addr("47.75.85.181", 5009);
//    iotsdk_add_server_ip_addr("47.254.33.237", 5009);
//    iotsdk_add_server_ip_addr("47.91.107.36", 5009);
//#endif
    
    iotsdk_user_set_app_verify_info("mktech", "2017mktech!@01");
    
    iotsdk_user_set_hardware_id(arg_uuid);
    
    iotsdk_start();
}

- (void)iotSdkSetUserName:(NSString *)userName pass:(NSString *)pass{
    const char* arg_user_name = [userName UTF8String];
    const char* arg_user_pass = [pass UTF8String];
    
    iotsdk_user_set_account_info(arg_user_name, arg_user_pass);
}

/**
 获取连接id
 
 @param deviceId 设备id
 */
- (int)getConnIdWithDeviceId:(NSString *)deviceId{
    return iotsdk_user_get_conn_id_by_name([deviceId UTF8String]);
}


/**
 获取视频传输流量
 
 @param connId 连接id
 @return 速度
 */
- (int)getWholeSpeedWithConnId:(int)connId{
    if (connId == 0) {
        connId = g_conn_id;
    }
    return iotsdk_get_whole_speed(connId);
}

/**
 dns状态改变回调
 
 @param addr 地址
 @return 状态
 */
int on_dns_server_change(const char* addr)
{
    printf("dns server change: %s \n", addr);
    return 0;
}

/**
 app状态改变回调
 
 @param state app状态
 */
void on_app_state_change(e_app_state state)
{
    if (state == e_state_iotgw_tcp_enter_loop || state == e_state_iotgw_udp_enter_loop)
    {
        NSLog(@"on_app_state_change");
    }else{
        NSLog(@"state 发生改变 %u",state);
    }
    
    glAppState = state;
}

/**
 媒体回调
 
 @param conn_id 连接id
 @param hdr_media 媒体头
 @param data_ptr 数据
 @param data_len 数据长度
 */
void on_media_data(uint32_t conn_id,
                   header_media_t *hdr_media,
                   const char *data_ptr,
                   uint32_t data_len)
{
    if ([iotlibTool.delegate respondsToSelector:@selector(onMediaDataWithConnId:headerMedia:dataPtr:dataLen:)]) {
        [iotlibTool.delegate onMediaDataWithConnId:conn_id headerMedia:hdr_media dataPtr:data_ptr dataLen:data_len];
    }
}







/**
 云存储状态回调

 @param status 状态
 @param err 错误信息
 */
void on_cloud_status_change(e_cloud_play_status status, e_cloud_error err){
    NSLog(@"e_cloud_play_status = %d\ne_cloud_error = %d", status, err);
}

//ipc全景nvr状态变化
void on_transport_conn_state_change(uint32_t conn_id,
                                    e_trans_conn_state state,
                                    const char* device_id,
                                    void* user_data)
{
    if (isReconnect){
        NSLog(@"重连变化 isReconnect");
        if (state == e_trans_conn_success){
            ConnectStatusChanged statusChanged = [reTaskDict objectForKey:[NSString stringWithUTF8String:device_id]];
            if (statusChanged){
                dispatch_async(dispatch_get_main_queue(), ^{
                    statusChanged(e_trans_conn_success, conn_id);
                    [reTaskDict removeObjectForKey:[NSString stringWithUTF8String:device_id]];
                });
            }
        }
        return;
    }
    
    ConnectStatus status;
    if (state == e_trans_conn_fail)
    {
        NSLog(@"e_trans_conn_fail 连接失败 : conn_id = %ld",(long)conn_id);
        status = connectFail;
        [[NSNotificationCenter defaultCenter] postNotificationName:CONNECTFAIL object:@{@"status":[NSValue value:&status withObjCType:@encode(ConnectStatus)],@"deviceId":[NSString stringWithUTF8String:device_id]}];
        
        ConnectStatusChanged statusChanged = [taskDict objectForKey:[NSString stringWithCString:device_id encoding:NSUTF8StringEncoding]];
        
        if (statusChanged) {
            statusChanged(state,conn_id);
        }
        
    }
    else if (state == e_trans_conn_success)
    {
        NSLog(@"e_trans_conn_success 连接成功 : conn_id = %ld",(long)conn_id);
        status = connectSuccess;
        [[NSNotificationCenter defaultCenter] postNotificationName:CONNECTSUCCESS object:@{@"status":[NSValue value:&status withObjCType:@encode(ConnectStatus)],@"deviceId":[NSString stringWithUTF8String:device_id],@"connID":@(conn_id)}];
        
        ConnectStatusChanged statusChanged = [taskDict objectForKey:[NSString stringWithCString:device_id encoding:NSUTF8StringEncoding]];
        
        if (statusChanged) {
            statusChanged(state,conn_id);
        }
        
        
    }
    else if (state == e_trans_conn_stoped)
    {
        NSLog(@"e_trans_conn_stoped 连接停止 : conn_id = %ld",(long)conn_id);
        status = connectStopped;
        [[NSNotificationCenter defaultCenter] postNotificationName:CONNECTSTOPPED object:@{@"status":[NSValue value:&status withObjCType:@encode(ConnectStatus)],@"deviceId":[NSString stringWithUTF8String:device_id]}];
        
        ConnectStatusChanged statusChanged = [taskDict objectForKey:[NSString stringWithCString:device_id encoding:NSUTF8StringEncoding]];
        
        if (statusChanged) {
            statusChanged(state,conn_id);
        }
        
    }else if (state == e_trans_conn_disconnect)
    {
        NSLog(@"e_trans_conn_disconnect 失去连接 : conn_id = %ld" ,(long)conn_id);
        status = connectDismiss;
        [[NSNotificationCenter defaultCenter] postNotificationName:CONNECTDISMISS object:@{@"status":[NSValue value:&status withObjCType:@encode(ConnectStatus)],@"deviceId":[NSString stringWithUTF8String:device_id],@"connID":@(conn_id)}];
        
        ConnectStatusChanged statusChanged = [taskDict objectForKey:[NSString stringWithCString:device_id encoding:NSUTF8StringEncoding]];
        
        if (statusChanged) {
            statusChanged(state,conn_id);
            [taskDict removeObjectForKey:[NSString stringWithCString:device_id encoding:NSUTF8StringEncoding]];
        }
    }else if (state == e_trans_conn_connecting){
        NSLog(@"e_trans_conn_connecting 连接中 : conn_id = %ld",(long)conn_id);
        status = connecting;
        [[NSNotificationCenter defaultCenter] postNotificationName:CONNECTING object:@{@"status":[NSValue value:&status withObjCType:@encode(ConnectStatus)],@"deviceId":[NSString stringWithUTF8String:device_id]}];
        
        ConnectStatusChanged statusChanged = [taskDict objectForKey:[NSString stringWithCString:device_id encoding:NSUTF8StringEncoding]];
        
        if (statusChanged) {
            statusChanged(state,conn_id);
        }
    }
}

//云存储状态变化
void on_cloud_storgae_play_status(e_cloud_play_status status, e_cloud_error err){
    
}


void stopAll(){
    NSLog(@"停止所有 stopAll");
    iotsdk_user_stop_video(g_conn_id, 1);
    iotsdk_user_stop_audio(g_conn_id, 1);
    iotsdk_user_stop_talkback(g_conn_id, 1);
}

#pragma mark - 音视频连接开启相关操作

/**
 断线重连
 
 @param deviceId 设备id
 */
- (void)reconnectWhenSettingConnectBrokenWithDeviceId:(NSString *)deviceId callback:(ConnectStatusChanged)connectStatus{
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        isReconnect = true;
        g_conn_id = iotsdk_user_start_conn_dev([deviceId UTF8String], (void*)0);
        if (g_conn_id > -1){
            NSLog(@"old conn id -------------%@",[idDict objectForKey:deviceId]);
            NSLog(@"start conn ok! conn id: %d", g_conn_id);
            [idDict setObject:@(g_conn_id) forKey:deviceId];
        }else{
            NSLog(@"start conn fail! err code: %d", g_conn_id);
        }
    });
}

/**
 获取当前连接id（本地的）
 
 @param deviceId 设备id
 @return 连接id
 */
- (void)getCurrentConnectIdWithDeviceId:(NSString *)deviceId callback:(ConnectStatusChanged)connectStatus{
    e_trans_conn_state state = [self getConnectStatusWithDeviceId:deviceId];
    if (state != e_trans_conn_success) {
        [self reconnectWhenSettingConnectBrokenWithDeviceId:deviceId callback:connectStatus];
    }else if (state == e_trans_conn_success){
        if (connectStatus){
            int connectId = [self getConnectIdWithDeviceId:deviceId];
            connectStatus(e_trans_conn_success,connectId);
        }
    }
    if (connectStatus) {
        [reTaskDict setObject:connectStatus forKey:deviceId];
    }
}


//连接服务
- (void)startConnectWithDeviceId:(NSString *)deviceId callback:(ConnectStatusChanged)connectStatus{
    
    NCWeakObj(self);
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        isReconnect = false;
        e_trans_conn_state status = [self getConnectStatusWithDeviceId:deviceId];
        if (status == e_trans_conn_success) {
            NSNumber * connectId = [idDict objectForKey:deviceId];
            NSLog(@"Already connected conn id: %d", connectId.intValue);
            statusChanged = connectStatus;
            if (connectStatus) {
                connectStatus(e_trans_conn_success,connectId.intValue);
            }
            return;
        }
        
        g_conn_id = iotsdk_user_start_conn_dev([deviceId UTF8String], (void*)0);
        if (g_conn_id > -1){
            NSLog(@"start conn ok! conn id: %d", g_conn_id);
            [idDict setObject:@(g_conn_id) forKey:deviceId];
        }
        else{
            NSLog(@"start conn fail! err code: %d", g_conn_id);
        }
        
        NSNumber * connect_id = [idDict objectForKey:deviceId];
        selfWeak.conn_id = connect_id.intValue;
        
        if (connectStatus) {
            [taskDict setObject:connectStatus forKey:deviceId];
        }
        
//        statusChanged = connectStatus;
    });
}

//开启视频连接
- (void)startVideoWithConnectId:(int)connectId chno:(int)chno videoQuality:(int)videoQuality callback:(void (^)(int status))startCallback{
    g_conn_id = connectId==0?g_conn_id:connectId;
    _conn_id = g_conn_id;
    videoQuality = videoQuality==0?500:videoQuality;
    NSLog(@"开启视频连接");
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        uint32_t fps;
        uint32_t resolution;
        int ret = iotsdk_user_start_video(g_conn_id, chno, 1, videoQuality, 0, &fps, &resolution);
        NSLog(@"start video ret: %d \n", ret);
        NSLog(@"开始视频 conn_id = %d",g_conn_id);
//        dispatch_async(dispatch_get_main_queue(), ^{
            if (startCallback) {
                startCallback(ret);
            }
//        });
    });
}

//开启音频连接
- (void)startAudioWithChannelID:(uint32_t )channelId Callback:(void (^)(int, uint32_t, uint32_t, uint32_t, uint32_t))startCallback{
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        uint32_t audio_codec;
        uint32_t rate;
        uint32_t bit;
        uint32_t track;
        int ret = iotsdk_user_start_audio(g_conn_id,
                                          channelId,
                                          &audio_codec,
                                          &rate,
                                          &bit,
                                          &track);
        NSLog(@"start audio ret: %d \n", ret);
//        dispatch_async(dispatch_get_main_queue(), ^{
            if (startCallback) {
                startCallback(ret,audio_codec,rate,bit,track);
            }
//        });
    });
}

//开启语音对话
- (void)startTalkWithChannelID:(uint32_t )channelId Callback:(void (^)(int status, uint32_t audio_codec, uint32_t rate, uint32_t bit))startCallback{
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        uint32_t audio_codec;
        uint32_t rate;
        uint32_t bit;
        int ret = iotsdk_user_start_talkback(g_conn_id,
                                             channelId,
                                             &audio_codec,
                                             &rate,
                                             &bit);
        NSLog(@"start talk ret: %d \n", ret);
//        dispatch_async(dispatch_get_main_queue(), ^{
            if (startCallback) {
                startCallback(ret,audio_codec,rate,bit);
            }
//        });
    });
}

/**
 发送语音

 @param payload_ptr 音频数据
 @param payload_len 音频数据长度
 */
- (void)writeAudioDataWithPayloadPtr:(const char *)payload_ptr payloadPtrLen:(int)payload_len streamType:(e_stream_type)stream_type{
    int64_t time = [[IotlibTool getNowTimeTimestamp3] integerValue];
    iotsdk_write_media_data(g_conn_id, e_pkt_audio, stream_type, 0, 0, 0, time, 0, payload_ptr, payload_len);
}


//获取当前时间戳  （以毫秒为单位）

+(NSString *)getNowTimeTimestamp3{
    NSDateFormatter *formatter = [[NSDateFormatter alloc] init] ;
    [formatter setDateStyle:NSDateFormatterMediumStyle];
    [formatter setTimeStyle:NSDateFormatterShortStyle];
    [formatter setDateFormat:@"YYYY-MM-dd HH:mm:ss SSS"]; // ----------设置你想要的格式,hh与HH的区别:分别表示12小时制,24小时制
    //设置时区,这个对于时间的处理有时很重要
    NSTimeZone * timeZone = [NSTimeZone systemTimeZone];
    [formatter setTimeZone:timeZone];
    
    NSTimeInterval nowtime = [[NSDate date] timeIntervalSince1970]*1000;
    long long theTime = [[NSNumber numberWithDouble:nowtime] longLongValue];
    NSString *curTime = [NSString stringWithFormat:@"%llu",theTime];
    return curTime;
}

- (int)stopVideoWithConnectId:(int)connectId chno:(int )chno complete:(SuccessCallback)complete{
    NSLog(@"停止视频 连接conn_id = %d",connectId);
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
//        e_trans_conn_state status = iotsdk_user_get_conn_state_by_id(g_conn_id);
//        if (status == e_trans_conn_success) {
        int ret = iotsdk_user_stop_video(connectId!=0?connectId:g_conn_id, chno);
            dispatch_async(dispatch_get_main_queue(), ^{
                if (complete) {
                    complete(@(ret));
                }
                NSLog(@"stopVideoSuccess ++++++++++++++++++++++++++ %d",ret);
            });
//        }
    });
    return 0;
}

- (int)stopAudioWithChannelID:(uint32_t )channelId Complete:(SuccessCallback)complete{
    NSLog(@"停止音频 连接conn_id = %d",g_conn_id);
    self.isResetAudioDecode = YES;
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
//        e_trans_conn_state status = iotsdk_user_get_conn_state_by_id(g_conn_id);
//        if (status == e_trans_conn_success) {
            iotsdk_user_stop_audio(g_conn_id, channelId);
            dispatch_async(dispatch_get_main_queue(), ^{
                if (complete) {
                    complete(@(0));
                }
            });
//        }
    });
    return 0;
}

- (int)stopTalkbackWithChannelID:(uint32_t )channelId Complete:(SuccessCallback)complete{
    NSLog(@"停止语音 连接conn_id = %d",g_conn_id);
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
//        e_trans_conn_state status = iotsdk_user_get_conn_state_by_id(g_conn_id);
//        if (status == e_trans_conn_success) {
            int ret = iotsdk_user_stop_talkback(g_conn_id, channelId);
            dispatch_async(dispatch_get_main_queue(), ^{
                if (complete) {
                    complete(@(ret));
                }
            });
//        }
    });
    return 0;
}

/**
 获取连接状态

 @return 连接状态
 */
- (e_trans_conn_state)getConnectStatusWithDeviceId:(NSString *)devId{
   return iotsdk_user_get_conn_state_by_name([devId UTF8String]);
}

/**
 获取连接状态
 
 @return 连接状态
 */
- (e_trans_conn_state)getConnectStatusWithConnId:(int)connId{
    return iotsdk_user_get_conn_state_by_id(connId);
}

/**
 获取连接id

 @param devId 设备id
 @return 连接id
 */
- (int)getConnectIdWithDeviceId:(NSString *)devId{
    return iotsdk_user_get_conn_id_by_name([devId UTF8String]);
}

/**
 中断连接
 */
- (void)stopConnect{
    NSLog(@"中断连接");
    iotsdk_user_stop_video(g_conn_id, 1);
    iotsdk_user_stop_audio(g_conn_id, 1);
    iotsdk_user_stop_talkback(g_conn_id, 1);
    
    g_conn_id = 0;
    glAppState = e_state_init;
    glTransState = e_trans_conn_disconnect;
}

//终端连接
- (void)stopConnectWithConnectId:(int)connectId complete:(void (^)(BOOL success))complete{
    connectId = connectId == 0?g_conn_id:connectId;
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
//        int ret = iotsdk_stop_conn(connectId);
        int ret = 0;
        dispatch_async(dispatch_get_main_queue(), ^{
            if (complete) {
                complete(ret == 0);
            }
        });
    });
}

@end

@implementation AlarmInfomations

- (void)setTimeZone:(NSString *)timeZone {
    if (timeZone.length > 0) {
        _timeZone = timeZone;
    } else {
        _timeZone = @"";
    }
}

- (void)setNtpServer1:(NSString *)ntpServer1 {
    if (ntpServer1.length > 0) {
        _ntpServer1 = ntpServer1;
    } else {
        _ntpServer1 = @"";
    }
}

- (void)setNtpServer2:(NSString *)ntpServer2 {
    if (ntpServer2.length > 0) {
        _ntpServer2 = ntpServer2;
    } else {
        _ntpServer2 = @"";
    }
}



@end
