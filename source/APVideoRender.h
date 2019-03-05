//
//  APVideoRender.h
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "MSGLWrapper.hpp"
#import "FFCodecContext.hpp"
#import "APCodecContext.hpp"

using namespace MS;
using namespace MS::APhard;

NS_ASSUME_NONNULL_BEGIN

@interface APVideoRender : GLKViewController

- (void)displayAVFrame:(AVFrame &)frame;

- (void)displayAPFrame:(APFrame &)frame;

@end

NS_ASSUME_NONNULL_END
