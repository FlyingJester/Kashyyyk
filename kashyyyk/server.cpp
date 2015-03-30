#include "server.hpp"
#include "servermessage.hpp"
#include "channelmessage.hpp"
#include "channel.hpp"
#include "prefs.hpp"
#include "background.hpp"
#include "socket.h"
#include "message.h"
#include "parse.h"
#include "csv.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Menu.H>
#include <FL/Fl_Browser.H>

#include <stack>

#ifdef SendMessage
#undef SendMessage
#endif


using namespace Kashyyyk::ServerMessage;

namespace Kashyyyk {

Server::find_channel::find_channel(const std::string &s)
  : n(s){

}


Server::find_channel::find_channel(const Channel *a)
  : n(a->name) {

}


bool Server::find_channel::operator () (const std::unique_ptr<Channel> &a){
    return a->name==n;
}


void Server::ReconnectServer_CB(Fl_Widget *, void *p){

    assert(p);

    Server *server = static_cast<Server *>(p);

    server->Reconnect();

    server->Parent->reconnect_item->deactivate();

}


ServerConnectTask::ServerConnectTask(const Server *aServer, WSocket *aSocket, long prt, bool rc, bool SSL)
  : server(aServer)
  , socket(aSocket)
  , reconnect_channels(rc)
  , port(prt)
  , promise(new PromiseValue<bool>(false)) {

}

void ServerConnectTask::Run(){
    int err = Connect_Socket(socket, server->GetName().c_str(), port, 10000);

    if(err!=eAlreadyConnected){
        repeating = true;
        //server->connected = false;
        return;
    }

    promise->SetReady();
    promise->Finalize(true);
    repeating = false;
    reconnect_channels = true;
    //server->connected = true;
}


class ServerTask : public Task {

    char *buffer;
    struct IRC_ParseState *old_state;
    Server *server;
    WSocket *socket;
    bool *task_died;

    std::shared_ptr<PromiseValue<bool> > promise;

public:

    ServerTask(Server *aServer, WSocket *aSocket, bool *deded)
    : Task()
    , buffer(nullptr)
    , old_state(nullptr)
    , server(aServer)
    , socket(aSocket)
    , task_died(deded)
    , should_die(false){
        repeating = true;
    }

    virtual ~ServerTask(){

        free(buffer);

        *task_died = true;

    }

    void Run() override {
        {
            AutoLocker<Server *> locker(server);

            if(should_die)
                repeating = false;

        }

        if(promise){
            if(promise->IsReady()){
                promise.reset();
                return;
            }
        }

        if(State_Socket(socket)!=eConnected){
            promise = server->Reconnect();
            return;
        }

        if(Length_Socket(socket)==0)
          return;

        Read_Socket(socket, &buffer);

        struct IRC_ParseState *state;
        if(old_state==nullptr)
          state = IRC_InitParse(buffer);
        else{
            state = IRC_StitchParse(old_state, buffer);
            IRC_DestroyParseState(old_state);
            old_state = nullptr;
        }
        struct IRC_Message *msg = IRC_ConsumeParse(state);

        while((msg!=nullptr) || (IRC_GetParseStatus(state)==IRC_badMessage)){

            if(IRC_GetParseStatus(state)==IRC_unexpectedEnd){
                old_state = state;
                return;
            }

            if(IRC_GetParseStatus(state)!=IRC_badMessage){
                Fl::lock();
                server->GiveMessage(msg);
                Fl::unlock();

                IRC_FreeMessage(msg);
            }

            msg = IRC_ConsumeParse(state);
        }
        IRC_DestroyParseState(state);

    }


    bool should_die;

};

Server::Server(const struct ServerState &init_state, Window *w)
  : LockingReciever<Window, Monitor> (w)
  , last_channel(nullptr)
  , widget(new Fl_Group(0, 0, 800, 600))
  , channel_list(new Fl_Tree_Item(tree_prefs))
  , tree_prefs()
  , task_died(false)
  , network_task(new ServerTask(this, init_state.socket, &task_died)){
    
    CopyState(state, init_state);
    state.socket = init_state.socket;

    Channel *channel = new Channel(this, "server");

    channel->Handlers.push_back(std::unique_ptr<MessageHandler>(new ChannelMessage::YourHost_Handler(channel)));
    channel->Handlers.push_back(std::unique_ptr<MessageHandler>(new ChannelMessage::Notice_Handler(channel)));
    channel->Handlers.push_back(std::unique_ptr<MessageHandler>(new ChannelMessage::TopicExtra_Handler(channel)));
    AddChannel(channel);

    channel_list->user_data(channel);

    w->SetChannel(channel);

    Thread::AddShortRunningTask(network_task);

    Thread::AddSocketToTaskGroup(state.socket, Thread::GetShortThreadPool());

    Handlers.push_back(std::unique_ptr<MessageHandler>(new Ping_Handler(this)));
    // This should all be put in the call before construction

    IRC_Message *msg_name = IRC_CreateUser(state.name.c_str(), "falcon", "millenium", state.real.c_str());
    IRC_Message *msg_nick = IRC_CreateNick(state.nick.c_str());

    Handlers.push_back(std::unique_ptr<MessageHandler>(new Debug_Handler()));

    Handlers.push_back(std::unique_ptr<MessageHandler>(new SendMessage_Handler(this, msg_name)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new SendMessage_Handler(this, msg_nick)));

    Handlers.push_back(std::unique_ptr<MessageHandler>(new Join_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Part_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Topic_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new PrivateMessage_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new NumericTopic_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new NumericNoTopic_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Namelist_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Quit_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Nick_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Notice_Handler(this)));

    for(std::list<std::string>::const_iterator iter = state.channels.cbegin(); iter!=state.channels.cend(); iter++){

        printf("Joining %s.\n", iter->c_str());

        IRC_Message *msg = IRC_CreateJoin(1, iter->c_str());
        SendMessage(msg);
        JoinChannel(iter->c_str());
        Handlers.push_back(std::unique_ptr<MessageHandler>(new SendMessageOn_Handler<OnMsgType<IRC_welcome_num> >(this, msg)));
        
    }

    printf("Creating Server.\n");
}

Server::~Server(){
    printf("Closing Server.\n");

    lock();
    network_task->should_die = true;

    Thread::RemoveSocketFromTaskGroup(state.socket, Thread::GetShortThreadPool());

    while(!task_died){
        unlock();

        lock();
    }
    unlock();

    channel_list.release();

    // Not particularly concerned with whether this fails or not.
    // There's not a lot we can do if it fails.
    Disconnect_Socket(state.socket);
    Destroy_Socket(state.socket);


}

void Server::SendMessage(IRC_Message *msg){
    const char *swap = nullptr;
    int swap_n = -1;
    std::string msg_r;


    if(msg->type==IRC_nick){
        state.nick = msg->parameters[0];
    }
    if((msg->type==IRC_privmsg) && (msg->num_parameters>1)){
        // TODO: Stop the evil empire that is freenode.
        swap_n = 1;
        swap = msg->parameters[swap_n];
        msg_r = msg->parameters[swap_n];
        msg_r.insert(0, ":");
        msg->parameters[swap_n] = msg_r.c_str();
    }

    const char *str = IRC_MessageToString(msg);

    Write_Socket(state.socket, str);

    printf("Writing message %s\n", str);

    free((void *)str);

    if((swap_n>0) && (msg->num_parameters>swap_n))
       msg->parameters[swap_n] = swap;
}

void Server::AddChannel_l(Channel *a){
    
    Channels.push_back(std::move(std::unique_ptr<Channel>(a)));
    Fl_Tree_Item *item = channel_list->add(tree_prefs, a->name.c_str());
    item->user_data(a);

    Parent->SetChannel(a);
    Parent->RedrawChannels();

    printf("Added channel %s\n", a->name.c_str());

}
void Server::AddChannel(Channel *a){

    lock();

    AddChannel_l(a);

    unlock();

}

void Server::AddChild(Fl_Group *a){

    a->resize(widget->x(), widget->y(), widget->w(), widget->h());
    widget->add(a);
    widget->redraw();

}


void Server::Show(){
    Show(last_channel);
}

void Server::Show(Channel *chan){
    
    if(last_channel)
      last_channel->FocusChanged();

    if((chan) && (chan!=last_channel)){
        if(last_channel)
          last_channel->widget->hide();

        if(chan)
          chan->widget->show();

    }

    last_channel = chan;

    widget->show();

    FocusChanged();

}


void Server::Hide(){

    Fl_Group *group = widget.get();

    if(group && group->visible())
      group->hide();

    FocusChanged();

}


void Server::FocusChanged(){
    channel_list->labelcolor(FL_FOREGROUND_COLOR);
}


void Server::Highlight(){
    channel_list->labelcolor(FL_DARK_BLUE);
}


std::shared_ptr<PromiseValue<Channel *> > Server::JoinChannel(const std::string &channel){

    JoinChannel_Handler *handler_raw = new JoinChannel_Handler(this, channel);

    std::shared_ptr<bool> started(new bool(false));

    lock();
    Handlers.push_back(std::move(std::unique_ptr<MessageHandler>(handler_raw)));
    unlock();

    return handler_raw->promise;
}

std::shared_ptr<PromiseValue<bool> > Server::Reconnect() const {
    
    if((last_reconnect.get()) && (!last_reconnect->IsReady())){
        ServerConnectTask *task = new ServerConnectTask(this, state.socket, state.port, true);

        Thread::AddLongRunningTask(task);

        last_reconnect.reset(task->promise.get());
    }

    return last_reconnect;

}

/*
std::shared_ptr<PromiseValue<bool> > Server::Reconnect(bool rc) const{

    if((last_reconnect.get()) && (!last_reconnect->IsReady())){
        ServerConnectTask *task = new ServerConnectTask(this, state.socket, state.port, rc);

        Thread::AddLongRunningTask(task);

        last_reconnect.reset(task->promise.get());
    }

    return last_reconnect;

}
*/

std::shared_ptr<PromiseValue<bool> >  Server::Disconnect() const{
    Disconnect_Socket(state.socket);

    PromiseValue<bool> *promise = new PromiseValue<bool>(true);
    promise->SetReady();
    return std::shared_ptr<PromiseValue<bool> >(promise);
}


bool Server::IsConnected() const{
    WSockErr e = State_Socket(state.socket);
    return (e==eConnected);
}


bool Server::SocketStatus(){
    WSockErr e = State_Socket(state.socket);
    return e==eConnected;
}

bool Server::CopyState(struct ServerState &to, const struct ServerState &from){
    
    to.name = from.name;
    
    to.nick = from.nick;
    to.user = from.user;
    to.real = from.real;
    
    to.port = from.port;
    to.socket = nullptr;
    to.SSL = from.SSL;
    
    to.channels.clear();
    
    for(auto i = from.channels.cbegin(); i!=from.channels.cend(); i++){
        to.channels.push_back(*i);
    }
    
    return true;
    
}

}
