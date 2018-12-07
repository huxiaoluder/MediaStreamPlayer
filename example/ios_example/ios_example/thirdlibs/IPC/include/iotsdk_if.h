#ifndef _iot_dev_if_h_
#define _iot_dev_if_h_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "header_def.h"
#include "iot_comm.pb.h"

//#define c_debug_dump_media 1
#ifndef MK_LITEOS
#define mk_debug_mode           1 
#endif

#define mk_compile_user         1
#define mk_compile_ipc          1
#define mk_compile_iot          1
#define mk_compile_cloud        1

// detect define device
#if defined(mk_compile_ipc) || defined(mk_compile_iot)
    #define mk_compile_device 1
#endif

////////////////////////////////////////////////////////////////////////////////
/// sdk's interface define

// define error code
#define  c_error_disconnect             (1001)
#define  c_error_add_queue_fail         (1002)
#define  c_error_wait_time_out          (1003)
#define  c_error_method_not_found       (1004)
#define  c_error_rpc_in_trans_cb        (1005)

// osd location
typedef enum
{
    e_osd_left_top      = 0,
    e_osd_left_bottom   = 1,
    e_osd_right_top     = 2,
    e_osd_right_bottom  = 3,
} e_osd_location;

// define callback
typedef int(*cb_dns_server_change)(const char* addr);

// sdk app state
typedef enum _app_state
{
    e_state_init = 1,
    e_state_dns_udp_pre_query = 20,
    e_state_dns_udp_post_query ,

    e_state_dns_tcp_pre_query = 30,
    e_state_dns_tcp_post_query ,

    e_state_sdns_tcp_pre_query = 40,
    e_state_sdns_tcp_post_query ,

    e_state_iotgw_udp_pre_conn = 50,
    e_state_iotgw_udp_enter_loop,        // 表示udp连接网关成功
    e_state_iotgw_udp_post_conn,

    e_state_iotgw_tcp_pre_conn = 60,
    e_state_iotgw_tcp_enter_loop,        // 表示tcp连接网关成功
    e_state_iotgw_tcp_post_conn,

    e_state_iotgw_wait_retry  = 70,      // 等待重联

} e_app_state;


typedef enum
{
    e_trans_conn_invalid,
    e_trans_conn_init,              // 初始化
    e_trans_conn_connecting,        // 连接中
    e_trans_conn_success,           // 连接成功
    e_trans_conn_fail,              // 连接失败
    e_trans_conn_disconnect,        // 连接被断开
    e_trans_conn_closing,           // 主动关闭连接中
    e_trans_conn_stoped,            // 最终状态

} e_trans_conn_state ;

typedef enum
{
    e_trans_mode_invalid,
    e_trans_mode_local,         // local server
    e_trans_mode_p2p,           // p2p
    e_trans_mode_relay          // relay
} e_trans_conn_mode;

typedef enum
{
    e_drop_disabled = 1,        // 不丢帧
    e_drop_none_keyframe,       // 丢非关键帧

} e_drop_strategy;

typedef enum
{
    // 方向控制
    VIDEO_PTZ_CTRL_STOP=10,
    VIDEO_PTZ_CTRL_MOVE_UP,
    VIDEO_PTZ_CTRL_MOVE_DOWN,
    VIDEO_PTZ_CTRL_MOVE_LEFT,
    VIDEO_PTZ_CTRL_MOVE_RIGHT,
    VIDEO_PTZ_CTRL_MOVE_UPLEFT,
    VIDEO_PTZ_CTRL_MOVE_DOWNLEFT,
    VIDEO_PTZ_CTRL_MOVE_UPRIGHT,
    VIDEO_PTZ_CTRL_MOVE_DOWNRIGHT,
    // 光圈
    VIDEO_PTZ_CTRL_IRIS_IN,
    VIDEO_PTZ_CTRL_IRIS_OUT,
    // 聚焦
    VIDEO_PTZ_CTRL_FOCUS_ON,
    VIDEO_PTZ_CTRL_FOCUS_OUT,
    // 放大
    VIDEO_PTZ_CTRL_ZOOM_IN,
    // 缩小
    VIDEO_PTZ_CTRL_ZOOM_OUT,

    // 巡航
    VIDEO_PTZ_CTRL_BEGIN_CRUISE_SET,
    VIDEO_PTZ_CTRL_SET_CRUISE,
    VIDEO_PTZ_CTRL_END_CRUISE_SET,
    VIDEO_PTZ_CTRL_CALL_CRUISE,
    VIDEO_PTZ_CTRL_DELETE_CRUISE,
    VIDEO_PTZ_CTRL_STOP_CRUISE,
    //
    VIDEO_PTZ_CTRL_AUTO_SCAN,
    //
    VIDEO_PTZ_CTRL_RAINBRUSH_START,
    VIDEO_PTZ_CTRL_RAINBRUSH_STOP,

    VIDEO_PTZ_CTRL_LIGHT_ON,
    VIDEO_PTZ_CTRL_LIGHT_OFF,

    VIDEO_PTZ_CTRL_MAX
} e_video_ptz_ctrl;

typedef enum
{
    // 1; pause 暂停; 2: play 播放
    e_rec_pause  = 1,
    e_rec_play   = 2
} e_rec_action;

typedef enum
{
    // 0: 没有富媒体;1: JPG; 2: PNG; 11: MP4; 12: AVI; 21: 云存储
    e_rich_type_none = 0,
    e_rich_type_jpg  = 1,
    e_rich_type_png  = 2,
    e_rich_type_mp4  = 11,
    e_rich_type_avi  = 12,
    e_rich_type_cloud  = 21
} e_rich_type ;

typedef enum
{
    // ## 设备事件定义
    e_event_sensing_human_body      = 50,       // 感应到人体
    e_event_door_bell               = 51,       // 门铃被触发
    e_event_move_detect             = 52,       // 移动侦测
    e_event_voice_detect            = 53,       // 声音侦测
    e_event_door_sensor_open        = 54,       // 门磁打开
    e_event_door_sensor_close       = 55,       // 门磁关闭
    e_event_smog_alarm              = 56,       // 烟雾报警
    e_event_combustible_gas_alarm   = 57,       // 可燃气体报警
    e_event_high_temperature_alarm  = 58,       // 高温报警(测温)
    e_event_waterlogging_alarm      = 59,       // 水渍报警
    e_event_wear_off                = 60,       // 佩戴取下
    e_event_wear_on                 = 61,       // 设备佩戴
    e_event_low_power               = 62,       // 电量不足告警
    e_event_sos_alarm               = 63,       // SOS报警
    e_event_screen_cover            = 64,       // 画面遮挡
    e_event_human_body_detected     = 65,       // 检测到人体
    e_event_human_body_disappear    = 66,       // 人体 disappear
    e_event_crying_detected         = 67,       // crying detection

    e_event_fall_alarm              = 101,      // 摔倒报警
    e_event_electronic_fence_line   = 102,      // 电子围栏越界
    e_event_monitor_of_abnormal_path = 103,     // 路径监护异常
    e_event_abnormal_blood_pressure  = 104,     // 血压数据异常
    e_event_abnormal_heart_rate      = 105,     // 心率数据异常
    e_event_abnormal_temperature     = 106,     // 体温数据异常
    e_event_abnormal_blood_glucose   = 107,     // 血糖数据异常

} e_event_id;

// 云存储存放区域:
typedef enum
{
    e_save_site_none      = 0,      // 0, none
    e_save_site_ali_china = 1,      // 1, 阿里云中国;
    e_save_site_ali_dubai = 2,      // 2, 阿里云迪拜;
                                    // 3.....

} e_save_site;

////////////////////////////////////////////////////////////////////////////////////////////////
// for conn info
typedef enum
{
    e_pkt_cmd,
    e_pkt_audio,
    e_pkt_video,
    e_pkt_self,
} e_pkt_type_t;


// event define
#define   IOT_EVENT_TIMER           1

///////////////////////////////////////////////////////////////
// state define
typedef enum {

    e_iot_state_udp_dns_query =    1,
    e_iot_state_tcp_dns_query   ,
    e_iot_state_udp_sdns_query  ,

    e_iot_state_udp_pre_conn_gw  ,
    e_iot_state_udp_conn_gw,

    e_iot_state_tcp_pre_conn_gw,
    e_iot_state_tcp_conn_gw,

    e_iot_state_user_final,

} e_iot_fsm_state;

typedef struct
{
    char device_id[64];
    char local_ip[16];
    char sn[64];
    char netmask[16];
    uint32_t listen_port;
    uint32_t listen_proto;
    char version_api[64];
    char prodt_code[64];

    uint8_t  class_type;    // 设备分类: 1, IPC; 2, NVR; 3, DVR, 4, PIPC(全景相机)
    uint8_t  dec;           // 支持的解码方式: 0: 多通道; 1: 多路合成
    uint8_t  chans_no;      // 设备通道数; 如果 Class为1/4 则该值忽略；否则该值为该NVR/DVR实际支持的通道数
    uint8_t  pipc_dv;       // 全景相机使用的校正解码算法厂家提供方; 1: 中科龙智
    uint8_t  audio;         // 音频通话 1, 半双工; 2, 全双工;
    uint8_t  mic;           // 麦克风 0, 不支持; 1, 支持
    uint8_t  speaker;       // 喇叭 0, 不支持; 1, 支持
    uint8_t  sdcard;        // SD卡 0, 不支持; 1, 支持
    uint8_t  yun;           // 云录 0, 不支持; 1, 支持
    uint8_t  ptzctrl;       // 云台 0, 不支持; 1,支持水平转动； 2,支持垂直转动; 3,水平+垂直转动

} local_device_t;

///////////////////////////////////////////////////////////////
#define byte_array_t(n) struct { uint32_t size; unsigned char bytes[n]; }

typedef void(*cb_on_write_log)(const char* log_content, int log_len);

// app state change
typedef void(*cb_app_state_change)(e_app_state state);

// transport state change
typedef void(*cb_transport_state_change)(
        uint32_t conn_id,
        e_trans_conn_state state,
        const char* device_id,
        void* user_data);

// config change
typedef void(*cb_on_config_change)(
        const char* cfg,
        size_t cfg_len
);

typedef void(*cb_on_media_data)(uint32_t conn_id,               // 连接id
                                header_media_t *hdr_media,      // 数据包头部
                                const char *data_ptr,           // 流媒体数据缓冲区
                                uint32_t data_len);             // 流媒体数据长度
typedef void(*cb_on_self_data)(uint32_t conn_id,
                               header_self_t *hdr_self,
                               const char *data_ptr,
                               uint32_t data_len);
typedef void(*cb_on_network_quality_change)(uint32_t conn_id,
                                            bool quality_good   // true　网络质量ok
);                                                              // false 网络质量差

typedef void(*cb_on_time_sync)(uint64_t utc_time_ms);
typedef void(*cb_on_view_conn_info)(void* ctx,
                                    uint32_t conn_id,
                                    void** user_data,
                                    e_trans_conn_state state);

typedef void (*cb_on_iotgw_msg_result)(uint64_t msg_id, bool success);
typedef void (*cb_on_iotgw_publish_msg)(
        const char *topic,
        const char *msg_buff,
        int msg_len,
        uint32_t msg_cmdid,
        uint32_t type // e_msg_format_type
);

typedef void (*cb_on_iotgw_run_iot_cmd)(IOTCMD* cmd);

typedef void (*cb_on_iotgw_iot_cmd_resp)(IOTCMD* cmd);

typedef void (*cb_on_rom_update)(
        uint32_t conn_id,
        const char *rom_ver,            // 当前最新固件的版本号
        const char *md5,                // 当前最新固件的md5值
        const char *url                 // 当前最新固件的url下载地
);

typedef void (*cb_on_send_message)(uint32_t conn_id,
                                   const char *client_id,
                                   int64_t req_time,
                                   const char *sign_key,
                                   uint32_t user_id,
                                   const char * device_id,
                                   const char * sub_device_id,
                                   int32_t event_id,
                                   int32_t chno,
                                   int32_t rich_type,
                                   const char * rich_url,
                                   int64_t start_time,
                                   int64_t time_len,
                                   int32_t save_site,
                                   const char *msg_title,
                                   const char *msg_body);

/*
升级错误代码定义(TODO： 需要进一步完善):
0: 正确;
11: 下载地址不正确
12: 固件校验不正确(MD5不匹配)
13: 升级失败(空间不足)
14: 升级失败(写入错误)
15: 升级失败(未知错误)
*/
typedef void (*cb_on_rom_update_progress)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t rate,  // step步骤执行的100%
        uint32_t code   // 如果执行错误，此处返回升级的错误代码
);

typedef void (*cb_on_notice_online_chans)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t chans_arr[32],
        uint32_t chan_count
);

typedef void(*cb_on_sleep_ready)(
        bool use_tcp,                   // 是否使用tcp
        int  fd,                        // 与iotgw通信的文件描述符
        const char* hb_data_ptr,        // 心跳包数据
        uint32_t    hb_data_len,        // 心跳包长度
        uint32_t    hb_interval,        // 心跳包间隔
        const char* wakeup_data_ptr,    // 设置低功耗模式下的唤醒数据包
        uint32_t    wakeup_data_len,    // 唤醒数据包长度
        const char* restart_data_ptr,   // 唤醒时需要的数据(用于初始化)
        uint32_t    restart_data_len    // 唤醒时数据的长度
);

// notify cloud storage open or close
typedef void(*cb_on_cloud_storage_notify)(
        uint64_t stream_exp,            // 7*24小时录像过期时间
        uint64_t alarm_exp,             // 告警录像过期时间
        uint64_t doorbell_exp,          // 门铃过期时间
        bool enable);

typedef enum {
    e_cloud_play_invalid,
    e_cloud_play_stream,
    e_cloud_play_file,

} e_cloud_play_type;

typedef enum {
    e_cloud_content_stream = 1,
    e_cloud_content_alram,
    e_cloud_content_doorbell,
} e_cloud_record_type;

typedef enum
{
    e_cloud_status_invalid = 0,
    e_cloud_status_begin_get_resource = 1,
    e_cloud_status_loading,
    e_cloud_status_playing,
    e_cloud_status_stop
} e_cloud_play_status;

typedef enum
{
    e_cloud_error_success,
    e_cloud_error_err_get_,

} e_cloud_error;
typedef void(*cb_on_cloud_storgae_play_status)(e_cloud_play_status status, e_cloud_error err);

///////////////////////////////////////////////////////////////////////////////////////
// struct define
typedef struct  {
    uint32_t bc_id;                         // 周界编号，数值为1-4.
    unsigned char bc_filter[60];
    // 按照长宽标号: 22 * 18, 设置相应的区块；区块按照从左到右，从上到下的顺序来标示.
    // 数据按位表示：选中的区块位1，未选中的区块位0.
    // 长度使用60字节即可.
    uint32_t      bc_fileter_size;
} bc_info_t;

typedef struct  {
    uint32_t psp_id;            // 预置点编号(预置点是顺序递增的)
    char psp_name[44];          // 预置点名称
    bool psp_default;           // 是否是看守位
    bool is_set;                // 是否有效 (false，无效; true 有效)
} psp_info_t;

typedef struct
{
    int64_t start_time;                 //  开始录像时间戳 单位秒(UTC时间)
    uint32_t length;                    //  录像时长, 单位秒.
    uint32_t record_type;               //  1, 正常录像; 2, 告警录像; 65, 人形检测
} record_info_t;

typedef struct   {
    uint32_t rec_type ;     // 1: 24小时录像; 2: 告警录像(图像变化时录制); 65, 人形检测
    uint32_t record_no ;    // 录像计划编号,可以是多个录像计划 [1-3]
    uint32_t  week[7] ;     // [数字只能是1-7,分别代表周一到周日]
    uint32_t  week_count;   //

    uint32_t start_time ;   // 开始时间.
    uint32_t end_time ;     // 结束时间.
    uint32_t status ;       // 状态： 0关闭；1开启
}rec_plan_t;


/////////////////////////////////////////////////////////////////////////////////////
// for ipc
#ifdef mk_compile_ipc

typedef bool (*cb_ipc_start_video)(uint32_t conn_id,
                                   uint32_t ch_no,
                                   uint32_t client_type,
                                   uint32_t video_quality,
                                   uint32_t vstrm,
                                   uint32_t* fps,
                                   uint32_t* resolution);

typedef bool (*cb_ipc_start_audio)(uint32_t conn_id,
                                   uint32_t ch_no,
                                   uint32_t *audio_codec,
                                   uint32_t *rate,
                                   uint32_t *bit,
                                   uint32_t *track);

typedef bool (*cb_ipc_start_talkback)(uint32_t conn_id,
                                      uint32_t ch_no,
                                      uint32_t *audio_codec,
                                      uint32_t *rate,
                                      uint32_t *bit);

typedef bool (*cb_ipc_stop_audio)(uint32_t conn_id,
                                  uint32_t ch_no);

typedef bool (*cb_ipc_stop_talkback)(uint32_t conn_id,
                                     uint32_t ch_no);

typedef bool (*cb_ipc_stop_video)(uint32_t conn_id,
                                  uint32_t ch_no);

////////////////////////////////////////////////////////////////////////////
typedef bool (*cb_ipc_call_psp)(uint32_t conn_id,
                                uint32_t ch_no,
                                uint32_t psp_id);

typedef bool (*cb_ipc_get_screen)(uint32_t conn_id,
                                  uint32_t ch_no,
                                  uint32_t *format,   // 图片格式： 1， png; 2, jpg
                                  char **data_buf,  // 图片内容 [ Qvga 格式的图片.]
                                  int *data_len);

typedef bool (*cb_ipc_get_psp)(uint32_t conn_id,
                               uint32_t ch_no,
                               psp_info_t pspinfo[10],
                               uint32_t *pspinfo_count);

typedef bool (*cb_ipc_get_time)(uint32_t conn_id,
                                uint32_t ch_no,
                                int64_t *now_time,
                                char time_zone[200],
                                int32_t* time_offset,
                                char ntp_server1[50],
                                char ntp_server2[50]);

typedef bool (*cb_ipc_get_powerfreq)(uint32_t conn_id,
                                     uint32_t ch_no,
                                     uint32_t *freq);

typedef bool (*cb_ipc_get_osd)(uint32_t conn_id,
                               uint32_t ch_no,
                               uint32_t *chan_name_show,
                               uint32_t *show_name_x,
                               uint32_t *show_name_y,
                               uint32_t *show_name_location,

                               uint32_t *datetime_show,
                               uint32_t *show_datetime_x,
                               uint32_t *show_datetime_y,
                               uint32_t *show_format,
                               uint32_t *hour_format,
                               uint32_t *show_week,
                               uint32_t *datetime_attr,
                               uint32_t *show_datetime_location,

                               uint32_t *custom1_show,
                               char     show_custom1_str[44],
                               uint32_t *show_custom1_x,
                               uint32_t *show_custom1_y,
                               uint32_t *show_custom1_location,

                               uint32_t *custom2_show,
                               char     show_custom2_str[44],
                               uint32_t *show_custom2_x,
                               uint32_t *show_custom2_y,
                               uint32_t *show_custom2_location
);

typedef bool (*cb_ipc_get_bc)(uint32_t conn_id,
                              uint32_t ch_no,
                              bc_info_t bcinfo[10],
                              uint32_t *bcinfo_count);

typedef bool (*cb_ipc_get_alarm)(uint32_t conn_id,
                                 uint32_t ch_no_orig,
                                 uint32_t *ch_no,
                                 uint32_t *motion_detection,
                                 uint32_t *opensound_detection,
                                 uint32_t *openi2o_detection,
                                 uint32_t *smoke_detection,
                                 uint32_t *shadow_detection,
                                 uint32_t *human_body_detection,
                                 uint32_t *crying_detection);

typedef bool (*cb_ipc_set_psp)(uint32_t conn_id,
                               uint32_t ch_no_orig,
                               uint32_t psp_id,
                               char psp_name[44],
                               bool psp_default,
                               bool is_set);

typedef bool (*cb_ipc_set_icr)(uint32_t conn_id,
                               uint32_t ch_no,
                               uint32_t mode);

typedef bool (*cb_ipc_set_flip)(uint32_t conn_id,
                               uint32_t ch_no,
                               uint32_t flip_type);

typedef bool (*cb_ipc_set_time)(uint32_t conn_id,
                                uint32_t ch_no,
                                int64_t now_time,
                                char time_zone[200],
                                int32_t time_offset,
                                char ntp_server1[50],
                                char ntp_server2[50]);

typedef bool (*cb_ipc_set_video)(uint32_t conn_id,
                                 uint32_t ch_no,
                                 uint32_t video_quality,
                                 uint32_t *fps,
                                 uint32_t *resolution);

typedef bool (*cb_ipc_set_powerfreq)(uint32_t conn_id,
                                     uint32_t ch_no,
                                     uint32_t freq);

typedef bool (*cb_ipc_set_osd)(uint32_t conn_id,
                               uint32_t ch_no,
                               uint32_t chan_name_show,
                               uint32_t show_name_x,
                               uint32_t show_name_y,
                               uint32_t show_name_location,

                               uint32_t datetime_show,
                               uint32_t show_datetime_x,
                               uint32_t show_datetime_y,
                               uint32_t show_format,
                               uint32_t hour_format,
                               uint32_t show_week,
                               uint32_t datetime_attr,
                               uint32_t show_datetime_location,

                               uint32_t custom1_show,
                               char     show_custom1_str[44],
                               uint32_t show_custom1_x,
                               uint32_t show_custom1_y,
                               uint32_t show_custom1_location,

                               uint32_t custom2_show,
                               char     show_custom2_str[44],
                               uint32_t show_custom2_x,
                               uint32_t show_custom2_y,
                               uint32_t show_custom2_location
);

typedef bool (*cb_ipc_set_bc)(uint32_t conn_id,
                              uint32_t ch_no,
                              bc_info_t bcinfo[10],
                              uint32_t bcinfo_count);

typedef bool (*cb_ipc_set_alarm)(uint32_t conn_id,
                                 uint32_t ch_no,
                                 uint32_t motion_detection,
                                 uint32_t opensound_detection,
                                 uint32_t openi2o_detection,
                                 uint32_t smoke_detection,
                                 uint32_t shadow_detection,
                                 uint32_t human_body_detection,
                                 uint32_t crying_detection);

typedef bool (*cb_ipc_set_wifi)(uint32_t conn_id,   // connection id
                                uint32_t ch_no,     //通道数.
                                char essid[256],    // ssid
                                char auth_key[256], // auth key
                                uint32_t enc_type   // encrypt type
);

typedef bool (*cb_ipc_ptz_ctrl)(uint32_t conn_id,   // connection id
                                uint32_t ch_no,     //通道数.
                                e_video_ptz_ctrl code,// uint32_t VIDEO_PTZ_xxx
                                uint32_t para1,  //
                                uint32_t para2  //
);

typedef bool (*cb_ipc_set_chan)(uint32_t conn_id,   // connection id
                                uint32_t ch_no,     //通道数.
                                uint32_t chans[16],
                                uint32_t chans_count
);

typedef bool (*cb_ipc_set_chanadv)(uint32_t conn_id,   // connection id
                                   uint32_t ch_no,     //通道数.
                                   uint32_t matrix_x, // 标准视频合成的矩阵，x轴切分次数
                                   uint32_t matrix_y, // 标准视频合成的矩阵，y轴切分次数
                                   uint32_t chans[16],
                                   uint32_t chans_count
);

typedef bool (*cb_ipc_rec_action)(uint32_t conn_id,
                                  uint32_t ch_no,
                                  e_rec_action action);

typedef bool (*cb_ipc_rec_list)(uint32_t conn_id,
                                uint32_t ch_no,      //  通道数.
                                const char*date_day, // 获取这一天的录像记录
                                int64_t last_time,   // 最后获得的时间 [ 单位秒 ]
                                uint32_t get_type,   // [default = 2]; // 1 表示取下一页(next)数据,取从last_time到最新的记录使用该值;2 表示向后(prev)数据,取比last_time更早的记录使用该值.
                                uint32_t get_num,    // [default = 30]; // 取多少条数据.
                                record_info_t *rec_arr,  // 返回的记录数组
                                uint32_t *rec_arr_count  // 返回的记录个数
);
typedef bool (*cb_ipc_rec_list_day)(uint32_t conn_id,
                                    uint32_t ch_no,      //  通道数.
                                    char *day_list[20],  //  调用者释放内存
                                    uint32_t *day_count
);

typedef bool (*cb_ipc_rec_plan_get)(uint32_t conn_id,
                                    uint32_t ch_no,      //  通道数.
                                    rec_plan_t plan[3],
                                    uint32_t *plan_count
);

typedef bool (*cb_ipc_rec_plan_set)(uint32_t conn_id,
                                    uint32_t ch_no,          //  通道数.
                                    uint32_t rec_type,       // 1: 24小时录像; 2: 告警录像(图像变化时录制); 65, 人形检测
                                    uint32_t record_no,      // 录像计划编号,可以是多个录像计划 [1-3]
                                    uint32_t week[7],        // [数字只能是1-7,分别代表周一到周日]
                                    uint32_t week_count,     // week count
                                    uint32_t start_time,     // 开始时间.
                                    uint32_t end_time,       // 结束时间.
                                    uint32_t status          // 状态： 0关闭；1开启
);

typedef bool (*cb_ipc_rec_set_rate)(uint32_t conn_id,
                                    uint32_t ch_no,          //  通道数.
                                    uint32_t rec_rate        // 1: 1/2; 2: normal; 3: time 2; 4: time 4
);

typedef bool (*cb_ipc_rec_start)(uint32_t conn_id,
                                 uint32_t ch_no,         //  通道数.
                                 const char *date_day,   // 格式: 20170801 (YYYYMMDD)
                                 int64_t time_stamp,     // 开始播放录像时间 单位: 秒; 相对于 date_day的相对时间；比如从当天的08:00:00 开始播放录像，该值: 8*60*60
                                 uint32_t* resolution    // 1, 144P; 2, 240P； 3, 360P; 4, 480P; 5, 720P; 6, 1080P; 7, 2K; 8, 4K
);

typedef bool (*cb_ipc_rec_stop)(uint32_t conn_id,
                                uint32_t ch_no          //  通道数.
);

typedef bool (*cb_ipc_storage_format)(uint32_t conn_id,
                                      uint32_t ch_no          //  通道数.
);

typedef bool (*cb_ipc_get_wifi_quality)(uint32_t conn_id,            // 连接ID
                                        uint32_t ch_no,              // 通道数
                                        uint32_t *quality);

typedef bool (*cb_ipc_storage_format_rate)(
        uint32_t conn_id,         // 连接ID
        uint32_t ch_no,           // 通道数
        uint32_t *rate            // 存储设备格式化执行百分比
);

typedef bool (*cb_ipc_storage_info)(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t *status,           // 存储设备当前状态(0: 正常使用; 1: 未格式化; 2: 存储卡损坏)
        uint32_t *total_size,       // 存储设备总大小(MB), 当只有 status = (0|1) 时返回后续两个参数
        uint32_t *use_size          // 存储设备使用空间(MB)
);

typedef bool (*cb_ipc_device_action)(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t action             // action: 1, reboot; 2, reset;
);

typedef bool (*cb_ipc_rom_get)(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t *mode               // 1: 每天自动检查更新; 2: 用户手动更新
);

typedef bool (*cb_ipc_rom_set)(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t mode               // 1: 每天自动检查更新; 2: 用户手动更新
);

/**
 * 此回调ipc端不可用,使用以下api注册的回调
 *
 * iotsdk_dev_set_on_rom_update(cb_on_rom_update cb);
 */
typedef bool (*cb_ipc_rom_update)(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no              // 通道数
);

typedef bool (*cb_ipc_rom_update_rate)(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t *step ,            // 当前执行的步骤：1，正在下载固件; 2, 正在校验固件; 3, 正在升级; 4, 升级完成.
        uint32_t *rate,             // step步骤执行的100%
        uint32_t *code              // 如果执行错误，此处返回升级的错误代码
);

typedef bool (*cb_ipc_get_online_chans)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t chans_arr[64],
        uint32_t *chan_count
);

typedef bool (*cb_ipc_get_fish_eye_param)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t *circle_x,
        uint32_t *circle_y,
        uint32_t *circle_r
);

typedef bool (*cb_ipc_set_led)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t flag
);

typedef bool (*cb_ipc_get_led)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t *flag
);

typedef bool (*cb_ipc_set_irlight)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t flag
);

typedef bool (*cb_ipc_get_irlight)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t *flag
);

typedef bool (*cb_ipc_set_event_rec_time)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t rec_time           // 单位秒
);

typedef bool (*cb_ipc_get_event_rec_time)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t* rec_time          // 单位秒
);

typedef bool (*cb_ipc_set_ringer_switch)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t ringer_switch      // o close, 1 open
);

typedef bool (*cb_ipc_get_ringer_switch)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t* ringer_switch      // o close, 1 open
);

typedef bool (*cb_ipc_set_talkback_volume)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t volume      // 音量高低: 1 低, 2 中, 3 高
);

typedef bool (*cb_ipc_get_talkback_volume)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t* volume      // 音量高低: 1 低, 2 中, 3 高
);

typedef bool (*cb_ipc_get_battery_status)(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t* power_percent,     // 电量百分比
        uint32_t* status             // 1 正常, 2 充电
);

#endif

// for ipc
/////////////////////////////////////////////////////////////////////////////////////


#endif