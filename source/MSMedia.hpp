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
#include "MSNaluParts.hpp"
#include "MSBinary.hpp"

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
        // nalUnit data by encoder or from network source, free by player
        uint8_t * MSNonnull const naluData;
        
        // nalUnit data size
        const size_t naluSize;
        
        // nalUnit picture type
        const bool isKeyFrame;
        
        // pictrue's encoder ID
        const MSCodecID codecID;
        
        MSMedia(uint8_t * MSNonnull const naluData,
                const size_t naluSize,
                const bool isKeyFrame,
                const MSCodecID codecID)
        :naluData(new uint8_t[naluSize]),
        naluSize(naluSize),
        isKeyFrame(isKeyFrame),
        codecID(codecID) {
            memcpy(this->naluData, naluData, naluSize);
        }
        
        MSMedia(const MSMedia &media)
        :naluData(new uint8_t[media.naluSize]),
        naluSize(media.naluSize),
        isKeyFrame(media.isKeyFrame),
        codecID(media.codecID) {
            memcpy(naluData, media.naluData, naluSize);
        }
        
        ~MSMedia() {
            if (naluParts) {
                delete naluParts;
            }
            delete [] naluData;
        }
        
        MSMedia * MSNonnull clone() {
            return new MSMedia(*this);
        }
        
        // Note: only I frame can return naluParts and enforce changed naluParts value
        const MSNaluParts & getNaluParts() const {
            if (!naluParts) {
                auto naluPartsRef = const_cast<MSNaluParts **>(&naluParts);
                *naluPartsRef = new MSNaluParts(naluData, naluSize);
            }
            return *naluParts;
        }
        
    private:
        const MSNaluParts * MSNullable naluParts = nullptr;
    };
    
#pragma mark - typeTraits: MSMedia<isDecode, CT>
    template <typename CT>
    struct MSMedia<isDecode, CT> {
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


