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
#import "MSViewController.h"

@interface ViewController ()

@property (weak, nonatomic) IBOutlet UIButton *push;

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}
- (IBAction)push:(UIButton *)sender {
    
}

// 私模(h264, aac): IOTSHMK000S0008EDA1FCDD
// 摇头机(h264, alaw): IOTSHMKP00300004F0716
- (IBAction)yellow:(UIButton *)sender {

}


- (IBAction)blue:(UIButton *)sender {

}


- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // 私模(h264, aac): IOTSHMK000S0008EDA1FCDD
    // 摇头机(h264, alaw): IOTSHMKP00300004F0716
    if ([segue.identifier isEqualToString:@"simo"]) {
        ((MSViewController *)segue.destinationViewController).deviceId = @"IOTSHMK000S0008EDA1FCDD";
    } else if ([segue.identifier isEqualToString:@"yaotou"]) {
        ((MSViewController *)segue.destinationViewController).deviceId = @"IOTSHMKP00300004F0716";
    } else {
        
    }
}

@end
