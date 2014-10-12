#include "notification.h"

#include "cocoa_guard.h"

#import <Foundation/NSString.h>
#import <Foundation/NSUserNotification.h>

#include <string.h>

#define STR_EMPTY_IF_NULL(S) ((S==NULL)?"":S)

static BOOL inited = NO;

@interface Kashyyyk_Notification : NSUserNotification

- (void)setup:(const char *)atitle message:(const char *)msg;

@end

@implementation Kashyyyk_Notification : NSUserNotification

- (void)setup:(const char *)atitle message:(const char *)msg{

    const char *title_c = STR_EMPTY_IF_NULL(atitle);
    const char *msg_c   = STR_EMPTY_IF_NULL(msg);

    [super setTitle:[[NSString alloc] initWithBytes:title_c length:strlen(title_c) encoding:NSUTF8StringEncoding]];
    [super setSubtitle:[[NSString alloc] initWithBytes:msg_c length:strlen(msg_c) encoding:NSUTF8StringEncoding]];
}

@end

enum NotificationSuccess Kashyyyk_InitNotifications(){
    inited = YES;
    return eNoteSuccess;
}

enum NotificationSuccess Kashyyyk_CloseNotifications(){
    if(inited==NO)
      return eNoteNotInitialized;

    inited = NO;
    return eNoteSuccess;
}

enum NotificationSuccess Kashyyyk_GiveNotification(const char *title, const char *msg){

    Kashyyyk_Notification *note;

    const char *title_c = STR_EMPTY_IF_NULL(title);
    const char *msg_c   = STR_EMPTY_IF_NULL(msg);

    if(inited==NO)
      return eNoteNotInitialized;

    note = [[Kashyyyk_Notification alloc] init];
    [note setTitle:[[NSString alloc] initWithBytes:title_c length:strlen(title_c) encoding:NSUTF8StringEncoding]];
    [note setSubtitle:[[NSString alloc] initWithBytes:msg_c length:strlen(msg_c) encoding:NSUTF8StringEncoding]];
    [note setResponsePlaceholder: @"Reply"];

    [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:note];

    return eNoteSuccess;
}

