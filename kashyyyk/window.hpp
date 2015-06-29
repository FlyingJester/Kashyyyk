#pragma once

#include "autolocker.hpp"
#include "background.hpp"
#include "promise.hpp"
#include "platform/pling.h"
#include "monitor.hpp"

#include <list>
#include <vector>
#include <memory>
#include <string>

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


class AskToConnectAgain_Task : public Task {
    PromiseValue<int> &promise;
    const std::string &name;
public:
    AskToConnectAgain_Task(PromiseValue<int> &p, const std::string &n)
      : promise(p)
      , name(n) {}

    virtual ~AskToConnectAgain_Task(){}

    void Run() override;
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
//    Fl_Tree   *channel_list;
    
    Monitor mutex;

    inline void lock(){mutex.Lock();}
    inline void unlock(){mutex.Unlock();}

    bool osx_style;

    Fl_Tree_Prefs prefs;

    Server *last_server;

public:

    friend class AutoLocker<Window *>;
    friend class WindowCallbacks;

    Window();
    Window(int w, int h, Thread::TaskGroup *g, Launcher *l = nullptr, bool osx = IS_OSX);
    ~Window();

    Fl_Menu_Item   *reconnect_item;
    Fl_Menu_Item   *disconnect_item;

    std::list<Channel *> channels;
    std::list<std::unique_ptr<Server> > servers;

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

   // Fl_Tree_Item *FindChannel(const char *);

    inline const Fl_Window *Handle(){
      return widget.get();
    }

    void Pling(){
        Kashyyyk::Pling(widget.get());
    }

    void ForgetLauncher();

    std::shared_ptr<PromiseValue<bool> >  ReconnectLastServer();
    void DisconnectLastServer();

    void GDebugReconnectLastServer();
    void GDebugDisconnectLastServer();

    // Does not need locking, since all callbacks are on the main thread.
    static std::list<const Window *> window_order;

};

}
