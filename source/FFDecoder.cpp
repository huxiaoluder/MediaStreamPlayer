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

FFDecoderProtocol::MSOutputData * const
FFDecoder::decodeVideo(const MSMediaData<isEncode> &videoData) {
    auto decoderContext = getDecoderContext(videoData.content->codecID);
    auto codec_ctx = decoderContext.codec_ctx;
    
    AVPacket packet;
    packet.data = videoData.content->packt;
    packet.size = static_cast<int>(videoData.content->size);
    
    AVFrame *frame = nullptr;
    
    int ret = avcodec_send_packet(codec_ctx, &packet);
    if (ret < 0) {
        av_err2str(ret);
        ErrorLocationLog;
        av_packet_unref(&packet);
        return nullptr;
    }
    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret < 0) {
        av_err2str(ret);
        ErrorLocationLog;
        av_packet_unref(&packet);
        return nullptr;
    }
    
    int rate = 0;
    
    if (videoData.content->codecID <= MSCodecID_HEVC) {
        rate = codec_ctx->framerate.num * codec_ctx->framerate.den;
    } else {
        rate = codec_ctx->sample_rate;
    }
    
    auto content = new MSOutputContent(frame,
                                       intervale(rate),
                                       av_frame_free,
                                       av_frame_clone);
    return new MSOutputData(content);
}

FFDecoderProtocol::MSOutputData * const
FFDecoder::decodeAudio(const MSMediaData<isEncode> &audioData) {
    return nullptr;
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
    auto decoderContext = decoderContexts[codecID];
    if (!decoderContext) {
        decoderContext = new FFCodecContext(FFCodecDecoder, codecID);
        auto pair = decltype(decoderContexts)::value_type(codecID, decoderContext);
        decoderContexts.insert(pair);
    }
    return *decoderContext;
}

