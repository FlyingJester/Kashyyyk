#include "window.hpp"
#include "channel.hpp"
#include "server.hpp"
#include "background.hpp"
#include "prefs.hpp"
#include "socket.h"
#include "message.h"
#include "serverlist.hpp"
#include "doubleinput.hpp"
#include "launcher.hpp"
#include "csv.h"

#include <cstdio>
#include <stack>
#include <cassert>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/fl_ask.H>

#ifdef SendMessage
#undef SendMessage
#endif

namespace Kashyyyk {

void Window::WindowDeleteTask::Run(){
    window->widget->do_callback();
}

void WindowCallbacks::JoinChannel_CB(Fl_Widget *, void *p){

    assert(p);

    Window *win = static_cast<Window *>(p);

    Server *server = win->last_server;

    if(server==nullptr){
        fl_alert("You need to join a server before you can join a channel.\n");
        return;
    }

    const char * const channel = fl_input("Enter Channel to Join for %s", "", server->name.c_str());

    if(!channel)
      return;

    Server::ChannelList::iterator iter = server->Channels.begin();

    while(iter!=server->Channels.end()){
        if(std::string(channel)==iter->get()->name)
          return;

        iter++;
    }

    IRC_Message *msg = IRC_CreateJoin(1, channel);
    server->SendMessage(msg);
    IRC_FreeMessage(msg);

    server->JoinChannel(channel);

}


void WindowCallbacks::ChangeNick_CB(Fl_Widget *, void *p){

    assert(p);

    Window *win = static_cast<Window *>(p);

    Server *server = win->last_server;
    if(server==nullptr){
        fl_alert("You need to join a server before you can set a nick.\n");
        return;
    }

    const char *nick = fl_input("Enter New Nick for %s", "", server->name.c_str());

    if(!nick)
      return;

    IRC_Message *msg = IRC_CreateNick(nick);

    win->last_server->SendMessage(msg);
    IRC_FreeMessage(msg);

}

void WindowCallbacks::ChannelList_CB(Fl_Widget *w, void *p){

    assert(w);
    assert(p);

    Fl_Tree *tree   = static_cast<Fl_Tree *>(w);
    Window *window = static_cast<Window *>(p);

    Fl_Tree_Item *item = tree->callback_item();

    if(item==nullptr)
      return;

    Channel *channel = static_cast<Channel *>(item->user_data());

    switch(tree->callback_reason()){
        case FL_TREE_REASON_SELECTED:
        window->SetChannel(channel);
        break;
        default:
        break;
    }

}


class AskToConnectAgain_Task : public Task {
    PromiseValue<int> &promise;
    const std::string &name;
public:
    AskToConnectAgain_Task(PromiseValue<int> &p, const std::string &n)
      : promise(p)
      , name(n) {

    }

    virtual ~AskToConnectAgain_Task(){

    }

    void Run() override {
        promise.Finalize(fl_choice("Could not connect to %s. Try again?", fl_no, fl_yes, nullptr, name.c_str()));
        promise.SetReady();
        Fl::awake();
    }

};


// Considered a long-running task.
class ConnectToServer_Task : public Task {
    Window *window;
    std::string server_name;
    long port;
public:
    ConnectToServer_Task(Window * win, const char *name, long p = 6667)
        : window(win)
        , server_name(name){
        port = p;
    }

    virtual ~ConnectToServer_Task(){

    }

    void Run() override {
        WSocket *sock;
        WSockErr err;
try_connect:
        sock = Create_Socket();

        if(!sock){
            printf("Could not create a socket.\n");
        }

        err = Connect_Socket(sock, server_name.c_str(), port, 10000);
        if(!err){
            Server * s = new Server(sock, server_name, window, port);
            window->AddServer(s);
        }
        else{
            printf("Couldn't connect. Asking if we should try again.\n");

            PromiseValue<int> promise(0);

            Task *task = new AskToConnectAgain_Task(promise, server_name);

            Thread::AddTask(window->task_group, task);

            Fl::awake();

            while(!promise.IsReady()){}

            bool again = (promise.Finish()==1);
            if(again){
                Destroy_Socket(sock);
                goto try_connect;
            }
        }
    }

};

void WindowCallbacks::ConnectToServer_CB(Fl_Widget *w, void *p){
    WindowCallbacks::ConnectToServer(static_cast<Window *>(p));
}

void WindowCallbacks::ConnectToServer(Window *win){

    assert(win);

    DoubleInput_Return r = DoubleInput("Enter Server Address", "URL", "", "Port", "6667");

    std::unique_ptr<void, void(*)(void *)> rone((void *)r.one, free);
    std::unique_ptr<void, void(*)(void *)> rtwo((void *)r.two, free);

    if(r.value==0)
      return;
    std::string inp = r.one;
    long port = atol(r.two);
    if(!port)
      port = 6667;

    printf("%s %s %ld\n", r.one, r.two, port);

    if(inp.empty())
      return;

    Thread::AddLongRunningTask(new ConnectToServer_Task(win, inp.c_str(), port));

}


Window::Window(){

}


class WindowKiller : public Task{
    Window *window;
public:
    WindowKiller(Window *w)
      : window(w) {
        repeating = false;
    }

    ~WindowKiller() override {}

    void Run() override {
        delete window;
        printf("We deleted a window!\n");
    }

};

void WindowCallbacks::WindowCallback(Fl_Widget *w, void *arg){
    Window *window = static_cast<Window *>(arg);
    Thread::AddTask(window->task_group, new WindowKiller(window));

}


Window::Window(int w, int h, Thread::TaskGroup *tg, Launcher *l, bool osx)
  : task_group(tg)
  , widget(new Fl_Double_Window(w, h, "Kashyyyk IRC Client"))
  , launcher(l)
  , chat_holder(new Fl_Group(128+8, 8+(osx?0:24), w-128-16, h-16-(osx?0:24)))
  , last_server(nullptr)
  , Servers() {

    osx_style = osx;

    widget->callback(WindowCallbacks::WindowCallback, this);
    widget->begin();

    channel_list = new Fl_Tree(8, 8+(osx?0:24), 128-8, h-16-(osx?0:24));
    channel_list->showroot(0);
    channel_list->callback(WindowCallbacks::ChannelList_CB, this);

    Fl_Menu_Bar *menubar;

    Fl_Menu_Item *items = new Fl_Menu_Item[11];
        items[0] = {"&File",0,0,0,FL_SUBMENU},
            items[1] = {"Connect To...", 0, WindowCallbacks::ConnectToServer_CB, this};
            items[2] = {"Server List", 0, ServerList, this};
        items[3] = {0};
        items[4] = {"&Edit",0,0,0,FL_SUBMENU},
            items[5] = {"Preferences", 0, OpenPreferencesWindow_CB, this};
        items[6] = {0};
        items[7] = {"&Server",0,0,0,FL_SUBMENU},
            items[8] = {"Change Nick", 0, WindowCallbacks::ChangeNick_CB, this};
            items[9] = {"Join Channel", 0, WindowCallbacks::JoinChannel_CB, this};
    items[10] = {0};

    if(osx){
        Fl_Sys_Menu_Bar *mb = new Fl_Sys_Menu_Bar(0, 0, 0, 30);
        mb->menu(items);
        menubar = mb;
    }
    else{
        menubar = new Fl_Menu_Bar(-2, 0, w+4, 24);
        menubar->menu(items);
    }

    widget->add(menubar);

    widget->resizable(chat_holder);
    widget->show();

}


Window::~Window(){
    if(launcher){
        launcher->Release(this);

        printf("Released from launcher.\n");
    }
    else
      printf("Did not release from launcher.\n");
}


void Window::AddServer(Server *a){

    assert(a);

    Servers.push_back(std::unique_ptr<Server>(a));

    AutoLocker<Server *> lock(a);
    Server::ChannelList::iterator iter = a->Channels.begin();

    while(iter!=a->Channels.end()){
        Channels.push_back(iter->get());
        iter++;
    }

    a->widget->resize(chat_holder->x(), chat_holder->y(), chat_holder->w(), chat_holder->h());
    chat_holder->add(a->widget.get());

    std::stack<void *> items;
    std::stack<std::string> labels;

    int n_children = a->channel_list->children();

    while(a->channel_list->has_children()){
        items.push(a->channel_list->next()->user_data());
        labels.push(a->channel_list->next()->label());
        a->channel_list->remove_child(a->channel_list->next());

        assert(a->channel_list->children()<n_children);
        n_children = a->channel_list->children();
    }

    void *d = a->channel_list->user_data();
    Fl_Tree_Item* i = channel_list->add(a->name.c_str());

    i->user_data(d);

    a->channel_list.reset(i);

    while(!items.empty()){
        Fl_Tree_Item* e =i->add(a->tree_prefs, labels.top().c_str());
        e->user_data(items.top());
        labels.pop();
        items.pop();
    }

    Fl::lock();

    if(a!=last_server){
        if(last_server)
          last_server->Hide();

        last_server = a;

        a->Show();
    }

    channel_list->redraw();
    chat_holder->redraw();
    Fl::unlock();

    SetChannel(a->last_channel);

}


void Window::RemoveChannel(Channel *a){

    assert(a);

    std::list<Channel *>::iterator iter = Channels.begin();
    while(iter!=Channels.end()){
        if(*iter==a){
            Channels.erase(iter);
            return;
        }
        iter++;
    }

}

void Window::SetChannel(Channel *channel){

    assert(channel);

    Server *new_server = channel->server();

    assert(new_server);

    if((last_server!=nullptr) && (last_server!=new_server))
      last_server->Hide();

    new_server->Show(channel);
    last_server=new_server;

    // Make the tree reflect the change.
    std::string path = new_server->name;
    path.push_back('/');
    path += channel->name;

    Fl_Tree_Item *i = channel_list->find_item(path.c_str());
    if(i){
        channel_list->deselect_all(channel_list->root(), 0);
        channel_list->select(i, 0);
    }

}


void Window::AutoJoinServers(void){
    char *autojoin;

    Fl_Preferences &prefs = GetPreferences();

    if(prefs.get(std::string("sys.server_autoconnect.default").c_str(), autojoin, "")!=0){
        const char **servers = FJ::CSV::ParseString(autojoin);
        const char *iter = servers[0];
        int i = 0;
        while(iter!=nullptr){

            printf("Connecting to %s.\n", iter);

            char *address;
            int port;

            if(prefs.get((std::string("server.")+iter+".address").c_str(), address, "")==0){
                continue;
            }
            prefs.get((std::string("server.")+iter+".port").c_str(), port, 6665);

            Thread::AddLongRunningTask(new ConnectToServer_Task(this, address, port));

            iter = servers[++i];
        }

        FJ::CSV::FreeParse(servers);

    }

    free(autojoin);
}


void Window::AutoJoinChannels(void){
    lock();
    for(std::list<std::unique_ptr<Server> >::iterator iter = Servers.begin(); iter!=Servers.end(); iter++){
        iter->get()->AutoJoinChannels();
    }
    unlock();
}


Fl_Tree_Item *Window::FindChannel(const char *a){
    return channel_list->find_item(a);
}

void Window::ForgetLauncher(){
    printf("Forgot launcher.\n");
    launcher = nullptr;
}

void Window::Show(){widget->show();}
void Window::Hide(){widget->hide();}
void Window::RedrawChannels() { channel_list->redraw(); }
void Window::RedrawChat()     { chat_holder->redraw();  }
void Window::Redraw()         { widget->redraw();       }


}
