#include "server.hpp"
#include "channel.hpp"
#include "prefs.hpp"
#include "background.hpp"
#include "socket.h"
#include "message.h"
#include "parse.h"
#include "csv.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Browser.H>

#include <stack>

#ifdef SendMessage
#undef SendMessage
#endif

namespace Kashyyyk {

// Functional-style objects for SendMessageOn_Handlers.
template<IRC_messageType type>
class OnMsgType {
public:
    bool operator() (IRC_Message *msg){
        if(msg->type==type)
          return true;
        return false;
    }
};

class OnMsgAlways{
public:
    bool operator() (IRC_Message *msg){
      return true;
    }

};

// FUnctional-style objects for finding certain channels in a Server
Server::find_channel::find_channel(const std::string &s)
  : n(s){
}

Server::find_channel::find_channel(const Channel *a)
  : n(a->name) {

}

bool Server::find_channel::operator () (std::unique_ptr<Channel> &a){
    return a->name==n;
}

// Honds onto a message until a message that causes T::(msg) to return true.
// It then sends the message, frees the message, and dies.
template <class T>
class SendMessageOn_Handler : public MessageHandler {
Server *server;
IRC_Message *r_msg;
T t;
public:
    SendMessageOn_Handler(Server *s, IRC_Message *m)
      : server(s)
      , r_msg(m) {

    }

    virtual ~SendMessageOn_Handler(){
        IRC_FreeMessage(r_msg);
    }

    bool HandleMessage(IRC_Message *msg) override {
        if(t(msg)){
            server->SendMessage(r_msg);
            return true;
        }
        return false;
    }

};

// Sends the message in response to any message at all.
// A SendMessageOn with a T::(msg) that always returns true.
class SendMessage_Handler : public SendMessageOn_Handler<OnMsgAlways> {

public:
    SendMessage_Handler(Server *s, IRC_Message *m)
      : SendMessageOn_Handler<OnMsgAlways>(s, m){

    }

    virtual ~SendMessage_Handler(){

    }

};

// Checks if a certain message parameter equals the channel name for
// a certain message type, and GiveMessage's the channel if it does.
// Most Handlers should be of this type.
template<IRC_messageType type, int n = 0>
class ChannelChecker_Handler : public MessageHandler {
protected:
    Server *server;
public:
    ChannelChecker_Handler(Server *s)
      : server(s) {

    }

    bool HandleMessage(IRC_Message *msg) override {
        if( (msg->type==type) && (msg->num_parameters>n)){

            Server::ChannelList::iterator iter =
              std::find_if(server->Channels.begin(), server->Channels.end(), Server::find_channel(msg->parameters[n]));

            if(iter!=server->Channels.end())
              iter->get()->GiveMessage(msg);
        }

        return false;
    }
};

// Known specializations
typedef ChannelChecker_Handler<IRC_join>  Join_Handler;
typedef ChannelChecker_Handler<IRC_part>  Part_Handler;
typedef ChannelChecker_Handler<IRC_topic> Topic_Handler;

// These types of messages are broad to all channels, and are up to the channel
// whether or not to act on them.
template<IRC_messageType type>
class ServerToAllChannels_Handler : public MessageHandler {
protected:
    Server *server;
public:
    ServerToAllChannels_Handler(Server *s)
      : server(s) {

    }

    bool HandleMessage(IRC_Message *msg) override {
        if(msg->type==type) {
            for(Server::ChannelList::iterator iter = server->Channels.begin(); iter!=server->Channels.end(); iter++)
              iter->get()->GiveMessage(msg);
        }

        return false;
    }

};

// Known specializations
typedef ServerToAllChannels_Handler<IRC_quit> Quit_Handler;
typedef ServerToAllChannels_Handler<IRC_nick> Nick_Handler;

class Notice_Handler : public ServerToAllChannels_Handler<IRC_notice> {
    static const std::string server_s;
public:
    Notice_Handler(Server *s)
      : ServerToAllChannels_Handler<IRC_notice> (s){

    }

    bool HandleMessage(IRC_Message *msg) override {

        if(msg->type==IRC_notice){
            Server::ChannelList::iterator server_chan =
              std::find_if(server->Channels.begin(), server->Channels.end(), Server::find_channel(server_s));

            if(server_chan==server->Channels.end()){
                fprintf(stderr, "Warning: server channel not found for Server %s.\n", server->name.c_str());
                return false; // Wait, what?
            }

            server_chan->get()->GiveMessage(msg);
        }

        return false;
    }

};
const std::string Notice_Handler::server_s = "server";


// Listens for an expected channel JOIN, and adds the channel when
// it is recieved.
class JoinChannel_Handler : public MessageHandler {
    Server *server;
    std::string channel_name;
public:
    JoinChannel_Handler(Server *s, const std::string &n)
      : server(s)
      , channel_name(n)
      , promise(new PromiseValue<Channel *>(nullptr)) {

    }
    bool HandleMessage(IRC_Message *msg) override {

        if(msg->type!=IRC_join)
          return false;

        char *str = IRC_MessageToString(msg);
        printf("Join handler examining message %s\nWe are looking for %s.\n", str, channel_name.c_str());
        free(str);

        if(std::string(msg->parameters[0])!=channel_name)
          return false;

        Fl::lock();
        Channel * channel = new Channel(server, channel_name);

        promise->Finalize(channel);
        server->AddChannel_l(channel);
        Fl::unlock();

        promise->SetReady();
        return true;
    }

    std::shared_ptr<PromiseValue<Channel *> > promise;

};

class Ping_Handler : public MessageHandler {
    Server *server;
public:
    Ping_Handler(Server *s)
      : server(s) {

    }

    bool HandleMessage(IRC_Message *msg) override {
        if(msg->type==IRC_ping){

            IRC_Message *r_msg = IRC_CreatePongFromPing(msg);
            server->SendMessage(r_msg);
            IRC_FreeMessage(r_msg);

        }

        return false;
    }

};

class ServerTask : public Task {

  char *buffer;
  struct IRC_ParseState *old_state;
  Server *server;
  WSocket *socket;

public:

  ServerTask(Server *aServer, WSocket *aSocket)
    : Task()
    , buffer(nullptr)
    , old_state(nullptr)
    , server(aServer)
    , socket(aSocket) {
    repeating = true;
  }

  ~ServerTask(){
      free(buffer);
  }

  void Run() override {

      if(Length_Socket(socket)==0)
        return;

      Read_Socket(socket, &buffer);

      struct IRC_ParseState *state;
      if(old_state==nullptr)
        state = IRC_InitParse(buffer);
      else{
        printf("Stitching state.\n");
        state = IRC_StitchParse(old_state, buffer);
        IRC_DestroyParseState(old_state);
        old_state = nullptr;
      }
      struct IRC_Message *msg = IRC_ConsumeParse(state);

      while(msg!=nullptr){

          if(IRC_GetParseStatus(state)==IRC_unexpectedEnd){
              old_state = state;
              return;
          }

          Fl::lock();
          server->GiveMessage(msg);
          Fl::unlock();

          IRC_FreeMessage(msg);

          msg = IRC_ConsumeParse(state);
      }

      IRC_DestroyParseState(state);
  }

};

Server::Server(WSocket *sock, const std::string &n, Window *w)
  : TypedReciever<Window> (w)
  , last_channel(nullptr)
  , socket(sock)
  , widget(new Fl_Group(0, 0, 800, 600))
  , channel_list(new Fl_Tree_Item(tree_prefs))
  , tree_prefs()
  , name(n) {

    Fl_Preferences &prefs = GetPreferences();


    Channel *channel = new Channel(this, "server");
    AddChannel(channel);

    channel_list->user_data(channel);
    printf("channel_list has a ud of %p (server %p, we are %p)\n", channel_list->user_data(), channel->server(), this);

    w->SetChannel(channel);

    AddShortRunningTask(new ServerTask(this, socket));

    Handlers.push_back(std::unique_ptr<MessageHandler>(new Ping_Handler(this)));

    char *nick_c = nullptr;
    char *name_c = nullptr;
    char *real_c = nullptr;

    std::string server_ident("server.");
    server_ident += name + ".identity.";

    int global = 1;

    prefs.get((server_ident+"use_globals").c_str(), global, global);

    if(!global){
        prefs.get((server_ident+"nickname").c_str(), nick_c, "KashyyykUser");
        prefs.get((server_ident+"fullname").c_str(), name_c, "KashyyykName");
        prefs.get((server_ident+"realname").c_str(), real_c, "KashyyykReal");
    }
    else{

        prefs.set((server_ident+"use_globals").c_str(), global);

        prefs.get("sys.identity.nickname", nick_c, "KashyyykUser");
        prefs.get("sys.identity.fullname", name_c, "KashyyykName");
        prefs.get("sys.identity.realname", real_c, "KashyyykReal");

    }

    IRC_Message *msg_name = IRC_CreateUser(name_c, "falcon", "millenium", real_c);
    IRC_Message *msg_nick = IRC_CreateNick(nick_c);

    nick = nick_c;

    free(nick_c);
    free(name_c);
    free(real_c);

    Handlers.push_back(std::unique_ptr<MessageHandler>(new SendMessage_Handler(this, msg_name)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new SendMessage_Handler(this, msg_nick)));

    Handlers.push_back(std::unique_ptr<MessageHandler>(new Join_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Part_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Topic_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Quit_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Nick_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Notice_Handler(this)));

    char *autojoin;

    if(prefs.get((std::string("server.")+name+".autojoin").c_str(), autojoin, "")!=0){
        const char **channels = FJ::CSV::ParseString(autojoin);
        const char *iter = channels[0];
        int i = 0;
        while(iter!=nullptr){

            printf("Joining %s.\n", iter);

            IRC_Message *msg = IRC_CreateJoin(1, iter);
            SendMessage(msg);
            JoinChannel(iter);
            Handlers.push_back(std::unique_ptr<MessageHandler>(new SendMessageOn_Handler<OnMsgType<IRC_welcome_num> >(this, msg)));
            iter = channels[++i];
        }

        FJ::CSV::FreeParse(channels);

    }

    free(autojoin);

    printf("Creating Server.\n");

}

Server::~Server(){
    printf("Closing Server.\n");

    // Not particularly concerned with whether this fails or not.
    // There's not a lot we can do if it fails.
    Disconnect_Socket(socket);
    Destroy_Socket(socket);

    channel_list.release();

}

void Server::GiveMessage(IRC_Message *msg){

    lock();

    HandlerList_t::iterator iter = Handlers.begin();
    while(iter!=Handlers.end()){

        if((*iter)->HandleMessage(msg)){
            HandlerList_t::iterator d_iter = iter;
            iter--;
            Handlers.erase(d_iter);
        }
        iter++;

    }

    unlock();

}

void Server::SendMessage(IRC_Message *msg){

    if(msg->type==IRC_nick){
        nick = msg->parameters[0];
    }

    const char *str = IRC_MessageToString(msg);

    Write_Socket(socket, str);

    printf("Writing message %s\n", str);

    free((void *)str);

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

}
