#include "serverdatabase.hpp"
#include "serverlist.hpp"
#include "editlist.hpp"
#include "prefs.hpp"
#include "doubleinput.hpp"
#include "identityframe.hpp"
#include "background.hpp"
#include "server.hpp"
#include "window.hpp"
#include "socket.h"
#include "csv.h"

#include <vector>
#include <string>
#include <array>
#include <functional>

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>

/*
struct ServerData {

    const char * UID;
    std::string name;
    std::string address;
    long port;

    std::vector<std::string>group_UIDs;
    std::vector<std::string>autojoin_channels;

    bool SSL;
    bool global;
    std::string nick;
    std::string user;
    std::string real;
};
*/

static std::unique_ptr<Fl_Window> serverlist_window = nullptr;
static struct Kashyyyk::ServerData * selected_server_data;
static Kashyyyk::ServerDB server_db;


static const int WindowWidth  = 600;
static const int WindowHeight = 400;


static const char *const server_property_names[] = {
    "name",           //0
    "nickname",       //1
    "username",       //2
    "realname",       //3
    "address",        //4
    "port",           //5
    "ssl",            //6
    "globalidentity", //7
    "autojoin"        //8
    "groupuids"       //9
};


namespace Kashyyyk {


struct IdentityFrame identity_frame;

struct GroupFrame {
    Fl_Group *group;
    Kashyyyk::EditList<Fl_Browser> *group_list;
    Fl_Button *edit_groups;
} group_frame;


struct AutoJoinFrame {
    Fl_Group *group;
    Kashyyyk::EditList<> *channel_list;
} autojoin_frame;


static Window * serverlist_associated_window;


void UpdateEnabled(){

    auto func = std::mem_fn(selected_server_data->global?&Fl_Widget::deactivate:&Fl_Widget::activate);

    func(identity_frame.nick);
    func(identity_frame.user);
    func(identity_frame.real);

    int type_enum = *static_cast<int *>(identity_frame.signon_type->menu()[identity_frame.signon_type->value()].user_data_);

    if(type_enum&1){
        identity_frame.password->value(selected_server_data->password.c_str());
        identity_frame.password->activate();
    }
    else{
        identity_frame.password->value("");
        identity_frame.password->deactivate();
    }

    if(type_enum&2){
        identity_frame.username->value(selected_server_data->username.c_str());
        identity_frame.username->activate();
    }
    else{
        identity_frame.username->value("");
        identity_frame.username->deactivate();
    }

}


void UpdateIdentity(){

    identity_frame.global->value(selected_server_data->global);

    if(selected_server_data->global){
        identity_frame.nick->value("");
        identity_frame.user->value("");
        identity_frame.real->value("");
    }
    else{
        identity_frame.nick->value(selected_server_data->nick.c_str());
        identity_frame.user->value(selected_server_data->user.c_str());
        identity_frame.real->value(selected_server_data->real.c_str());
    }

    UpdateEnabled();

}


void UpdateServerIdentity(){

    selected_server_data->global = identity_frame.global->value();
    selected_server_data->nick   = identity_frame.nick->value();
    selected_server_data->user   = identity_frame.user->value();
    selected_server_data->real   = identity_frame.real->value();

}


void SelectServer(const char * UID){

    struct equals_uid predicate(UID);
    ServerDB::iterator iter = std::find_if(server_db.begin(), server_db.end(), predicate);

    selected_server_data = iter->get();

    UpdateIdentity();

}


template<class T, typename as = const char *>
void InputCallback_CB(Fl_Widget *w, long index){

    assert(w);
    assert(index>=0);
    assert(index<=7);

    UpdateServerIdentity();

    T *widget = static_cast<T *>(w);
    as value = widget->value();

    std::string to_set = std::string("server.") + selected_server_data->UID + "." + server_property_names[index];

    GetPreferences().set(to_set.c_str(), value);

    UpdateEnabled();

}


static EditList<>::ItemType ServerListAddCallback(EditList<>::ItemType item, void *p){

    DoubleInput_Return r = DoubleInput("Add a New Server", "Server Name", item.first, "Server Address", "", nullptr);

    if(r.value==0){
        free((void *)r.one);
        free((void *)r.two);
        return {nullptr, nullptr};
    }

    Fl_Preferences &prefs = GetPreferences();

    struct ServerData *data = server_db.GenerateServer();
    assert(data);

    ServerDB::LoadServer(data, prefs);
    server_db.push_back(data);

    data->name = r.one;
    data->address = r.two;

    free((void *)r.two);

    return {r.one, data};

}


static EditList<>::ItemType ServerListEdtCallback(EditList<>::ItemType item, void *p){

    struct ServerData *data = static_cast<struct ServerData *>(item.second);
    assert(data);

    DoubleInput_Return r = DoubleInput("Edit Server", "Server Name", item.first, "Server Address", data->address.c_str());

    if(r.value==0){

        free((void *)r.one);
        free((void *)r.two);
        return item;

    }

    data->name    = r.one;
    data->address = r.two;

    free((void *)r.two);

    return {r.one, data};

}


void ServerListDelCallback(EditList<>::ItemType item, void *p){

    struct equals_uid predicate(static_cast<struct ServerData *>(item.second)->UID);
    server_db.erase(std::find_if(server_db.begin(), server_db.end(), predicate));

}


void ServerListSelCallback(EditList<>::ItemType item, void *p){

    const struct ServerData *data = static_cast<struct ServerData*>(item.second);
    assert(data);
    SelectServer(data->UID);

}


class ConnectCallback_Task : public Task{

    class AddServerTask : public Task{

    WSocket *socket;
    ServerData *data;
    public:
        AddServerTask(WSocket *sock, ServerData *dat)
          : socket(sock)
          , data(dat){

        }

        void Run() override {
            repeating = false;
            Window *to = const_cast<Window *>(Window::window_order.front());
            to->AddServer(new Server(socket, data->address, to, data->port, data->UID, data->SSL));
        }

    };

    void *p;
public:

    ConnectCallback_Task(void *arg)
      : p(arg){}

    void Run() override {
        assert(p);

        repeating = false;
        ServerData *data = *static_cast<ServerData **>(p);
        assert(data);

        WSocket *sock;
        int err;

try_connect:
        sock = Create_Socket();

        if(!sock){
            printf("Could not create a socket.\n");
        }

        err = Connect_Socket(sock, data->address.c_str(), data->port, 10000);
        if(!err){
            Task *task = new AddServerTask(sock, data);
            Thread::AddTask(serverlist_associated_window->task_group, task);
        }
        else{
            printf("Couldn't connect. Asking if we should try again.\n");

            PromiseValue<int> promise(0);

            Task *task = new AskToConnectAgain_Task(promise, data->name);

            Thread::AddTask(serverlist_associated_window->task_group, task);

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


void ConnectCallback_CB(Fl_Widget *w, void *p){

    Thread::AddLongRunningTask(new ConnectCallback_Task(p));

}


Fl_Group *GenerateServerListFrame(int x, int y, int w, int h){

    Fl_Group *server_group = new Fl_Group(x, y+24, w, h-24, "Servers");

    EditList<> *servers_editlist = new EditList<>(x, y+24, w, h-52);
    servers_editlist->end();
    Fl_Preferences &prefs = GetPreferences();

    server_db.open(prefs);

    servers_editlist->SetAddCallback(ServerListAddCallback, nullptr);
    servers_editlist->SetEdtCallback(ServerListEdtCallback, nullptr);
    servers_editlist->SetDelCallback(ServerListDelCallback, nullptr);
    servers_editlist->SetSelCallback(ServerListSelCallback, nullptr);
    servers_editlist->Activate();

    for(ServerDB::iterator iter = server_db.begin(); iter!=server_db.end(); iter++){
        servers_editlist->AddItem({iter->get()->name.c_str(), iter->get()});
    }

    Fl_Button *connect_button = new Fl_Button(x, y+h-24, w, 24, "Connect");
    connect_button->callback(ConnectCallback_CB, &selected_server_data);
    server_group->add(connect_button);
    server_group->end();

    return server_group;

}

struct GroupFrame GenerateGroupFrame(int x, int y, int w, int h){
    struct GroupFrame frame;
    frame.group  = new Fl_Group(x, y+24, w, h-24, "Groups");

    frame.group_list = new EditList<Fl_Browser>(x+4, y+28, w-8, h-24-28-8);

    frame.edit_groups = new Fl_Button(x+4, y+28+h-24-28-8+4, (w/2)-12, 24, "Edit Groups");
    frame.edit_groups->deactivate();

    frame.group->end();
    return frame;
}


struct AutoJoinFrame GenerateAutoJoinFrame(int x, int y, int w, int h){
    struct AutoJoinFrame frame;
    frame.group  = new Fl_Group(x, y+24, w, h-24, "AutoJoin");

    frame.channel_list = new EditList<>(x+4, y+28, w-8, h-28);

    frame.group->end();
    return frame;
}


void ServerList(Fl_Widget *w, void *p){
    static bool first = true;

    assert(p);

    serverlist_associated_window = static_cast<Window *>(p);

    if(first){
        first = false;

        serverlist_window.reset(new Fl_Window(WindowWidth, WindowHeight, "Server List"));
        Fl_Tabs *server_property_tab = new Fl_Tabs(12+WindowWidth/2, 8, WindowWidth/2-24, WindowHeight-16);
        identity_frame = GenerateIdentityFrame(12+WindowWidth/2, 8, WindowWidth/2-24, WindowHeight-16, {IdentityFrameCallbacks::Long, {InputCallback_CB<Fl_Input>}, {InputCallback_CB<Fl_Button, bool>}, {1l}, {2l}, {3l}, {7l}});
        group_frame = GenerateGroupFrame(12+WindowWidth/2, 8, WindowWidth/2-24, WindowHeight-16);
        autojoin_frame = GenerateAutoJoinFrame(12+WindowWidth/2, 8, WindowWidth/2-24, WindowHeight-16);
        server_property_tab->end();
        GenerateServerListFrame(8, 8, WindowWidth/2-12, WindowHeight-16);

    }

    serverlist_window->show();

}


}
