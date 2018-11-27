//
//  MSDecoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSDecoder.hpp"

using namespace MS;

MSData * const
MSDecoder::decode(const MSData& encodeData) {
    MSData *data = new MSData(encodeData);
    return data;
}
