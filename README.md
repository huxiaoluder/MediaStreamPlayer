# MSPlayer

功能: 
1. 支持音视频网络数据数据解码
2. 支持重编码,录制MP4
3. MS 系列核心组件
4. FF 系列组件跨平台依赖 FFMpeg
5. AP 系列组件Apple平台依赖 CoreMedia,VideoToolBox,AudioToolBox,GLKit,FFMpeg(仅封装 mp4 依赖)
```c
// 使用说明:
using namespace std;
using namespace MS;
using namespace MS::APhard;

    /* 软硬编解方式, 根据机器性能自行选择, 这里以 iOS 硬编解为例进行说明 */
    
    // 视频渲染器
    auto videoRender = [APVideoRender renderTo:/*targetView*/
                                      drawRect:/*drawRect*/
                                      syncLock:/*commonLock 多个渲染器需要共用一把公共锁*/];
    
    // 音频渲染器
    auto audioRender = [APAudioRender renderWithChannels:1 frequency:8000];

    // 音视频解码器
    auto decoder = new APDecoder;
    
    // 重编码封装器, 封装 mp4(aac, h264)
    auto encoder = new APEncoder;
    
    // 视频解码数据回调
    auto videoPlayFunc = [videoRender/*,animationView*/](const APDecoderOutputMeida &media) {
        auto frame = media.frame;
        if (frame) {
            // 渲染视频
            [videoRender displayAPFrame:*frame];
        } else {
            // 可以显示加载动画
        }
    };
    
    // 音频解码数据回调
    auto audioPlayFunc = [audioRender](const APDecoderOutputMeida &media) {
        auto frame = media.frame;
        if (frame) {
            // 渲染音频
            [audioRender updateChannels:frame->audioParameters.channels
                              frequency:frame->audioParameters.frequency.value];
            [audioRender displayAPFrame:*frame];
        }
    };
    
    // 初始化播放器
    auto player = new MSPlayer<APFrame>(decoder, encoder, videoPlayFunc, audioPlayFunc);

    // 开始播放
    player->startPlayVideo();
    player->startPlayAudio();
    
    // 填充音视频数据
    while (true) {
        auto videoData = new MSMedia<MSEncodeMedia>(/*dataPtr*/,/*dataLen*/,/*isKeyFrame*/,/*MSCodecID*/);
        auto audioData = new MSMedia<MSEncodeMedia>(/*dataPtr*/,/*dataLen*/,/*ignore*/,/*MSCodecID*/);
        player->pushVideoStreamData(videoData);
        player->pushVideoStreamData(audioData);
    }


    // 使用解码器相关参数,初始化编码器, 但不影响编码方式(主要为了解决 h265 编码的 mp4, APPLE端目前不支持 h265 MP4, iOS无法存入相册, 只能重编码为 h264 视频)
    bool ret = encoder.configureEncoder(/*mp4OutPath*/,
                                        decoder.getCurrentVideoParameters(),
                                        decoder.getCurrentAudioParameters());
    if (ret) {
        player->startReEncode();
    }
```
