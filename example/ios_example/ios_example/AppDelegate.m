//
//  AppDelegate.m
//  ios_example
//
//  Created by 胡校明 on 2018/12/5.
//  Copyright © 2018 freecoder. All rights reserved.
//

#import "AppDelegate.h"
#import "IotlibTool.h"
#import <AVKit/AVKit.h>


@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    
    
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback
                                     withOptions:AVAudioSessionCategoryOptionAllowBluetooth
                                           error:NULL];
    [[AVAudioSession sharedInstance] setActive:true error:NULL];
    
    iotsdk_debug(false);
    iotsdk_set_log_level(e_log_error);
    [[IotlibTool shareIotlibTool] initIotSdkWithUuid:[UIDevice currentDevice].identifierForVendor.UUIDString];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
//        [[IotlibTool shareIotlibTool] iotSdkSetUserName:@"86-18584843282" pass:@"que123456"];
        [[IotlibTool shareIotlibTool] iotSdkSetUserName:@"86-18801381344" pass:@"aaaa0000"];
    });
    
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end
