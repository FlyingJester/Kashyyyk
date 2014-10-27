#include "serverdatabase.hpp"
#include "serverlist.hpp"
#include "editlist.hpp"
#include "prefs.hpp"
#include "doubleinput.hpp"
#include "csv.h"

#include <vector>
#include <string>

#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Preferences.H>

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
static struct Kashyyyk::ServerData selected_server_data;


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


#define GET_SERVER_PROPERTY(INDEX, INTO, MEMBER, DEFAULT)\
        GetPreferences().get((std::string("server.") + selected_server_data.UID + "." + server_property_names[INDEX]).c_str(), INTO, DEFAULT);\
        selected_server_data.MEMBER = INTO;\

#define GET_SERVER_PROPERTY_CHAR(INDEX, MEMBER, DEFAULT)\
do{ char *c;\
        GET_SERVER_PROPERTY(INDEX, c, MEMBER, DEFAULT);\
free(c);}while(0)

#define GET_SERVER_PROPERTY_INT(INDEX, MEMBER, DEFAULT)\
do{ int i;\
        GET_SERVER_PROPERTY(INDEX, i, MEMBER, DEFAULT);\
}while(0)


namespace Kashyyyk {


void SelectServer(const char *UID){

    const std::string prefix = std::string("server.") + UID + ".";

    selected_server_data.UID = UID;

    GET_SERVER_PROPERTY_CHAR(0, name, "Unnamed Server");
    GET_SERVER_PROPERTY_CHAR(1, nick, "KashyyykUser");
    GET_SERVER_PROPERTY_CHAR(2, user, "KashyyykName");
    GET_SERVER_PROPERTY_CHAR(3, real, "KashyyykReal");
    GET_SERVER_PROPERTY_CHAR(4, address, "irc.server.net");
    GET_SERVER_PROPERTY_INT( 5, port, 6665);
    GET_SERVER_PROPERTY_INT( 6, SSL, 0);
    GET_SERVER_PROPERTY_INT( 7, global, 1);

    {
        char *autojoin;
        char *group_uids;
        GetPreferences().get((prefix+server_property_names[8]).c_str(), autojoin,  "");
        GetPreferences().get((prefix+server_property_names[9]).c_str(), group_uids,"");

        const char **channels = FJ::CSV::ParseString(autojoin);
        const char **groups = FJ::CSV::ParseString(group_uids);

        for(int i = 0; channels[i]!=nullptr; i++)
          selected_server_data.autojoin_channels.push_back(channels[i]);

        for(int i = 0; groups[i]!=nullptr; i++)
          selected_server_data.group_UIDs.push_back(groups[i]);

        free(autojoin);
        free(group_uids);
        FJ::CSV::FreeParse(channels);
        FJ::CSV::FreeParse(groups);
    }

}


template<class T, typename as = const char *>
void InputCallback_CB(Fl_Widget *w, long index){

    assert(w);
    assert(index>=0);
    assert(index<=7);

    T *widget = static_cast<T *>(w);
    as value = widget->value();

    std::string to_set = std::string("server.") + selected_server_data.UID + "." + server_property_names[index];

    GetPreferences().set(to_set.c_str(), value);

}


static EditList<>::ItemType ServerListAddCallback(EditList<>::ItemType item, void *p){

    DoubleInput_Return r = DoubleInput("Add a New Server", "Server Name", item.first, "Server Address", "", nullptr);

    if(r.value==0){
        free((void *)r.one);
        free((void *)r.two);
        return {nullptr, nullptr};
    }

    //struct ServerData *data = pref_items->DataBase->GenerateServer();
    //ServerDB::LoadServer(data, GetPreferences());

        return {nullptr, nullptr};

}


Fl_Group *GenerateServerListFrame(int x, int y, int w, int h){

    EditList<> *servers_editlist = new EditList<>(x, y, w, h, "Servers");

    char *serverlist = nullptr;
    GetPreferences().get("sys.server_uids", serverlist, "");

    const char **servers = FJ::CSV::ParseString(serverlist);

    for(int i = 0; servers[i]!=nullptr; i++){
        char *name = nullptr;

        GetPreferences().get((std::string("server.")+servers[i]+".name").c_str(), name, "Unnamed Server");

        servers_editlist->AddItem({name, const_cast<char *>(servers[i])});
    }

    free(servers);

    servers_editlist->SetAddCallback(ServerListAddCallback, nullptr);

    return servers_editlist;

}


Fl_Group *GenerateIdentityFrame(int x, int y, int w, int h){
    Fl_Group *group = new Fl_Pack(x, y, w, h, "Identity");
    group->box(FL_DOWN_FRAME);

    Fl_Check_Button *global_button = new Fl_Check_Button(0, 0, 32, 24, "Use Global Identity");
    global_button->callback(InputCallback_CB<Fl_Button, int>, 7l);
    global_button->deactivate();

    {
        Fl_Group *input_group = new Fl_Group(0, 0, 32, 24*3);
        input_group->resizable(nullptr);
        Fl_Input *nick_input = new Fl_Input(48,  0, w-48, 24, "Nick");
        Fl_Input *user_input = new Fl_Input(48, 24, w-48, 24, "User");
        Fl_Input *real_input = new Fl_Input(48, 48, w-48, 24, "Real");
        input_group->end();

        nick_input->callback(InputCallback_CB<Fl_Input>, 1l);
        user_input->callback(InputCallback_CB<Fl_Input>, 2l);
        real_input->callback(InputCallback_CB<Fl_Input>, 3l);

        nick_input->deactivate();
        user_input->deactivate();
        real_input->deactivate();

    }

    group->end();
    return group;
}


void ServerList(Fl_Widget *w, void *p){
    static bool first = true;

    if(first){
        first = false;


        serverlist_window.reset(new Fl_Window(WindowWidth, WindowHeight, "Server List"));

        GenerateServerListFrame(0, 0, WindowWidth/2, WindowHeight);
        GenerateIdentityFrame(WindowWidth/2, 0, WindowWidth/2, WindowHeight);

    }

    serverlist_window->show();

}


}
