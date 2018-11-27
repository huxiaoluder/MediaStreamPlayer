//
//  MSDecoder.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#ifndef MSDecoder_hpp
#define MSDecoder_hpp

#include <string>

#include "MSType.hpp"
#include "MSCodecProtocol.h"

namespace MS {
    
    class MSDecoder : public MSDecoderProtocol {
    public:
        virtual MSData * const decode(const MSData& encodeData);
    };
    
}

#endif /* MSDecoder_hpp */
