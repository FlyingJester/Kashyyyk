#include "window.hpp"
#include "background.hpp"
#include "prefs.hpp"
#include "platform/notification.h"

#include <stack>
#include <string>
#include <cstdlib>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>

namespace Kashyyyk {
// SINGLETON ONLY!
class NotificationRAII {
public:
    NotificationRAII(){
        InitNotifications();
    }

    ~NotificationRAII(){
        CloseNotifications();
    }

};

}

void GetDefaultServers(std::stack<std::string> &default_servers, Fl_Preferences &prefs){

    char *default_servers_str = nullptr;

    prefs.get("sys.servers.default.addresses", default_servers_str, "");
    char *server_iter1 = default_servers_str;
    char *server_iter2 = strchr(default_servers_str, ',');

    while(server_iter2!=nullptr){

        default_servers.push(std::string(server_iter1, server_iter2));

         // Move past the comma.
        server_iter2++;

        if(*server_iter2=='\0')
          break;

        server_iter1 = server_iter2;
        server_iter2 = strchr(server_iter1, ',');

    }

    free(default_servers_str);

}

void SetTheme(Fl_Preferences &prefs){

    char *theme = nullptr;
    prefs.get("sys.appearance.theme", theme, "gtk+");

    Fl::scheme(theme);
    free(theme);
}

int main(int argc, char *argv[]){

    Kashyyyk::NotificationRAII note_raii;

    Fl_Preferences &prefs = Kashyyyk::GetPreferences();

    std::stack<std::string> default_servers;

    GetDefaultServers(default_servers, prefs);
    SetTheme(prefs);

    Kashyyyk::Window window(1024, 600);

    Fl::lock();

    Kashyyyk::Thread thread1(Kashyyyk::Thread::GetShortThreadPool());
    Kashyyyk::Thread thread2(Kashyyyk::Thread::GetLongThreadPool());

    Fl::run();
}
