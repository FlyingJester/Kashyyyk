#include <AppKit/NSApplication.h>

#include "cocoa_guard.h"

void Kashyyyk_Pling(void * a){
    NSApplication *NSApp = [NSApplication sharedApplication];
    [NSApp requestUserAttention:NSInformationalRequest];
}
