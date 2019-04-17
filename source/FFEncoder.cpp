//
//  MSEncoder.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "FFEncoder.hpp"

using namespace std;
using namespace MS;
using namespace MS::FFmpeg;

void
FFEncoder::beginEncode() {
    assert(outputFormatContext && (videoEncoderContext || audioEncoderContext));
    
    videoPts = 0;
    audioPts = 0;
    
    int ret = avio_open(&outputFormatContext->pb, filePath.c_str(), AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        releaseEncoderConfiguration();
        return;
    }
    
    ret = avformat_write_header(outputFormatContext, nullptr);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        releaseEncoderConfiguration();
        return;
    }
    _isEncoding = true;
}

bool
FFEncoder::isEncoding() {
    return _isEncoding;
}

void
FFEncoder::encodeVideo(const FFEncoderInputMedia &pixelData) {
    assert(_isEncoding);

    if (videoEncoderContext) {
        AVFrame &frame = *pixelData.frame;
        videoPts += 1;
        frame.pts = videoPts;
        frame.pict_type = AV_PICTURE_TYPE_NONE;
    
        encodeData(&frame, videoStream, videoEncoderContext->codec_ctx);
    }
}

void
FFEncoder::encodeAudio(const FFEncoderInputMedia &sampleData) {
    assert(_isEncoding);

    if (audioEncoderContext) {
        AVFrame &frame = *sampleData.frame;
        audioPts += frame.nb_samples;
        frame.pts = audioPts;
        frame.pict_type = AV_PICTURE_TYPE_NONE;
        
        encodeData(&frame, audioStream, audioEncoderContext->codec_ctx);
    }
}

void
FFEncoder::endEncode() {
    while (!fileWriteMutex.try_lock());
    
    _isEncoding = false;
    
    avio_flush(outputFormatContext->pb);
    
    int ret = av_write_trailer(outputFormatContext);
    
    fileWriteMutex.unlock();
    
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
    }
    releaseEncoderConfiguration();
}

FFEncoder::FFEncoder(const MSCodecID videoCodecID,
                     const MSCodecID audioCodecID)
:videoCodecID(videoCodecID), audioCodecID(audioCodecID) {
    
}

FFEncoder::~FFEncoder() {
    if (_isEncoding) {    
        endEncode();
    } else {
        releaseEncoderConfiguration();
    }
}

bool
FFEncoder::configureEncoder(const string &muxingfilePath,
                            const FFCodecContext * const videoDecoderContext,
                            const FFCodecContext * const audioDecoderContext) {
    filePath = muxingfilePath;
    
    outputFormatContext = configureOutputFormatContext();
    if (!outputFormatContext) {
        releaseEncoderConfiguration();
        return false;
    }
    
    if (videoDecoderContext) {
        videoEncoderContext = configureVideoEncoderContext(*videoDecoderContext);
        if (!videoEncoderContext) {
            releaseEncoderConfiguration();
            return false;
        }
    }
    
    if (audioDecoderContext) {
        audioEncoderContext = configureAudioEncoderContext(*audioDecoderContext);
        if (!audioEncoderContext) {
            releaseEncoderConfiguration();
            return false;
        }
    }
    
    return true;
}

AVFormatContext *
FFEncoder::configureOutputFormatContext() {
    AVFormatContext *outputFormatContext = nullptr;
    int ret = avformat_alloc_output_context2(&outputFormatContext, nullptr, nullptr, filePath.c_str());
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        return nullptr;
    }
    
    outputFormatContext->oformat->video_codec = FFCodecContext::getAVCodecId(videoCodecID);
    outputFormatContext->oformat->audio_codec = FFCodecContext::getAVCodecId(audioCodecID);
    
    return outputFormatContext;
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
        
        encoderContext.profile  = decoderContext.profile;
    }
    
    encoderContext.pix_fmt      = decoderContext.pix_fmt;
    encoderContext.width        = decoderContext.width;
    encoderContext.height       = decoderContext.height;
    encoderContext.qmin         = decoderContext.qmin;
    encoderContext.qmax         = decoderContext.qmax;
    encoderContext.bit_rate     = decoderContext.width * decoderContext.height * 3 * 2;
    encoderContext.gop_size     = 100;
    encoderContext.max_b_frames = 0;
    encoderContext.time_base    = av_inv_q(decoderContext.framerate);
    encoderContext.sample_aspect_ratio = decoderContext.sample_aspect_ratio;
    if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        encoderContext.flags   |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    int ret = avcodec_open2(&encoderContext, videoEncoderContext->codec, &dict);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        delete videoEncoderContext;
        return nullptr;
    }
    
    av_dict_free(&dict);
    
    AVStream &outStream = *avformat_new_stream(outputFormatContext, videoEncoderContext->codec);
    
    ret = avcodec_parameters_from_context(outStream.codecpar, &encoderContext);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        delete videoEncoderContext;
        return nullptr;
    }
    outStream.time_base = encoderContext.time_base;
    
    videoStream = &outStream;
    
    return videoEncoderContext;
}

FFCodecContext *
FFEncoder::configureAudioEncoderContext(const FFCodecContext &audioDecoderContext) {
    FFCodecContext *audioEncoderContext = new FFCodecContext(FFCodecEncoder,audioCodecID);
    AVCodecContext &encoderContext = *audioEncoderContext->codec_ctx;
    const AVCodecContext &decoderContext = *audioDecoderContext.codec_ctx;
    
    // 编码器参数配置
    if (encoderContext.codec_id == AV_CODEC_ID_AAC) {
        // FFmpeg 只支持 FF_PROFILE_AAC_LOW(可以不设置, 内部默认采用也是 FF_PROFILE_AAC_LOW)
        encoderContext.profile = FF_PROFILE_AAC_LOW;
    }
    
    encoderContext.sample_fmt       = decoderContext.sample_fmt;
    encoderContext.frame_size       = decoderContext.frame_size;
    encoderContext.bit_rate         = decoderContext.bit_rate;
    encoderContext.sample_rate      = decoderContext.sample_rate;
    encoderContext.time_base        = (AVRational){1, encoderContext.sample_rate};
    encoderContext.channels         = decoderContext.channels;
    encoderContext.channel_layout   = decoderContext.channel_layout;
    encoderContext.strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    if (outputFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        encoderContext.flags       |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    
    int ret = avcodec_open2(&encoderContext, audioEncoderContext->codec, nullptr);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        delete audioEncoderContext;
        return nullptr;
    }
    
    AVStream &outStream = *avformat_new_stream(outputFormatContext, audioEncoderContext->codec);
    
    ret = avcodec_parameters_from_context(outStream.codecpar, &encoderContext);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        delete audioEncoderContext;
        return nullptr;
    }
    outStream.time_base = encoderContext.time_base;
    
    audioStream = &outStream;
    
    return audioEncoderContext;
}

void
FFEncoder::releaseEncoderConfiguration() {
    if (videoEncoderContext) {
        delete videoEncoderContext;
        videoEncoderContext = nullptr;
    }
    if (audioEncoderContext) {
        delete audioEncoderContext;
        audioEncoderContext = nullptr;
    }
    if (outputFormatContext) {
        avio_closep(&outputFormatContext->pb);
        avformat_free_context(outputFormatContext);
        outputFormatContext = nullptr;
    }
    if (videoStream) {
        videoStream = nullptr;
    }
    if (audioStream) {
        audioStream = nullptr;
    }
}

void
FFEncoder::encodeData(AVFrame * const frame,
                      AVStream * const outStream,
                      AVCodecContext * const encoderContext) {
    AVPacket packet;
    av_init_packet(&packet);
    
    int ret = avcodec_send_frame(encoderContext, frame);
    if (ret < 0) {
        ErrorLocationLog(av_err2str(ret));
        return;
    }
    
    ret = avcodec_receive_packet(encoderContext, &packet);
    if (ret < 0) {
        if (ret != AVERROR(EAGAIN)) { 
            ErrorLocationLog(av_err2str(ret));
        }
        return;
    }
    
    av_packet_rescale_ts(&packet, encoderContext->time_base, outStream->time_base);
    packet.stream_index = outStream->index;
    
    while (!fileWriteMutex.try_lock());
    if (_isEncoding) {
        ret = av_interleaved_write_frame(outputFormatContext, &packet);
        fileWriteMutex.unlock();
        if (ret < 0) {
            ErrorLocationLog(av_err2str(ret));
        }
    }
}
