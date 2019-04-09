//
//  ViewController.m
//  MediaStreamPlayer
//
//  Created by xiaoming on 2018/12/5.
//  Copyright © 2018 freecoder. All rights reserved.
//

#import "ViewController.h"
#import "IotlibTool.h"
#include <math.h>
#include <AVFoundation/AVFoundation.h>
#include <MSUtil.hpp>

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UIButton *push;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
//    AVAssetWriterInput *input = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo outputSettings:nullptr];
//    [input appendSampleBuffer:nullptr];
    
    uint8_t sps[] = {0x67, 0x42, 0x00, 0x29, 0xab, 0x40, 0x64, 0x09, 0xbf, 0x2c, 0xdc, 0x08, 0x08, 0x0a, 0x90 ,0x20};
    MS::MSVideoParameters p;
    MS::decode_h264_sps(sps, sizeof(sps), p);
    printf("/");
}

- (IBAction)yellow:(UIButton *)sender {

}

- (IBAction)blue:(UIButton *)sender {

}

@end
