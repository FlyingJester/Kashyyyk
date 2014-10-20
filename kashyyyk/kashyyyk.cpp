#include "window.hpp"
#include "background.hpp"
#include "prefs.hpp"
#include "launcher.hpp"
#include "platform/notification.h"
#include "platform/init.h"

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

    Kashyyyk::Init();

    Kashyyyk::NotificationRAII note_raii;

    Fl_Preferences &prefs = Kashyyyk::GetPreferences();

    std::unique_ptr<Kashyyyk::Window>   window;


    std::unique_ptr<Kashyyyk::Thread::TaskGroup, void(*)(Kashyyyk::Thread::TaskGroup*)>
      group(Kashyyyk::Thread::CreateTaskGroup(), Kashyyyk::Thread::DestroyTaskGroup);

    {

        Kashyyyk::Launcher *launcher;
        int startup_launcher         = 1;
        int startup_launchertype     = 1;
        int startup_window           = 1;
        int startup_autojoin_servers = 1;
        int startup_autojoin_channels= 1;

        prefs.get("sys.startup.launcher", startup_launcher, startup_launcher);
        prefs.get("sys.startup.launcher.type", startup_launchertype, startup_launchertype);
        prefs.get("sys.startup.window", startup_window, startup_window);
        prefs.get("sys.startup.autojoin.servers", startup_autojoin_servers, startup_autojoin_servers);
        prefs.get("sys.startup.autojoin.channels", startup_autojoin_channels, startup_autojoin_channels);

        if(startup_launcher){
            switch(startup_launchertype){
            case 1:
                launcher = new Kashyyyk::BoringLauncher(group.get());
            break;
#ifndef NO_ICONLAUNCHER
            case 2:
                launcher = new Kashyyyk::IconLauncher(group.get());
            break
#endif
            break;
            case 0:
            default:
                launcher = Kashyyyk::Launcher::CreatePlatformLauncher(group.get());
            }

            launcher->NewWindow();
        }
        else{
            window.reset(new Kashyyyk::Window(800, 600, group.get()));

        }
    }

    SetTheme(prefs);

    Fl::lock();

    Kashyyyk::Thread thread1(Kashyyyk::Thread::GetShortThreadPool());

    Kashyyyk::Thread thread2(Kashyyyk::Thread::GetLongThreadPool());

    while(Fl::wait()){
        Kashyyyk::Thread::PerformTask(group.get());
    }

    Kashyyyk::Close();

    return EXIT_SUCCESS;

}
