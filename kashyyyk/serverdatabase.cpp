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
static const unsigned uid_coefficient = 32;
//! This is the number of calls for entropy that will be made to fill a UID.
static const size_t uid_len = uid_coefficient/sizeof(rand_holder);

struct ServerDB::ServerDB_Impl{
    std::mutex mutex;
    std::vector<ServerDataP> list;
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

    char *autojoin;
    GetAndExist(prefs, ServerPrefix+"autojoin", autojoin, "");

    const char **channels = FJ::CSV::ParseString(autojoin);

    for(int i = 0; channels[i]!=nullptr; i++){
        server->autojoin_channels.push_back(std::string(channels[i]));
    }

    FJ::CSV::FreeParse(channels);

    free(autojoin);

}


struct ServerData *ServerDB::GenerateServer() const{
    struct ServerData *ret = new struct ServerData();
    ret->owner = this;
    ret->UID = GenerateUID();

     // Cleat data
    ret->name = nullptr;
    ret->address = nullptr;
    ret->port = 0;
    ret->SSL = -1;
    ret->global = true;
    ret->nick = nullptr;
    ret->user = nullptr;
    ret->real = nullptr;
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

    for(int i = 0; server_uids[i]!=nullptr; i++){
        ServerDataP server(new ServerData());
        server->UID = server_uids[i];
        server->owner = this;
        LoadServer(server.get(), prefs);

        push_back(std::move(server));
    }

    // The individual items are now owned by the ServerData.
    free(server_uids);


}


void ServerDB::save(Fl_Preferences &prefs) const {
    AutoLocker<const ServerDB * const> locker(this);

    std::string UIDs;

    for(iterator iter = begin(); iter!= end(); iter++){
        UIDs+= iter->get()->UID;
        UIDs.push_back(',');
    }
    UIDs.pop_back();

    prefs.set("sys.server_uids", UIDs.c_str());

    std::for_each(begin(), end(), [&prefs](const ServerDataP& server){SaveServer(server.get(), prefs);});

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


}
