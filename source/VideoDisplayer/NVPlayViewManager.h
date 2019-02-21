//
//  NVPlayViewManager.h
//  SmartHome
//
//  Created by 贾锟 on 2018/6/11.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "NVGLPlayView.h"

@interface NVPlayViewManager : NSObject

@property (nonatomic , weak) NVGLPlayView *playView;
@property (nonatomic , weak) NVGLPlayView *playView1;
@property (nonatomic , weak) NVGLPlayView *playView2;
@property (nonatomic , weak) NVGLPlayView *playView3;

+ (instancetype)share;
@end
