#include "pling.h"
#include <AppKit/NSApplication.h>

void Kashyyyk_Pling(void){
    NSApplication *NSApp = [NSApplication sharedApplication];
    [NSApp requestUserAttention:NSCriticalRequest];
}
