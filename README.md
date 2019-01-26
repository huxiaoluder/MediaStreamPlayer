# MSPlayer

功能: 
1. 支持音视频网络数据数据解码
2. 支持重编码,录制MP4
3. MS 系列核心组件
4. FF 系列组件跨平台依赖 FFMpeg
5. AP 系列组件Apple平台依赖 VideoToolBox,AudioToolBox
```c
// 使用说明:
using namespace std;
using namespace MS;
using namespace MS::FFmpeg;

    auto decoder = new FFDecoder();
    // 编码器自定义编码方式
    auto encoder = new FFEncoder(MSCodecID_H264,MSCodecID_AAC);
    player = new MSPlayer<AVFrame>(decoder,encoder,
                                   [&](const MSMedia<isDecode,AVFrame> &data) {
                                      //display decode video data
                                   },
                                   [&](const MSMedia<isDecode,AVFrame> &data) {
                                      //display decode audio data
                                   });

// 使用解码器相关参数,初始化编码器, 但不影响编码方式(主要为了解决 h265 编码的 mp4, APPLE端目前不支持 h265 MP4, iOS无法存入相册, 只能重编码为 h264 视频)
bool ret = encoder.configureEncoder(/*videoPath*/,
                                    decoder.findDecoderContext(MSCodecID_H264),
                                    decoder.findDecoderContext(MSCodecID_AAC));
if (ret) {
  player->startReEncode();
}
```
