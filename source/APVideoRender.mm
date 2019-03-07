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

@property (nonatomic, strong) EAGLContext *context;

@end

@implementation APVideoRender

- (void)viewDidLoad {
    [super viewDidLoad];
    // 指定 OpenGL 版本, 并绑定到当前上下文(指定3.0, 以支持 3.0 着色器语法)
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    self.context.multiThreaded = true;
    [EAGLContext setCurrentContext:self.context];
    
    string vshPath = [NSBundle.mainBundle pathForResource:@"yuv420p" ofType:@"vsh"].UTF8String;
    string fshPath = [NSBundle.mainBundle pathForResource:@"yuv420p" ofType:@"fsh"].UTF8String;
    glWrapper = new MSGLWrapper(vshPath, fshPath);
    
    GLKView *view = (GLKView *)self.view;
    
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    glClearColor(.7f, .7f, .7f, .0f);
}

- (void)viewDidAppear:(BOOL)animated {
    [super viewDidAppear:animated];
    glViewport(0, 0, (GLsizei)((GLKView *)self.view).drawableWidth, (GLsizei)((GLKView *)self.view).drawableHeight);
    
}

// 该代理函数为系统调用(频率为屏幕的刷新频率), 可以不重写该函数, 刷新缓冲区可以在外部手动进行(注: 必须在主线程, 否则不生效)
- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
    // draw 之前, 必须 clear, 否则屏幕会闪烁
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    NSLog(@"-------- draw -------- thread: %@", [NSThread currentThread]);
}


- (void)displayAVFrame:(AVFrame &)frame {
    
}

- (void)displayAPFrame:(APFrame &)frame {
    
}

- (void)dealloc {
    if (glWrapper) {
        delete glWrapper;
    }
    [EAGLContext setCurrentContext:nil];
}

@end
