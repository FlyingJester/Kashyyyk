#include "serverdatabase.hpp"
#include "serverlist.hpp"
#include "editlist.hpp"
#include "prefs.hpp"
#include "doubleinput.hpp"
#include "csv.h"

#include <vector>
#include <string>
#include <functional>

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tile.H>
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

struct IdentityFrame {
    Fl_Group *group;
    Fl_Button *global;
    Fl_Group *resize_group;
    Fl_Input *nick;
    Fl_Input *user;
    Fl_Input *real;
} identity_frame;

static std::unique_ptr<Fl_Window> serverlist_window = nullptr;
static struct Kashyyyk::ServerData *selected_server_data;
static Kashyyyk::ServerDB server_db;


static const int WindowWidth  = 400;
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


void UpdateIdentity(){
    identity_frame.global->value(selected_server_data->global);

    auto func = std::mem_fn(selected_server_data->global?&Fl_Widget::deactivate:&Fl_Widget::activate);

    func(identity_frame.nick);
    func(identity_frame.user);
    func(identity_frame.real);
}


void SelectServer(const char * UID){

    struct equals_uid predicate(UID);
    ServerDB::iterator iter = std::find_if(server_db.begin(), server_db.end(), predicate);

    selected_server_data = iter->get();

}


template<class T, typename as = const char *>
void InputCallback_CB(Fl_Widget *w, long index){

    assert(w);
    assert(index>=0);
    assert(index<=7);

    T *widget = static_cast<T *>(w);
    as value = widget->value();

    std::string to_set = std::string("server.") + selected_server_data->UID + "." + server_property_names[index];

    GetPreferences().set(to_set.c_str(), value);

}

/*
    struct CallBacks_T {
            ItemCallback AddCallback;
            void *AddCallbackArg;
            ItemCallback EdtCallback;
            void *EdtCallbackArg;
            NoReturnItemCallback DelCallback;
            void *DelCallbackArg;
            NoReturnItemCallback SelCallback;
            void *SelCallbackArg;
            NumCallbackT  NumCallback;
            void *NumCallbackArg;
*/

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


Fl_Group *GenerateServerListFrame(int x, int y, int w, int h){

    EditList<> *servers_editlist = new EditList<>(x, y+24, w, h-24, "Servers");
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

    return servers_editlist;

}


struct IdentityFrame GenerateIdentityFrame(int x, int y, int w, int h){

    struct IdentityFrame frame;
    frame.group  = new Fl_Pack(x, y+24, w, h-24, "Identity");
    frame.group->box(FL_DOWN_FRAME);

    frame.global = new Fl_Check_Button(0, 0, 32, 24, "Use Global Identity");
    frame.global->callback(InputCallback_CB<Fl_Button, int>, 7l);

    frame.resize_group = new Fl_Group(0, 0, 32, 24*3);
    frame.resize_group->resizable(nullptr);

    frame.nick = new Fl_Input(44,  0, w-48, 24, "Nick");
    frame.user = new Fl_Input(44, 24, w-48, 24, "User");
    frame.real = new Fl_Input(44, 48, w-48, 24, "Real");

    frame.nick->callback(InputCallback_CB<Fl_Input>, 1l);
    frame.user->callback(InputCallback_CB<Fl_Input>, 2l);
    frame.real->callback(InputCallback_CB<Fl_Input>, 3l);

    frame.resize_group->end();
    frame.group->end();

    return frame;
}


void ServerList(Fl_Widget *w, void *p){
    static bool first = true;

    if(first){
        first = false;

        serverlist_window.reset(new Fl_Window(WindowWidth, WindowHeight, "Server List"));

        identity_frame = GenerateIdentityFrame(12+WindowWidth/2, 8, WindowWidth/2-24, WindowHeight-16);
        GenerateServerListFrame(8, 8, WindowWidth/2-12, WindowHeight-16);

    }

    serverlist_window->show();

}


}
