#include "serverlist.hpp"
#include "window.hpp"
#include "editlist.hpp"
#include "prefs.hpp"
#include "doubleinput.hpp"
#include <cassert>
#include <cstdio>
#include <string>

#include <memory>

#include <FL/Fl_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Preferences.H>

namespace Kashyyyk {

struct ServerData {
    const char *Name;
};

struct ServerPrefItems {
    Fl_Input_ *Nick;
    Fl_Input_ *Name;
    Fl_Input_ *Real;
    Fl_Button *Global;
    EditList<> *AutoJoin;
    std::string server;
    std::string server_nickname;
    std::string server_fullname;
    std::string server_realname;
};

// The window is associated with the last window passed as `p' into ServerList.
// ANy attempt to join a server will add it to that window.

static std::unique_ptr<Fl_Window> serverlist_window;
static Window *serverlist_associated_window;

template <class T = Fl_Input, int w = 80, int h = 24, typename ValueT>
inline std::pair<Fl_Pack *, T *> CreatePackWidget(const char *title, ValueT value, Fl_Callback *cb = nullptr, void *a = nullptr){
    Fl_Pack *pack = new Fl_Pack(0, 0, 256-(4*2), 24);
    pack->type(Fl_Pack::HORIZONTAL);

    Fl_Box *box = new Fl_Box(0, 0, w, h, title);
    box->box(FL_NO_BOX);
    T *i = new T(0, 0, pack->w()-w, h);

    i->value(value);

    if(cb)
      i->callback(cb, a);

    return {pack, i};
}

static void InputCallback(Fl_Widget *w, void *p){

    Fl_Preferences &prefs = GetPreferences();

    const char *pref_name = static_cast<std::string *>(p)->c_str();
    const Fl_Input *input = static_cast<Fl_Input *>(w);

    prefs.set(pref_name, input->value());

}

static void GlobalCheckboxCallback(Fl_Widget *w, void *p){
    struct ServerPrefItems *pref_items = static_cast<struct ServerPrefItems *>(p);
    Fl_Button *button = static_cast<Fl_Check_Button *>(w);

    if(button->value()){
        pref_items->Nick->deactivate();
        pref_items->Name->deactivate();
        pref_items->Real->deactivate();
    }
    else{
        pref_items->Nick->activate();
        pref_items->Name->activate();
        pref_items->Real->activate();
    }

    std::string server_ident("server.");
    server_ident += pref_items->server;
    server_ident += ".identity.";

    Fl_Preferences &prefs = GetPreferences();

    prefs.set((server_ident+"use_globals").c_str(), button->value());

}

static EditList<>::ItemType ServerListAddCallback(EditList<>::ItemType item, void *){

    DoubleInput_Return r = DoubleInput("Add a New Server", "Server Name", item.first, "Server Address", "", nullptr);

    if(r.value==0){
        free((void *)r.one);
        free((void *)r.two);
        return {nullptr, nullptr};
    }

    ServerData *data = new ServerData();
    data->Name = r.two;

    free((void *)item.first);

    item.first  = r.one;
    item.second = data;

    return item;
}

static void ServerListNumCallback(int i, void *p) {
    ServerPrefItems *pref_items = static_cast<ServerPrefItems *>(p);

      if(i==0){
          pref_items->AutoJoin->Deactivate();
          pref_items->Global->set();
          pref_items->Global->do_callback();
          pref_items->Global->deactivate();
      }
      else{
          pref_items->AutoJoin->Activate();
          pref_items->Global->activate();
      }
}

static EditList<>::ItemType ServerListSelCallback(EditList<>::ItemType in, void *p) {

    struct ServerData *data = static_cast<struct ServerData *>(in.second);

    struct ServerPrefItems *pref_items =  static_cast<struct ServerPrefItems *>(p);

    char *nick = nullptr;
    char *name = nullptr;
    char *real = nullptr;

    Fl_Preferences &prefs = GetPreferences();

    int global = 1;

    std::string server_ident("server.");
    server_ident += data->Name;
    server_ident += ".identity.";

    pref_items->server = data->Name;
    pref_items->server_nickname = server_ident+"nickname";
    pref_items->server_fullname = server_ident+"fullname";
    pref_items->server_realname = server_ident+"realname";

    prefs.get((server_ident+"use_globals").c_str(), global, global);

    if(global){
        prefs.get("sys.identity.nickname", nick, "KashyyykUser");
        prefs.get("sys.identity.fullname", name, "KashyyykName");
        prefs.get("sys.identity.realname", real, "KashyyykReal");
    }
    else{
        prefs.get(pref_items->server_nickname.c_str(), nick, "KashyyykUser");
        prefs.get(pref_items->server_fullname.c_str(), name, "KashyyykName");
        prefs.get(pref_items->server_realname.c_str(), real, "KashyyykReal");
    }

    pref_items->Nick->value(nick);
    pref_items->Name->value(name);
    pref_items->Real->value(real);

    pref_items->Nick->callback(InputCallback, &(pref_items->server_nickname));
    pref_items->Name->callback(InputCallback, &(pref_items->server_fullname));
    pref_items->Real->callback(InputCallback, &(pref_items->server_realname));

    free(nick);
    free(name);
    free(real);

    pref_items->AutoJoin->Clear();

    return {nullptr, nullptr};

}

void ServerList(Fl_Widget *w, void *p){

    assert(p);

    serverlist_associated_window = static_cast<Window *>(p);

    static bool first = true;
    if(first){
        first = false;

        const unsigned H = (24*6)+(16*7)+(8*8);

        Fl_Window *serverlist = new Fl_Window((256*2)+(8*3), H);
        serverlist_window.reset(serverlist);

        EditList<> *servers = new EditList<>(8, 24, 256, H-32, "Servers");

        serverlist->begin();

        Fl_Group *g = new Fl_Group((8*2)+256, 24, 256, H-32, "Server Settings");
        g->box(FL_DOWN_FRAME);

        serverlist->add(g);
        Fl_Pack *propertypack = new Fl_Pack((8*2)+4+256, 24, 256-(4*2), H-32);
        propertypack->spacing(4);
        propertypack->add(new Fl_Box(0, 0, 0, 0));

        struct ServerPrefItems *pref_items = new ServerPrefItems();

        {

            Fl_Preferences &prefs = GetPreferences();

            int global = 1;

            char *nick = nullptr;
            char *name = nullptr;
            char *real = nullptr;


            std::string server_ident("sys.identity.");
            pref_items->server_nickname = server_ident+"nickname";
            pref_items->server_fullname = server_ident+"fullname";
            pref_items->server_realname = server_ident+"realname";

            prefs.get((server_ident+"use_globals").c_str(), global, global);

            prefs.get("server.identity.use_globals", global, global);
            prefs.set("server.identity.use_globals", global);

            if(global){
                prefs.get("sys.identity.nickname", nick, "KashyyykUser");
                prefs.get("sys.identity.fullname", name, "KashyyykName");
                prefs.get("sys.identity.realname", real,   "KashyyykReal");
            }
            else{
                prefs.get(pref_items->server_nickname.c_str(), nick, "KashyyykUser");
                prefs.get(pref_items->server_fullname.c_str(), name, "KashyyykName");
                prefs.get(pref_items->server_realname.c_str(), real, "KashyyykReal");
            }

            auto NickWidget = CreatePackWidget("Nick", nick);
            propertypack->add(NickWidget.first);
            pref_items->Nick = NickWidget.second;
            NickWidget.first->callback(InputCallback, &(pref_items->server_nickname));

            auto NameWidget = CreatePackWidget("Name", name);
            propertypack->add(NameWidget.first);
            pref_items->Name = NameWidget.second;
            NameWidget.first->callback(InputCallback, &(pref_items->server_fullname));

            auto RealWidget = CreatePackWidget("Real", real);
            propertypack->add(RealWidget.first);
            pref_items->Real = RealWidget.second;
            RealWidget.first->callback(InputCallback, &(pref_items->server_realname));

            free(nick);
            free(name);
            free(real);

            auto GlobalWidget = CreatePackWidget<Fl_Check_Button, 224>("Use Global Settings", global, GlobalCheckboxCallback, pref_items);
            propertypack->add(GlobalWidget.first);
            GlobalWidget.second->do_callback();
            pref_items->Global = GlobalWidget.second;

        }

        propertypack->add(new Fl_Box(0, 0, 0, 8));

        EditList<> *editlist = new EditList<>(8, 24, 256, 156, "AutoJoin Channels");
        pref_items->AutoJoin = editlist;

        printf("AutoJoin: %p\n", editlist);

        servers->SetNumCallback(ServerListNumCallback, pref_items);
        servers->SetAddCallback(ServerListAddCallback, nullptr);
        servers->SetSelCallback(ServerListSelCallback, pref_items);

        if(servers->GetNumItems()==0){
            editlist->Deactivate();
            pref_items->Global->deactivate();
        }
        propertypack->add(editlist);





    }

    serverlist_window->show();

}

}
