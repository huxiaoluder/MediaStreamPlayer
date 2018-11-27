//
//  MSType.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/15.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSType_hpp
#define MSType_hpp

#include <functional>
#include <chrono>
#include <cstring>

#define intervale(rate) microseconds(1000000LL/rate)

namespace MS {
    
    typedef unsigned char uint8_t;
    
    typedef unsigned int size_t;
    
    struct MSData {
        uint8_t * const bytes;
        
        const size_t len;
        
        std::chrono::microseconds timeInterval;
        
        MSData(uint8_t * const bytes, size_t const len, std::chrono::microseconds const timeInterval);
        
        MSData(const MSData& data);
        
        ~MSData();
        
        MSData * copy();
    };
    
    typedef std::function<void()> TaskType;
    
    typedef std::function<void(MSData& decodeData)> ThrowData;
    
}

#endif /* MSType_hpp */
