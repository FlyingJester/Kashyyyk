#include "server.hpp"
#include "servermessage.hpp"
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


class ServerTask : public Task {

    char *buffer;
    struct IRC_ParseState *old_state;
    Server *server;
    WSocket *socket;
    bool *task_died;

public:

    ServerTask(Server *aServer, WSocket *aSocket, bool *deded)
    : Task()
    , buffer(nullptr)
    , old_state(nullptr)
    , server(aServer)
    , socket(aSocket)
    , task_died(deded)
    , should_die(false) {
        repeating = true;
    }

    virtual ~ServerTask(){

        printf("ServerTask is being deleted.\n");

        free(buffer);

        *task_died = true;

    }

    void Run() override {
        {
            AutoLocker<Server *> locker(server);

            if(should_die){

                printf("ServerTask signalled to deletes.\n");
                repeating = false;
            }
        }
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
        printf("ServerTask Ren.\n");

        IRC_DestroyParseState(state);

    }

    bool should_die;

};

Server::Server(WSocket *sock, const std::string &n, Window *w, const char *uid)
  : LockingReciever<Window, std::mutex> (w)
  , last_channel(nullptr)
  , socket(sock)
  , widget(new Fl_Group(0, 0, 800, 600))
  , channel_list(new Fl_Tree_Item(tree_prefs))
  , tree_prefs()
  , task_died(false)
  , network_task(new ServerTask(this, socket, &task_died))
  , UID(uid?uid:"")
  , name(n){

    Fl_Preferences &prefs = GetPreferences();


    Channel *channel = new Channel(this, "server");
    AddChannel(channel);

    channel_list->user_data(channel);

    w->SetChannel(channel);

    Thread::AddShortRunningTask(network_task);

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


    printf("Creating Server.\n");

}


void Server::AutoJoinChannels(void){
    char *autojoin;

    if(GetPreferences().get((std::string("server.")+UID+".autojoin").c_str(), autojoin, "")!=0){
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
}


Server::~Server(){
    printf("Closing Server.\n");

    lock();
    network_task->should_die = true;
    while(!task_died){
        unlock();

        lock();
    }
    unlock();

    channel_list.release();

    // Not particularly concerned with whether this fails or not.
    // There's not a lot we can do if it fails.
    Disconnect_Socket(socket);
    Destroy_Socket(socket);


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
