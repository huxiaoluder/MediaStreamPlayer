//
//  APDecoder.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/26.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef APDecoder_hpp
#define APDecoder_hpp

#include <map>
#include "MSDecoderProtocol.h"
#include "APCodecContext.hpp"

namespace MS {
    namespace APhard {
        
        typedef MSAsynDecoderProtocol<APFrame>  APDecoderProtocol;
        typedef MSAsynDataReceiver<APFrame>     APAsynDataReceiver;
        typedef MSMedia<MSDecodeMedia,APFrame>  APDecoderOutputMeida;
        
        // 视频解码回调附加参数(回调透传)
        struct APVideoAttachment {
            const MSMedia<MSEncodeMedia> * MSNullable videoSource = nullptr;
            const MSVideoParameters * MSNullable videoParameters = nullptr;
        };
        
        // 音频解码回调附加参数(回调透传)
        struct APAudioAttachment {
            const MSMedia<MSEncodeMedia> * MSNullable audioSource = nullptr;
            const MSAudioParameters * MSNullable audioParameters = nullptr;
        };
        
        class APDecoder : public APDecoderProtocol {
            static map<APDecoder *, const MSVideoParameters *> videoParametersMap;
            
            static map<APDecoder *, const MSAudioParameters *> audioParametersMap;
            
            map<MSCodecID,APCodecContext *> decoderContexts;
            
            const VTDecodeFrameFlags decodeFlags;
            
            const CMBlockBufferFlags bufferFlags;
            
            const CFAllocatorRef MSNonnull blockAllocator;
            
            APVideoAttachment videoAttachment;
            
            APAudioAttachment audioAttachment;
            
            /**
             be related by decodeFlags,
             if decodeFlags == NULL, decoder must realloc memery for source data, avert double free crash.
             
             because decodeFlags != NULL, the decoder may process frames asynchronously,
             but source data has be freed already, so need copy a new memery.
             */
            CMBlockBufferFlags initBufferFlags();
            
            /**
             be related by bufferFlags,
             if bufferFlags == NULL, decoder do not realloc memery for source data, so needn't the allocator.
             */
            CFAllocatorRef MSNonnull initBlockAllocator();
            
            APCodecContext * MSNullable getVideoDecoderContext(const MSMedia<MSEncodeMedia> &sourceData);
            
            APCodecContext * MSNullable getAudioDecoderContext(const MSMedia<MSEncodeMedia> &sourceData);
            
            
            static void decompressionOutputCallback(void * MSNullable decompressionOutputRefCon,
                                                    void * MSNullable sourceFrameRefCon,
                                                    OSStatus status,
                                                    VTDecodeInfoFlags infoFlags,
                                                    CVImageBufferRef MSNullable imageBuffer,
                                                    CMTime presentationTimeStamp,
                                                    CMTime presentationDuration);
            
            static OSStatus decompressionConverterInputProc(AudioConverterRef MSNonnull inAudioConverter,
                                                            UInt32 * MSNonnull ioNumberDataPackets,
                                                            AudioBufferList * MSNonnull ioData,
                                                            AudioStreamPacketDescription * MSNullable * MSNullable outDataPacketDescription,
                                                            void * MSNullable inUserData);
            
        public:
            void decodeVideo(const MSMedia<MSEncodeMedia> * MSNonnull const videoData);
            void decodeAudio(const MSMedia<MSEncodeMedia> * MSNonnull const audioData);
            
            /**
             if decodeFlags == NULL, the decoder still decode source data synchronously.
             if decodeFlags != NULL, the decoder will decode source data asynchronously.
             */
            APDecoder(const VTDecodeFrameFlags decodeFlags = NULL);
            ~APDecoder();
            
            const MSVideoParameters * MSNullable getCurrentVideoParameters();
            const MSAudioParameters * MSNullable getCurrentAudioParameters();
        };
        
    }
}


#endif /* APDecoder_hpp */
