//
//  MSDecoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFDecoder.hpp"

using namespace std;
using namespace MS::FFmpeg;

const FFDecoderProtocol::MSDecoderOutputData *
FFDecoder::decodeVideo(const MSMediaData<isEncode> &videoData) {
    return decodeData(videoData);
}

const FFDecoderProtocol::MSDecoderOutputData *
FFDecoder::decodeAudio(const MSMediaData<isEncode> &audioData) {
    return decodeData(audioData);
}

FFDecoder::FFDecoder() {
    
}

FFDecoder::~FFDecoder() {
    // 释放解码器
    for (auto element : decoderContexts) {
        delete element.second;
    }
    decoderContexts.clear();
}

const FFCodecContext &
FFDecoder::getDecoderContext(MSCodecID codecID) {
    FFCodecContext *decoderContext = decoderContexts[codecID];
    if (!decoderContext) {
        decoderContext = new FFCodecContext(FFCodecDecoder, codecID);
        decoderContexts[codecID] = decoderContext;
    }
    return *decoderContext;
}

const FFDecoderProtocol::MSDecoderOutputData *
FFDecoder::decodeData(const MSMediaData<isEncode> &data) {
    const FFCodecContext &decoderContext = getDecoderContext(data.content->codecID);
    AVCodecContext *codec_ctx = decoderContext.codec_ctx;
    AVPacket packet{0};
    packet.data = data.content->packt;
    packet.size = static_cast<int>(data.content->size);
    
    AVFrame * const frame = av_frame_alloc();
    
    int ret = avcodec_send_packet(codec_ctx, &packet);
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        ErrorLocationLog;
        av_packet_unref(&packet);
        return nullptr;
    }
    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        ErrorLocationLog;
        av_packet_unref(&packet);
        return nullptr;
    }
    
    int rate = 0;
    
    if (data.content->codecID <= MSCodecID_HEVC) {
        rate = codec_ctx->framerate.num / codec_ctx->framerate.den;
    } else {
        rate = codec_ctx->sample_rate;
    }
    
    MSDecoderOutputContent *content = new MSDecoderOutputContent(frame,
                                                                 intervale(rate),
                                                                 av_frame_free,
                                                                 av_frame_clone);
    return new MSDecoderOutputData(content);
}
