//
//  MSBinaryData.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/12/28.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSBinaryData_hpp
#define MSBinaryData_hpp

#include <cstring>
#include <cstdint>

namespace MS {
    
    struct MSBinaryData {
        uint8_t * const bytes;
        
        const size_t size;
        
        MSBinaryData(const uint8_t * const bytes, const size_t size);
        
        MSBinaryData(const MSBinaryData &data);
        
        ~MSBinaryData();
    };

}

#endif /* MSBinaryData_hpp */
