#include "paths.h"

#import <Foundation/Foundation.h>
#import <Foundation/NSString.h>

const char * Kashyyyk_HomeDirectory(){
    return [NSHomeDirectory() UTF8String];
}


const char * Kashyyyk_ConfigDirectory(){
    NSString *confdir = [NSHomeDirectory() stringByAppendingPathComponent:@".conf"];
    return [confdir UTF8String];
}

void Kashyyyk_MakeDir(const char *dir_){
    NSFileManager *manager = [NSFileManager defaultManager];
    NSString *dir = [[NSString alloc] initWithUTF8String:dir_];
    [manager createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
    [dir release];
}
