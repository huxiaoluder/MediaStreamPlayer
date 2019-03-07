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

@property (weak, nonatomic) IBOutlet UIButton *push;
@property (nonatomic, strong) APVideoRender *videoRender;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    APVideoRender *render = [[APVideoRender alloc] init];
    render.view.frame = CGRectMake(0, 50,
                                   UIScreen.mainScreen.bounds.size.width,
                                   UIScreen.mainScreen.bounds.size.width * 9 / 16);
    [self.view addSubview:render.view];
    self.videoRender = render;
    
}


- (IBAction)yellow:(UIButton *)sender {
    glClearColor(1.0f, 1.0f, .0f, .0f);
}

- (IBAction)blue:(UIButton *)sender {
    glClearColor(.0f, 1.0f, 1.0f, .0f);
}

@end
