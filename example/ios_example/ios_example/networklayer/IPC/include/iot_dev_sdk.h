#ifndef _mk_iot_dev_sdk_h_
#define _mk_iot_dev_sdk_h_


#include "iotsdk_if.h"


// define log level
#ifndef _mk_e_log_level
#define _mk_e_log_level

typedef enum
{
    e_log_debug = 1,
    e_log_info,
    e_log_warn,
    e_log_error,
} e_log_level;

#endif


#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__GNUC__) && \
        ((__GNUC__ >= 4) || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))
    #define NP_VISIBILITY_DEFAULT __attribute__((visibility("default")))
#else
    #define NP_VISIBILITY_DEFAULT
#endif
#define NP_EXPORT NP_VISIBILITY_DEFAULT

///////////////////////////////////////////////////////////////////////////////
// sdk ()
NP_EXPORT bool iotsdk_init(bool user); // 初始化sdk(最先调用,只需要调用一次)
    NP_EXPORT void iotsdk_uninit(void);        // 不必调用
    NP_EXPORT bool iotsdk_start(void);         // 启动sdk(参数设置好后调用,内部开启线程,只需要调用一次)
    NP_EXPORT bool iotsdk_stop(void);          // 不必调用
NP_EXPORT void iotsdk_debug(bool);     // 设置是否是测试模式

    NP_EXPORT const char* iotsdk_version(void); // return the sdk version string
    NP_EXPORT int         iotsdk_build(void);


NP_EXPORT void iotsdk_set_proto(int ver);
    NP_EXPORT int  iotsdk_get_proto(void);

///////////////////////////////////////////////////////////////////////////////
// for common
/**
 * 设置dns server ip变化的时候的回调
 * @param cb        [in]
 */
NP_EXPORT void iotsdk_set_cb_server_ip_change(cb_dns_server_change cb);

/**
 * 设置app状态改变时候的回调
 * @param cb        [in]
 */
NP_EXPORT void iotsdk_set_cb_app_state_change(cb_app_state_change cb);

/**
 * 设置配置修改的回调
 * @param cb
 */
NP_EXPORT void iotsdk_set_cb_on_config_change(cb_on_config_change cb);
/**
 * 设置接收到流媒体数据时候的回调
 * @param on_media  [in]
 */
NP_EXPORT void iotsdk_set_cb_on_media(cb_on_media_data on_media);
/**
 * 设置自定数据的回调
 * @param cb        [in]
 */
NP_EXPORT void iotsdk_set_cb_on_self_data(cb_on_self_data cb);

/**
 * 网络质量改变的回调
 * @param cb
 */
NP_EXPORT void iotsdk_set_cb_on_network_quality_change(cb_on_network_quality_change cb);

/**
 * 查询网络质量
 * @param conn_id           [in] 连接ID
 * @param quality           [out] 连接质量
 */
NP_EXPORT void iotsdk_query_network_quality(uint32_t conn_id, bool* quality);

/**
 * 设置连接状态改变的回调( user <---> device)
 * @param cb_state_notice   [in]
 */
NP_EXPORT void iotsdk_set_cb_trans_state_change(cb_transport_state_change cb_state_notice);

/**
 * 设置时间同步回调
 * @param cb_sync_time      [in]
 */
NP_EXPORT void iotsdk_set_cb_on_time_sync(cb_on_time_sync cb_sync_time);

/**
 * 设置收到网关回复消息的回调
 * @param cb
 * @return
 */
NP_EXPORT void iotsdk_set_iotgw_msg_cb(cb_on_iotgw_msg_result cb);

/**
 * 设置本端的ip和端口(可以不用设置,不设置的时候内部自动获取),用于本地连接和建立localserver
 * @param addr              [in]
 * @param port              [in]
 */
NP_EXPORT void iotsdk_set_client_addr(const char *addr, uint16_t port);

/**
 * 添加dns server 地址信息(可以调用多次,添加多个地址)
 * @param ip_addr           [in]
 * @param port              [in]
 */
NP_EXPORT void iotsdk_add_server_ip_addr(const char* ip_addr, uint16_t port);

/**
 * 设置重连平台间隔
 * @param min_wait_ms        [in] 最小等待时长
 * @param max_wait_ms        [in] 最大等待时长
 * @return
 */
NP_EXPORT int iotsdk_set_retry_conn_gw_limit(uint64_t min_wait_ms,
                                             uint64_t max_wait_ms);

/**
 * 设置是否使用tcp连接iotgw
 * @param use_tcp
 * @return
 */
NP_EXPORT int iotsdk_set_conn_use_tcp(bool use_tcp);

/**
 * 添加服务器域名(当ip连不上时,会尝试用域名进行连接)
 * @param domain            [in]
 * @param port              [in]
 */
NP_EXPORT void iotsdk_add_server_domain_addr(const char* domain, uint16_t port);

/**
 * 设置加密选项(默认是不加密)
 * @param enc_ctrl     [in]是否加密指令数据
 * @param enc_media    [in]是否加密流媒体数据
 * @param media_enc_length  [in]流媒体数据加密长度(0表示全部加密)
 * @param enc_type          [in]encrypt type (reference mrpc protocol)
*/
NP_EXPORT void iotsdk_set_encrypt_options(bool enc_ctrl,
                                          bool enc_media,
                                          uint32_t media_enc_length,
                                          e_mrpc_enc_type enc_type);
/**
 * 设置密钥固定部分值(密钥生成的部分固定内容)
 * @param fix_key
 */
NP_EXPORT void iot_set_fix_enc_key(char* fix_key);

// for custom
/**
 * 设置传队列的大小
 * @param max_cmd_count          [in]指令队列大小(默认10),优先级1
 * @param max_video_count        [in]视频队列大小(默认100),优先级3
 * @param max_audio_count        [in]音频队列大小(默认100),优先级2
 * @param max_self_count         [in]自定义队列大小(默认10),优先级4
 */
NP_EXPORT void iotsdk_set_max_send_queue_size(uint32_t max_cmd_count,
                                              uint32_t max_video_count,
                                              uint32_t max_audio_count,
                                              uint32_t max_self_count,
                                              uint32_t max_buffer_limit);

/**
 * 设置日志等级
 * @param level
 */
NP_EXPORT void iotsdk_set_log_level(e_log_level level);
NP_EXPORT void iotsdk_set_log_async(bool async);

// for debug(调试时使用,将状态id转换成字符串)
NP_EXPORT const char* iotsdk_get_app_state_name(e_app_state state);
NP_EXPORT const char* iotsdk_get_trans_conn_name(e_trans_conn_state state);
NP_EXPORT const char* iotsdk_get_app_fsm_name(e_iot_fsm_state state);
NP_EXPORT const char* iotsdk_get_cmd_id_name(uint32_t cmd_id);
NP_EXPORT const char* iotsdk_get_conn_mode_name(e_trans_conn_mode);
NP_EXPORT void iotsdk_write_log(e_log_level, const char* log_content, int log_len);
NP_EXPORT void iotsdk_set_log_cb(cb_on_write_log cb);

/**
 * 判断是否连接到网关
 * @return
 */
    NP_EXPORT bool iotsdk_is_online(void);

///////////////////////////////////////////////////////////////////////////////
// for device

/**
 * 进入低功耗模式
 * @param use_tcp
 * @param cb
 * @return
 */
NP_EXPORT int iotsdk_dev_start_sleep(bool use_tcp, cb_on_sleep_ready cb);

/**
 * 低功耗快速启动时使用
 * @param restart_data_ptr
 * @param restart_data_len
 * @return
 */
NP_EXPORT int iotsdk_dev_quick_init(const char* restart_data_ptr,   // restart --> after wakeup
                                    uint32_t    restart_data_len);

/**
 * 门铃发起接听请求
 * @return
 */
    NP_EXPORT int iotsdk_dev_send_acs_request(void);

// for device
///////////////////////////////////////////////////////////////////////////////

// for common
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// for ipc common
/**
 * 通过连接ID获取user_data
 * @param conn_id               [in] 连接ID
 * @return
 */
NP_EXPORT void* iotsdk_get_user_data_by_id(uint32_t conn_id);
/**
 * set user data by conn id
 * @param conn_id               [in] 连接ID
 * @param user_data             [in] 用户数据
 * @return
 */
NP_EXPORT bool iotsdk_set_user_data_by_id(uint32_t conn_id, void *user_data);

/**
 * 通过设备ID获取连接信息描述信息（例如conn3的详细信息）
 */
NP_EXPORT int iotsdk_get_conn_info_by_did(const char * did, char *buf, int len);

/**
 * 通过回调的方式操作连接信息(内部加锁,不能在回调里再调用其它api)
 * @param conn_id               [in] 连接ID
 * @param cb                    [in] 回调函数
 * @param ctx                   [in] 上下文参数,回调的时候回传
 * @return
 */
NP_EXPORT int iotsdk_view_conn_info(uint32_t conn_id, cb_on_view_conn_info cb, void* ctx);

/**
 * 获取连接的模式(user与device连接的方式)
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT e_trans_conn_mode iotsdk_get_conn_mode(uint32_t conn_id);

/**
 * 获取连接是否支持加密
 */
NP_EXPORT bool iotsdk_get_conn_support_enc(uint32_t conn_id);

/**
 * 获取下行传输速度(bytes/s)
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT uint32_t iotsdk_get_conn_down_speed(uint32_t conn_id);
/**
 * 获取上行传输速度(bytes/s)
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT uint32_t iotsdk_get_conn_up_speed(uint32_t conn_id);
/**
 * 获取上行和下行总的传输速度(bytes/s)
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT uint32_t iotsdk_get_whole_speed(uint32_t conn_id);
/**
 * 主动断开连连
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT int iotsdk_stop_conn(uint32_t conn_id);
/**
 * 设备初始化配置信息
 * @param cfg
 * @param cfg_len
 * @return
 */
NP_EXPORT int iotsdk_init_conf(
        const char* device_ip,
        uint16_t    device_port,
        const char*cfg,
        size_t cfg_len
);
// for ipc common
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// for iot
#ifdef mk_compile_iot

/**
 * 设置来自iotgw的publish消息回调
 * @param cb
 */
NP_EXPORT void iotsdk_iot_set_publish_cb(cb_on_iotgw_publish_msg cb);

/**
 * 订阅主题
 * @param msg_id                    [out] 该消息的消息ID(可填0表示不关注消息是否成功)，用于判断是否成功发送，从iotsdk_set_iotgw_msg_cb设置的回调返回
 * @param topic                     [in] 主题
 * @return
 */
NP_EXPORT int iotsdk_iot_subscribe_topic(uint64_t* msg_id, const char* topic);

/**
 * 取消订阅
 * @param msg_id                    [out] 该消息的消息ID(可填0表示不关注消息是否成功)，用于判断是否成功发送，从iotsdk_set_iotgw_msg_cb设置的回调返回
 * @param topic                 [in] 主题
 * @return
 */
NP_EXPORT int iotsdk_iot_unsub_topic(uint64_t* msg_id, const char *topic);

/**
 * 发布消息
 * @param msg_id                    [out] 该消息的消息ID(可填0表示不关注消息是否成功)，用于判断是否成功发送，从iotsdk_set_iotgw_msg_cb设置的回调返回
 * @param topic                 [in] 主题
 * @param msg_buff              [in] 消息内容
 * @param msg_len               [in] 消息长度
 * @param msg_cmdid             [in] 消息命令ID
 * @param type                  [in] 消息格式类型
 * @return
 */
NP_EXPORT int iotsdk_iot_publish_msg(
        uint64_t* msg_id,
        const char* topic,
        const char* msg_buff,
        int msg_len,
        uint32_t msg_cmdid,
        uint32_t msg_format_type); // e_msg_format_type

/**
 *  向topic发送执行命令消息
 * @param msg_id                    [out] 该消息的消息ID(可填0表示不关注消息是否成功)，用于判断是否成功发送，从iotsdk_set_iotgw_msg_cb设置的回调返回
 * @param topic                     [in]  消息发送的目标topic
 * @param cmd                       [in]  消息结构
 * @return
 */
NP_EXPORT int iotsdk_iot_run_iot_cmd(
        uint64_t *msg_id,
        const char *topic,
        IOTCMD *cmd);

/**
 * 设置处理run iot cmd命令的回调
 * @param cb                        [in] cb
 * @return
 */
NP_EXPORT int iotsdk_iot_set_run_iot_cmd_cb(cb_on_iotgw_run_iot_cmd cb);

/**
 * 设置处理mqtt run cmd命令的回调
 * @param cb                        [in] cb
 * @return
 */
NP_EXPORT int iotsdk_iot_set_run_mqtt_cmd_cb(cb_on_iotgw_run_mqtt_cmd cb);

/**
 * 设置处理iot cmd resp命令的回调
 * @param cb
 * @return
 */
NP_EXPORT int iotsdk_iot_set_iot_cmd_resp_cb(cb_on_iotgw_iot_cmd_resp cb);

/**
 *　回应run iot cmd结果
 * @param msg_id                    [out] 该消息的消息ID(可填0表示不关注消息是否成功)，用于判断是否成功发送，从iotsdk_set_iotgw_msg_cb设置的回调返回
 * @param topic                     [in] topic to response
 * @param cmd                       [in] fill the cmd response
 * @return
 */
NP_EXPORT int iotsdk_iot_cmd_response(
        uint64_t *msg_id,
        const char *topic,
        IOTCMD *cmd);

#endif

// for iot
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// for dev ipc
#ifdef mk_compile_ipc

/**
 * 设置丢帧策略(默认丢非关键帧)
 * @param strategy      [in]
 */
NP_EXPORT void iotsdk_set_drop_strategy(e_drop_strategy strategy);
/**
 * 清除对应的发送队列
 * @param conn_id           [in]连接ID
 * @param pkt_type          [in]队列类型
 */
NP_EXPORT void iotsdk_clear_send_queue(uint32_t conn_id, e_pkt_type_t pkt_type);
/**
 * 发送流媒体数据(音频视频都使用这个接口)
 * @param conn_id               [in]连接ID
 * @param pkt_type              [in]队列类型(用于优先级队列)
 * @param stream_type           [in]流媒体类型(参考mrpc)
 * @param is_key_frame          [in]是否是关键帧(用于丢帧策略)
 * @param channel_number        [in]通道编号
 * @param encrypt_flag          [in]加密类型(填0)
 * @param timestamp             [in]时间戳
 * @param encrypt_length        [in]加密长度(填0)
 * @param payload_ptr           [in]流媒体数据缓冲区
 * @param payload_len           [in]流媒体数据长度
 */
NP_EXPORT void iotsdk_write_media_data(uint32_t conn_id,
                                       e_pkt_type_t pkt_type,
                                       e_stream_type stream_type,
                                       bool is_key_frame,
                                       uint8_t channel_number,
                                       uint8_t encrypt_flag,
                                       uint64_t timestamp,
                                       uint64_t encrypt_length,
                                       const char *payload_ptr,
                                       int payload_len);


/**
 * 发送自定义数据
 * @param conn_id               [in] 连接ID
 * @param payload_ptr           [in] 数据缓冲区
 * @param payload_len           [in] 数据长度
 */
NP_EXPORT void iotsdk_write_self_def_data(uint32_t conn_id,
                             const char *payload_ptr,
                             int payload_len);

#endif

// for dev ipc
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// for user ipc

#ifdef mk_compile_user
/**
 * 平台认证信息
 * @param app_id                [in] 平台提供(应用的ID)
 * @param app_seckey            [in] 平台提供(应用的key)
 */
NP_EXPORT void iotsdk_user_set_app_verify_info(const char *app_id, const char *app_seckey);
/**
 * 设置user端的帐号信息
 * @param username              [in] 用户名
 * @param password              [in] 用户密码
 */
NP_EXPORT void iotsdk_user_set_account_info(const char *username, const char *password);
/**
 * 设置硬件信息(IMEI MAC)
 * @param hw_feature            [in] 硬件信息
 */
NP_EXPORT void iotsdk_user_set_hardware_id(const char *hw_feature);

/**
 * retry conn gw(when speed limit triggered, app need use this api to refresh conn to gw)
 */
    NP_EXPORT void iotsdk_user_retry_conn_gw(void);

/**
 * 获取本地局域网搜索到的设备ID
 * @param device_ptr            [in out] 接收device, 调用者释放内存
 * @param device_count_ptr      [in out] 填入可存放device个数,返回搜索到的device个数
 * @return
 */
NP_EXPORT int iotsdk_user_get_local_devices(local_device_t* device_arr[20], uint32_t* device_count_ptr);

/**
 * 开启连接设备端(通过p2p 或者relay或localserver)
 * @param device_id             [in] 设备id
 * @param user_data             [in] 用户数据(回调时传递)
 * @return  < 0 error
 *          > 0 conn id
 */
NP_EXPORT int iotsdk_user_start_conn_dev(const char* device_id,
                                         void* user_data);

/**
 * 开连连接设备端(只通过本地连接)
 * @param device_id
 * @param local_ip
 * @param local_port
 * @param user_data
 * @return　< 0 error
 *          > 0 conn id
 */
NP_EXPORT int iotsdk_user_start_conn_dev_local(const char* device_id,
                                               const char* local_ip,
                                               uint16_t    local_port,
                                               void* user_data);

/**
 * 开连连接设备端(只通过本地连接)
 * @param device_id
 * @param lan_ip            局域网中设备的ip地址
 * @param lan_port          局域网中设备的端口
 * @param user_name         用于局域网验证的用户名
 * @param pass_word         用于局域网验证的密码
 * @param user_data
 * @return　< 0 error
 *          > 0 conn id
 */
NP_EXPORT int iotsdk_user_start_conn_dev_lan(const char *device_id,
                                             const char *lan_ip,
                                             uint16_t lan_port,
                                             const char* user_name,
                                             const char* pass_word,
                                             void *user_data);

/**
 * 设置send message消息的回调
 * @param cb
 * @return
 */
NP_EXPORT int iotsdk_user_set_cb_send_message(cb_on_send_message cb);

/**
 * 缓存连接信息(在获取到设备列表的时候可以调用),加快连接速度
 * @param device_ids
 * @param id_count
 */
NP_EXPORT void iotsdk_user_cache_conn_info(char** device_ids, int id_count);

/**
 * 获取连接状态(通过连接id)
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT e_trans_conn_state iotsdk_user_get_conn_state_by_id(uint32_t conn_id);
/**
 * 获取连接状态(通过device id)
 * @param device_id             [in] 设备ID
 * @return
 */
NP_EXPORT e_trans_conn_state iotsdk_user_get_conn_state_by_name(const char* device_id);
/**
 * 根据device_id获取相应的conn_id
 * @param device_id             [in] 设备ID
 * @return
 */
NP_EXPORT uint32_t iotsdk_user_get_conn_id_by_name(const char* device_id);

/**
 * 根据连接ID获取设备ID(调用者释放内存)
 * @param conn_id               [in] 连接id
 * @return
 */
NP_EXPORT char*  iotsdk_user_get_device_id_by_conn_id(uint32_t conn_id);

/**
 * 设置ipc请求时的超时时间(ms)
 * @param wait_ms               [in] 超时时长(ms)
 */
NP_EXPORT void iotsdk_user_set_wait_timeo(uint64_t wait_ms);
/**
 * 获取ipc请求的超时时间
 * @return
 */
    NP_EXPORT uint64_t iotsdk_user_get_wait_timeo(void);

/**
 * sync conn (liteos can update sleep time)
 * @param conn_id
 */
NP_EXPORT int iotsdk_user_sync_conn(uint32_t conn_id);

/**
 * 开启视频
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param client_type           [in] 1, Phone; 2, Pad; 3, PC
 * @param video_quality         [in] 图像质量: 1-100; 数字越大画面质量越高. 0: auto
 * @param vstrm                 [in] 0:main; 1:sub1; 2:sub2...
                                如果是开启DVR/NVR, 默认给出的视频为0，1，2，3这4路视频.
 * @param fps                   [out] 帧率
 * @param resolution            [out] 1, 144P; 2, 240P； 3, 360P; 4, 480P; 5, 720P; 6, 1080P; 7, 2K; 8, 4K
 * @return
 */
NP_EXPORT int iotsdk_user_start_video(uint32_t conn_id,
                                      uint32_t ch_no,            // 通道数.
                                      uint32_t client_type,       // 1, Phone; 2, Pad; 3, PC
                                      uint32_t video_quality,     // 图像质量: 1-100; 数字越大画面质量越高. 0: auto
                                      uint32_t vstrm,             // 0:main; 1:sub1; 2:sub2...
                                                                    // 如果是开启DVR/NVR, 默认给出的视频为0，1，2，3这4路视频.
                                      uint32_t *fps,
                                      uint32_t *resolution
);

/**
 * 开启音频
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param audio_codec           [out] 返回设备端支持的声音编码.(参考:e_stream_type)
 * @param rate                  [out] 返回 默认 8000 Hz
 * @param bit                   [out] 返回 默认 16 bit
 * @param track                 [out] 返回 1: mono 2: stereo
 * @return
 */
NP_EXPORT int iotsdk_user_start_audio(uint32_t conn_id,
                            uint32_t ch_no,
                            uint32_t *audio_codec,
                            uint32_t *rate,
                            uint32_t *bit,
                            uint32_t *track);

/**
 * 开启对聊
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param audio_codec           [out] 返回设备端支持的声音编码.(参考:e_stream_type)
 * @param rate                  [out] 返回 默认 8000 Hz
 * @param bit                   [out] 返回 默认 16 bit
 * @return
 */
NP_EXPORT int iotsdk_user_start_talkback(uint32_t conn_id,
                               uint32_t ch_no,
                               uint32_t* audio_codec,
                               uint32_t* rate,
                               uint32_t* bit);

/**
 * 停止视频
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_stop_video(uint32_t conn_id, uint32_t ch_no);

/**
 * 停止音频
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_stop_audio(uint32_t conn_id, uint32_t ch_no);

/**
 * 停止对聊
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_stop_talkback( uint32_t conn_id, uint32_t ch_no);

/**
 * 设置报警
 * @param conn_id                   [in] 连接ID
 * @param ch_no                     [in] 通道数. 0 合成通道；1-N为各个独立通道, IPC该值为0
 * @param motion_detection          [in] 0 标示关闭移动侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高.
 * @param opensound_detection       [in] 0 关闭声音侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高.
 * @param openi2o_detection         [in] 红外体感
 * @param smoke_detection           [in] 烟雾感应
 * @param shadow_detection          [in] 遮蔽感应
 * @param human_body_detection      [in] human_body_detection
 * @param crying_detection          [in] crying_detection
 * @return
 */
NP_EXPORT int iotsdk_user_set_alarm(uint32_t conn_id,
                          uint32_t ch_no,
                          uint32_t motion_detection,
                          uint32_t opensound_detection,
                          uint32_t openi2o_detection,
                          uint32_t smoke_detection,
                          uint32_t shadow_detection,
                          uint32_t human_body_detection,
                          uint32_t crying_detection);

/**
 *
 * @param conn_id                   [in]连接ID
 * @param ch_no                     [in]通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_set_bc(uint32_t conn_id,
                                 uint32_t ch_no,
                                 bc_info_t bcinfo[10],
                                 uint32_t bcinfo_count);

/**
 * 设置图片翻转
 * @param conn_id                   [in]连接ID
 * @param ch_no                     [in]通道数.
 * @param flip_type                 [in] 0:Upright, 1:Flip Horizontal，2:Flip Vertical，3:turn 180
 * @return
 */
NP_EXPORT int iotsdk_user_set_flip(uint32_t conn_id,
                         uint32_t ch_no,
                         uint32_t flip_type);

/**
 * 设置图片模式
 * @param conn_id                   [in] 连接ID
 * @param ch_no                     [in] 通道数.
 * @param mode                      [in] 1 彩色; 2 黑白; 3 自动
 * @return
 */
NP_EXPORT int iotsdk_user_set_icr(uint32_t conn_id,
                        uint32_t ch_no,
                        uint32_t mode);

/**
 * 设置OSD显示字符串
 * @param conn_id                   [in] 连接ID
 * @param ch_no                     [in] 通道数.
 * @param chan_name_show            [in] 0 不现实通道名; 1 显示通道名
 * @param show_name_x               [in] (x,y)计算方式: 左上角为0,0为起点，x,y 以百分比表示，百分比从0-100之间.
 * @param show_name_y               [in]
 * @param show_name_location        [in] ref: e_osd_location
 * @param datetime_show             [in] 0 不显示时间； 1 显示时间.
 * @param show_datetime_x           [in]
 * @param show_datetime_y           [in]
 * @param show_format               [in] 0: XXXX-XX-XX 年月日 ; 1: XX-XX-XXXX 月日年 ; 2: XXXX年XX月XX日; 3: XX月XX日XXXX年; 4: XX-XX-XXXX 日月年; 5: XX日XX月XXXX年
 * @param hour_format               [in] OSD小时制:0-24小时制,1-12小时制
 * @param show_week                 [in] 0 不显示； 1 显示
 * @param datetime_attr             [in] OSD属性:透明，闪烁 (保留)
 * @param show_datetime_location    [in] ref: e_osd_location
 * @param custom1_show              [in] 自定义显示字符: 0 不显示； 1 显示
 * @param show_custom1_str          [in]
 * @param show_custom1_x            [in]
 * @param show_custom1_y            [in]
 * @param show_custom1_location     [in] ref: e_osd_location
 * @param custom2_show              [in] 自定义显示字符: 0 不显示； 1 显示
 * @param show_custom2_str          [in]
 * @param show_custom2_x            [in]
 * @param show_custom2_y            [in]
 * @param show_custom2_location     [in] ref: e_osd_location
 * @return
 */
NP_EXPORT int iotsdk_user_set_osd(uint32_t conn_id,
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
                                  char show_custom1_str[44],
                                  uint32_t show_custom1_x,
                                  uint32_t show_custom1_y,
                                  uint32_t show_custom1_location,
                                  uint32_t custom2_show,
                                  char show_custom2_str[44],
                                  uint32_t show_custom2_x,
                                  uint32_t show_custom2_y,
                                  uint32_t show_custom2_location);

/**
 * 设置电源频率
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数
 * @param freq              [in] 50 50HZ, 60 60HZ
 * @return
 */
NP_EXPORT int iotsdk_user_set_powerfreq(uint32_t conn_id,
                              uint32_t ch_no,
                              uint32_t freq);

/**
 * 设置预置点(最大8个预置点)
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数
 * @param psp_id            [in] 预置点编号(1-8)
 * @param psp_name          [in] 预置点名称
 * @param psp_default       [in] 是否是看守位(所有预置点只能有一个看守位,需要清除其它看守位)
 * @param is_set            [in] 是否有效 (false，无效; true 有效)
 * @return
 */
NP_EXPORT int iotsdk_user_set_psp(uint32_t conn_id,
                        uint32_t ch_no,
                        uint32_t psp_id,
                        char psp_name[44],
                        bool psp_default,
                        bool is_set);

/**
 * 设置时间
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数
 * @param now_time          [in] 单位秒.
 * @param time_zone         [in] 时区
 * @param time_offset       [in] 时间偏移,单位分钟: 480, -60
 * @param ntp_server1       [in] NTP 服务器1
 * @param ntp_server2       [in] NTP 服务器2
 * @return
 */
NP_EXPORT int iotsdk_user_set_time(uint32_t conn_id,
                                   uint32_t ch_no,
                                   int64_t now_time,
                                   char time_zone[200],
                                   int32_t time_offset,
                                   char ntp_server1[50],
                                   char ntp_server2[50]);

/**
 * 设置视频参数
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数
 * @param video_quality     [in] 图像质量: 1-100; 数字越大画面质量越高.
 * @param fps               [out] 返回设备帧率.
 * @param resolution        [out] 1, 144P; 2, 240P； 3, 360P; 4, 480P; 5, 720P; 6, 1080P; 7, 2K; 8, 4K
 * @return
 */
NP_EXPORT int iotsdk_user_set_video(uint32_t conn_id,
                                    uint32_t ch_no,
                                    uint32_t video_quality,
                                    uint32_t *fps,
                                    uint32_t *resolution);

/**
 * 设置wifi参数
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数
 * @param essid             [in] wifi ssid
 * @param auth_key          [in] wifi auth key
 * @param enc_type          [in] wifi 加密类型
 * @return
 */
NP_EXPORT int iotsdk_user_set_wifi(uint32_t conn_id,
                                   uint32_t ch_no,    //通道数.
                                   char essid[256],    // ssid
                                   char auth_key[256], // auth key
                                   uint32_t enc_type   // wifi 加密类型
);

/**
 * 云台控制
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数.
 * @param code              [in] e_video_ptz_ctrl 定义参考
 * @param para1             [in] 未知
 * @param para2             [in] 未知
 * @return
 */
NP_EXPORT int iotsdk_user_ptz_ctrl(uint32_t conn_id,
                                   uint32_t ch_no, // 通道数.
                                   e_video_ptz_ctrl code,// uint32_t VIDEO_PTZ_xxx
                                   uint32_t para1,  //
                                   uint32_t para2  //
);

/**
 * 如果是NVR/DVR [ 设置NVR/DVR输出视频的通道 ]
 * @param conn_id            [in] 连接ID
 * @param ch_no              [in] 通道数.
 * @param chans              [in] 出视频的通道数组
 * @param chans_count        [in] 数组元素个数
 * @return
 */
NP_EXPORT int iotsdk_user_set_chan(uint32_t conn_id,
                                   uint32_t ch_no, // 通道数.
                                   uint32_t chans[16],
                                   uint32_t chans_count
);

/**
 * // 如果是NVR/DVR [ 设置NVR/DVR输出视频的通道 ]
// 设备类型为8，9 才支持该接口
// 该接口支持不规则分屏的视频合成显示.
 * @param conn_id                  [in] 连接ID
 * @param ch_no                    [in] 通道数
 * @param matrix_x                 [in] 标准视频合成的矩阵，x轴切分次数
 * @param matrix_y                 [in] 标准视频合成的矩阵，y轴切分次数
 * @param chans                    [in]
 * @param chans_count              [in]
 * @return
 */
NP_EXPORT int iotsdk_user_set_chanadv(uint32_t conn_id,
                                      uint32_t ch_no, // 通道数.
                                      uint32_t matrix_x, // 标准视频合成的矩阵，x轴切分次数
                                      uint32_t matrix_y, // 标准视频合成的矩阵，y轴切分次数
                                      uint32_t chans[16],
                                      uint32_t chans_count
);

/**
 * 设置通知在线通道数的回调
 * @param cb
 * @return
 */
NP_EXPORT int iotsdk_user_set_online_chans_cb(cb_on_notice_online_chans cb);

/**
 * 获取在线通道数
 * @param conn_id
 * @param ch_no
 * @param chans_arr
 * @param chans_count
 * @return
 */
NP_EXPORT int iotsdk_user_get_online_chans(uint32_t conn_id,
                                           uint32_t ch_no, // 通道数.
                                           uint32_t *chans_arr,
                                           uint32_t *chans_count
);

/**
 * 获取全景图像的⻥眼校正参数
 * @param conn_id
 * @param ch_no
 * @param circle_x                  [out] 全景图像圆心坐标x
 * @param circle_y                  [out] 全景图像圆心坐标y
 * @param circle_r                  [out] 全景图像圆心半径
 * @return
 */
NP_EXPORT int iotsdk_user_get_fish_eye_param(uint32_t conn_id,
                                             uint32_t ch_no, // 通道数.
                                             uint32_t* circle_x,
                                             uint32_t* circle_y,
                                             uint32_t* circle_r
);

/**
 * 获取报警设置
 * @param conn_id                   [in] 连接ID
 * @param ch_no_orig                [in] 原通道数
 * @param ch_no                     [out] 返回通道数. 0 合成通道；1-N为各个独立通道, IPC该值为0
 * @param motion_detection          [out] 0 标示关闭移动侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高.
 * @param opensound_detection       [out] 0 关闭声音侦测 // > 0表示灵敏度. 1,2,3 -> 数字越大灵敏度越高.
 * @param openi2o_detection         [out] 红外体感
 * @param smoke_detection           [out] 烟雾感应
 * @param shadow_detection          [out] 遮蔽感应
 * @param human_body_detection      [out] human_body_detection
 * @param crying_detection          [out] crying_detection
 * @return
 */
NP_EXPORT int iotsdk_user_get_alarm( uint32_t conn_id,
                                     uint32_t ch_no_orig,
                                     uint32_t *ch_no,
                                     uint32_t *motion_detection,
                                     uint32_t *opensound_detection,
                                     uint32_t *openi2o_detection,
                                     uint32_t *smoke_detection,
                                     uint32_t *shadow_detection,
                                     uint32_t *human_body_detection,
                                     uint32_t *crying_detection);

/**
 * 获取周界设定
 * @param conn_id                   [in] 连接ID
 * @param ch_no                     [in] 通道数.
 * @param bcinfo                    [out] 周界数组，最大支持4组.
 * @param bcinfo_count              [in out] 数组个数[in out]
 * @return
 */
NP_EXPORT int iotsdk_user_get_bc( uint32_t conn_id,
                        uint32_t ch_no,
                        bc_info_t bcinfo[10],
                        uint32_t* bcinfo_count);

/**
 * 获取OSD参数
 * @param conn_id                   [in] 连接ID
 * @param ch_no                     [in] 通道数.
 * @param chan_name_show            [out] 0 不现实通道名; 1 显示通道名
 * @param show_name_x               [out] chan_name_show = 1 必须填写下面值.
                                          (x,y)计算方式: 左上角为0,0为起点，x,y 以百分比表示，百分比从0-100之间.
 * @param show_name_y               [out]
 * @param show_name_location        [out] ref: e_osd_location
 * @param datetime_show             [out] 0 不显示时间； 1 显示时间.
 * @param show_datetime_x           [out]
 * @param show_datetime_y           [out]
 * @param show_format               [out] 0: XXXX-XX-XX 年月日 ; 1: XX-XX-XXXX 月日年 ; 2: XXXX年XX月XX日; 3: XX月XX日XXXX年; 4: XX-XX-XXXX 日月年; 5: XX日XX月XXXX年
 * @param hour_format               [out] OSD小时制:0-24小时制,1-12小时制
 * @param show_week                 [out] 0 不显示； 1 显示
 * @param datetime_attr             [out] OSD属性:透明，闪烁 (保留)
 * @param show_datetime_location    [out] ref: e_osd_location
 * @param custom1_show              [out] 自定义显示字符: 0 不显示； 1 显示
 * @param show_custom1_str          [out] 自定义显示的字符串 ( 最大长度为44字节 )
 * @param show_custom1_x            [out]
 * @param show_custom1_y            [out]
 * @param show_custom1_location     [out] ref: e_osd_location
 * @param custom2_show              [out] 自定义显示字符: 0 不显示； 1 显示
 * @param show_custom2_str          [out] 自定义显示的字符串 ( 最大长度为44字节 )
 * @param show_custom2_x            [out]
 * @param show_custom2_y            [out]
 * @param show_custom2_location     [out] ref: e_osd_location
 * @return
 */
NP_EXPORT int iotsdk_user_get_osd( uint32_t conn_id,
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
                                   char show_custom1_str[44],
                                   uint32_t *show_custom1_x,
                                   uint32_t *show_custom1_y,
                                   uint32_t *show_custom1_location,
                                   uint32_t *custom2_show,
                                   char show_custom2_str[44],
                                   uint32_t *show_custom2_x,
                                   uint32_t *show_custom2_y,
                                   uint32_t *show_custom2_location);

/**
 * 获取电源频率
 * @param conn_id       [in] 连接ID
 * @param ch_no         [in] 通道数.
 * @param freq          [out] 50 50HZ, 60 60HZ
 * @return
 */
NP_EXPORT int iotsdk_user_get_powerfreq( uint32_t conn_id,
                               uint32_t ch_no,
                               uint32_t *freq);

/**
 * 获取psp信息
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数.
 * @param pspinfo           [out] 接收psp信息数组
 * @param pspinfo_count     [in out] 数组大小[in out]
 * @return
 */
NP_EXPORT int iotsdk_user_get_psp( uint32_t conn_id,
                         uint32_t ch_no,
                         psp_info_t pspinfo[10],
                         uint32_t* pspinfo_count);

/**
 *  获取截图 (@warning 调用者释放 img_ptr)
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数.
 * @param format            [out] 1， png; 2, jpg
 * @param img_ptr           [out] 图片内容 [ Qvga 格式的图片.]
 * @param img_len           [out] 图片长度
 * @return
 */
NP_EXPORT int iotsdk_user_get_screen(uint32_t conn_id,
                                     uint32_t ch_no,
                                     uint32_t* format,
                                     char** img_ptr,
                                     int*   img_len
);

/**
 * 获取时间(参考设置时间)
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数.
 * @param now_time          [out] 当前时间(单位秒).
 * @param time_zone         [out] 时区
 * @param time_offset       [out] 时间偏移,单位分钟: 480, -60
 * @param ntp_server1       [out] NTP 服务器1
 * @param ntp_server2       [out] NTP 服务器2
 * @return
 */
NP_EXPORT int iotsdk_user_get_time(uint32_t conn_id,
                                   uint32_t ch_no,
                                   int64_t *now_time,
                                   char time_zone[200],
                                   int32_t *time_offset, // 时间偏移,单位分钟: 480, -60
                                   char ntp_server1[50],
                                   char ntp_server2[50]);

///// record relative
/**
 * 暂停/播放视频
 * @param conn_id           [in] 连接ID
 * @param ch_no             [in] 通道数.
 * @param action            [in] 1; pause 暂停; 2: play 播放
 * @return
 */
NP_EXPORT int iotsdk_user_rec_action(uint32_t conn_id,
                                     uint32_t ch_no,
                                     e_rec_action action);

/**
 * 获取录像列表
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param date_day              [in] 获取这一天的录像记录
 * @param last_time             [in] 最后获得的时间 [ 单位秒 ]
 * @param get_type              [in] default = 2  1 表示取下一页(next)数据,取从last_time到最新的记录使用该值;2 表示向后(prev)数据,取比last_time更早的记录使用该值.
 * @param get_num               [in] default = 30 取多少条数据.
 * @param rec_arr               [out] 返回的记录数组[max limit = 200]
 * @param rec_arr_count         [in out] 返回的记录个数
 * @return
 */
NP_EXPORT int iotsdk_user_rec_list(uint32_t conn_id,
                                   uint32_t ch_no,      //  通道数.
                                   const char*date_day, // 获取这一天的录像记录
                                   int64_t last_time,   // 最后获得的时间 [ 单位秒 ]
                                   uint32_t get_type,   // [default = 2]; // 1 表示取下一页(next)数据,取从last_time到最新的记录使用该值;2 表示向后(prev)数据,取比last_time更早的记录使用该值.
                                   uint32_t get_num,    // [default = 30]; // 取多少条数据.
                                   record_info_t* rec_arr,  // 返回的记录数组
                                   uint32_t* rec_arr_count  // 返回的记录个数
);

/**
 * 获得本地录像记录
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param start_day            [in] 查询的开始时期,从该日期往前查询,格式: 20170801 (YYYYMMDD)
 * @param day_list              [out] 日期字符串数组,调用者释放内存(存在录像的日期,格式: 20170801 (YYYYMMDD))
 * @param day_count             [in out] 数组个数[in out]
 * @return
 */
NP_EXPORT int iotsdk_user_rec_list_day(uint32_t conn_id,
                                       uint32_t ch_no,      //  通道数.
                                       const char* start_day,
                                       char *day_list[50],  //  调用者释放内存
                                       uint32_t *day_count
);

/**
 * 获得本地存储计划
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param plan                  [in] 本地计划数组
 * @param plan_count            [in out] 数组大小
 * @return
 */
NP_EXPORT int iotsdk_user_rec_plan_get(uint32_t conn_id,
                                       uint32_t ch_no,      //  通道数.
                                       rec_plan_t plan[3],
                                       uint32_t* plan_count
);

/**
 * 设置本地存储计划
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param rec_type              [in] 1: 24小时录像; 2: 告警录像(图像变化时录制)
 * @param record_no             [in] 录像计划编号,可以是多个录像计划 [1-3]
 * @param week                  [in] [数字只能是1-7,分别代表周一到周日]
 * @param week_count,           [in] week中存放的个数(即多少有效数据)
 * @param start_time            [in] 开始时间.(从0点开始的秒数)
 * @param end_time              [in] 结束时间.(从0点开始的秒数)
 * @param status                [in] 状态： 0关闭；1开启
 * @returnuser conn dev success, conn id
 */
NP_EXPORT int iotsdk_user_rec_plan_set(uint32_t conn_id,
                                       uint32_t ch_no,          //  通道数.
                                       uint32_t rec_type,       // 1: 24小时录像; 2: 告警录像(图像变化时录制); 65, 人形检测
                                       uint32_t record_no,      // 录像计划编号,可以是多个录像计划 [1-3]
                                       uint32_t week[7],        // [数字只能是1-7,分别代表周一到周日]
                                       uint32_t week_count,     // week中存放的个数(即多少有效数据)
                                       uint32_t start_time,     // 开始时间.(从0点开始的秒数)
                                       uint32_t end_time,       // 结束时间.(从0点开始的秒数)
                                       uint32_t status          // 状态： 0关闭；1开启
);

/**
 * 设置播放速率
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param rec_rate              [in] 1: 1/2; 2: normal; 3: time 2; 4: time 4
 * @return
 */
NP_EXPORT int iotsdk_user_rec_set_rate(uint32_t conn_id,
                                       uint32_t ch_no,          //  通道数.
                                       uint32_t rec_rate        // 1: 1/2; 2: normal; 3: time 2; 4: time 4
);

/**
 * 播放录像
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param date_day              [in] 格式: 20170801 (YYYYMMDD)
 * @param time_stamp            [in] 开始播放录像时间 单位: 秒; 相对于 date_day的相对时间；比如从当天的08:00:00 开始播放录像，该值: 8*60*60
 * @param resolution            [out] 1, 144P; 2, 240P； 3, 360P; 4, 480P; 5, 720P; 6, 1080P; 7, 2K; 8, 4K
 * @return
 */
NP_EXPORT int iotsdk_user_rec_start(uint32_t conn_id,
                                    uint32_t ch_no,         // 通道数.
                                    const char *date_day,   // 格式: 20170801 (YYYYMMDD)
                                    int64_t time_stamp,     // 开始播放录像时间 单位: 秒; 相对于 date_day的相对时间；比如从当天的08:00:00 开始播放录像，该值: 8*60*60
                                    uint32_t* resolution    // 1, 144P; 2, 240P； 3, 360P; 4, 480P; 5, 720P; 6, 1080P; 7, 2K; 8, 4K
);

/**
 * 结束录像播放
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_rec_stop(uint32_t conn_id,
                                   uint32_t ch_no          //  通道数.
);

/**
 * 格式化设备端存储设备
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_storage_format(uint32_t conn_id,              // 连接ID
                                         uint32_t ch_no                 // 通道数.
);

/**
 * 获取wifi质量
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param quality               [out] WIFI网络质量
 * @return
 */
NP_EXPORT int iotsdk_user_get_wifi_quality(uint32_t conn_id,            // 连接ID
                                           uint32_t ch_no,              // 通道数
                                           uint32_t *quality            // WIFI网络质量
);

/**
 * 获取格式化执行百分比
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param rate
 * @return
 */
NP_EXPORT int iotsdk_user_storage_format_rate(uint32_t conn_id,         // 连接ID
                                              uint32_t ch_no,           // 通道数
                                              uint32_t *rate            // 存储设备格式化执行百分比
);

/**
 * 获取存储设备状态
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param status                [out] 存储设备当前状态(0: 正常使用; 1: 未格式化; 2: 存储卡损坏; 3:未插卡)
 * @param total_size            [out] 存储设备总大小(MB), 当只有 status = (0|1) 时返回后续两个参数
 * @param use_size              [out] 存储设备使用空间(MB)
 * @return
 */
NP_EXPORT int iotsdk_user_storage_info(
        uint32_t conn_id,
        uint32_t ch_no,
        uint32_t *status,
        uint32_t *total_size,
        uint32_t *use_size
);

/**
 * 执行设备动作
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param action                [in] action: 1, reboot; 2, reset;
 * @return
 */
NP_EXPORT int iotsdk_user_device_action(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t action             // action: 1, reboot; 2, reset;
);

/**
 * 设置升级的进度回调(ipc主动通知)
 * @param cb
 * @return
 */
NP_EXPORT int iotsdk_user_set_cb_upgrade_progress(cb_on_rom_update_progress cb );

/**
 * 获取升级参数
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param mode                  [out] 1: 每天自动检查更新; 2: 用户手动更新
 * @return
 */
NP_EXPORT int iotsdk_user_rom_get(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t *mode               // 1: 每天自动检查更新; 2: 用户手动更新
);

/**
 * 设置升级参数
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @param mode                  [in]
 * @return
 */
NP_EXPORT int iotsdk_user_rom_set(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t mode               // 1: 每天自动检查更新; 2: 用户手动更新
);

/**
 * 通知固件进行升级
 * @param conn_id               [in] 连接ID
 * @param ch_no                 [in] 通道数.
 * @return
 */
NP_EXPORT int iotsdk_user_rom_update(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no              // 通道数
);

NP_EXPORT int iotsdk_user_get_led(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t* flag              // 1 常开；2 常关；
);

NP_EXPORT int iotsdk_user_set_led(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t flag               // 1 常开；2 常关；
);

NP_EXPORT int iotsdk_user_get_irlight(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t *flag              // 1 常开；2 常关； 3 自动
);

NP_EXPORT int iotsdk_user_set_irlight(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t flag              // 1 常开；2 常关； 3 自动
);

/**
 * 设置事件触发后的录像时长
 * @param conn_id
 * @param ch_no
 * @param rec_time
 * @return
 */
NP_EXPORT int iotsdk_user_set_event_rec_time(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t rec_time           // 单信秒
);

/**
 * 获取事件触发后的录像时长
 * @param conn_id
 * @param ch_no
 * @param rec_time
 * @return
 */
NP_EXPORT int iotsdk_user_get_event_rec_time(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t* rec_time           // 单信秒
);

NP_EXPORT int iotsdk_user_set_ringer_switch(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t ringer_switch      // 0 close, 1 open
);

NP_EXPORT int iotsdk_user_get_ringer_switch(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t* ringer_switch      // 0 close, 1 open
);

NP_EXPORT int iotsdk_user_set_talkback_volume(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t volume             // 音量高低: 1 低, 2 中, 3 高
);

NP_EXPORT int iotsdk_user_get_talkback_volume(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t* volume            // 音量高低: 1 低, 2 中, 3 高
);

NP_EXPORT int iotsdk_user_get_battery_status(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        uint32_t *power_percent,     // 电量百分比
        uint32_t *status             // 1 正常, 2 充电
);

/**
 *
 * @param conn_id
 * @param ch_no
 * @param cfg                   [in]
 * @return
 */
NP_EXPORT int iotsdk_user_get_dont_disturb(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        dont_disturb_cfg_t *cfg     //
);

/**
 *
 * @param conn_id
 * @param ch_no
 * @param cfg                   [out]
 * @return
 */
NP_EXPORT int iotsdk_user_set_dont_disturb(
        uint32_t conn_id,           // 连接ID
        uint32_t ch_no,             // 通道数
        dont_disturb_cfg_t *cfg     //
);

NP_EXPORT int iotsdk_user_get_motion_detection(uint32_t conn_id,           // 连接ID
                                               uint32_t ch_no,             // 通道数
                                               uint32_t* left,
                                               uint32_t* top,
                                               uint32_t* right,
                                               uint32_t* bottom
);

NP_EXPORT int iotsdk_user_set_motion_detection(uint32_t conn_id,           // 连接ID
                                               uint32_t ch_no,             // 通道数
                                               uint32_t left,
                                               uint32_t top,
                                               uint32_t right,
                                               uint32_t bottom
);

#endif

// for user ipc
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// for cloud storage dev

#ifdef  mk_compile_cloud

/**
 * 设备端初始化云存储
 * @param cb            [in]　云存今朝是否开启的通知
 * @return
 */
NP_EXPORT int iotsdk_dev_cloud_storage_enable(cb_on_cloud_storage_notify cb);

/**
 * 设置云存储录像类型
 * 使唤用事件录像时,先设置录像类型(iotsdk_dev_cloud_storage_set_rec_type),然后调用iotsdk_dev_cloud_write_media写相应的流媒体数据,
 * 待事件完毕后,调用iotsdk_dev_cloud_finish_rec结束录像,并上传相关信息
 * @param type
 */
NP_EXPORT void iotsdk_dev_cloud_storage_set_rec_type(e_cloud_record_type type);

/**
 * 写云存储流媒体数据(用于上传到云端)
 * @param pkt_type              [in]　
 * @param stream_type           [in]
 * @param is_key_frame          [in]
 * @param is_warning            [in]
 * @param channel_number        [in]
 * @param encrypt_flag          [in]
 * @param timestamp             [in]
 * @param encrypt_length        [in]
 * @param payload_ptr           [in]
 * @param payload_len           [in]
 * @return
 */
NP_EXPORT int iotsdk_dev_cloud_write_media(
        e_pkt_type_t pkt_type,
        e_stream_type stream_type,
        bool is_key_frame,
        bool is_warning,
        uint8_t channel_number,
        uint8_t encrypt_flag,
        uint64_t timestamp,
        uint64_t encrypt_length,
        const char *payload_ptr,
        int payload_len
);

/**
 * 结束云录像(主要是针对alarm,doorbell类型)
 * @return
 */
    NP_EXPORT int iotsdk_dev_cloud_finish_rec(void);

/**
 * 设置云录控制选项
 * @param max_cache_frames              [in] 最大的缓存帧数(视频+音频)
 * @return
 */
NP_EXPORT int iotsdk_dev_cloud_set_option(int max_cache_frames);

#endif

// for cloud storage dev
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// for cloud storage user

#ifdef  mk_compile_cloud

/**
 * 启用云存储(进程启动时调用,只需要调用一次)
 * @param enable                [in]　启用,默认true
 * @param media_cb              [in] 音视频回调
 * @param status_cb             [in]　播放云存储流媒体数据时的状态回调
 * @return
 */
NP_EXPORT int iotsdk_user_cloud_storage_enable(
        bool enable,
        cb_on_media_data media_cb,
        cb_on_cloud_storgae_play_status status_cb
);

/**
 * 设置access_token(从平台获得的),只有有变化就得调用一次
 * @param access_token          [in]　app从平台获得的
 */
NP_EXPORT void iotsdk_user_cloud_storage_access_token(
        const char *access_token);

/**
 * 播放云存储流媒体数据
 * @param cloud_play_type       [in] 云录播放类型
 * @param device_id             [in] 设备ＩＤ
 * @param ch_no                 [in] 通道数
 * @param timestamp_ms          [in] 时间戳
 * @param cloud_file_name       [in] 要播放的文件名
 * @return
 */
NP_EXPORT int iotsdk_user_cloud_storage_play(
        e_cloud_play_type  cloud_play_type,
        const char* device_id,     // [both need]
        uint32_t ch_no,            // [stream play] channel no
        uint64_t timestamp_ms,     // [stream play] true; 开始时间戳, 单位ms
        const char* cloud_file_name,    // [file play] 文件名
        uint64_t file_offset,           // [file play] 文件偏移
        uint64_t file_size              // [file play] 文件大小
);

/**
 * 设置云录的播放速率
 * @param rate                  [in] support: 0.25x, 0.5x, 1x, 2x, 4x
 * @return
 */
NP_EXPORT int iotsdk_user_cloud_set_play_rate(float rate);

/**
 * 停止播放云存储流媒体数据
 * @param device_id             [in] 设备ＩＤ
 * @param ch_no                 [in] 通道数
 * @return
 */
NP_EXPORT int iotsdk_user_cloud_storage_stop(const char* device_id,
                                             uint32_t ch_no);

/**
 * 解密从云存储下载的数据文件(内部会开启线程和复制数据)
 * @param input_ptr             [in] stream data addr
 * @param input_len             [in] stream data len
 * @param aes_key               [in] aes key
 * @param key_len               [in] key len(256/8)
 * @param play_ctrl             [in] play ctrl(detect fps -> auto wait)
 * @param cb                    [in] callback for play
 * @return ctx
 */
NP_EXPORT int iotsdk_user_cloud_start_data_decode(const char *input_ptr,
                                                    int input_len,
                                                    const char *aes_key, // aes cbc 256
                                                    int key_len,
                                                    bool play_ctrl,
                                                    cb_on_media_data cb);

/**
 * 停止云存储数据解密
 * @param arg                   [in] ctx(return from iotsdk_user_cloud_start_data_decode)
 * @return
 */
    NP_EXPORT int iotsdk_user_cloud_stop_data_decode(void);

#endif

// for cloud storage user
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
// for device
#ifdef mk_compile_device

/**
 * 从内存中加载配置(同样支持使用api设置配置)
 * @param cfg_buff              [in]
 * @param buff_len              [in]
 */
NP_EXPORT void iotsdk_dev_load_cfg(const char* cfg_buff, size_t buff_len);
/**
 * 设置设备信息
 * @param dev_id                [in]
 * @param dev_seckey            [in]
 */
NP_EXPORT void iotsdk_dev_set_dev_info(const char* dev_id, const char* dev_seckey);
/**
 * 发送报警信息
 * @param sub_device_id             [in] sub device id
 * @param event_id                  [in] 对应了报警等级,具体参见:`event_id.md`
 * @param ch_no                     [in] channel no (default 1)
 * @param rich_type                 [in] 0: 没有富媒体;1: JPG; 2: PNG; 11: MP4; 12: AVI; 21: 云存储
 * @param rich_url                  [in] rich_type = 1,2,11,12
 * @param start_time                [in] rich_type = 21； 开始时间,单位秒
 * @param time_len                  [in] 云录像时间长度
 * @param save_site                 [in] 云存储存放区域: 1, 阿里云中国; 2, 阿里云迪拜; 3.....
 * @param msg_title                 [in] msg title
 * @param msg_body                  [in] JSON数据
 * @return
 */
NP_EXPORT int iotsdk_send_message(
        const char* sub_device_id,       // sub device id
        e_event_id  event_id,            // 对应了报警等级,具体参见:`event_id.md`
        int32_t  chh_no,                 // channel no
        e_rich_type rich_type,           // 0: 没有富媒体;1: JPG; 2: PNG; 11: MP4; 12: AVI; 21: 云存储
        const char* rich_url,            // rich_type = 1,2,11,12
        int64_t  start_time,             // rich_type = 21； 开始时间,单位秒
        int64_t  time_len,               // 云录像时间长度
        e_save_site  save_site,          // 云存储存放区域: 1, 阿里云中国; 2, 阿里云迪拜; 3.....
        const char* msg_title,           // msg title
        const char* msg_body             // JSON数据
);

/**
 * 设置硬件信息
 * @param hw_feature            [in] 硬件ID
 */
NP_EXPORT void iotsdk_dev_set_hw_feature(const char* hw_feature);

/**
 * 设置rom版本号
 * @param ver           [in] 版本号
 */
NP_EXPORT void iotsdk_dev_set_rom_ver(const char * ver);

/**
 * 设置产品信息
 * @param device_info
 */
NP_EXPORT void iotsdk_dev_set_prod_info(local_device_t* device_info);

/**
 * 设置⻥眼校证参数
 *　
 */
NP_EXPORT void iotsdk_dev_set_fix_param(const char* fix_param);

/**
 * 设置硬件版本号
 * @param ver           [in] 版本号
 */
NP_EXPORT void iotsdk_dev_set_hdw_ver(const char * ver);

/**
 * 设置rom升级的回调处理函数
 * @param cb            [in] callback
 */
NP_EXPORT void iotsdk_dev_set_on_rom_update(cb_on_rom_update cb);

/**
 * 通知升级状态
 * @param conn_id       [in] 连接ID
 * @param ch_no         [in] 通道数
 * @param rate          [in] step步骤执行的100%
 * @param code          [in] 如果执行错误，此处返回升级的错误代码
 */
NP_EXPORT void iotsdk_dev_notify_rom_update_rate(uint32_t conn_id,
                                                 uint32_t ch_no,
                                                 uint32_t rate,
                                                 uint32_t code);

/**
 * 通知在线能道数
 * @param conn_id
 * @param ch_no
 * @param chans
 * @param chan_count
 */
NP_EXPORT void iotsdk_dev_notice_online_chans(uint32_t conn_id,
                                              uint32_t ch_no,
                                              uint32_t chans_arr[64],
                                              uint32_t chan_count);

/**
 * 发起查询升级信息查询,最新固件信息将通过iotsdk_dev_set_on_rom_update设置的回调返回
 *
 * @remark 当自动升级时由device端主动调用
 */
    NP_EXPORT void iotsdk_dev_query_upgrade(void);

// for device
///////////////////////////////////////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
// for dev ipc
#ifdef mk_compile_ipc

/**
 * 设置局域网认证参数
 * @param auth_user
 * @param auth_pass
 */
NP_EXPORT void iotsdk_dev_set_lan_auth_info(const char *auth_user,
                                            const char *auth_pass);

NP_EXPORT void iotsdk_dev_set_cb_ipc_conn_sync(cb_ipc_conn_sync cb);

/**
 * 设置开启视频回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_start_video(cb_ipc_start_video cb);
/**
 * 开启音频回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_start_audio(cb_ipc_start_audio cb);
/**
 * 开启对讲回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_start_talkback(cb_ipc_start_talkback cb);

/**
 * 停止音频回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_stop_audio(cb_ipc_stop_audio  cb);
/**
 * 停止对聊回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_stop_talkback(cb_ipc_stop_talkback cb);
/**
 * 停止视频回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_stop_video(cb_ipc_stop_video    cb);

/**
 * 设置执行巡视回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_call_psp(cb_ipc_call_psp       cb);

/**
 * 设置周界回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_bc(cb_ipc_get_bc  cb);

/**
 * 设置截屏回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_screen(cb_ipc_get_screen     cb);

/**
 * 设置获取巡视回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_psp(cb_ipc_get_psp        cb);

/**
 * 设置获取时间回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_time(cb_ipc_get_time       cb);

/**
 * 设置获取电源频率回调
 * @param cb                    [in]
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_powerfreq(cb_ipc_get_powerfreq  cb);

/**
 * 设置获取OSD参数回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_osd(cb_ipc_get_osd        cb);

/**
 * 设置获取报警参数回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_alarm(cb_ipc_get_alarm      cb);

/**
 * 设置设置巡视回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_psp(cb_ipc_set_psp       cb);

/**
 * 设置设置彩色制式回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_icr(cb_ipc_set_icr       cb);

/**
 * 设置图片翻转回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_flip(cb_ipc_set_flip      cb);

/**
 * 设置设置时间回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_time(cb_ipc_set_time      cb);

/**
 * 设置设置视频质量回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_video(cb_ipc_set_video     cb);

/**
 * 设置设置电源频率回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_powerfreq(cb_ipc_set_powerfreq cb);

/**
 * 设置设置osd显示回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_osd(cb_ipc_set_osd       cb);

/**
 * 设置设置周界回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_bc(cb_ipc_set_bc        cb);

/**
 * 设置设置报警参数回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_alarm(cb_ipc_set_alarm     cb);

/**
 * 设置设置wifi连接参数回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_wifi(cb_ipc_set_wifi     cb);

/**
 * 设置云台控制回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_ptz_ctrl(cb_ipc_ptz_ctrl     cb);

/**
 * 视置通道回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_chan(cb_ipc_set_chan     cb);

/**
 * 设置高级通道回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_chanadv(cb_ipc_set_chanadv  cb);

/**
 * 设置录像动作回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_action  (cb_ipc_rec_action      cb);

/**
 * 设置录像列表回调(指定某天的录像)
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_list    (cb_ipc_rec_list        cb);

/**
 * 设置录像列表回调(存在录像的某些天)
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_list_day(cb_ipc_rec_list_day    cb);

/**
 * 设置获取录像计划回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_plan_get(cb_ipc_rec_plan_get    cb);

/**
 * 设置设置录像计划的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_plan_set(cb_ipc_rec_plan_set    cb);

/**
 * 设置设置录像速率回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_set_rate(cb_ipc_rec_set_rate    cb);

/**
 * 设置播放录像的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_start   (cb_ipc_rec_start       cb);

/**
 * 设置录像停止的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rec_stop    (cb_ipc_rec_stop        cb);

/**
 * 设置格式化存储设备的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_format_storage (cb_ipc_storage_format cb);

/**
 * 设置获取wifi连接质量的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_wifi_quality(cb_ipc_get_wifi_quality  cb);

/**
 * 设置获取格式化进度的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_storage_format_rate(cb_ipc_storage_format_rate  cb);

/**
 * 设置获取存储设备信息的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_storage_info(cb_ipc_storage_info  cb);

/**
 * 设置执行设备命令的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_device_action(cb_ipc_device_action  cb);

/**
 * 设置获取rom升级模式的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rom_get(cb_ipc_rom_get  cb);

/**
 * 设置设置rom升级模式的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_rom_set(cb_ipc_rom_set  cb);

/**
 * 设置获取在线通道的接口
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_online_chans(cb_ipc_get_online_chans cb);

/**
 * 获取全景图像的⻥眼校正参数的回调
 * @param cb
 */
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_fish_eye_param(cb_ipc_get_fish_eye_param cb);

NP_EXPORT void iotsdk_dev_set_cb_ipc_set_led(cb_ipc_set_led cb);
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_led(cb_ipc_get_led);
NP_EXPORT void iotsdk_dev_set_cb_ipc_set_irlight(cb_ipc_set_irlight);
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_irlight(cb_ipc_get_irlight);

NP_EXPORT void iotsdk_dev_set_cb_ipc_set_event_rec_time(cb_ipc_set_event_rec_time);
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_event_rec_time(cb_ipc_get_event_rec_time);

NP_EXPORT void iotsdk_dev_set_cb_ipc_set_ringer_switch(cb_ipc_set_ringer_switch);
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_ringer_switch(cb_ipc_get_ringer_switch);

NP_EXPORT void iotsdk_dev_set_cb_ipc_set_talkback_volume(cb_ipc_set_talkback_volume);
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_taskback_volume(cb_ipc_get_talkback_volume);

NP_EXPORT void iotsdk_dev_set_cb_ipc_get_battery_status(cb_ipc_get_battery_status);

NP_EXPORT void iotsdk_dev_set_cb_ipc_set_dont_disturb(cb_ipc_set_dont_disturb );
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_dont_disturb(cb_ipc_get_dont_disturb );

NP_EXPORT void iotsdk_dev_set_cb_ipc_set_motion_detection(cb_ipc_set_motion_detection );
NP_EXPORT void iotsdk_dev_set_cb_ipc_get_motion_detection(cb_ipc_get_motion_detection );


#endif

// for dev ipc
///////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
}
#endif


#endif
