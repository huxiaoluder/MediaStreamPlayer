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
        
        const uint8_t * MSNonnull   _spsRef;
        const uint8_t * MSNonnull   _ppsRef;
        const uint8_t * MSNullable  _seiRef;
        const uint8_t * MSNonnull   _idrRef;
        
        size_t _spsSize;
        size_t _ppsSize;
        size_t _seiSize;
        size_t _idrSize;
    
    public:
        MSNaluParts(const uint8_t * MSNonnull const nalUnit, const size_t naluSize);
        
        const uint8_t * MSNullable spsRef() const;
        const uint8_t * MSNullable ppsRef() const;
        const uint8_t * MSNullable seiRef() const;
        const uint8_t * MSNullable idrRef() const;
        
        size_t spsSize() const;
        size_t ppsSize() const;
        size_t seiSize() const;
        size_t idrSize() const;
    };
    
}


#endif /* MSNaluParts_hpp */
