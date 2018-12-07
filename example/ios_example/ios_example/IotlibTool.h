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

@class AlarmInfomations;

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

#ifdef DEBUG
/**
 DEBUG 状态下, 动态设置 log 等级
 */
- (void)setIotLogLevel;
#endif

#pragma mark - 获取预览图
/**
 获取预览图

 @param connId 连接id
 @param chno 通道
 @param success 成功回调  {@"imageData":imageData,@"format":format}   format:1、png 2、jpg
 @param failure 失败回调
 */
- (void)userGetScreenWithConnId:(int)connId
                           chno:(int)chno
                        success:(SuccessCallback)success
                        failure:(FailureCallback)failure;

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

#pragma mark - ap模式
/**
 获取本地设备

 @param complete 本地设备数组
 */
- (void)userGetLocalDeviceComplete:(SuccessCallback)complete;

/**
 开始本地设备连接

 @param deviceId 设备id
 @param localIp 本地ip
 @param port 端口
 @param userData 自定义对象
 @param complete 自定义对象
 */
- (void)userStartConnDevLocalWithDeviceId:(NSString *)deviceId localIp:(NSString *)localIp port:(uint16_t)port userData:(void *)userData complete:(SuccessCallback)complete;

/**
 开连连接设备端(只通过本地连接)

 @param deviceId 设备Id
 @param lanIp 局域网中设备的ip地址
 @param lanPort 局域网中设备的端口
 @param userName 用于局域网验证的用户名
 @param passWord 用于局域网验证的密码
 @param userData 自定义对象
 @param complete < 0 error, > 0 conn id
 */
- (void)userStartConnDevLanWithDeviceId:(NSString *)deviceId lanIp:(NSString *)lanIp lanPort:(uint16_t)lanPort userName:(NSString *)userName passWord:(NSString *)passWord userData:(void *)userData complete:(SuccessCallback)complete;

/**
 根据是否登录选择不同的连接设备端方法

 @param deviceId 设备Id
 @param lanIp 局域网中设备的ip地址
 @param lanPort 局域网中设备的端口
 @param userLogined 用户是否已经登录,YES:已登录,NO:未登录
 @param complete < 0 error, > 0 conn id
 */
- (void)userStartConnDevWithDeviceId:(NSString *)deviceId lanIp:(NSString *)lanIp lanPort:(uint16_t)lanPort userLogined:(BOOL)userLogined complete:(SuccessCallback)complete;


#pragma mark - 云存储
/**
 开启/关闭云存储功能(开启仅需要调用一次)

 @param enable 开启或关闭
 @param complete 回调
 */
- (void)userCloudStorageEnable:(bool)enable complete:(SuccessCallback)complete;

/**
 设置云存储token

 @param complete 回调
 */
- (void)userCloudStorageAccessToken:(NSString *)accessToken complete:(SuccessCallback)complete;

/**
 播放云视频

 @param deviceId 设备id
 @param chno 通道
 @param timestamp 播放开始时间
 @param timeOffeSet 偏移量
 @param complete 回调 
 */
- (void)userCloudStoragePlayWithCloudPlayType:(e_cloud_play_type)playType
                                     deviceId:(NSString *)deviceId
                                    chno:(int)chno
                               timestamp:(uint64_t)timestamp
                           cloudFileName:(NSString *)cloudFileName
                                  timeOffeSet:(uint64_t)offset
                                complete:(SuccessCallback)complete;
/**
 停止云存储

 @param complete 回调
 */
- (void)userCloudStorageStopWithDeviceId:(NSString *)deviceId
                                    chno:(int)chno
                                complete:(SuccessCallback)complete;

/**
 解密云存储文件同时播放文件
 
 @param data 文件数据
 @param dataLen 数据长度
 @param key 密钥
 @param keyLen 密钥长度
 @param playCtrl true
 @param cb 数据回调
 @param complete 方法回调
 */
- (void)userCloudDataDecodeWithData:(NSData *)data dataLen:(int)dataLen aesKey:(NSString *)key keyLen:(int)keyLen playCtrl:(bool)playCtrl callBack:(cb_on_media_data)cb complete:(SuccessCallback)complete;


/**
 终止播放云存储消息视频

 @param complete 回调
 */
- (void)userCloudInterrupteToStopWithComplete:(SuccessCallback)complete;

#pragma mark - 录像回放

/**
 获取本地录像记录

 @param connId 连接id
 @param chno 通道数
 @param day 请求前多少天的数据
 @param success 成功回调
 @param failure 失败回调
 */
- (void)getUserRecListDayWithConnId:(int)connId
                               chno:(int)chno
                            dayList:(int)day
                            success:(SuccessCallback)success
                            failure:(FailureCallback)failure;

/**
 获取当天录像列表

 @param connId 连接ID
 @param chno 通道数
 @param dateDay 获取这一天的录像记录 "20180101"
 @param lastTime 最后获得的时间 [ 单位秒 ]
 @param getType default = 2  1 表示取下一页(next)数据,取从last_time到最新的记录使用该值;2 表示向后(prev)数据,取比last_time更早的记录使用该值
 @param getNum default = 30 取多少条数据
 @param success 成功回调
 @param failure 失败回调
 */
- (void)getUserRecListWithConnId:(int)connId
                            chno:(int)chno
                         dateDay:(NSString *)dateDay
                        lastTime:(NSInteger)lastTime
                         getType:(int)getType
                          getNum:(int)getNum
                         success:(SuccessCallback)success
                         failure:(FailureCallback)failure;

/**
 播放录像

 @param connId 连接id
 @param chno 通道数
 @param timeStamp 开始播放的时间
 @param success 成功回调
 @param failure 失败回调
 */
- (void)startPlayRecordWithConnId:(int)connId
                             chno:(int)chno
                          dateDay:(NSString *)dateDay
                        timeStamp:(NSInteger)timeStamp
                          success:(SuccessCallback)success
                          failure:(FailureCallback)failure;
/**
 结束录像
 
 @param connId 连接id
 @param chno 通道数
 @param success 成功回调
 @param failure 失败回调
 */
- (void)stopPlayRecordWithConnId:(int)connId
                             chno:(int)chno
                          success:(SuccessCallback)success
                          failure:(FailureCallback)failure;

/**
 暂停/播放

 @param connId 连接id
 @param chno 通道
 @param isPause 是否暂停 YES暂停 NO播放
 @param success 成功回调
 @param failure 失败回调
 */
- (void)pauseOrPlayRecWithConnId:(int)connId
                            chno:(int)chno
                         isPause:(BOOL)isPause
                         success:(SuccessCallback)success
                         failure:(FailureCallback)failure;

#pragma mark - 播放画面属性设置
/**
 设置画面属性

 @param connId 连接id
 @param chno 通道
 @param flipType 0:正置, 1:水平翻转，2:垂直翻转，3:180°翻转
 */
- (void)userSetFlipWithConnId:(int)connId
                         chno:(int)chno
                     flipType:(int)flipType
                      success:(SuccessCallback)success
                      failure:(FailureCallback)failure;

/**
 设置画面清晰度

 @param connId 连接id
 @param chno 通道
 @param videoQuality 视频质量 1-100;
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetVideoWithConnId:(int)connId
                          chno:(int)chno
                  videoQuality:(int)videoQuality
                       success:(SuccessCallback)success
                       failure:(FailureCallback)failure;

#pragma mark - 摄像头转向设置
- (void)userPtzCtrl:(uint32_t)connect
               chno:(uint32_t)chno
          videoType:(e_video_ptz_ctrl)type
              para1:(uint32_t)para1
              para2:(uint32_t)para2
            success:(SuccessCallback)success
            failure:(FailureCallback)failure;

#pragma mark - 设备信息设置

/**
 设置报警

 @param connId 连接ID
 @param chno 通道数. 0 合成通道；1-N为各个独立通道, IPC该值为0
 @param motionDetection 0 标示关闭移动侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高
 @param opensoundDetection 0 关闭声音侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高
 @param openi2oDetection 红外体感
 @param smokeDetection 烟雾感应
 @param shadowDetection 遮蔽感应
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetAlarmWithConnId:(int)connId
                          chno:(int)chno
               motionDetection:(int)motionDetection
            opensoundDetection:(int)opensoundDetection
              openi2oDetection:(int)openi2oDetection
                smokeDetection:(int)smokeDetection
               shadowDetection:(int)shadowDetection
            humanBodyDetection:(int)humanBodyDetection
               cryingDetection:(int)cryingDetection
                       success:(SuccessCallback)success
                       failure:(FailureCallback)failure;

/**
 获取报警信息
 
 @param connId 连接ID
 @param chnoOrg 通道数. 0 合成通道；1-N为各个独立通道, IPC该值为0
 @param success 成功回调 AlarmInfomations类
 @param failure 失败回调
 */
- (void)userGetAlarmWithConnId:(int)connId
                       chnoOrg:(int)chnoOrg
                       success:(SuccessCallback)success
                       failure:(FailureCallback)failure;


/**
 时间设置

 @param connId 连接id
 @param chno 通道
 @param nowTime 当前时间戳
 @param timeZone 时区
 @param timeOffset 时间偏移,单位分钟: 480, -60
 @param ntpServer1 ntpServer1
 @param ntpServer2 ntpServer2
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetTimeWithConnId:(int)connId
                         chno:(int)chno
                      nowTime:(int64_t)nowTime
                     timeZone:(NSString *)timeZone
                   timeOffset:(int)timeOffset
                   ntpServer1:(NSString *)ntpServer1
                   ntpServer2:(NSString *)ntpServer2
                      success:(SuccessCallback)success
                      failure:(FailureCallback)failure;

/**
 获取时间(参考设置时间)

 @param connId 连接ID
 @param chno 通道数
 @param success 成功回调 AlarmInfomations类
 @param failure 失败回调
 */
- (void)userGetTimeithConnId:(int)connId
                        chno:(int)chno
                     success:(SuccessCallback)success
                     failure:(FailureCallback)failure;

/**
 设置电源频率

 @param connId 连接ID
 @param chno 通道数
 @param freq 50 50HZ, 60 60HZ
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetPowerfreqWithConnId:(int)connId
                              chno:(int)chno
                              freq:(int)freq
                           success:(SuccessCallback)success
                           failure:(FailureCallback)failure;

/**
 获取电源频率

 @param connId 连接ID
 @param chno 通道数
 @param success 成功回调 AlarmInfomations类
 @param failure 失败回调
 */
- (void)userGetPowerfreqWithConnId:(int)connId
                              chno:(int)chno
                           success:(SuccessCallback)success
                           failure:(FailureCallback)failure;

/**
 获取本地局域网搜索到的设备ID

 @param success 成功回调 AlarmInfomations类
 @param failure 失败回调
 */
- (void)userGetLocalDevicesSuccess:(SuccessCallback)success
                           failure:(FailureCallback)failure;

/**
 获取全景设备矫正参数

 @param conn_id 连接ID
 @param ch_no 通道ID
 @return
 */

/**
 获取全景设备矫正参数

 @param conn_id 连接ID
 @param ch_no 通道ID
 @param success 成功回调。block中参数为矫正参数数组。[0]:圆心X  [1]:圆心Y。 [2]:半径
 @param failure 失败回调
 */
-(void)getPanoCorrectWithChnnID:(int)conn_id
                           chno:(int )ch_no
                        Success:(SuccessCallback)success
                        failure:(FailureCallback)failure;

/**
 获取LED灯状态

 @param conn_id 连接id
 @param ch_no 通道
 @param success 成功回调 返回状态 1、2
 @param failure 失败回调
 */
- (void)userGetLedWithConnId:(int)conn_id
                        chno:(int)ch_no
                     Success:(SuccessCallback)success
                     failure:(FailureCallback)failure;

/**
 设置LED灯状态

 @param conn_id 连接id
 @param ch_no 通道
 @param flag 1 常开；2 常关；
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetLedWithConnId:(int)conn_id
                        chno:(int)ch_no
                        flag:(int)flag
                     Success:(SuccessCallback)success
                     failure:(FailureCallback)failure;

/**
 获取夜视状态

 @param conn_id 连接id
 @param ch_no 通道
 @param success 成功回调 返回状态  1 常开；2 常关； 3 自动
 @param failure 失败回调
 */
- (void)userGetIrlightWithConnId:(int)conn_id
                            chno:(int)ch_no
                         Success:(SuccessCallback)success
                         failure:(FailureCallback)failure;
/**
 设置夜视状态
 
 @param conn_id 连接id
 @param ch_no 通道
 @param flag 1 常开；2 常关； 3 自动
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetIrlightWithConnId:(int)conn_id
                            chno:(int)ch_no
                            flag:(int)flag
                         Success:(SuccessCallback)success
                         failure:(FailureCallback)failure;

#pragma mark - OSD配置
/**
 * 设置OSD显示字符串
 * @param conn_id                   [in] 连接ID
 * @param ch_no                     [in] 通道数.
 * @param chan_name_show            [in] 0 不现实通道名; 1 显示通道名
 * @param show_name_x               [in] (x,y)计算方式: 左上角为0,0为起点，x,y 以百分比表示，百分比从0-100之间.
 * @param show_name_y               [in]
 * @param datetime_show             [in] 0 不显示时间； 1 显示时间.
 * @param show_datetime_x           [in]
 * @param show_datetime_y           [in]
 * @param show_format               [in] 0: XXXX-XX-XX 年月日 ; 1: XX-XX-XXXX 月日年 ; 2: XXXX年XX月XX日; 3: XX月XX日XXXX年; 4: XX-XX-XXXX 日月年; 5: XX日XX月XXXX年
 * @param hour_format               [in] OSD小时制:0-24小时制,1-12小时制
 * @param show_week                 [in] 0 不显示； 1 显示
 * @param datetime_attr             [in] OSD属性:透明，闪烁 (保留)
 * @param custom1_show              [in] 自定义显示字符: 0 不显示； 1 显示
 * @param show_custom1_str          [in]
 * @param show_custom1_x            [in]
 * @param show_custom1_y            [in]
 * @param custom2_show              [in] 自定义显示字符: 0 不显示； 1 显示
 * @param show_custom2_str          [in]
 * @param show_custom2_x            [in]
 * @param show_custom2_y            [in]
 */
- (void)userSetOsdWithConnId:(int)conn_id
                        chno:(int)ch_no
              chan_name_show:(int)chan_name_show
                 show_name_x:(int)show_name_x
                 show_name_y:(int)show_name_y
          show_name_location:(int)show_name_location
               datetime_show:(int)datetime_show
             show_datetime_x:(int)show_datetime_x
             show_datetime_y:(int)show_datetime_y
                 show_format:(int)show_format
                 hour_format:(int)hour_format
                   show_week:(int)show_week
               datetime_attr:(int)datetime_attr
      show_datetime_location:(int)show_datetime_location
                custom1_show:(int)custom1_show
              show_custom1_x:(int)show_custom1_x
              show_custom1_y:(int)show_custom1_y
       show_custom1_location:(int)show_custom1_location
                custom2_show:(int)custom2_show
              show_custom2_x:(int)show_custom2_x
              show_custom2_y:(int)show_custom2_y
            show_custom1_str:(NSString *)show_custom1_str
            show_custom2_str:(NSString *)show_custom2_str
       show_custom2_location:(int)show_custom2_location
                     Success:(SuccessCallback)success
                     failure:(FailureCallback)failure;

/**
 获取osd设置

 @param conn_id 连接id
 @param ch_no 连接通道
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userGetOsdWithConnId:(int)conn_id
                        chno:(int)ch_no
                     Success:(SuccessCallback)success
                     failure:(FailureCallback)failure;

#pragma mark - 周界报警

/**
 获取周界设定

 @param conn_id 连接ID
 @param ch_no 通道数
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userGetBcWithConnId:(int)conn_id
                      ch_no:(int)ch_no
                    success:(SuccessCallback)success
                    failure:(FailureCallback)failure;

/**
 设置周界

 @param conn_id 连接ID
 @param ch_no 通道数
 @param infoArr 周界数组
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userSetBcWithConnId:(int)conn_id
                      ch_no:(int)ch_no
                    infoArr:(NSMutableArray<AlarmInfomations *> *)infoArr
                    success:(SuccessCallback)success
                    failure:(FailureCallback)failure;


#pragma mark - 录像计划
/**
 获得本地存储计划

 @param connId 连接ID
 @param chno 通道数
 @param success 成功回调    AlarmInfomations
 @param failure 失败回调
 */
- (void)userRecPlanGetWithConnId:(int)connId
                            chno:(int)chno
                         Success:(SuccessCallback)success
                         failure:(FailureCallback)failure;

/**
 设置本地存储计划

 @param connId 连接ID
 @param chno 通道数
 @param recType 1: 24小时录像; 2: 告警录像(图像变化时录制)
 @param recordNo 录像计划编号,可以是多个录像计划 [1-3]
 @param week [数字只能是1-7,分别代表周一到周日]
 @param startTime 开始时间.(从0点开始的秒数)
 @param endTime 结束时间.(从0点开始的秒数)
 @param planStatus 状态： 0关闭；1开启
 @param success 成功回调
 @param failure 失败回调
 */
- (void)userRecPlanSetWithConnId:(int)connId
                            chno:(int)chno
                         recType:(int)recType
                        recordNo:(int)recordNo
                            week:(NSMutableArray<NSNumber *> *)week
                       startTime:(int)startTime
                         endTime:(int)endTime
                      planStatus:(int)planStatus
                         Success:(SuccessCallback)success
                         failure:(FailureCallback)failure;

#pragma mark - 固件升级
/**
 设置设备固件升级模式

 @param connectId 设备连接 ID
 @param chno  通道
 @param mode 升级模式
 */
- (void)romModeSetWithConnectId:(int)connectId
                              chno:(int)chno
                              mode:(RomUpdateMode)mode
                        success:(SuccessCallback)success
                        failure:(FailureCallback)failure;;

/**
 获取设备固件升级模式

 @param connectId 设备连接 ID
 @param chno  通道
 */
- (void)romModeGetWithConnectId:(int)connectId
                           chno:(int)chno
                        success:(SuccessCallback)success
                        failure:(FailureCallback)failure;

/**
 升级设备固件版本

 @param connectId 设备连接 ID
 */
- (void)romUpdateWithConnectId:(int)connectId
             romUpdateProgress:(cb_on_rom_update_progress)romUpdateProgress
                       success:(SuccessCallback)success
                       failure:(FailureCallback)failure;

#pragma mark - storage
/**
 获取存储设备状态
 
 @param connectId   连接ID
 @param chno        通道数.
 */
- (void)getStorageStatusWithConnectId:(uint32_t)connectId
                                 chno:(uint32_t)chno
                              success:(SuccessCallback)success
                              failure:(FailureCallback)failure;

/**
 格式化设备端存储设备
 
 @param connectId   连接ID
 @param chno        通道数
 */
- (void)formatStorageWithConnectId:(uint32_t)connectId
                              chno:(uint32_t)chno
                           success:(SuccessCallback)success
                           failure:(FailureCallback)failure;

/**
 获取格式化执行百分比

 @param connectId 连接ID
 @param chno 通道数
 */
- (void)getFormatStorageRateWithConnectId:(uint32_t)connectId
                                     chno:(uint32_t)chno
                                  success:(SuccessCallback)success
                                  failure:(FailureCallback)failure;

/**
 设置Nvr/dvr 出视频的通道

 @param connectionId 设备id
 @param chnoCount 通道数
 @param arr 需要出视频的通道的数组
 @param success 成功
 @param failure 失败
 */
-(void)changeChnoconnectionId:(int)connectionId
          chnoCount:(int)chnoCount
            chnoArr:(NSArray<NSNumber *> *)arr
            success:(SuccessCallback)success
            failure:(FailureCallback)failure;


/**
 获取nvr设备在线状态

 @param connectionId 连接id
 @param success 成功回调
 @param failure 失败回调
 */
- (void)getNVROnlineStatusConnectionId:(int)connectionId
                                           success:(SuccessCallback)success
                                           failure:(FailureCallback)failure;

/**
 获取tf卡的状态

 @param conId 连接id
 @param chno 通道数
 @param success 成功
 @param failure 失败
 */
-(void)getTfCardStatusConectionId:(int)conId
                             chno:(int)chno
                          success:(SuccessCallback)success
                          failure:(FailureCallback)failure;


/**
 监听NVR设备在线情况

 @param success 回调
 */
- (void)setNVROnlineCallback:(SuccessCallback)success;


/**
 * 订阅主题
 * @param msg_id                [out] 返回消息ID(可用于判断订阅是否成功, iotsdk_set_iotgw_msg_cb注册的回调)
 * @param topic                 [in] 主题
 * @return
 */
- (void)subscribeTopic:(NSString *)topic
               success:(SuccessCallback)success
               failure:(FailureCallback)failure;


/**
 * 取消订阅
 * @param msg_id                [out] 返回消息ID(可用于判断取消订阅是否成功, iotsdk_set_iotgw_msg_cb注册的回调)
 * @param topic                 [in] 主题
 * @return
 */
- (void)unSubscribeTopic:(NSString *)topic
               success:(SuccessCallback)success
               failure:(FailureCallback)failure;

/**
 改变播放速率

 @param index 速率
 @param success 成功
 @param failure 失败
 */
-(void)setplayRate:(float)index
           success:(SuccessCallback)success
           failure:(FailureCallback)failure;

@end

@interface AlarmInfomations : NSObject

#pragma mark - 报警信息
/**
 通道数
 */
@property(nonatomic , assign)int chno;
/**
 0 标示关闭移动侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高
 */
@property(nonatomic , assign)int motionDetection;
/**
 0 关闭声音侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高
 */
@property(nonatomic , assign)int opensoundDetection;
/**
 红外体感
 */
@property(nonatomic , assign)int openi2oDetection;
/**
 烟雾感应
 */
@property(nonatomic , assign)int smokeDetection;
/**
 遮蔽感应
 */
@property(nonatomic , assign)int shadowDetection;
/**
 人形侦测
 */
@property(nonatomic, assign)int humanBodyDetection;
/**
 crying_detection
 */
@property(nonatomic, assign)int cryingDetection;
/**
 报警状态
 */
@property ( nonatomic, assign) BOOL status;

#pragma mark - 时间信息
/**
 当前时间
 */
@property(nonatomic , assign)int64_t nowTime;
/**
 时区
 */
@property(nonatomic , copy)NSString * timeZone;
/**
 时间偏移
 */
@property(nonatomic , assign)int timeOffset;
/**
 ntp1
 */
@property(nonatomic , copy)NSString * ntpServer1;
/**
 ntp2
 */
@property(nonatomic , copy)NSString * ntpServer2;

#pragma mark - 电源频率

/**
 电源频率
 */
@property(nonatomic , assign)int freq;

#pragma mark - loca_device_info
/**
 本地设备id
 */
@property(nonatomic , copy)NSString * localDeviceId;
/**
 本地id
 */
@property(nonatomic , copy)NSString * localIp;
/**
 序列号
 */
@property(nonatomic , copy)NSString * sn;
/**
 掩码
 */
@property(nonatomic , copy)NSString * netmask;
/**
 监听的端口
 */
@property(nonatomic , assign)NSInteger listenPort;
/**
 监听的协议
 */
@property(nonatomic , assign)NSInteger listenProto;
/**
 api版本
 */
@property(nonatomic , copy)NSString * versionApi;
/**
 产品编号
 */
@property(nonatomic , copy)NSString * prodtCode;

#pragma mark - 计划录像
/**
 1: 24小时录像; 2: 告警录像(图像变化时录制)
 */
@property(nonatomic , assign)int recType;
/**
 录像计划编号,可以是多个录像计划 [1-3]
 */
@property(nonatomic , assign)int recordNo;
/**
 [数字只能是1-7,分别代表周一到周日]
 */
@property(nonatomic , strong)NSMutableArray * week;
/**
 开始时间
 */
@property(nonatomic , assign)int startTime;
/**
 结束时间
 */
@property(nonatomic , assign)int endTime;
/**
 状态： 0关闭；1开启
 */
@property(nonatomic , assign)int planStatus;

#pragma mark - storage status
/**
 存储设备当前状态(0: 正常使用; 1: 未格式化; 2: 存储卡损坏)
 */
@property ( nonatomic, assign) int sdCardStatus;

/**
 存储设备总大小(MB), 当只有 status = (0|1) 时返回后续两个参数
 */
@property ( nonatomic, assign) unsigned int totalSize;

/**
 存储设备使用空间(MB)
 */
@property ( nonatomic, assign) unsigned int useSize;

#pragma mark - 显示配置
/**
 0 不现实通道名; 1 显示通道名
 */
@property(nonatomic , assign)int chan_name_show;
@property(nonatomic , assign)int show_name_location;
/**
 chan_name_show = 1 必须填写下面值.
 (x,y)计算方式: 左上角为0,0为起点，x,y 以百分比表示，百分比从0-100之间.
 */
@property(nonatomic , assign)int show_name_x;
/**
 chan_name_show = 1 必须填写下面值.
 (x,y)计算方式: 左上角为0,0为起点，x,y 以百分比表示，百分比从0-100之间.
 */
@property(nonatomic , assign)int show_name_y;
/**
 0 不显示时间； 1 显示时间.
 */
@property(nonatomic , assign)int datetime_show;
@property(nonatomic , assign)int show_datetime_x;
@property(nonatomic , assign)int show_datetime_y;
@property(nonatomic , assign)int show_datetime_location;
/**
 0: XXXX-XX-XX 年月日 ; 1: XX-XX-XXXX 月日年 ; 2: XXXX年XX月XX日; 3: XX月XX日XXXX年; 4: XX-XX-XXXX 日月年; 5: XX日XX月XXXX年
 */
@property(nonatomic , assign)int show_format;
/**
 OSD小时制:0-24小时制,1-12小时制
 */
@property(nonatomic , assign)int hour_format;
/**
 0 不显示； 1 显示
 */
@property(nonatomic , assign)int show_week;
/**
 OSD属性:透明，闪烁 (保留)
 */
@property(nonatomic , assign)int datetime_attr;
/**
 自定义显示字符: 0 不显示； 1 显示
 */
@property(nonatomic , assign)int custom1_show;
@property(nonatomic , assign)int show_custom1_x;
@property(nonatomic , assign)int show_custom1_y;
@property(nonatomic , assign)int show_custom1_location;
/**
 自定义显示字符: 0 不显示； 1 显示
 */
@property(nonatomic , assign)int custom2_show;
@property(nonatomic , assign)int show_custom2_x;
@property(nonatomic , assign)int show_custom2_y;
@property(nonatomic , assign)int show_custom2_location;
/**
 自定义显示的字符串 ( 最大长度为44字节 )
 */
@property(nonatomic , copy)NSString * show_custom1_str;
/**
 自定义显示的字符串 ( 最大长度为44字节 )
 */
@property(nonatomic , copy)NSString * show_custom2_str;

#pragma mark - 周界报警
@property(nonatomic , assign)int bc_id;
@property(nonatomic , strong)NSMutableArray * bc_filter;
@property(nonatomic , assign)int bc_fileter_size;


#pragma mark - ap
/**
 本地设备id
 */
@property(nonatomic , copy)NSString * device_id;
/**
 本地设备ip
 */
@property(nonatomic , copy)NSString * local_ip;
/**
 端口号
 */
@property(nonatomic , assign)uint32_t listen_port;

@property(nonatomic , assign)uint8_t  class_type;    // 设备分类: 1, IPC; 2, NVR; 3, DVR, 4, PIPC(全景相机)
@property(nonatomic , assign)uint8_t  dec;           // 支持的解码方式: 0: 多通道; 1: 多路合成
@property(nonatomic , assign)uint8_t  chans_no;      // 设备通道数; 如果 Class为1/4 则该值忽略；否则该值为该NVR/DVR实际支持的通道数
@property(nonatomic , assign)uint8_t  pipc_dv;       // 全景相机使用的校正解码算法厂家提供方; 1: 中科龙智
@property(nonatomic , assign)uint8_t  audio;         // 音频通话 1, 半双工; 2, 全双工;
@property(nonatomic , assign)uint8_t  mic;           // 麦克风 0, 不支持; 1, 支持
@property(nonatomic , assign)uint8_t  speaker;       // 喇叭 0, 不支持; 1, 支持
@property(nonatomic , assign)uint8_t  sdcard;        // SD卡 0, 不支持; 1, 支持
@property(nonatomic , assign)uint8_t  yun;           // 云录 0, 不支持; 1, 支持
@property(nonatomic , assign)uint8_t  ptzctrl;       // 云台 0, 不支持; 1,支持水平转动； 2,支持垂直转动; 3,水平+垂直转动

@end
