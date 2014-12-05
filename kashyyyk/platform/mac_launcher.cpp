#include "../window.hpp"
#include "../launcher.hpp"
#include "cocoa_launcher.h"

#include <FL/Fl_Menu_Item.H>
#include <FL/Fl_Sys_Menu_Bar.H>

using Kashyyyk::Launcher;
using Kashyyyk::WindowCallbacks;

extern "C"
Kashyyyk::Launcher *CreateLauncher(void /*Thread::TaskGroup*/ *a){

    Kashyyyk::Launcher *launcher = new Kashyyyk::EmptyLauncher(static_cast<Kashyyyk::Thread::TaskGroup *>(a));

    {
        int i = 0;
        Fl_Menu_Item *items = new Fl_Menu_Item[64];
            items[i++] = {"&File",0,0,0,FL_SUBMENU},
                items[i++] = {"New Window", FL_COMMAND + 'm', Launcher::NewWindow_CB, launcher};
                items[i++] = {"Connect To...", FL_COMMAND + 'g', Launcher::DirectConnect_CB, launcher};
                items[i++] = {"Server List", FL_COMMAND + 'l', Launcher::ServerList_CB, launcher};
            items[i++] = {0};
            items[i++] = {"&Edit",0,0,0,FL_SUBMENU},
                items[i++] = {"Preferences", FL_COMMAND + ',', Launcher::Preferences_CB, launcher};
            items[i++] = {0};
            items[i++] = {"&Server",0,0,0,FL_SUBMENU},
                items[i++] = {"Reconnect", FL_COMMAND + FL_SHIFT + 'a', Launcher::Reconnect_CB, launcher};
                items[i++] = {"Disconnect", FL_COMMAND + FL_SHIFT + 'w', Launcher::Disconnect_CB, launcher};
                items[i++] = {"Join Channel", FL_COMMAND + 'j', Launcher::JoinChannel_CB, launcher};
                items[i++] = {"Change Nick", FL_COMMAND + 'k', Launcher::ChangeNick_CB, launcher};
            items[i++] = {0};
        items[i++] = {0};

        Fl_Sys_Menu_Bar *menubar = new Fl_Sys_Menu_Bar(0, 0, 0, 24);
        menubar->menu(items);
    }


    return launcher;

}
