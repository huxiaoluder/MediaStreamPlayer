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
    MSGLWrapper *glWrapper;
}

@property (nonatomic, weak)     NSLock      *lock;
@property (nonatomic, strong)   GLKView     *view;
@property (nonatomic, strong)   EAGLContext *context;

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
        glClearColor(.7f, .7f, .7f, .0f);
        
        glWrapper = new MSGLWrapper([NSBundle.mainBundle pathForResource:@"yuv420p" ofType:@"vsh"].UTF8String,
                                    [NSBundle.mainBundle pathForResource:@"yuv420p" ofType:@"fsh"].UTF8String);
    }
    return self;
}

- (void)updateDrawRect:(CGRect)rect {
    self.view.frame = rect;
}

- (void)displayAVFrame:(AVFrame &)frame {
    if (UIApplication.sharedApplication.applicationState != UIApplicationStateActive) {
        return;
    }
    [self.lock lock];
    [EAGLContext setCurrentContext:self.context];
    
    //TODO: 构造缓冲区
    
    [self.lock unlock];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.view display];
    });
}

- (void)displayAPFrame:(APFrame &)frame {
    if (UIApplication.sharedApplication.applicationState != UIApplicationStateActive) {
        return;
    }
    [self.lock lock];
    [EAGLContext setCurrentContext:self.context];
    
    //TODO: 构造缓冲区
    
    [self.lock lock];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.view display];
    });
}

- (void)dealloc {
    if (glWrapper) {
        delete glWrapper;
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
    NSLog(@"-------- draw -------- thread: %@", [NSThread currentThread]);
}

@end
