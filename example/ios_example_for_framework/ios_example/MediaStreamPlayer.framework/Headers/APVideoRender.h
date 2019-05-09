//
//  APVideoRender.h
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#import <GLKit/GLKit.h>
#import "MSGLHanlder.hpp"
#import "FFCodecContext.hpp"
#import "APCodecContext.hpp"

using namespace MS;
using namespace MS::APhard;

NS_ASSUME_NONNULL_BEGIN

@interface APVideoRender : NSObject<GLKViewDelegate>

/**
 渲染范围
 */
@property (readonly) CGRect drawRect;

/**
 OpenGL es 渲染器构造器

 @param targetView 目标视图
 @param rect 范围
 @param bgTexture 背景图(YUV420P)纹理
 @param lock EAGLContext 访问锁(注: 需要多渲染器共用一把锁), 防止多个 EAGLContext 之间数据错误
 @return APVideoRender
 */
+ (instancetype)renderTo:(UIView * MSNonnull)targetView
                drawRect:(CGRect)rect
               bgTexture:(APYUV420PTexture * MSNullable)bgTexture
                syncLock:(NSLock * MSNonnull)lock;

/**
 移动渲染器到目标视图

 @param targetView 目标视图
 */
- (void)moveTo:(UIView * MSNonnull)targetView;

/**
 刷新渲染范围

 @param rect 范围
 */
- (void)updateDrawRect:(CGRect)rect;

/**
 截取视口当前图片
 
 @return 截图
 */
- (UIImage *)snapshot;

/**
 渲染 FFmpeg 软解视频数据 YUV420P(YYYY...U...V)
 AV_PIX_FMT_YUV420P
 AV_PIX_FMT_YUVJ420P

 @param frame AVFrame
 */
- (void)displayAVFrame:(AVFrame &)frame;

/**
 渲染 VideoToolBox 硬解视频数据 YUV420P(YYYY...U...V)
 kCVPixelFormatType_420YpCbCr8Planar
 kCVPixelFormatType_420YpCbCr8PlanarFullRange
 
 @param frame APFrame
 */
- (void)displayAPFrame:(APFrame &)frame;

/**
 清除渲染画面, 渲染背景纹理
 */
- (void)clearPicture;

@end

NS_ASSUME_NONNULL_END
