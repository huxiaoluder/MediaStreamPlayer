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
        
        // 设置 viewport 背景色
        glClearColor(.7f, .7f, .7f, .0f);
        
        glHandler = new MSGLHanlder([NSBundle.mainBundle pathForResource:@"yuv420p" ofType:@"vsh"].UTF8String,
                                    [NSBundle.mainBundle pathForResource:@"yuv420p" ofType:@"fsh"].UTF8String);
    }
    return self;
}

- (void)updateDrawRect:(CGRect)rect {
    self.view.frame = rect;
}

- (void)displayAVFrame:(AVFrame &)frame {
    
    int yLen = frame.width * frame.height;

    uint8_t *oldUData = frame.data[1];
    uint8_t *oldVData = frame.data[2];
    
    int yColLen = frame.width;
    int uvColLen = frame.width  / 2;
    int uvRowLen = frame.height / 2;
    
    uint8_t *newYData = frame.data[0];
    uint8_t *newUData = new uint8_t[yLen];
    uint8_t *newVData = new uint8_t[yLen];
    
    for (int row = 0; row < uvRowLen; row++) {
        for (int col = 0; col < uvColLen; col++) {
            int idx  = uvColLen * row + col;
            int newrow = row * 2;
            int newCol = col * 2;
            int idx1_1 = newrow * yColLen  + newCol;
            int idx1_2 = idx1_1 + 1;
            int idx2_1 = idx1_1 + yColLen;
            int idx2_2 = idx2_1 + 1;
            newUData[idx1_1] = newUData[idx1_2] = newUData[idx2_1] = newUData[idx2_2] = oldUData[idx];
            newVData[idx1_1] = newVData[idx1_2] = newVData[idx2_1] = newVData[idx2_2] = oldVData[idx];
        }
    }
    
    // 异步更新纹理数据, 防止多个 EAGLContext 数据混乱, 必须要加锁
    [self.lock lock];
    [EAGLContext setCurrentContext:self.context];
    /*-------------------Y texture-------------------*/
    // 提交纹理数据
    MSOpenGLES::commitTexture2DPixels(glHandler->getYtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      frame.width,
                                      frame.height,
                                      GL_UNSIGNED_BYTE,
                                      newYData);
//                                      frame.data[0]);
    // 激活纹理单元, 并分配采样器位置
    MSOpenGLES::activeTexture2DToProgram(glHandler->getYtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE0,
                                         "ySampler2D");
    
    /*-------------------U texture-------------------*/
    MSOpenGLES::commitTexture2DPixels(glHandler->getUtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      frame.width  ,
                                      frame.height ,
                                      GL_UNSIGNED_BYTE,
                                      newUData);
//                                      frame.data[1]);
    MSOpenGLES::activeTexture2DToProgram(glHandler->getUtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE1,
                                         "uSampler2D");

    /*-------------------V texture-------------------*/
    MSOpenGLES::commitTexture2DPixels(glHandler->getVtexture(),
                                      GL_LUMINANCE,
                                      GL_LUMINANCE,
                                      frame.width  ,
                                      frame.height ,
                                      GL_UNSIGNED_BYTE,
                                      newVData);
//                                      frame.data[2]);
    MSOpenGLES::activeTexture2DToProgram(glHandler->getVtexture(),
                                         glHandler->getProgram(),
                                         GL_TEXTURE2,
                                         "vSampler2D");
    [self.lock unlock];
    
    delete [] newUData;
    delete [] newVData;
    
    // 主线程刷新 UI
    dispatch_async(dispatch_get_main_queue(), ^{
        // @Note: GLKView 当 APP 进入后台后, 会自动停止渲染
        [self.view display];
    });
}

- (void)displayAPFrame:(APFrame &)frame {
    [self.lock lock];
    [EAGLContext setCurrentContext:self.context];
    
    //TODO: 构造缓冲区
    
    [self.lock lock];
    
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
