#include "serverlist.hpp"
#include "window.hpp"
#include "editlist.hpp"
#include "prefs.hpp"
#include "doubleinput.hpp"
#include "serverdatabase.hpp"
#include "csv.h"
#include <cassert>
#include <cstdio>
#include <string>

#include <memory>

#include <FL/Fl_Window.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tabs.H>

namespace Kashyyyk {

struct ServerPrefItems {
    int dummy;
    Fl_Input_ *Nick;
    Fl_Input_ *Name;
    Fl_Input_ *Real;
    Fl_Button *Global;
    Fl_Button *SSL;
    ServerData *Data;
    EditList<> *AutoJoin;
    EditList<> *Servers;
    ServerDB *DataBase;
};

// The window is associated with the last window passed as `p' into ServerList.
// Any attempt to join a server will add it to that window.
static std::unique_ptr<Fl_Window> serverlist_window;
static Window *serverlist_associated_window;
//static ServerDB *server_db = nullptr;

template <class T = Fl_Input, int w = 80, int h = 24, typename ValueT>
inline std::pair<Fl_Pack *, T *> CreatePackWidget(const char *title, ValueT value, Fl_Callback *cb = nullptr, void *a = nullptr){
    Fl_Pack *pack = new Fl_Pack(0, 0, 256-(4*2), 24);
    pack->type(Fl_Pack::HORIZONTAL);

    Fl_Box *box = new Fl_Box(0, 0, w, h, title);
    box->box(FL_NO_BOX);
    T *i = new T(0, 0, pack->w()-w, h);

    i->value();

    if(cb)
      i->callback(cb, a);

    return {pack, i};
}


static void ServerDataChangedCallback(const struct ServerData* that, void *arg){

    struct ServerPrefItems *pref_items = static_cast<ServerPrefItems *>(arg);
    const struct ServerData* active = static_cast<const ServerData*>(pref_items->Servers->GetItem().second);


    if((active) && strcmp(that->UID, active->UID)!=0)
      return;

    pref_items->Nick->value(that->Nick);
    pref_items->Name->value(that->User);
    pref_items->Real->value(that->Real);
    pref_items->Global->value(that->UserGlobalIdentity);

    pref_items->Servers->SetText(that->Name);
    pref_items->AutoJoin->Clear();
    for(std::vector<std::string>::const_iterator iter = that->AutoJoins.cbegin(); iter!=that->AutoJoins.cend(); iter++){
        pref_items->AutoJoin->AddItem({iter->c_str(), nullptr});
    }

}


static void WindowCallback(Fl_Widget *w, void *p){
    Fl_Group *that = static_cast<Fl_Group *>(w);
    that->hide();

    static_cast<ServerDB *>(p)->save(GetPreferences());

}

template<size_t s>
void InputCallback(Fl_Widget *w, void *p){

    struct ServerPrefItems *pref_items = static_cast<struct ServerPrefItems *>(p);
    const Fl_Input *input = static_cast<Fl_Input *>(w);

    const char ** value= (const char **)(pref_items->Data)+s;

    GetPreferences().set(*value, input->value());

}


static EditList<>::ItemType AutoJoinAddCallback(EditList<>::ItemType item, void *p){

    free((void*)item.first);

    item.first = fl_input("Add channel", "");
    if(item.first==nullptr){
        return {nullptr, nullptr};
    }

    item.first = strdup(item.first);

    struct ServerPrefItems *pref_items = static_cast<struct ServerPrefItems *>(p);

    pref_items->Data->AutoJoins.push_back(item.first);

    return item;
}


static void AutoJoinDelCallback(EditList<>::ItemType item, void *p){

    struct ServerPrefItems *pref_items = static_cast<struct ServerPrefItems *>(p);

    for(std::vector<std::string>::iterator iter = pref_items->Data->AutoJoins.begin();
      iter != pref_items->Data->AutoJoins.end(); iter++){
        if(strcmp(iter->c_str(), item.first)==0){
            pref_items->Data->AutoJoins.erase(iter);
            break;
        }
    }

    pref_items->Data->owner->MarkDirty(pref_items->Data);
}


static EditList<>::ItemType AutoJoinEdtCallback(EditList<>::ItemType item, void *p){

    struct ServerPrefItems *pref_items = static_cast<struct ServerPrefItems *>(p);

    for(std::vector<std::string>::iterator iter = pref_items->Data->AutoJoins.begin();
      iter != pref_items->Data->AutoJoins.end(); iter++){
        printf("AutoJoin %s\n", iter->c_str());
    }


    const char *copy = strdup(item.first);
    item.first = fl_input("Edit Channel %s", copy, copy);

    if(!item.first)
      return {nullptr, nullptr};


    for(std::vector<std::string>::iterator iter = pref_items->Data->AutoJoins.begin();
      iter != pref_items->Data->AutoJoins.end(); iter++){
        if(strcmp(iter->c_str(), copy)==0){
            iter->assign(item.first);
            break;
        }
    }

    free((void*)copy);

    pref_items->Data->owner->MarkDirty(pref_items->Data);

    return item;
}


static void SSLCheckboxCallback(Fl_Widget *w, void *p){
    Fl_Button *b = static_cast<Fl_Button *>(w);
    struct ServerData *d = static_cast<ServerData *>(p);
    d->SSL = b->value();
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

}


static EditList<>::ItemType ServerListEdtCallback(EditList<>::ItemType item, void *p){

    struct ServerData *data = static_cast<ServerData *>(item.second);

    DoubleInput_Return r = DoubleInput("Add a New Server", "Server Name", item.first, "Server Address", "", nullptr);

    if(r.value==0){
        free((void *)r.one);
        free((void *)r.two);
        return item;
    }

    free(data->Name);
    free(data->Address);

    data->Name = strdup(r.one);
    data->Address = strdup(r.two);

    item.first  = strdup(data->Name);
    item.second = data;

    free((void *)r.one);
    free((void *)r.two);

    data->owner->MarkDirty(data);

    return item;

}


static EditList<>::ItemType ServerListAddCallback(EditList<>::ItemType item, void *p){

    struct ServerPrefItems *pref_items = static_cast<struct ServerPrefItems *>(p);

    DoubleInput_Return r = DoubleInput("Add a New Server", "Server Name", item.first, "Server Address", "", nullptr);

    if(r.value==0){
        free((void *)r.one);
        free((void *)r.two);
        return {nullptr, nullptr};
    }

    struct ServerData *data = pref_items->DataBase->GenerateServer();
    ServerDB::LoadServer(data, GetPreferences());

    data->Name = strdup(r.one);
    data->Address = strdup(r.two);

    printf("%s|%s(%p)\n", data->Name, data->Address, data->Name);

    item.first  = strdup(data->Name);
    item.second = data;

    pref_items->DataBase->push_back(data);
    pref_items->DataBase->SaveServer(data, GetPreferences());

    pref_items->Nick->value(data->Nick);
    pref_items->Name->value(data->User);
    pref_items->Real->value(data->Real);

    pref_items->Data = data;

    pref_items->Global->value(data->UserGlobalIdentity);

    pref_items->AutoJoin->Clear();

    for(std::vector<std::string>::iterator string_iter=data->AutoJoins.begin();
    string_iter!=data->AutoJoins.end(); string_iter++){
        pref_items->AutoJoin->AddItem({string_iter->c_str(), nullptr});
    }

    free((void *)r.one);
    free((void *)r.two);

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

static void ServerListSelCallback(EditList<>::ItemType in, void *p) {

    struct ServerData *data = static_cast<struct ServerData *>(in.second);

    struct ServerPrefItems *pref_items =  static_cast<struct ServerPrefItems *>(p);

    pref_items->Nick->value(data->Nick);
    pref_items->Name->value(data->User);
    pref_items->Real->value(data->Real);
    pref_items->Global->value(data->UserGlobalIdentity);

    pref_items->Global->do_callback();

    pref_items->AutoJoin->Clear();

    for(std::vector<std::string>::iterator string_iter=data->AutoJoins.begin();
    string_iter!=data->AutoJoins.end(); string_iter++){
        pref_items->AutoJoin->AddItem({string_iter->c_str(), nullptr});
    }

}

void ServerList(Fl_Widget *w, void *p){

    assert(p);

    serverlist_associated_window = static_cast<Window *>(p);

    static bool first = true;
    if(first){
        first = false;


        Fl_Preferences &prefs = GetPreferences();

        struct ServerPrefItems *pref_items = new ServerPrefItems();

        pref_items->DataBase = new ServerDB();
        pref_items->DataBase->open(prefs);
        pref_items->DataBase->CallBacks.push_back({ServerDataChangedCallback, pref_items});

        const unsigned H = (24*6)+(16*8)+(8*9)+(2*1);

        Fl_Window *serverlist = new Fl_Window((256*2)+(8*3), H);
        serverlist_window.reset(serverlist);

        EditList<> *servers = new EditList<>(8, 24, 256, H-32, "Servers");

        pref_items->Servers = servers;

        for(ServerDB::iterator iter = pref_items->DataBase->begin(); iter!=pref_items->DataBase->end(); iter++){
            servers->AddItem({iter->get()->Name, iter->get()});
        }

        serverlist->begin();

        Fl_Tabs *tabs = new Fl_Tabs((8*2)+256, 8, 256, H-16);

        Fl_Group *g = new Fl_Group((8*2)+256, 24, 256, H-32, "Server Settings");

        tabs->add(g);
        Fl_Pack *propertypack = new Fl_Pack((8*2)+4+256, 48, 256-(4*2), H-32);
        propertypack->spacing(4);
        propertypack->add(new Fl_Box(0, 0, 0, 0));


        {

            struct ServerData *first_server_deleter = nullptr;
            ServerDB::iterator server_iter = pref_items->DataBase->begin();
            if(server_iter==pref_items->DataBase->end()){
                first_server_deleter = new struct ServerData();
                pref_items->Data = first_server_deleter;
                pref_items->Data->Name = nullptr;
                pref_items->Data->Nick = strdup("KashyyykUser");
                pref_items->Data->User = strdup("KashyyykName");
                pref_items->Data->Real = strdup("KashyyykReal");
                pref_items->Data->UserGlobalIdentity = 1;
            }
            else{
                pref_items->Data = server_iter->get();
            }

            auto NickWidget = CreatePackWidget("Nick", pref_items->Data->Nick);
            propertypack->add(NickWidget.first);
            pref_items->Nick = NickWidget.second;
            NickWidget.first->callback(InputCallback<offsetof(ServerData, Nick)>, pref_items);

            auto NameWidget = CreatePackWidget("Name", pref_items->Data->User);
            propertypack->add(NameWidget.first);
            pref_items->Name = NameWidget.second;
            NameWidget.first->callback(InputCallback<offsetof(ServerData, User)>, pref_items);

            auto RealWidget = CreatePackWidget("Real", pref_items->Data);
            propertypack->add(RealWidget.first);
            pref_items->Real = RealWidget.second;
            RealWidget.first->callback(InputCallback<offsetof(ServerData, Real)>, pref_items);

            auto GlobalWidget = CreatePackWidget<Fl_Check_Button, 224>
              ("Use Global Settings", pref_items->Data->UserGlobalIdentity,
              GlobalCheckboxCallback, pref_items);

            propertypack->add(GlobalWidget.first);
            GlobalWidget.second->do_callback();
            pref_items->Global = GlobalWidget.second;


            auto SSLWidget   = CreatePackWidget<Fl_Check_Button, 224>
              ("Use SSL", pref_items->Data->SSL);
            propertypack->add(SSLWidget.first);
            pref_items->SSL = SSLWidget.second;
            SSLWidget.first->callback(SSLCheckboxCallback, pref_items->Data);


            delete first_server_deleter;

        }

        propertypack->add(new Fl_Box(0, 0, 0, 8));

        EditList<> *autojoin = new EditList<>(8, 24, 256, 130, "AutoJoin Channels");
        pref_items->AutoJoin = autojoin;

        printf("AutoJoin: %p\n", static_cast<void *>(autojoin));

        servers->SetNumCallback(ServerListNumCallback, pref_items);
        servers->SetAddCallback(ServerListAddCallback, pref_items);
        servers->SetSelCallback(ServerListSelCallback, pref_items);
        servers->SetEdtCallback(ServerListEdtCallback, pref_items);
        autojoin->SetDelCallback(AutoJoinDelCallback, pref_items);
        autojoin->SetEdtCallback(AutoJoinEdtCallback, pref_items);
        autojoin->SetAddCallback(AutoJoinAddCallback, pref_items);

        if(servers->GetNumItems()==0){
            autojoin->Deactivate();
            pref_items->Global->deactivate();
        }
        propertypack->add(autojoin);

        tabs->add(new Fl_Group((8*2)+256, 24, 256, H-32, "Group Info"));

        serverlist_window->callback(WindowCallback, pref_items->DataBase);

    }

    serverlist_window->show();

}

}
