//
//  MSCodecSupport.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFCodecContext.hpp"

using namespace MS::FFmpeg;

void
MS::FFmpeg::av_frame_free(AVFrame * const frame) {
    AVFrame *_frame = const_cast<AVFrame *>(frame);
    av_frame_free(&_frame);
}

FFCodecContext::FFCodecContext(const FFCodecType codecType, const MSCodecID codecID)
:codecType(codecType),
codecID(codecID),
codec(initCodec()),
codec_ctx(initCodecContext()) {
    assert(codec_ctx && codec);
}

FFCodecContext::FFCodecContext(const FFCodecContext &codecContext)
:codecType(codecContext.codecType),
codecID(codecContext.codecID),
codec(initCodec()),
codec_ctx(initCodecContext()) {
    
}

FFCodecContext::~FFCodecContext() {
    AVCodecContext *_codec_ctx = const_cast<AVCodecContext *>(codec_ctx);
    avcodec_free_context(&_codec_ctx);
}

AVCodec *
FFCodecContext::initCodec() {
    AVCodec *codec = nullptr;
    if (codecType == FFCodecDecoder) {
        codec = avcodec_find_decoder(getAVCodecId(codecID));
    } else {
        codec = avcodec_find_encoder(getAVCodecId(codecID));
    }
    return codec;
}

AVCodecContext *
FFCodecContext::initCodecContext() {
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    
    if (codecID == MSCodecID_ALAW) {
        // ALAW 标准不含声道数, 必须设置
        codec_ctx->channels = 1;
        // 用于提供渲染频率使用(解码器可忽略)
        codec_ctx->sample_rate = 8000;
        // codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;
        // codec_ctx->channel_layout = av_get_channel_layout_nb_channels(1);
    }
    
    // 这里只打开解码器, 编码器需要额外配置编码参数, 交给 FFEncoder 处理
    if (codecType == FFCodecDecoder) {
        int ret = avcodec_open2(codec_ctx, codec, nullptr);
        
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
        }
        assert(ret == 0);
    }
    
    return codec_ctx;
}

AVFormatContext * 
FFCodecContext::initFormatContex() {
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    return fmt_ctx;
}

AVCodecID
FFCodecContext::getAVCodecId(const MSCodecID codecID) {
    AVCodecID codec_id;
    switch (codecID) {
        case MSCodecID_None:    codec_id = AV_CODEC_ID_NONE;        break;
        case MSCodecID_H264:    codec_id = AV_CODEC_ID_H264;        break;
        case MSCodecID_H265:    codec_id = AV_CODEC_ID_HEVC;        break;
        case MSCodecID_AAC:     codec_id = AV_CODEC_ID_AAC;         break;
        case MSCodecID_OPUS:    codec_id = AV_CODEC_ID_OPUS;        break;
        case MSCodecID_ALAW:    codec_id = AV_CODEC_ID_PCM_ALAW;    break;
    }
    return codec_id;
}
