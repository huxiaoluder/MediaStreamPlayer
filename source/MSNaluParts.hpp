//
//  MSNaluParts.hpp
//  ios_example
//
//  Created by 胡校明 on 2019/1/17.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSNaluParts_hpp
#define MSNaluParts_hpp

#include <cstring>
#include <cstdint>
#include "MSUtils.h"

namespace MS {
    
    class MSNaluParts {
        
        const uint8_t * MSNullable _slcRef = nullptr;
        const uint8_t * MSNullable _dpaRef = nullptr;
        const uint8_t * MSNullable _dpbRef = nullptr;
        const uint8_t * MSNullable _dpcRef = nullptr;
        const uint8_t * MSNullable _idrRef = nullptr;
        const uint8_t * MSNullable _seiRef = nullptr;
        const uint8_t * MSNullable _spsRef = nullptr;
        const uint8_t * MSNullable _ppsRef = nullptr;
        
        size_t _slcSize = 0;
        size_t _dpaSize = 0;
        size_t _dpbSize = 0;
        size_t _dpcSize = 0;
        size_t _idrSize = 0;
        size_t _seiSize = 0;
        size_t _spsSize = 0;
        size_t _ppsSize = 0;
    
    public:
        MSNaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize);
        
        // slice refference
        const uint8_t * MSNullable slcRef() const;
        const uint8_t * MSNullable dpaRef() const;
        const uint8_t * MSNullable dpbRef() const;
        const uint8_t * MSNullable dpcRef() const;
        const uint8_t * MSNullable idrRef() const;
        const uint8_t * MSNullable seiRef() const;
        const uint8_t * MSNullable spsRef() const;
        const uint8_t * MSNullable ppsRef() const;
        
        size_t slcSize() const;
        size_t dpaSize() const;
        size_t dpbSize() const;
        size_t dpcSize() const;
        size_t idrSize() const;
        size_t seiSize() const;
        size_t spsSize() const;
        size_t ppsSize() const;
    };
    
}


#endif /* MSNaluParts_hpp */
