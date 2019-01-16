//
//  MSMedia.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/30.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSMedia_hpp
#define MSMedia_hpp

#include <chrono>
#include <cstring>
#include <functional>
#include <type_traits>
#include "MSUtils.h"

namespace MS {
    
    using namespace std;
    
    using namespace chrono;
    
    enum MSCodecID {
        /*---video---*/
        MSCodecID_H264,
        MSCodecID_H265,
        MSCodecID_HEVC = MSCodecID_H265,
        /*---audio---*/
        MSCodecID_AAC,
        MSCodecID_OPUS,
        MSCodecID_ALAW,//G711A
    };
    
    enum MSMediaType {
        isDecode,
        isEncode,
    };
    
    template <MSMediaType DT, typename CT = uint8_t>
    struct MSMedia {
    };
    
#pragma mark - typeTraits: MSMedia<isEncode,uint8_t>
    template <>
    struct MSMedia<isEncode, uint8_t> {
        // content data by encoder or from network source, free by player
        uint8_t * MSNonnull const content;
        
        // content data size
        const size_t size;
        
        // frame type
        const bool isKeyFrame;
        
        // packt data's encode ID
        const MSCodecID codecID;
        
        MSMedia(uint8_t * MSNonnull const packt,
                const size_t size,
                const bool isKeyFrame,
                const MSCodecID codecID)
        :content(new uint8_t[size]),
        size(size),
        isKeyFrame(isKeyFrame),
        codecID(codecID) {
            memcpy(this->content, packt, size);
        }
        
        MSMedia(const MSMedia &media)
        :content(new uint8_t[media.size]),
        size(media.size),
        isKeyFrame(media.isKeyFrame),
        codecID(media.codecID) {
            memcpy(content, media.content, size);
        }
        
        ~MSMedia() {
            delete[] content;
        }
        
        MSMedia * MSNonnull clone() {
            return new MSMedia(*this);
        }
    };
    
#pragma mark - typeTraits: MSMedia<isDecode, CT>
    template <typename CT>
    struct MSMedia<isDecode, CT> {
        typedef MSMedia<isDecode,CT> MSDecoderOutputMedia;
        typedef MSMedia<isDecode,CT> MSEncoderInputMedia;
        
        // frame data by user's decoder decoded, free by player
        CT * MSNonnull const frame;
        
        // associated with frame rate, to refresh palyer timer's interval dynamically
        const microseconds timeInterval;
        
        // source data referrence
        const MSMedia<isEncode> * MSNullable const packt;
        
    private:
        // free function, T frame set by user's custom decoder, must provide the decoder's free method
        const function<void(CT * MSNonnull const)> free;
        
        // copy function, T frame set by user's custom decoder, must provide the decoder's copy method
        const function<CT *(CT * MSNonnull const)> copy;
        
    public:
        MSMedia(typename enable_if<!is_pointer<CT>::value,CT>::type * MSNonnull const frame,
                const microseconds timeInterval,
                const MSMedia<isEncode> * MSNullable const packt,
                const function<void(CT * MSNonnull const)> free,
                const function<CT *(CT * MSNonnull const)> copy)
        :frame(frame),
        timeInterval(timeInterval),
        packt(packt),
        free(free),
        copy(copy) {
            if (frame) {
                assert(free != nullptr && copy != nullptr);
            }
        }
        
        MSMedia(const MSMedia &content)
        :frame(content.copy(content.frame)),
        timeInterval(content.timeInterval),
        free(content.free),
        copy(content.copy) {
            
        }
        
        ~MSMedia() {
            free(frame);
            if (packt) {
                delete packt;
            }
        }
        
        MSMedia * MSNonnull clone() {
            return new MSMedia(*this);
        }
    };
    
}

#endif /* MSMedia_hpp */


