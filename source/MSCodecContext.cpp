//
//  MSCodecSupport.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSCodecContext.hpp"

using namespace MS;

MSCodecContext::MSCodecContext(MSCodecType codecType, MSCodecID codecID)
:codecType(codecType), codecID(codecID),
codec_ctx(initCodecContext()), fmt_ctx(initFormatContex()) {
    assert(codecID != MSCodecID_None && codec_ctx != nullptr);
}

MSCodecContext::~MSCodecContext() {
    avcodec_close(codec_ctx);
    av_free(codec_ctx);
    avformat_free_context(fmt_ctx);
}

AVCodecID const
MSCodecContext::getAVCodecId() {
    AVCodecID codec_id;
    switch (codecID) {
        case MSCodecID_None:    codec_id = AV_CODEC_ID_NONE; break;
        case MSCodecID_H264:    codec_id = AV_CODEC_ID_H264; break;
        case MSCodecID_H265:    codec_id = AV_CODEC_ID_NONE; break;
        case MSCodecID_AAC:     codec_id = AV_CODEC_ID_AAC;  break;
        case MSCodecID_G711:    codec_id = AV_CODEC_ID_NONE; break;
        case MSCodecID_OPUS:    codec_id = AV_CODEC_ID_OPUS; break;
    }
    return codec_id;
}

AVCodecContext * const
MSCodecContext::initCodecContext() {
    AVCodec *codec = nullptr;
    if (codecType == MSCodecDecoder) {
        codec = avcodec_find_decoder(getAVCodecId());
    } else {
        codec = avcodec_find_encoder(getAVCodecId());
    }
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    avcodec_open2(codec_ctx, codec, nullptr);
    return codec_ctx;
}

AVFormatContext * const
MSCodecContext::initFormatContex() {
    AVFormatContext *fmt_ctx = avformat_alloc_context();
    return fmt_ctx;
}
