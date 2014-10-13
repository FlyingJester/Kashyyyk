#include "server.hpp"
#include "channel.hpp"
#include "prefs.hpp"
#include "background.hpp"
#include "socket.h"
#include "message.h"
#include "parse.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Browser.H>

#include <stack>

#ifdef _WIN32
#undef SendMessage
#endif

namespace Kashyyyk {

class SendMessage_Handler : public MessageHandler {
Server *server;
IRC_Message *r_msg;

public:
    SendMessage_Handler(Server *s, IRC_Message *m)
      : server(s)
      , r_msg(m) {

    }

    virtual ~SendMessage_Handler(){
        IRC_FreeMessage(r_msg);
    }

    bool HandleMessage(IRC_Message *msg) override {
        server->SendMessage(r_msg);
        return true;
    }


};



class NickChange_Handler : public MessageHandler {
    Server *server;
public:
    NickChange_Handler(Server *s)
      : server(s){

    }

    bool HandleMessage(IRC_Message *msg) override {
        if(msg->type!=IRC_nick)
          return false;

        if(msg->from==NULL)
          return false;

        assert(msg->num_parameters>0);
        assert(msg->parameters[0]);

        Server::ChannelList::iterator iter = server->Channels.begin();

        while(iter!=server->Channels.end()){
            (*iter)->GiveMessage(msg);
            iter++;
        }

        return false;

    }

};


class PrivateMessage_Handler : public MessageHandler {
    Server *server;
public:
    PrivateMessage_Handler(Server *s)
      : server(s) {

    }

    bool HandleMessage(IRC_Message *msg) override {

        if(msg->type!=IRC_privmsg)
          return false;

        printf("Message for channel %s\n", msg->parameters[0]);
        std::string name = msg->parameters[0];

        Server::ChannelList::iterator iter = server->Channels.begin();


        while(iter!=server->Channels.end()){
            if(iter->get()->name==name){

                Fl::lock();
                iter->get()->GiveMessage(msg);
                Fl::unlock();

                return false;
            }

            iter++;

        }

        Fl::lock();

        Channel * channel = new Channel(server, name);

        server->AddChannel_l(channel);
        channel->GiveMessage(msg);

        Fl::unlock();

        return false;

    }


};

class Topic_Handler : public MessageHandler {
    Server *server;
public:

    Topic_Handler(Server *s)
      : server(s){

    }

    bool HandleMessage(IRC_Message *msg) override {

        if((msg->type!=IRC_topic) && (msg->type!=IRC_topic_num))
          return false;

        assert(msg->num_parameters>=2);

        std::string name = msg->parameters[1];

        const char *str = IRC_MessageToString(msg);

        printf("Handling topic for %s (%s) (%s).\n", msg->parameters[1], msg->parameters[2], str);

        free((void *)str);

        Server::ChannelList::iterator iter = server->Channels.begin();

        while(iter!=server->Channels.end()){
            if(iter->get()->name==name){

                Fl::lock();
                iter->get()->SetTopic(msg->parameters[2]);
                Fl::unlock();

                break;
            }

            iter++;

        }

        return false;

    }


};

class NameList_Handler : public MessageHandler {
protected:
    Server *server;
    std::string channel_name;

public:
    NameList_Handler(Server *s, const std::string &n, std::shared_ptr<bool> &b, std::shared_ptr<PromiseValue<Channel *> > &promise)
      : server(s)
      , channel_name(n)
      , ChannelPromise(promise)
      , Started(b){

    }

    virtual bool HandleMessage(IRC_Message *msg) = 0;

    std::shared_ptr<PromiseValue<Channel *> > ChannelPromise;

    // This is referenced by the namelist end handler (which should also be installed at the same time.
    std::shared_ptr<bool> Started;
};

class NameListBegin_Handler : public NameList_Handler{

public:
    NameListBegin_Handler(Server *s, const std::string &n, std::shared_ptr<bool> &b, std::shared_ptr<PromiseValue<Channel *> > &p)
      : NameList_Handler(s, n, b, p) {

    }

    bool HandleMessage(IRC_Message *msg) override {

        printf("Namelist Begin Promise ready? %i\n", ChannelPromise->IsReady());
        if(msg->type!=IRC_namelist_start_num)
          return false;

        printf("Namelist message:");
        for(int i = 0; i<msg->num_parameters; i++)
          printf("%s __ ", msg->parameters[i]);

        printf("\n");

        if(!ChannelPromise->IsReady())
          return false;

        if(std::string(msg->parameters[2])!=channel_name)
          return false;

        *Started = true;

        Channel *channel = ChannelPromise->Finish();

        Fl::lock();

        std::stack<User> name_stack;
        {
            std::string names = msg->parameters[msg->num_parameters-1];

            std::string::iterator first = names.begin(), next = names.begin(), last = names.end();
            last--;

            while(next!=names.end()){

                while((next!=last) && (*next!=' '))
                  next++;

                std::string::iterator mode = first;

                char c = *mode;
                std::string mode_s;
                if((c=='=')||(c=='&')||(c=='%')||(c=='*')||(c==' ')||(c=='~')){
                    mode++;
                    mode_s = std::string(first, mode);
                }
                else{
                    mode_s = "";
                }

                std::string l(mode, next);
                name_stack.push({l, mode_s});
                first = next;

                next++;
            }
        }

        while(!name_stack.empty()){
            channel->AddUser(name_stack.top());
            name_stack.pop();
        }

        Fl::unlock();


        return true;

    }
};

class NameListEnd_Handler : public NameList_Handler {

public:
    NameListEnd_Handler(Server *s, const std::string &n, std::shared_ptr<bool> &b, std::shared_ptr<PromiseValue<Channel *> > &p)
      : NameList_Handler(s, n, b, p) {

    }

    bool HandleMessage(IRC_Message *msg) override {


        printf("Namelist End Promises ready? %i %i\n", ChannelPromise->IsReady(), *Started);

        if(!ChannelPromise->IsReady())
          return false;


        if((msg->type!=IRC_namelist_end_num) || (!*Started) ||
           (std::string(msg->parameters[1])!=channel_name)){
            return false;
        }

        return true;

    }
};

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


template <IRC_messageType type>
class ServerChannel_Handler : public MessageHandler {
  Channel *channel;
  Server  *server;
public:
    ServerChannel_Handler(Server *s, Channel *Server_Channel)
      : channel(Server_Channel)
      , server(s) {

    }

    bool HandleMessage(IRC_Message *msg) override {

        if(msg->type==type){
            Fl::lock();
            channel->GiveMessage(msg);
            Fl::unlock();
        }
        return false;
    }

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

    Handlers.push_back(std::unique_ptr<MessageHandler>(new ServerChannel_Handler<IRC_notice>(this, channel)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new ServerChannel_Handler<IRC_error_m>(this, channel)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Ping_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new PrivateMessage_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Topic_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new NickChange_Handler(this)));

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

    HandlerList::iterator iter = Handlers.begin();
    while(iter!=Handlers.end()){
        HandlerList::iterator d_iter = Handlers.end();

        if((*iter)->HandleMessage(msg)){
            d_iter = iter;
        }

        iter++;

        if(d_iter!=Handlers.end())
          Handlers.erase(d_iter);

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

    Handlers.push_back(std::move(std::unique_ptr<MessageHandler>(new ServerChannel_Handler<IRC_quit>(this, a))));
    Handlers.push_back(std::move(std::unique_ptr<MessageHandler>(new ServerChannel_Handler<IRC_join>(this, a))));


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

    NameList_Handler *name_beg_handler = new NameListBegin_Handler(this, channel, started, handler_raw->promise);
    NameList_Handler *name_end_handler = new NameListEnd_Handler(this, channel, started, handler_raw->promise);

    lock();
    Handlers.push_back(std::move(std::unique_ptr<MessageHandler>(handler_raw)));
    Handlers.push_back(std::move(std::unique_ptr<MessageHandler>(name_beg_handler)));
    Handlers.push_back(std::move(std::unique_ptr<MessageHandler>(name_end_handler)));
    unlock();

    return handler_raw->promise;
}

}
