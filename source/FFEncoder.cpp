//
//  MSEncoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFEncoder.hpp"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;

void
FFEncoder::beginEncode() {
    _isEncoding = true;
    this->filePath = filePath;
    
    int ret = avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, filePath.c_str());
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        assert(ret == 0);
    }
    
}

void
FFEncoder::encodeVideo(const MSEncoderInputData &pixelData) {
    assert(_isEncoding);
    
}

void
FFEncoder::encodeAudio(const MSEncoderInputData &sampleData) {
    assert(_isEncoding);

}

void
FFEncoder::endEncode() {
    _isEncoding = false;
    
    // 释放编码环境
    if (videoEncoderContext) {
        delete videoEncoderContext;
        videoEncoderContext = nullptr;
    }
    
    if (audioEncoderContext) {
        delete audioEncoderContext;
        audioEncoderContext = nullptr;
    }
    
    if (outputFormatContext) {
        avformat_free_context(outputFormatContext);
    }
}

bool
FFEncoder::isEncoding() {
    return _isEncoding;
}

FFEncoder::FFEncoder(const MSCodecID videoCodecID,
                     const MSCodecID audioCodecID)
:videoCodecID(videoCodecID), audioCodecID(audioCodecID) {
    
}

FFEncoder::~FFEncoder() {
    endEncode();
}

bool
FFEncoder::configureEncoder(const string muxingfilePath,
                            const FFCodecContext &videoDecoderContext,
                            const FFCodecContext &audioDecoderContext) {
    
  
    int ret = avio_open(&outputFormatContext->pb, filePath.c_str(), AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        return nullptr;
    }
    
    ret = avformat_write_header(outputFormatContext, nullptr);
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        return nullptr;
    }
    
    return true;
}

AVFormatContext *
FFEncoder::configureOutputFormatContext() {
    return nullptr;
}

FFCodecContext *
FFEncoder::configureVideoEncoderContext(const FFCodecContext &videoDecoderContext) {
    FFCodecContext *videoEncoderContext = new FFCodecContext(FFCodecEncoder,videoCodecID);
    AVCodecContext &encoderContext = *videoEncoderContext->codec_ctx;
    const AVCodecContext &decoderContext = *videoDecoderContext.codec_ctx;
    
    // 编码器参数配置
    AVDictionary *dict = nullptr;
    if (outputFormatContext->oformat->video_codec == AV_CODEC_ID_H264) {
        av_dict_set(&dict, "preset", "medium", 0);
        av_dict_set(&dict, "tune", "zerolatency", 0);
        
        encoderContext.profile  = FF_PROFILE_H264_HIGH;
    }
    encoderContext.pix_fmt      = decoderContext.pix_fmt;
    encoderContext.width        = decoderContext.width;
    encoderContext.height       = decoderContext.height;
    encoderContext.gop_size     = decoderContext.gop_size;
    encoderContext.qmin         = decoderContext.qmin;
    encoderContext.qmax         = decoderContext.qmax;
    encoderContext.max_b_frames = decoderContext.max_b_frames;
    encoderContext.time_base    = av_inv_q(decoderContext.framerate);
    encoderContext.sample_aspect_ratio = decoderContext.sample_aspect_ratio;
    if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        encoderContext.flags   |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    int ret = avcodec_open2(&encoderContext, videoEncoderContext->codec, &dict);
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        return nullptr;
    }
    
    av_dict_free(&dict);
    
    AVStream *videoStream = avformat_new_stream(outputFormatContext, videoEncoderContext->codec);
    
    ret = avcodec_parameters_from_context(videoStream->codecpar, &encoderContext);
    if (ret < 0) {
        printf("error: %s\n",av_err2str(ret));
        return nullptr;
    }
    
    return videoEncoderContext;
}

FFCodecContext *
FFEncoder::configureAudioEncoderContext(const FFCodecContext &audioDecoderContext) {
    FFCodecContext *audioEncoderContext = new FFCodecContext(FFCodecEncoder,audioCodecID);
    AVCodecContext &encoderContext = *audioEncoderContext->codec_ctx;
    const AVCodecContext &decoderContext = *audioDecoderContext.codec_ctx;
    encoderContext.profile = FF_PROFILE_AAC_MAIN;
    encoderContext.sample_fmt = decoderContext.sample_fmt;
    
    
    avcodec_open2(&encoderContext, audioEncoderContext->codec, nullptr);
    
    AVStream *audioStream;
    return audioEncoderContext;
}

