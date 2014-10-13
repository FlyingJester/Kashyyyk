#include "serverlist.hpp"
#include "window.hpp"
#include "editlist.hpp"
#include "prefs.hpp"
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

void GlobalCheckboxCallback(Fl_Widget *w, void *p){
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

}

void ServerListNumCallback(int i, void *p) {
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

EditList<>::ItemType ServerListSelCallback(EditList<>::ItemType in, void *p) {

    struct ServerData *data = static_cast<struct ServerData *>(in.second);

    struct ServerPrefItems *pref_items =  static_cast<struct ServerPrefItems *>(p);

    char *nick = nullptr;
    char *name = nullptr;
    char *real = nullptr;

    Fl_Preferences &prefs = GetPreferences();

    int global = 1;

    prefs.get("server.identity.use_globals", global, global);

    if(global){
        prefs.get("sys.identity.nickname", nick, "KashyyykUser");
        prefs.get("sys.identity.fullname", name, "KashyyykName");
        prefs.get("sys.identity.realname", real, "KashyyykReal");
    }
    else{
        std::string server_ident("server.");
        server_ident += data->Name;
        server_ident += "identity.";

        prefs.get((server_ident+"nickname").c_str(), nick, "KashyyykUser");
        prefs.get((server_ident+"fullname").c_str(), name, "KashyyykName");
        prefs.get((server_ident+"realname").c_str(), real, "KashyyykReal");
    }

    pref_items->Nick->value(nick);
    pref_items->Name->value(name);
    pref_items->Real->value(real);

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

            prefs.get("server.identity.use_globals", global, global);

            if(global){
                prefs.get("sys.identity.nickname", nick, "KashyyykUser");
                prefs.get("sys.identity.fullname", name, "KashyyykName");
                prefs.get("sys.identity.realname", real,   "KashyyykReal");
            }
            else{
                prefs.get("server.identity.nickname", nick, "KashyyykUser");
                prefs.get("server.identity.fullname", name, "KashyyykName");
                prefs.get("server.identity.realname", real, "KashyyykReal");
            }

            auto NickWidget = CreatePackWidget("Nick", nick);
            propertypack->add(NickWidget.first);
            pref_items->Nick = NickWidget.second;

            auto NameWidget = CreatePackWidget("Name", name);
            propertypack->add(NameWidget.first);
            pref_items->Name = NameWidget.second;

            auto RealWidget = CreatePackWidget("Real", real);
            propertypack->add(RealWidget.first);
            pref_items->Real = RealWidget.second;

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

        servers->SetNumCallback(ServerListNumCallback, pref_items);

        if(servers->GetNumItems()==0){
            editlist->Deactivate();
            pref_items->Global->deactivate();
        }
        propertypack->add(editlist);





    }

    serverlist_window->show();

}

}
