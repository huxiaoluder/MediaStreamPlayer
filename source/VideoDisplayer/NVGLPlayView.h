//
//  NVGLPlayView.h
//  SmartHome
//
//  Created by 贾锟 on 2018/6/11.
//  Copyright © 2018年 mysoul. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

@interface NVGLPlayView : UIView
@property (nonatomic , assign) BOOL isFullYUVRange;

- (void)setupGL;
- (void)displayPixelBuffer:(CVPixelBufferRef)pixelBuffer;

@end
