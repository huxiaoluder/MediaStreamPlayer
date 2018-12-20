//
//  MSCodecSupport.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/28.
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
realCodecID(getAVCodecId()),
codec(initCodec()),
codec_ctx(initCodecContext()) {
    assert(codecID != MSCodecID_None && codec_ctx != nullptr);
}

FFCodecContext::FFCodecContext(const FFCodecContext & codecContext)
:codecType(codecContext.codecType),
codecID(codecContext.codecID),
realCodecID(getAVCodecId()),
codec(initCodec()),
codec_ctx(initCodecContext()) {
    
}

FFCodecContext::~FFCodecContext() {
    AVCodecContext *_codec_ctx = const_cast<AVCodecContext *>(codec_ctx);
    avcodec_free_context(&_codec_ctx);
}

AVCodecID
FFCodecContext::getAVCodecId() {
    AVCodecID codec_id;
    switch (codecID) {
        case MSCodecID_None:    codec_id = AV_CODEC_ID_NONE;    break;
        case MSCodecID_H264:    codec_id = AV_CODEC_ID_H264;    break;
        case MSCodecID_H265:    codec_id = AV_CODEC_ID_HEVC;    break;
        case MSCodecID_AAC:     codec_id = AV_CODEC_ID_AAC;     break;
        case MSCodecID_G711:    codec_id = AV_CODEC_ID_NONE;    break;
        case MSCodecID_OPUS:    codec_id = AV_CODEC_ID_OPUS;    break;
    }
    return codec_id;
}

AVCodec *
FFCodecContext::initCodec() {
    AVCodec *codec = nullptr;
    if (codecType == FFCodecDecoder) {
        codec = avcodec_find_decoder(realCodecID);
    } else {
        codec = avcodec_find_encoder(realCodecID);
    }
    return codec;
}

AVCodecContext *
FFCodecContext::initCodecContext() {
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    
    if (codecType == FFCodecDecoder) {
        int ret = avcodec_open2(codec_ctx, codec, nullptr);
        
        if (ret < 0) {
            printf("error: %s\n",av_err2str(ret));
            assert(ret == 0);
        }
    }
    
    return codec_ctx;
}

AVFormatContext * 
FFCodecContext::initFormatContex() {
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    return fmt_ctx;
}
