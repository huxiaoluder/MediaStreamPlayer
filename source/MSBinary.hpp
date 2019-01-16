//
//  MSBinary.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSBinary_hpp
#define MSBinary_hpp

#include <cstring>
#include <cstdint>
#include "MSUtils.h"

namespace MS {
    
    struct MSBinary {
        uint8_t * MSNonnull const bytes;
        
        const size_t size;
        
        MSBinary(const uint8_t * MSNonnull const bytes, const size_t size);
        
        MSBinary(const MSBinary &data);
        
        ~MSBinary();
    };

}

#endif /* MSBinary_hpp */
