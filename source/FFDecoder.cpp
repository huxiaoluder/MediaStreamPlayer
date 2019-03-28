//
//  MSDecoder.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFDecoder.hpp"

using namespace std;
using namespace MS::FFmpeg;

const FFDecoderOutputMedia *
FFDecoder::decodeVideo(const MSMedia<MSEncodeMedia> * MSNonnull const videoData) {
    return decodeData(videoData);
}

const FFDecoderOutputMedia *
FFDecoder::decodeAudio(const MSMedia<MSEncodeMedia> * MSNonnull const audioData) {
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
FFDecoder::getDecoderContext(const MSCodecID codecID) {
    FFCodecContext *decoderContext = decoderContexts[codecID];
    if (!decoderContext) {
        decoderContext = new FFCodecContext(FFCodecDecoder, codecID);
        decoderContexts[codecID] = decoderContext;
    }
    return *decoderContext;
}

const FFDecoderOutputMedia *
FFDecoder::decodeData(const MSMedia<MSEncodeMedia> * const mediaData) {
    const MSMedia<MSEncodeMedia> &data = *mediaData;
    const FFCodecContext &decoderContext = getDecoderContext(data.codecID);
    AVCodecContext *codec_ctx = decoderContext.codec_ctx;
    
    AVPacket packet;
    av_init_packet(&packet);
    
    packet.data = data.naluData;
    packet.size = static_cast<int>(data.naluSize);
    
    AVFrame * const frame = av_frame_alloc();
    
    int ret = avcodec_send_packet(codec_ctx, &packet);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        av_packet_unref(&packet);
        delete mediaData;
        return nullptr;
    }
    ret = avcodec_receive_frame(codec_ctx, frame);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        av_packet_unref(&packet);
        delete mediaData;
        return nullptr;
    }
    
    long long rate = 0;
    microseconds timeInterval;
    
    if (data.codecID <= MSCodecID_HEVC) {
        rate = (long long)codec_ctx->framerate.num / codec_ctx->framerate.den;
        timeInterval = intervale(rate);
    } else {
        rate = ((long long)codec_ctx->sample_rate << 16) / frame->nb_samples;
        timeInterval = adv_intervale(rate);
    }
    
    av_packet_unref(&packet);
    
    return new FFDecoderOutputMedia(frame,
                                    timeInterval,
                                    mediaData,
                                    av_frame_free,
                                    av_frame_clone);
}

const FFCodecContext * 
FFDecoder::findDecoderContext(MSCodecID codecID) {
    return decoderContexts[codecID];
}
