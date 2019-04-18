#ifndef _mk_mrpc_header_def_h
#define _mk_mrpc_header_def_h

#include <stdint.h>


// for udp fix header define
extern unsigned char g_fix_udp_header[] ;
#define  g_fix_udp_header_len 2


// 流媒体类型定义
typedef enum
{
    e_stream_type_H264      = 1  , // 视频 H264
    e_stream_type_H265      = 2  , // 视频 H265
    e_stream_type_MPEG      = 3  , // 视频 MPEG
    e_stream_type_MJPEG     = 4  , // 视频 MJPEG
    e_stream_type_G711A     = 21 , // 音频 G711A
    e_stream_type_ULAW      = 22 , // 音频 ULAW
    e_stream_type_G711U     = 23 , // 音频 G711U
    e_stream_type_PCM       = 24 , // 音频 PCM
    e_stream_type_ADPCM     = 25 , // 音频 ADPCM
    e_stream_type_G721      = 26 , // 音频 G721
    e_stream_type_G723      = 27 , // 音频 G723
    e_stream_type_G726_16   = 28 , // 音频 G726_16
    e_stream_type_G726_24   = 29 , // 音频 G726_24
    e_stream_type_G726_32   = 30 , // 音频 G726_32
    e_stream_type_G726_40   = 31 , // 音频 G726_40
    e_stream_type_AAC       = 32 , // 音频 AAC
    e_stream_type_Speex     = 33 , // 音频 Speex
    e_stream_type_Opus      = 34 , // 音频 Opus
} e_stream_type ;

// 固定头部
typedef struct
{
    unsigned char ver;                      // c_mrpc_tcp_ver

    union {
        struct {
            unsigned char var_hdr_len:5;    // 可变头部长度
            unsigned char type:3;           // e_cmd_ctrl_bin
        };
        unsigned char data;
    };
} __attribute__((packed)) header_t;

// 指令类型
typedef enum _e_cmd_type
{
    e_cmd_request  = 0,     // 请求指令
    e_cmd_response = 1      // 响应指令
} e_cmd_type;

// 控制指令的扩展头部
typedef struct
{
    uint64_t command_id;
    uint64_t command_seq;
    union {
        struct {
            unsigned char cmd_type:1;           // req or resp (e_cmd_type)
            unsigned char encrypt_type:7;       // encrypt type
        };
        unsigned char command_flag;
    };
    uint64_t command_code;                      // only for response packet
    uint64_t payload_len;
}header_ex_t;

// 自定义数据的扩展头部
typedef struct
{
    uint64_t payload_len;
} header_self_t;

// 流媒体数据的头部
typedef struct
{
    uint64_t stream_seq_;                      // 流媒体包的序号
    union {
        struct {
            unsigned char stream_type:7;       // media stream type describe
            unsigned char is_key_frame:1;      // is key frame or not
        };
        unsigned char stream_format_;           // 流媒体格式描述,具体参⻅流媒体编码格式描述部分
    };
    uint8_t   channel_number_;                  // 通道编号,可以在一条连接中进行多路音视频的传输,主要用于NVR/DVR
    uint8_t   encrypt_flag_;                    // 流媒体加密类型,具体数值和加密类型对应关系
    // 参⻅文档加密类型部分;如果支持加密,加密为
    // 一帧一密钥;密钥计算方法参⻅:流数据密钥计算
    // 方法
    uint64_t  timestamp_;                       // 媒体包时间戳,单位毫秒
    uint64_t  encrypt_length_;                  // 加密数据的⻓度,对于流媒体加密的数据⻓度;0,
    // 表示Payload数据全部加密
    uint64_t  payload_length_;                  // 协议载荷的⻓度,即后续数据⻓度

    // extend usage
    uint64_t  file_offset_;                     // for cloud play

} header_media_t;

// 加密类型
typedef enum
{
    e_enc_none = 0,
    e_enc_aes_128_cbc = 1,
    e_enc_aes_128_cfb = 2,
    e_enc_aes_128_ecb = 3,
    e_enc_aes_128_ofb = 4,
    e_enc_aes_128_ctr = 5,
    e_enc_aes_192_cbc = 6,
    e_enc_aes_192_cfb = 7,
    e_enc_aes_192_ecb = 8,
    e_enc_aes_192_ofb = 9,
    e_enc_aes_192_ctr = 10,
    e_enc_aes_256_cbc = 11,
    e_enc_aes_256_cfb = 12,
    e_enc_aes_256_ecb = 13,
    e_enc_aes_256_ofb = 14,
    e_enc_aes_256_ctr = 15
} e_mrpc_enc_type;
//current system support type: 0,1,2,6,7,11,12



#endif