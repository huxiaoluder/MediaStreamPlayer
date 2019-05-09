//
//  APCodecContext.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "APCodecContext.hpp"

using namespace MS;
using namespace MS::APhard;

CGImageRef
APhard::permuteARGBBitmap(CGImageRef const image, CGBitmapInfo const bitmapInfo) {
    // 这里提供两种方式作为助记******
    // 注: 使用函数 vImagePermuteChannels_ARGB8888(...), 是性能更好的方式(Accelerate.framework 的指令优化)
    size_t width = CGImageGetWidth(image);
    size_t height = CGImageGetHeight(image);

    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    void *bitmapData = malloc(width * height);
    
    CGContextRef context = CGBitmapContextCreate (bitmapData,
                                                  width,
                                                  height,
                                                  8,
                                                  width,
                                                  colorSpace,
                                                  bitmapInfo);

    CGRect rect = {{0,0},{(CGFloat)width, (CGFloat)height}};
    CGContextDrawImage(context, rect, image);
    
    CGColorSpaceRelease(colorSpace);
    CGContextRelease(context);
    
    return CGBitmapContextCreateImage(context);
}

APYUV420PTexture::APYUV420PTexture(CGImageRef image) {
    vImage_Error err = kvImageNoError;

    vImage_CGImageFormat format {
        .bitsPerComponent = 8,
        .bitsPerPixel = 32,
        .colorSpace = CGColorSpaceCreateDeviceRGB(), // don't forget to release this!
        .bitmapInfo = kCGImageAlphaFirst, // 想要的 ARGB 排列方式
        .version = 0, // must be 0
        .decode = nullptr,
        .renderingIntent = kCGRenderingIntentDefault
    };
    
    vImage_Buffer srcImg;
    err = vImageBuffer_InitWithCGImage(&srcImg, &format, nullptr, image, kvImageNoFlags);
    if (err) {
        ErrorLocationLog("call vImageBuffer_InitWithCGImage fail");
    }
    
    // 该函数初始化 vImage_Buffer, 会产生对齐 rowByte 会比 width 大, 是数据不连续, 需求不符合u要求, 所以手动初始化
    // vImageBuffer_Init(&Yp, srcImg.height, srcImg.width, 8, kvImageNoFlags);
    size_t width = srcImg.width;
    size_t height = srcImg.height;
    size_t length = width * height;
    
    size_t halfWidth = width >> 1;
    size_t halfHeight = height >> 1;
    size_t quarterLength = length >> 2;
    
    Yp = {0, height, width, width};
    Cb = {0, halfHeight, halfWidth, halfWidth};
    Cr = {0, halfHeight, halfWidth, halfWidth};
    Yp.data = malloc(length);
    Cb.data = malloc(quarterLength);
    Cr.data = malloc(quarterLength);

    // 转换参数
    vImage_ARGBToYpCbCr info{0};
    vImage_YpCbCrPixelRange pixelRange{0, 128, 255, 255, 255, 1, 255, 0};
    err = vImageConvert_ARGBToYpCbCr_GenerateConversion(kvImage_ARGBToYpCbCrMatrix_ITU_R_709_2,
                                                        &pixelRange,
                                                        &info,
                                                        kvImageARGB8888,
                                                        kvImage420Yp8_Cb8_Cr8,
                                                        kvImagePrintDiagnosticsToConsole);
    if (err) {
        ErrorLocationLog("call vImageConvert_ARGBToYpCbCr_GenerateConversion fail");
    }
    
    // permuteMap 定义了源数据中 ARGB 的排列顺序:
    // permuteMap 的 index --> {0:A, 1:R, 2:G, 3:B}
    // permuteMap 的 value --> ARGB 在源数据单位像素中的 index
    uint8_t permuteMap[4] = {0, 1, 2, 3};
    // ARGB 转 Yp8_Cb8_Cr8
    err = vImageConvert_ARGB8888To420Yp8_Cb8_Cr8(&srcImg, &Yp, &Cb, &Cr, &info, permuteMap, kvImageNoFlags);
    if (err) {
        ErrorLocationLog("call vImageConvert_ARGB8888To420Yp8_Cb8_Cr8 fail");
    }
    
    CGColorSpaceRelease(format.colorSpace);
    free(srcImg.data);
}

APYUV420PTexture::~APYUV420PTexture() {
    if (Yp.data) {
        free(Yp.data);
    }
    if (Cb.data) {
        free(Cb.data);
    }
    if (Cr.data) {
        free(Cr.data);
    }
}

APFrame::APFrame(__CVBuffer  * MSNonnull const video, const MSVideoParameters &videoParameters)
:video(video), videoParameters(videoParameters) {
    
}

APFrame::APFrame(AudioBuffer * MSNonnull const audio, const MSAudioParameters &audioParameters)
:audio(audio), audioParameters(audioParameters) {
    
}

void
APFrame::freeVideoFrame(const APFrame * MSNonnull const frame) {
    CVPixelBufferRelease(frame->video);
    delete frame;
}

void
APFrame::freeAudioFrame(const APFrame * MSNonnull const frame) {
    free(frame->audio->mData);
    delete frame->audio;
    delete frame;
}

APFrame * MSNonnull
APFrame::copyVideoFrame(const APFrame * MSNonnull const frame) {
    return new APFrame(CVPixelBufferRetain(frame->video), frame->videoParameters);
}

APFrame * MSNonnull
APFrame::copyAudioFrame(const APFrame * MSNonnull const frame) {
    AudioBuffer *audio = new AudioBuffer();
    audio->mData = malloc(frame->audio->mDataByteSize);
    audio->mDataByteSize = frame->audio->mDataByteSize;
    audio->mNumberChannels = frame->audio->mNumberChannels;
    memcpy(audio->mData, frame->audio->mData, audio->mDataByteSize);
    return new APFrame(audio, frame->audioParameters);
}


APCodecContext::APCodecContext(const MSCodecID codecID,
                               const MSAudioParameters &audioParameters,
                               const APAsynDataProvider &asynDataProvider)
:codecID(codecID),
asynDataProvider(asynDataProvider),
videoFmtDescription(nullptr),
audioDecoderConvert(initAudioConvert(audioParameters)) {
    
}

APCodecContext::APCodecContext(const MSCodecID codecID,
                               const bool isColorFullRange,
                               const MSNaluParts &naluParts,
                               const APAsynDataProvider &asynDataProvider)
:codecID(codecID),
asynDataProvider(asynDataProvider),
videoFmtDescription(initVideoFmtDescription(naluParts)),
videoDecoderSession(initVideoDecoderSession(isColorFullRange)) {
    
}

APCodecContext::~APCodecContext() {
    if (codecID < MSCodecID_AAC && videoDecoderSession) {
        VTDecompressionSessionWaitForAsynchronousFrames(videoDecoderSession);
        VTDecompressionSessionInvalidate(videoDecoderSession);
    }
    if (codecID > MSCodecID_HEVC && audioDecoderConvert) {
        AudioConverterDispose(audioDecoderConvert);
    }
    if (videoFmtDescription) {
        CFRelease(videoFmtDescription);
    }
}

CMVideoFormatDescriptionRef
APCodecContext::initVideoFmtDescription(const MSNaluParts &naluParts) {
    
    CMVideoFormatDescriptionRef videoFmtDescription = nullptr;

    OSStatus status = 0;
    
    if (codecID == MSCodecID_H264) {
        // MSNaluParts, 此处直接使用 sps, pps 内存引用, 不需要使用 NALUnitHeaderLength 替换开始码.
        const uint8_t *datas[] = {naluParts.spsRef(),  naluParts.ppsRef()};
        const size_t lengths[] = {naluParts.spsSize(), naluParts.ppsSize()};
        
        status = CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
                                                                     sizeof(datas)/sizeof(uint8_t *),
                                                                     datas, lengths, 4,
                                                                     &videoFmtDescription);
    } else if (codecID == MSCodecID_HEVC) {
        if (__builtin_available(iOS 11.0, *)) {
            
            const uint8_t *datas[] = {naluParts.vpsRef(),  naluParts.spsRef(),  naluParts.ppsRef()};
            const size_t lengths[] = {naluParts.vpsSize(), naluParts.spsSize(), naluParts.ppsSize()};
            
            status = CMVideoFormatDescriptionCreateFromHEVCParameterSets(kCFAllocatorDefault,
                                                                         sizeof(datas)/sizeof(uint8_t *),
                                                                         datas, lengths, 4, nullptr,
                                                                         &videoFmtDescription);
        } else {
            ErrorLocationLog("current iOS version is not 11.0+, not support the hevc");
        }
    } else {
        ErrorLocationLog("not support the codecID");
    }
    
    if (status) {
        OSStatusErrorLocationLog("fail to instance CMFormatDescriptionRef",status);
    }
    
    return videoFmtDescription;
}

AudioConverterRef
APCodecContext::initAudioConvert(const MSAudioParameters &audioParameters) {

    AudioFormatID audioFormatID = getAPCodecId(codecID);
    
    AudioConverterRef audioConverter = nullptr;
    
    OSStatus status = 0;
    
    AudioStreamBasicDescription sourceFormat = {
        .mSampleRate        = (Float64)audioParameters.frequency.value,
        .mFormatID          = audioFormatID,
        .mFormatFlags       = 0,
        .mBytesPerPacket    = 0,
        .mFramesPerPacket   = 0,
        .mBytesPerFrame     = 0,
        .mChannelsPerFrame  = (UInt32)audioParameters.channels,
        .mBitsPerChannel    = 0,
        .mReserved          = 0
    };
    
    switch (codecID) {
        case MSCodecID_AAC: {
            //kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger
            sourceFormat.mFormatFlags       = (UInt32)audioParameters.profile + 1;
            sourceFormat.mFramesPerPacket   = AacPacketFrameNum; // AAC: 1 packet 1024 frame
        }   break;
        case MSCodecID_ALAW: {
            sourceFormat.mFramesPerPacket   = AlawPacketFrameNum;  // AAC: 1 packet 160 frame
        }   break;
        case MSCodecID_OPUS: {
            
        }   break;
        default: break;
    }
    
    AudioStreamBasicDescription destinationFormat = {
        .mSampleRate        = (Float64)audioParameters.frequency.value,
        .mFormatID          = kAudioFormatLinearPCM,
        .mFormatFlags       = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked,
        .mBytesPerPacket    = 1 * 2 * (UInt32)audioParameters.channels,
        .mFramesPerPacket   = 1, // PCM: 1 packet 1 frame, 否则报错(code: -50)
        .mBytesPerFrame     = 2 * (UInt32)audioParameters.channels,
        .mChannelsPerFrame  = (UInt32)audioParameters.channels,
        .mBitsPerChannel    = 16,
        .mReserved          = 0
    };
        
    /**
     // 获取对应类型的解码器描述, 经测试 iphone 中 AAC 编解码只有一种类型 'appl'
     // kAppleSoftwareAudioCodecManufacturer = 'appl' => 软编解码
     // kAppleHardwareAudioCodecManufacturer = 'aphw' => 硬编解码
     UInt32 decodersSize;
     status = AudioFormatGetPropertyInfo(kAudioFormatProperty_Decoders,
                                         sizeof(audioFormatID),
                                         &audioFormatID,
                                         &decodersSize);
     if (status != noErr) {
         OSStatusErrorLocationLog("call AudioFormatGetPropertyInfo fail!",status);
     }
    
     int decodersCount = decodersSize / sizeof(AudioClassDescription);
     AudioClassDescription decoders[decodersCount];
     status = AudioFormatGetProperty(kAudioFormatProperty_Decoders,
                                     sizeof(audioFormatID),
                                     &audioFormatID,
                                     &decodersSize,
                                     decoders);
     if (status != noErr) {
         OSStatusErrorLocationLog("call AudioFormatGetProperty fail!",status);
     }
     
     for (int i = 0; i < decodersCount; i++) {
         OSType mType = decoders[i].mType;
         OSType mSubType = decoders[i].mSubType;
         OSType mManufacturer = decoders[i].mManufacturer;
     }
     */
    
    status = AudioConverterNew(&sourceFormat, &destinationFormat, &audioConverter);
    if (status) {
        OSStatusErrorLocationLog("instance audio converter fail!",status);
    }

    return audioConverter;
}

VTDecompressionSessionRef
APCodecContext::initVideoDecoderSession(const bool isColorFullRange) {
    
    VTDecompressionSessionRef videoDecoderSession = nullptr;
    
    assert(videoFmtDescription);
    
    OSStatus status = noErr;
    
    VTDecompressionOutputCallbackRecord outputCallback;
    outputCallback.decompressionOutputCallback = (VTDecompressionOutputCallback)asynDataProvider.asynCallBack();
    outputCallback.decompressionOutputRefCon = (void *)&asynDataProvider;
    
    /* 指定输出数据格式为 yuv420p '420f' 或 'y420', 方便 yuv 数据分离, 提供给着色器, isColorFullRange 要区分清楚, 否则解码数据将出现色差 */
    SInt32 pixFmtNum = (isColorFullRange ?
                        kCVPixelFormatType_420YpCbCr8PlanarFullRange :
                        kCVPixelFormatType_420YpCbCr8Planar);
    CFNumberRef pixFmtType = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &pixFmtNum);
    
    const void * keys[] = { (void *)kCVPixelBufferPixelFormatTypeKey };
    const void * values[] = { (void *)pixFmtType };
    
    CFDictionaryRef dstBufferAttr = CFDictionaryCreate(kCFAllocatorDefault,
                                                       keys,
                                                       values,
                                                       sizeof(keys)/sizeof(void *),
                                                       &kCFTypeDictionaryKeyCallBacks,
                                                       &kCFTypeDictionaryValueCallBacks);
    
    status = VTDecompressionSessionCreate(kCFAllocatorDefault,
                                          videoFmtDescription,
                                          nullptr,
                                          dstBufferAttr,
                                          &outputCallback,
                                          &videoDecoderSession);
    CFRelease(dstBufferAttr);
    CFRelease(pixFmtType);
    
    if (status) {
        OSStatusErrorLocationLog("fail to instance VTDecompressionSessionRef",status);
    }
    
    return videoDecoderSession;
}

bool
APCodecContext::canAcceptNewFormatDescription(const MSNaluParts &naluParts) {
    if (videoFmtDescription) {
        CFRelease(videoFmtDescription);
    }
    videoFmtDescription = initVideoFmtDescription(naluParts);
    
    bool ret = VTDecompressionSessionCanAcceptFormatDescription(videoDecoderSession, videoFmtDescription);
    if (!ret) {
        ErrorLocationLog("call VTDecompressionSessionCanAcceptFormatDescription fail");
    }
    return ret;
}

CMVideoFormatDescriptionRef
APCodecContext::getVideoFmtDescription() const {
    return videoFmtDescription;
}

APCodecID
APCodecContext::getAPCodecId(const MSCodecID codecID) {
    APCodecID codec_id;
    switch (codecID) {
        case MSCodecID_None:    codec_id = NULL;                    break;
        case MSCodecID_H264:    codec_id = kCMVideoCodecType_H264;  break;
        case MSCodecID_H265:    codec_id = kCMVideoCodecType_HEVC;  break;
        case MSCodecID_AAC:     codec_id = kAudioFormatMPEG4AAC;    break;
        case MSCodecID_OPUS:    codec_id = kAudioFormatOpus;        break;
        case MSCodecID_ALAW:    codec_id = kAudioFormatALaw;        break;
    }
    return codec_id;
}
