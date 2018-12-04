//
//  MSDecoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFDecoder.hpp"

using namespace MS;

using namespace std;

FFDecoderProtocol::outputType * const
FFDecoder::decodeVideo(const MSMediaData<isEncode> &videoData) {
    auto decoderContext = getDecoderContext(videoData.content.codecID);
    auto &codec_ctx = *decoderContext.codec_ctx;
    AVPacket packet;
    packet.data = videoData.content.packt;
    packet.size = static_cast<int>(videoData.content.size);
    AVFrame *frame = nullptr;
    avcodec_send_packet(&codec_ctx, &packet);
    avcodec_receive_frame(&codec_ctx, frame);
//    AVPixelFormat
//    frame->sample_rate
//    AVSampleFormat
//    frame->format;
    
//    MSData *data = new MSData(<#const uint8_t *const bytes#>, <#const size_t len#>, <#const MSCodecID codecID#>, <#const MSMediaFormat mediaFormat#>, <#const std::chrono::microseconds timeInterval#>);
    return nullptr;
}

FFDecoderProtocol::outputType * const
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

const MSCodecContext &
FFDecoder::getDecoderContext(MSCodecID codecID) {
    auto decoderContext = decoderContexts[codecID];
    if (!decoderContext) {
        decoderContext = new MSCodecContext(MSCodecEncoder, codecID);
        auto pair = decltype(decoderContexts)::value_type(codecID, decoderContext);
        decoderContexts.insert(pair);
    }
    return *decoderContext;
}

