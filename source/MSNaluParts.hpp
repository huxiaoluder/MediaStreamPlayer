//
//  MSNaluParts.hpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/1/17.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSNaluParts_hpp
#define MSNaluParts_hpp

#include "MSMacros.h"
#include "MSUtil.hpp"

namespace MS {
    
    enum MSCodecID {
        MSCodecID_None,
        /*---video---*/
        MSCodecID_H264,
        MSCodecID_H265,
        MSCodecID_HEVC = MSCodecID_H265,
        /*---audio---*/
        MSCodecID_AAC,
        MSCodecID_OPUS,// Not supported temporarily
        MSCodecID_ALAW,// G711A
    };
    
    /**
     curren not surport p frame is sliced a,b,c
     */
    class MSNaluParts {
        const uint8_t * MSNullable _vpsRef = nullptr;
        union {
            // i frame use
            const uint8_t * MSNullable _spsRef = nullptr;
            // aac frame use
            const uint8_t * MSNullable _adtsRef;
        };
        const uint8_t * MSNullable _ppsRef = nullptr;
        const uint8_t * MSNullable _seiRef = nullptr;
        union {
            // (compress data refference)video and aac audio common use
            const uint8_t * MSNonnull _dataRef = nullptr;
            // i frame use
            const uint8_t * MSNonnull _idrRef;
            // p frame use
            const uint8_t * MSNonnull _slcRef;
        };
//        const uint8_t * MSNullable _dpaRef = nullptr;
//        const uint8_t * MSNullable _dpbRef = nullptr;
//        const uint8_t * MSNullable _dpcRef = nullptr;
        
        size_t _vpsSize = 0;
        union {
            size_t _spsSize = 0;
            size_t _adtsSize;
        };
        size_t _ppsSize = 0;
        size_t _seiSize = 0;
        union {
            size_t _dataSize = 0;
            size_t _idrSize;
            size_t _slcSize;
        };
//        size_t _dpaSize = 0;
//        size_t _dpbSize = 0;
//        size_t _dpcSize = 0;
        
        void initH264NaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize);
        void initH265NaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize);
        void initAACNaluParts(const uint8_t * MSNonnull const nalUnit,  const size_t naluSize);
        
    public:
        MSNaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize, const MSCodecID codecID);
        
        /*---------------- video ---------------- */
        const uint8_t * MSNullable vpsRef() const;
        const uint8_t * MSNullable spsRef() const;
        const uint8_t * MSNullable ppsRef() const;
        const uint8_t * MSNullable seiRef() const;
        const uint8_t * MSNullable idrRef() const;
        // slice refference
        const uint8_t * MSNullable slcRef() const;
//        const uint8_t * MSNullable dpaRef() const;
//        const uint8_t * MSNullable dpbRef() const;
//        const uint8_t * MSNullable dpcRef() const;
        
        size_t vpsSize() const;
        size_t spsSize() const;
        size_t ppsSize() const;
        size_t seiSize() const;
        size_t idrSize() const;
        size_t slcSize() const;
//        size_t dpaSize() const;
//        size_t dpbSize() const;
//        size_t dpcSize() const;
        
        /*---------------- audio ---------------- */
        const uint8_t * MSNullable adtsRef() const;
        
        size_t adtsSize() const;
        
        /*---------------- common ---------------- */
        const uint8_t * MSNonnull dataRef() const;
        
        size_t dataSize() const;
        
        const MSVideoParameters * MSNonnull parseH264Sps() const;
        
        const MSVideoParameters * MSNonnull parseH265Sps() const;
        
        const MSAudioParameters * MSNonnull parseAacAdts() const;
    };
    
}


#endif /* MSNaluParts_hpp */
