//
//  ViewController.m
//  ios_example
//
//  Created by 胡校明 on 2018/12/5.
//  Copyright © 2018 freecoder. All rights reserved.
//

#import "ViewController.h"
#import "IotlibTool.h"
#include <math.h>
#include <APVideoRender.h>

@interface ViewController ()
{
     MSGLWrapper *glWrapper;
}

@property (weak, nonatomic) IBOutlet UIButton *push;
@property (nonatomic, strong) APVideoRender *videoRender;
@property (nonatomic, strong) EAGLContext *context;
@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    CGRect rect = CGRectMake(0, 50,
                             UIScreen.mainScreen.bounds.size.width,
                             UIScreen.mainScreen.bounds.size.width * 9 / 16);
    NSLock *lock = [NSLock new];
    self.videoRender = [APVideoRender renderTo:self.view drawRect:rect syncLock:lock];
}

- (IBAction)yellow:(UIButton *)sender {
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        AVFrame frame;
        [self.videoRender displayAVFrame:frame];
    });
}

- (IBAction)blue:(UIButton *)sender {
    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        APFrame frame((__CVBuffer *)nullptr);
        [self.videoRender displayAPFrame:frame];
    });
}

@end
