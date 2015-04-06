#include "cocoa_launcher.h"
#include "launcher.h"

#import <AppKit/NSApplication.h>
#import <AppKit/NSMenu.h>
#import <stdio.h>

#define CREATE_MENU_ITEM(NAME, PHRASE, ADD_TO, TARGET)\
do{\
    NSMenuItem *new_item = [NSMenuItem alloc];\
    new_item = [new_item initWithTitle:@PHRASE action:@selector(NAME) keyEquivalent:@""];\
    [new_item setTarget:TARGET];\
    [ADD_TO addItem:new_item];\
    [new_item release];\
}while(0)

#define DECL_LAUNCHER_WRAPPER(NAME)\
- (void)NAME

#define DECLARE_LAUNCHER_WRAPPER(NAME)\
DECL_LAUNCHER_WRAPPER(NAME);

#define DEFINE_LAUNCHER_WRAPPER(NAME)\
DECL_LAUNCHER_WRAPPER(NAME) {\
    Kashyyyk_Launcher##NAME(launcher);\
}

@interface OSXMenuWrapper : NSObject

DECLARE_LAUNCHER_WRAPPER(NewWindow)
DECLARE_LAUNCHER_WRAPPER(DirectConnect)
DECLARE_LAUNCHER_WRAPPER(JoinChannel)
DECLARE_LAUNCHER_WRAPPER(ServerList)
DECLARE_LAUNCHER_WRAPPER(Preferences)

@property(nonatomic) Launcher *launcher;
@end

@implementation OSXMenuWrapper : NSObject

@synthesize launcher;

DEFINE_LAUNCHER_WRAPPER(NewWindow)
DEFINE_LAUNCHER_WRAPPER(DirectConnect)
DEFINE_LAUNCHER_WRAPPER(JoinChannel)
DEFINE_LAUNCHER_WRAPPER(ServerList)
DEFINE_LAUNCHER_WRAPPER(Preferences)

@end


void Kashyyyk_CreateOSXMenu(void *a){
    OSXMenuWrapper *wrapper = [OSXMenuWrapper alloc];
    NSMenu *main_menu = [NSApp mainMenu];

    NSMenu *file_menu = [[NSMenu alloc] initWithTitle:@"File"];
    NSMenuItem *file_menu_item = [[NSMenuItem alloc] initWithTitle:@"File" action:NULL keyEquivalent:@""];
    NSMenu *server_menu = [[NSMenu alloc] initWithTitle:@"Server"];
    NSMenuItem *server_menu_item = [[NSMenuItem alloc] initWithTitle:@"Server" action:NULL keyEquivalent:@""];

    [wrapper setLauncher:a];
/*

    void Kashyyyk_LauncherNewWindow(void *);
    void Kashyyyk_LauncherDirectConnect(void *);
    void Kashyyyk_LauncherJoinChannel(void *);
    void Kashyyyk_LauncherServerList(void *);
    void Kashyyyk_LauncherPreferences(void *);
    void Kashyyyk_LauncherQuit(void *);
*/

    CREATE_MENU_ITEM(NewWindow,     "New Chat Window", file_menu, wrapper);
    CREATE_MENU_ITEM(DirectConnect, "Connect To...", file_menu, wrapper);
    CREATE_MENU_ITEM(JoinChannel,   "Join Channel...", server_menu, wrapper);
    CREATE_MENU_ITEM(ServerList,    "Server List", file_menu, wrapper);

    [main_menu setSubmenu:file_menu forItem:file_menu_item];
    [main_menu setSubmenu:server_menu forItem:server_menu_item];

    for(int i = 0; i<[main_menu numberOfItems]; i++){
        printf("%s\n", [[[main_menu itemAtIndex:i] title] UTF8String]);
    }

    [NSMenu setMenuBarVisible:YES];

    [file_menu release];
    [file_menu_item release];
    [server_menu release];
    [server_menu_item release];
    [wrapper release];
}
