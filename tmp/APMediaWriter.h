//
// APMediaWriter.h
// MediaStreamPlayer
// 
// Created by xiaoming on 2019/3/21.
// Copyright © 2019 freecoder. All rights reserved.
//
// Email: huxiaoluder@163.com
//

#import <Foundation/Foundation.h>

#include "APCodecContext.hpp"

NS_ASSUME_NONNULL_BEGIN

@interface APMediaWriter : NSObject

- (void)writeVideoBuffer:(CVPixelBufferRef)pixleBuffer;

@end

NS_ASSUME_NONNULL_END
