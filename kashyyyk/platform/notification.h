#pragma once

/* On systems that support it, put a message as a notification.
 This is primarily intended for OS X's Notification Center, and may be just
 dummied out on other platforms.

 NO MATTER WHAT, Gnome will NOT be expressly supported.
*/

#ifdef __cplusplus
extern "C" {
#endif

enum NotificationSuccess {eNoteSuccess, eNoteFailure, eNoteNotInitialized};

enum NotificationSuccess Kashyyyk_InitNotifications();
enum NotificationSuccess Kashyyyk_CloseNotifications();

/* Title and/or Message can be NULL, which will result in an empty string.
*/

enum NotificationSuccess Kashyyyk_GiveNotification(const char *title,
                                                  const char *message);

#ifdef __cplusplus
}

#include <string>

namespace Kashyyyk{

    inline NotificationSuccess InitNotifications(void){
        return Kashyyyk_InitNotifications();
    }

    inline NotificationSuccess CloseNotifications(void){
        return Kashyyyk_CloseNotifications();
    }

    inline NotificationSuccess GiveNotification(const char *title,
                                                const char *message){
        return Kashyyyk_GiveNotification(title, message);
    }


    // Thing wrappers to allow std::strings to be passed directly.
    inline NotificationSuccess GiveNotification(const char *title,
                                                const std::string &message){
        return GiveNotification(title, message.c_str());
    }

    inline NotificationSuccess GiveNotification(const std::string &title,
                                                const char *message){
        return GiveNotification(title.c_str(), message);
    }

    inline NotificationSuccess GiveNotification(const std::string &title,
                                                const std::string &message){
        return GiveNotification(title.c_str(), message.c_str());
    }


}

#endif

