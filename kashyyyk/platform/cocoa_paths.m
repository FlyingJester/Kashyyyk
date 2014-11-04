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

void Kashyyyk_MakeDir(const char *dir){
    NSFileManager *manager = [NSFileManager defaultManager];

    [manager createDirectoryAtPath:[[NSString alloc] initWithUTF8String:dir] withIntermediateDirectories:YES attributes:nil error:nil];

}
