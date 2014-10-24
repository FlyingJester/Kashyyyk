#include "window.hpp"
#include "background.hpp"
#include "networkwatch.hpp"
#include "prefs.hpp"
#include "launcher.hpp"
#include "platform/notification.h"
#include "platform/init.h"

#include <stack>
#include <string>
#include <cstdlib>
#include <cassert>
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

    std::unique_ptr<Kashyyyk::Thread::TaskGroup, void(*)(Kashyyyk::Thread::TaskGroup*)>
      group(Kashyyyk::Thread::CreateTaskGroup(), Kashyyyk::Thread::DestroyTaskGroup);

    Kashyyyk::Thread::TaskGroup *group_raw = group.get();

    {

        Kashyyyk::Window *window = nullptr;
        std::function<Kashyyyk::Window *()> new_window_function;

        Kashyyyk::Launcher *launcher;
        int startup_launcher         = 1;
        int startup_launchertype     = 0;
        int startup_window           = 1;
        int startup_autojoin_servers = 1;
        int startup_autojoin_channels= 1;

        prefs.get("sys.startup.launcher.enabled", startup_launcher, startup_launcher);
        prefs.get("sys.startup.launcher.type", startup_launchertype, startup_launchertype);
        prefs.get("sys.startup.window.enabled", startup_window, startup_window);
        prefs.get("sys.startup.window.autojoin.servers", startup_autojoin_servers, startup_autojoin_servers);
        prefs.get("sys.startup.window.autojoin.channels", startup_autojoin_channels, startup_autojoin_channels);

        if(startup_launcher){
            switch(startup_launchertype){
            case 1:
                launcher = new Kashyyyk::BoringLauncher(group_raw);
            break;
#ifndef NO_ICONLAUNCHER
            case 2:
                launcher = new Kashyyyk::IconLauncher(group_raw);
            break
#endif
            break;
            case 0:
            default:
                launcher = Kashyyyk::Launcher::CreatePlatformLauncher(group_raw);
            }

            new_window_function = std::bind(std::mem_fn(&Kashyyyk::Launcher::NewWindow), launcher);
        }
        else{
            new_window_function = std::bind([=](){return new Kashyyyk::Window(800, 600, group_raw, nullptr, false);});
        }

        if(startup_window){
            window = new_window_function();
            assert(window);

        }

        if(window){

            if(startup_autojoin_servers)
              window->AutoJoinServers();

            if(startup_autojoin_channels)
              window->AutoJoinChannels();

        }


    }

    SetTheme(prefs);

    Fl::lock();

    Kashyyyk::NetworkWatch watch(Kashyyyk::Thread::GetShortThreadPool());
    Kashyyyk::Thread::AddWatchToTaskGroup(&watch, Kashyyyk::Thread::GetShortThreadPool());
    {
        Kashyyyk::Thread thread1(Kashyyyk::Thread::GetShortThreadPool());

        Kashyyyk::Thread thread2(Kashyyyk::Thread::GetLongThreadPool());

        while(Fl::wait()){
            Kashyyyk::Thread::PerformTask(group_raw);
        }

        Kashyyyk::Close();
    }
    return EXIT_SUCCESS;

}
