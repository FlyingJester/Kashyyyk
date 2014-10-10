#include "window.hpp"
#include "channel.hpp"
#include "server.hpp"
#include "background.hpp"
#include "prefs.hpp"
#include "socket.h"
#include "message.h"

#include <cstdio>
#include <stack>
#include <cassert>

#include <FL/Fl_Window.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/fl_ask.H>

#ifdef _WIN32
#undef SendMessage
#endif

namespace Kashyyyk {

void WindowCallbacks::JoinChannel_CB(Fl_Widget *, void *p){

    assert(p);

    Window *win = static_cast<Window *>(p);

    Server *server = win->last_server;

    if(server==nullptr){
        fl_alert("You need to join a server before you can join a channel.\n");
        return;
    }

    const char *channel = fl_input("Enter Channel to Join for %s", "", server->name.c_str());

    if(!channel)
      return;

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

    ~ConnectToServer_Task(){

    }

    void Run() override {
        WSocket *sock;
        WSockErr err;
try_connect:
        sock = Create_Socket();
        err = Connect_Socket(sock, server_name.c_str(), port, 10000);
        if(!err){
            Server * s = new Server(sock, server_name, window);
            window->AddServer(s);
        }
        else{
            printf("Couldn't connect. Trying again.\n"); bool again = false;
            if(again){
                Destroy_Socket(sock);
                goto try_connect;
            }
        }
    }

};

void ConnectToServer(Fl_Widget *w, void *p){

    assert(p);

    Window *win = static_cast<Window *>(p);
    const char *str = fl_input("Enter Server Address", "");
    if(str==nullptr)
      return;
    std::string inp = str;
    long port = 6667;

    if(inp.empty())
      return;

    size_t del_at = inp.find(':');

    if(del_at!=std::string::npos){
        std::string port_str = inp.substr(del_at+1);
        port = std::stoul(port_str);

        inp = inp.substr(0, del_at);

    }

    AddLongRunningTask(new ConnectToServer_Task(win, inp.c_str(), port));
}

Window::Window(){

}

Window::Window(int w, int h, bool osx)
  : widget(new Fl_Window(w, h, "Kashyyyk IRC Client"))
  , chat_holder(new Fl_Group(128+8, 8+(osx?0:24), w-128-16, h-16-(osx?0:24)))
  , channel_list(new Fl_Tree(8, 8+(osx?0:24), 128-16, h-16-(osx?0:24)))
  , last_server(nullptr)
  , Servers() {

    osx_style = osx;

    channel_list->showroot(0);
    channel_list->callback(WindowCallbacks::ChannelList_CB, this);
    widget->add(channel_list.get());

    Fl_Menu_Bar *menubar = (osx)?new Fl_Sys_Menu_Bar(-2, 0, w+4, 24):new Fl_Menu_Bar(-2, 0, w+4, 24);

    menubar->add("File/Connect To...", 0, ConnectToServer, this);
    menubar->add("File/Quit", 0, ConnectToServer, this);
    menubar->add("Edit/Preferences", 0, OpenPreferencesWindow_CB, this);
    menubar->add("Server/Change Nick", 0, WindowCallbacks::ChangeNick_CB, this);
    menubar->add("Server/Join Channel", 0, WindowCallbacks::JoinChannel_CB, this);

    widget->add(menubar);
    //widget->add(channel_list.get());
    //widget->add(chat_holder.get());
    widget->resizable(chat_holder.get());
    widget->show();

}


Window::~Window(){
    chat_holder.release();
    channel_list.release();
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

    unsigned n_children = a->channel_list->children();

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

    printf("Moved item %s (%p)\n",  a->name.c_str(), d);

    while(!items.empty()){
        Fl_Tree_Item* e =i->add(a->tree_prefs, labels.top().c_str());
        e->user_data(items.top());
        printf("Moved item %s (%p)\n",  labels.top().c_str(), items.top());
        labels.pop();
        items.pop();
    }


    printf("%p: %s, %i\n", a->channel_list.get(), a->channel_list.get()->label(), a->channel_list.get()->is_visible());
    printf("%p: %s, %i\n", i, i->label(), i->is_visible());
    for(Fl_Tree_Item *item = channel_list->first(); item; item = channel_list->next(item) ) {
        printf("Item: %s\n", item->label());
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


Fl_Tree_Item *Window::FindChannel(const char *a){
    return channel_list->find_item(a);
}


void Window::Show(){widget->show();}
void Window::Hide(){widget->hide();}
void Window::RedrawChannels() { channel_list->redraw(); }
void Window::RedrawChat()     { chat_holder->redraw();  }
void Window::Redraw()         { widget->redraw();       }


}
