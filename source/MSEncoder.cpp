//
//  MSEncoder.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2018/11/14.
//  Copyright © 2018 freecoder. All rights reserved.
//

#include "MSEncoder.hpp"

using namespace MS;

MSData * const
MSEncoder::encode(const MSData& decodeData) {
    MSData *data = new MSData(decodeData);
    return data;
}
