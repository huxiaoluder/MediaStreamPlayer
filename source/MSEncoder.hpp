//
//  MSEncoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSEncoder_hpp
#define MSEncoder_hpp

#include <string>

#include "MSType.hpp"
#include "MSCodecProtocol.h"

namespace MS {
    
    class MSEncoder : public MSEncoderProtocol {
    public:
        virtual MSData * const encode(const MSData& decodeData);
    };
    
}

#endif /* MSEncoder_hpp */
