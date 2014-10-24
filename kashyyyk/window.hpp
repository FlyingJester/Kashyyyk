#pragma once

#include <list>
#include <vector>
#include <memory>
#include <mutex>

#include "autolocker.hpp"
#include "background.hpp"
#include "platform/pling.h"

#include <FL/Fl_Tree.H>

#ifdef __APPLE__
#define IS_OSX true
#else
#define IS_OSX false
#endif

class Fl_Window;
class Fl_Group;
class Fl_Tree;
class Fl_Tree_Item;
class Fl_Scroll;

struct Fl_Menu_Item;

namespace Kashyyyk {

class Server;
class Channel;
class Launcher;

class Window;

class WindowCallbacks{
public:
    static void ChangeNick_CB(Fl_Widget *, void *);
    static void JoinChannel_CB(Fl_Widget *, void *);
    static void ChannelList_CB(Fl_Widget *, void *);
    static void WindowCallback(Fl_Widget *w, void *arg);
    static void ConnectToServer_CB(Fl_Widget *w, void *p);
    static void ConnectToServer(Window *p);
};


class Window {
public:
    Thread::TaskGroup *task_group;

    class WindowDeleteTask : public Task {
        Window *window;
    public:
        WindowDeleteTask(Window *w)
          : window(w){}
        ~WindowDeleteTask() override {};

        void Run() override;
    };

protected:

    std::unique_ptr<Fl_Window>  widget;
    Launcher *launcher;
    Fl_Group  *chat_holder;
    Fl_Tree   *channel_list;

    std::mutex mutex;

    inline void lock(){mutex.lock();}
    inline void unlock(){mutex.unlock();}

    bool osx_style;

    Fl_Tree_Prefs prefs;

    Server *last_server;

    static std::mutex order_mutex;
    static std::list<Window *> window_order;

public:

    friend class AutoLocker<Window *>;
    friend class WindowCallbacks;

    Window();
    Window(int w, int h, Thread::TaskGroup *g, Launcher *l = nullptr, bool osx = IS_OSX);
    ~Window();

    Fl_Menu_Item   *reconnect_item;
    Fl_Menu_Item   *disconnect_item;

    std::list<Channel *> Channels;
    std::list<std::unique_ptr<Server> > Servers;

    void AddServer(Server *);
    void RemoveChannel(Channel *);

    void SetChannel(Channel *);

    // Only call on the main thread.
    void Show();
    void Hide();

    // Remember to call Fl::lock() before calling these on other threads.
    void RedrawChannels();
    void RedrawChat();
    void Redraw();

    Fl_Tree_Item *FindChannel(const char *);

    inline const Fl_Window *Handle(){
      return widget.get();
    }

    void Pling(){
        Kashyyyk::Pling(widget.get());
    }

    void AutoJoinServers(void);
    void AutoJoinChannels(void);

    void ForgetLauncher();

};

}
