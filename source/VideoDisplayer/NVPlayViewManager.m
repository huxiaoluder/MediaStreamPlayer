//
//  NVPlayViewManager.m
//  SmartHome
//
//  Created by 贾锟 on 2018/6/11.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import "NVPlayViewManager.h"

static NVPlayViewManager * _singleton;
@implementation NVPlayViewManager

+ (instancetype)allocWithZone:(struct _NSZone *)zone{
    
    static dispatch_once_t onceToken;
    // 一次函数
    dispatch_once(&onceToken, ^{
        if (_singleton == nil) {
            _singleton = [super allocWithZone:zone];
        }
    });
    return _singleton;
}
+ (instancetype)share{
    
    return  [[self alloc] init];
}
@end
