#include "serverdatabase.hpp"

#include "autolocker.hpp"
#include "csv.h"

#include <cassert>
#include <cstdlib>
#include <climits>
#include <ctime>
#include <mutex>
#include <string>
#include <sstream>

#include "platform/strdup.h"

namespace Kashyyyk {


equals_uid::equals_uid(const char * &target)
  : uid(target){

}

//! @typedef rand_holder
//! A type that is as close as possible to the maximum entropy value.
#if USHRT_MAX >= RAND_MAX
typedef unsigned short rand_holder;
#elif UINT_MAX >= RAND_MAX
typedef unsigned int rand_holder;
#elif ULONG_MAX >= RAND_MAX
typedef unsigned long rand_holder;
#elif ULLONG_MAX >= RAND_MAX
typedef unsigned long long rand_holder;
#endif


//! This is the number you adjust to change the end size of UIDs.
static const unsigned uid_coefficient = 16;
//! This is the number of calls for entropy that will be made to fill a UID.
static const size_t uid_len = uid_coefficient/sizeof(rand_holder);

struct ServerDB::ServerDB_Impl{
    std::mutex mutex;
    ServerData global;
    std::vector<ServerDataP> list;
    std::vector<std::string>groups;
};

const char *ServerDB::GenerateUID(){

    assert(uid_coefficient >= sizeof(rand_holder));

    std::stringstream str_value;

    srand(time(nullptr));

    for(unsigned i = 0; i < uid_len; i++){
        rand_holder r = rand();
        str_value << r;
    }

    return strdup(str_value.str().c_str());

}

ServerDB::ServerDB()
  : guts(new ServerDB_Impl()){

}

ServerDB::~ServerDB(){

}


    struct ServerData *GetGlobal();
    void SetGlobal(struct ServerData *);


void ServerDB::LoadServer(struct ServerData *server, Fl_Preferences &prefs){

    assert(server);

    const std::string ServerPrefix = std::string("server.")+server->UID + ".";

    GetAndExist(prefs, ServerPrefix+"name",     server->name,   "New Server");
    GetAndExist(prefs, ServerPrefix+"nickname", server->nick,   "KashyyykUser");
    GetAndExist(prefs, ServerPrefix+"username", server->user,   "KashyyykName");
    GetAndExist(prefs, ServerPrefix+"realname", server->real,   "KashyyykReal");
    GetAndExist(prefs, ServerPrefix+"address",  server->address,"irc.website.net");
    GetAndExist(prefs, ServerPrefix+"port",     server->port,   6665);
    GetAndExist(prefs, ServerPrefix+"ssl",      server->SSL,    false);
    GetAndExist(prefs, ServerPrefix+"globalidentity", server->global, true);

    char *autojoin, *groupuids;
    GetAndExist(prefs, ServerPrefix+"autojoin", autojoin, "");
    GetAndExist(prefs, ServerPrefix+"groupuids", groupuids, "");

    const char **channels = FJ::CSV::ParseString(autojoin);
    const char **groups   = FJ::CSV::ParseString(groupuids);

    for(int i = 0; channels[i]!=nullptr; i++){
        server->autojoin_channels.push_back(std::string(channels[i]));
    }

    for(int i = 0; groups[i]!=nullptr; i++){
        server->group_UIDs.push_back(std::string(groups[i]));
    }

    FJ::CSV::FreeParse(channels);
    FJ::CSV::FreeParse(groups);

    free(autojoin);
    free(groupuids);
}


struct ServerData *ServerDB::GenerateServer() const{
    struct ServerData *ret = new struct ServerData();
    ret->owner = this;
    ret->UID = GenerateUID();

     // Clean data
    ret->name = '\0';
    ret->address = '\0';
    ret->port = 0;
    ret->SSL = false;
    ret->global = true;
    ret->nick = '\0';
    ret->user = '\0';
    ret->real = '\0';
    ret->autojoin_channels.clear();

    return ret;
}


void ServerDB::MarkDirty(const struct ServerData *server) const{

    for(CallBackVector::const_iterator iter = CallBacks.cbegin(); iter!=CallBacks.cend(); iter++){
        iter->first(server, iter->second);
    }

}


void ServerDB::open(Fl_Preferences &prefs){
    AutoLocker<const ServerDB * const> locker(this);

    clear();

    char *uid_list;
    if(!prefs.get("sys.server_uids", uid_list, "")){
        prefs.set("sys.server_uids", "");
        free(uid_list);
        return;
    }

    const char **server_uids = FJ::CSV::ParseString(uid_list);
    free(uid_list);

    char *groups;
    GetAndExist(prefs, "sys.group_uids", groups, "");
    const char **group_uids = FJ::CSV::ParseString(groups);
    free(groups);

    for(int i = 0; group_uids[i]!=nullptr; i++){
        guts->groups.push_back(group_uids[i]);
    }

    FJ::CSV::FreeParse(group_uids);

    for(int i = 0; server_uids[i]!=nullptr; i++){
        struct ServerData *server = new ServerData();
        server->UID = server_uids[i];
        server->owner = this;
        LoadServer(server, prefs);

        for(std::vector<std::string>::const_iterator iter = server->group_UIDs.cbegin();
              iter != server->group_UIDs.cend(); iter++){
start:
            if(std::find(guts->groups.cbegin(), guts->groups.cend(), *iter)==guts->groups.cend()){
                guts->groups.push_back(*iter);
                goto start;
            }
        }

        push_back(ServerDataP(server));
    }

    // The individual items are now owned by the ServerData.
    free(server_uids);

    GetAndExist(prefs, "sys.global.nickname", guts->global.nick, "KashyyykUser");
    GetAndExist(prefs, "sys.global.username", guts->global.user, "KashyyykUser");
    GetAndExist(prefs, "sys.global.realname", guts->global.real, "KashyyykUser");

}


void ServerDB::save(Fl_Preferences &prefs) const {
    AutoLocker<const ServerDB * const> locker(this);

    std::string UIDs, group_UIDs;

    for(iterator iter = begin(); iter!= end(); iter++){
        UIDs+= iter->get()->UID;
        UIDs.push_back(',');
    }
    UIDs.pop_back();

    for(std::vector<std::string>::const_iterator iter = guts->groups.cbegin();
      iter!= guts->groups.cend(); iter++){
        group_UIDs+= *iter;
        group_UIDs.push_back(',');
    }
    group_UIDs.pop_back();

    prefs.set("sys.server_uids",   UIDs.c_str());
    prefs.set("sys.sys.group_uids",group_UIDs.c_str());

    std::for_each(begin(), end(), [&prefs](const ServerDataP& server){SaveServer(server.get(), prefs);});

    prefs.set("sys.global.nickname", guts->global.nick.c_str());
    prefs.set("sys.global.username", guts->global.user.c_str());
    prefs.set("sys.global.realname", guts->global.real.c_str());

}


void ServerDB::SaveServer(struct ServerData *server, Fl_Preferences &prefs){
        const std::string ServerPrefix = std::string("server.")+server->UID + ".";

        prefs.set((ServerPrefix+"name").c_str(),     server->name.c_str());
        prefs.set((ServerPrefix+"nickname").c_str(), server->nick.c_str());
        prefs.set((ServerPrefix+"username").c_str(), server->user.c_str());
        prefs.set((ServerPrefix+"realname").c_str(), server->real.c_str());
        prefs.set((ServerPrefix+"address").c_str(),  server->address.c_str());
        prefs.set((ServerPrefix+"port").c_str(),     server->port);
        prefs.set((ServerPrefix+"ssl").c_str(),      server->SSL);
        prefs.set((ServerPrefix+"globalidentity").c_str(), server->global);

        if(server->autojoin_channels.empty())
          return;

        std::string autojoin;
        for(std::vector<std::string>::iterator iter = server->autojoin_channels.begin(); iter!=server->autojoin_channels.end(); iter++){
            autojoin+=*iter;
            autojoin+=",";
        }

        autojoin.pop_back();

        prefs.set((ServerPrefix+"autojoin").c_str(), autojoin.c_str());

}


void ServerDB::lock() const{
    guts->mutex.lock();
}


void ServerDB::unlock() const{
    guts->mutex.unlock();
}

ServerDB::iterator ServerDB::begin(void) const{
    return guts->list.begin();
}


ServerDB::iterator ServerDB::end(void) const{
    return guts->list.end();
}


ServerDB::reverse_iterator ServerDB::rbegin(void) const{
    return guts->list.rbegin();
}


ServerDB::reverse_iterator ServerDB::rend(void) const{
    return guts->list.rend();
}


ServerDB::iterator ServerDB::erase(iterator iter){
    return guts->list.erase(iter);
}


void ServerDB::push_back(ServerDataP a){
    a->owner = this;
    guts->list.push_back(std::move(a));
}


void ServerDB::push_back(struct ServerData *a){
    guts->list.push_back(ServerDataP(a));
}


void ServerDB::clear(){
    guts->list.clear();
}


size_t ServerDB::size() const{
    return guts->list.size();
}


struct ServerDB::GroupDB::GroupDB_Impl{
    std::mutex mutex;
    ServerData global;
    std::vector<GroupDataP> list;
};


void ServerDB::GroupDB::lock() const {
    guts->mutex.lock();
}


void ServerDB::GroupDB::unlock() const{
    guts->mutex.unlock();
}


template<typename T1, typename T2>
void GetAndExistGroup(Fl_Preferences &prefs, const std::string &valuename, struct OptData<T1> &into, bool def1, T2 def2){
    int i;
    GetAndExist(prefs, valuename+".enabled", i, def1);
    into.apply = i;
    GetAndExist(prefs, valuename, into.value, def2);
}


void ServerDB::GroupDB::LoadGroup(struct GroupData *group, Fl_Preferences &prefs){

    assert(group);

    const std::string GroupPrefix = std::string("group.")+group->UID + ".";


    GetAndExist(prefs, GroupPrefix+"name", group->name, "New Group");

    GetAndExistGroup(prefs, GroupPrefix+"nickname", group->nick, false, "KashyyykUser");
    GetAndExistGroup(prefs, GroupPrefix+"username", group->user, false, "KashyyykName");
    GetAndExistGroup(prefs, GroupPrefix+"realname", group->real, false, "KashyyykReal");
    GetAndExistGroup(prefs, GroupPrefix+"ssl",      group->SSL, false, false);
    GetAndExistGroup(prefs, GroupPrefix+"globalidentity", group->global, true, false);

    GetAndExist(prefs, GroupPrefix+"authtype.enabled", group->auth_type.apply, false);
    GetAndExist(prefs, GroupPrefix+"auth.enabled",  group->auth.apply, false);
    GetAndExist(prefs, GroupPrefix+"auth.username", group->auth.value[0], "");
    GetAndExist(prefs, GroupPrefix+"auth.password", group->auth.value[1], "");

    {
        int i;
        GetAndExist(prefs, GroupPrefix+"authtype", i, (int)AuthType::Nothing);
        group->auth_type.value = static_cast<enum AuthType>(i);
    }

}


template<typename T>
void GroupSet(Fl_Preferences &prefs, const std::string &valuename, struct OptData<T> &into){
    int i = into.apply;
    prefs.set((valuename+".enabled").c_str(), i);
    prefs.set(valuename.c_str(), into.value);
}

template<>
void GroupSet<std::string>(Fl_Preferences &prefs, const std::string &valuename, struct OptData<std::string> &into){
    int i = into.apply;
    prefs.set((valuename+".enabled").c_str(), i);
    prefs.set(valuename.c_str(), into.value.c_str());
}

template<>
void GroupSet<bool>(Fl_Preferences &prefs, const std::string &valuename, struct OptData<bool> &into){
    int i = into.apply;
    prefs.set((valuename+".enabled").c_str(), i);
    i = into.value;
    prefs.set(valuename.c_str(), i);
}

template<>
void GroupSet<std::string[2]>(Fl_Preferences &prefs, const std::string &valuename, struct OptData<std::string[2]> &into){
    int i = into.apply;
    prefs.set((valuename+".enabled").c_str(), i);
    prefs.set((valuename+"username").c_str(), into.value[0].c_str());
    prefs.set((valuename+"password").c_str(), into.value[1].c_str());
}


void ServerDB::GroupDB::SaveGroup(struct GroupData *group, Fl_Preferences &prefs){
        const std::string GroupPrefix = std::string("group.")+group->UID + ".";

        prefs.set((GroupPrefix+"name").c_str(),     group->name.c_str());

        GroupSet(prefs, GroupPrefix+"nickname", group->nick);
        GroupSet(prefs, GroupPrefix+"username", group->user);
        GroupSet(prefs, GroupPrefix+"realname", group->real);
        GroupSet(prefs, GroupPrefix+"ssl", group->SSL);
        GroupSet(prefs, GroupPrefix+"globalidentity", group->global);
        GroupSet(prefs, GroupPrefix+"authtype", group->auth_type);
        GroupSet(prefs, GroupPrefix+"auth", group->auth);

}


struct GroupData *ServerDB::GroupDB::GenerateGroup() const {
    struct GroupData *ret = new struct GroupData();
    ret->UID = GenerateUID();

    LoadGroup(ret, GetPreferences());

    return ret;

};


void ServerDB::GroupDB::push_back(GroupDataP data){
    push_back(data.get());
}


void ServerDB::GroupDB::push_back(struct GroupData *group){
    guts->list.push_back(GroupDataP(group));
}


void ServerDB::GroupDB::clear(){
    guts->list.clear();
}


size_t ServerDB::GroupDB::size() const{
    return guts->list.size();
}


ServerDB::GroupDB::iterator ServerDB::GroupDB::begin(void) const{
    return guts->list.begin();

}


ServerDB::GroupDB::iterator ServerDB::GroupDB::end(void) const{
    return guts->list.end();
}


ServerDB::GroupDB::reverse_iterator ServerDB::GroupDB::rbegin(void) const{
    return guts->list.rbegin();
}


ServerDB::GroupDB::reverse_iterator ServerDB::GroupDB::rend(void) const{
    return guts->list.rend();
}


ServerDB::GroupDB::iterator ServerDB::GroupDB::erase(iterator i){
    return guts->list.erase(i);
}


}
