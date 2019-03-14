//
//  APVideoRender.m
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#import "APVideoRender.h"

@interface APVideoRender ()
{
    MSGLHanlder *glHandler;
}

@property (nonatomic, strong) NSLock      *lock;
@property (nonatomic, strong) GLKView     *view;
@property (nonatomic, strong) EAGLContext *context;

@end

@implementation APVideoRender

+ (instancetype)renderTo:(UIView * MSNonnull)targetView
                drawRect:(CGRect)rect
                syncLock:(NSLock * MSNullable)lock {
    APVideoRender *render = [APVideoRender new];
    render.lock = lock;
    render.view.frame = rect;
    [targetView addSubview:render.view];
    return render;
}

- (CGRect)drawRect {
    return self.view.frame;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        // 指定 OpenGL 版本, 并绑定到当前上下文(指定3.0, 以支持 3.0 着色器语法)
        self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context.multiThreaded = true;
        [EAGLContext setCurrentContext:self.context];
        
        self.view = [[GLKView alloc] initWithFrame:CGRectZero context:self.context];
        self.view.userInteractionEnabled = false;
        self.view.delegate = self;
        self.view.context = self.context;
        self.view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
        self.view.drawableMultisample = GLKViewDrawableMultisample4X; // 4倍反走样(抗锯齿)
        
        // 设置 viewport 清屏色
        glClearColor(0.7f, 0.7f, 0.7f, 0.0f);
        
        glHandler = new MSGLHanlder([NSBundle.mainBundle pathForResource:@"yuv" ofType:@"vsh"].UTF8String,
                                    [NSBundle.mainBundle pathForResource:@"yuv" ofType:@"fsh"].UTF8String);
    }
    return self;
}

- (void)updateDrawRect:(CGRect)rect {
    self.view.frame = rect;
}

- (UIImage *)snapshot {
    return self.view.snapshot;
}

- (void)displayAVFrame:(AVFrame &)frame {
    
    GLsizei uvWidth  = frame.width / 2;
    GLsizei uvHeight = frame.height / 2;
    
    // 异步更新纹理数据, 防止多个 EAGLContext 数据混乱, 必须要加锁
    [self.lock lock];
    [EAGLContext setCurrentContext:self.context];
    // 提交纹理数据
    MSOpenGLES::commitTexture2DPixels(glHandler->getYtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      frame.width,
                                      frame.height,
                                      GL_UNSIGNED_BYTE,
                                      frame.data[0]);
    MSOpenGLES::commitTexture2DPixels(glHandler->getUtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      uvWidth,
                                      uvHeight,
                                      GL_UNSIGNED_BYTE,
                                      frame.data[1]);
    MSOpenGLES::commitTexture2DPixels(glHandler->getVtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      uvWidth,
                                      uvHeight,
                                      GL_UNSIGNED_BYTE,
                                      frame.data[2]);
    /*
     @Note (查找了一整天的 BUG), 所有纹理数据上传后才能激活, 否则之前的纹理会失效,
            同问题: 激活纹理完成后, 不能解绑 glBindTexture(GL_TEXTURE_2D, 0), 否则无法出图
     */
    // 激活纹理单元, 并分配采样器位置
    MSOpenGLES::activeTexture2DToProgram(glHandler->getYtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE0,
                                         "ySampler2D");
    MSOpenGLES::activeTexture2DToProgram(glHandler->getUtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE1,
                                         "uSampler2D");
    MSOpenGLES::activeTexture2DToProgram(glHandler->getVtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE2,
                                         "vSampler2D");
    [self.lock unlock];
    
    // 主线程刷新 UI
    dispatch_async(dispatch_get_main_queue(), ^{
        // @Note: GLKView 当 APP 进入后台后, 会自动停止渲染
        [self.view display];
    });
}

- (void)displayAPFrame:(APFrame &)frame {
    
    GLsizei yWidth = (GLsizei)CVPixelBufferGetWidth(frame.video);
    GLsizei yHeight = (GLsizei)CVPixelBufferGetHeight(frame.video);
    GLsizei uvWidth  = yWidth / 2;
    GLsizei uvHeight = yHeight / 2;

    CVPixelBufferLockBaseAddress(frame.video, kCVPixelBufferLock_ReadOnly);
    GLvoid *yData = CVPixelBufferGetBaseAddressOfPlane(frame.video, 0);
    GLvoid *uData = CVPixelBufferGetBaseAddressOfPlane(frame.video, 1);
    GLvoid *vData = CVPixelBufferGetBaseAddressOfPlane(frame.video, 2);
    
    [self.lock lock];
    [EAGLContext setCurrentContext:self.context];
    
    MSOpenGLES::commitTexture2DPixels(glHandler->getYtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      yWidth,
                                      yHeight,
                                      GL_UNSIGNED_BYTE,
                                      yData);
    MSOpenGLES::commitTexture2DPixels(glHandler->getUtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      uvWidth,
                                      uvHeight,
                                      GL_UNSIGNED_BYTE,
                                      uData);
    MSOpenGLES::commitTexture2DPixels(glHandler->getVtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      uvWidth,
                                      uvHeight,
                                      GL_UNSIGNED_BYTE,
                                      vData);
    
    MSOpenGLES::activeTexture2DToProgram(glHandler->getYtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE0,
                                         "ySampler2D");
    MSOpenGLES::activeTexture2DToProgram(glHandler->getUtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE1,
                                         "uSampler2D");
    MSOpenGLES::activeTexture2DToProgram(glHandler->getVtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE2,
                                         "vSampler2D");
    [self.lock unlock];
    
    CVPixelBufferUnlockBaseAddress(frame.video, kCVPixelBufferLock_ReadOnly);
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.view display];
    });
}

- (void)dealloc {
    if (glHandler) {
        delete glHandler;
    }
    [EAGLContext setCurrentContext:nil];
    [self.view removeFromSuperview];
}

#pragma mark - GLKViewDelegate
/*
 GLKView --call--> display 时, 会调用该函数然后刷新 OpenGL 缓冲区来刷新屏幕, 本身支持多线程刷新 UI.
 @Note: 系统的 GLKViewController 作为 GLKView 的 delegate, 该代理函数为 VC 内部调用(频率为屏幕的刷新频率),
        可以不重写该函数, 刷新缓冲区(call display)在外部手动进行, 但必须在主线程调用, 否则不生效,
        因为 GLKViewController 内部实现限制了子线程访问 EAGLContext 和刷新 UI.
 @Note: 自定义对象作为 GLKView 的 delegate, 更加灵活, 可以完全控制 OpenGL 刷新频率, 可以子线程构造缓冲区数据,
        为了同步手势缩放视图, 实时更新 viewport 大小, 原则上保持主线程刷新 UI.
 @Note: 子类继承时, 需忽略父类实现, 防止 glDraw* 调用冲突
 */
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    // draw 之前, 必须 clear, 否则屏幕会闪烁
    glViewport(0, 0, (GLsizei)view.drawableWidth, (GLsizei)view.drawableHeight);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

@end
