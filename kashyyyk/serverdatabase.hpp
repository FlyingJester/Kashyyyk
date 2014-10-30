#pragma once

#include "prefs.hpp"

#include <memory>
#include <iterator>
#include <vector>
#include <list>
#include <string>

#include <FL/Fl_Preferences.H>

namespace Kashyyyk {

struct ServerData;
typedef std::unique_ptr<struct ServerData> ServerDataP;

class ServerDB;
struct ServerData;

struct equals_uid{
    const char * &uid;
    equals_uid(const char * &target);
    bool operator() (const ServerDataP &data);
};

//! @brief The data about a server stored in the server database
//!
//! This is used together with a ServerDB to record and retrieve data about
//! servers.
//! @sa ServerDB::GenerateServer
struct ServerData {

    //! Universal identifier for this server data. Because it is possible for
    //! two servers to have the same name or address, this is the recommended
    //! way to identify the server uniquely.
    const char * UID;
    std::string name;       //!< Human-readable name
    std::string address;    //!< Network address
    int port;              //!< Port to connect on

    std::vector<std::string>group_UIDs;
    std::vector<std::string>autojoin_channels;

    bool SSL;               //!< Use SSL
    bool global;            //!< Use global identity information
    std::string nick;       //!< Default nickname on connecting
    std::string user;       //!< Default username on connecting
    std::string real;       //!< Default realname on connecting

    const ServerDB *owner;  //!< Server's human-readable name
};


class ServerDB{
    struct ServerDB_Impl;
    std::unique_ptr<struct ServerDB_Impl> guts;
public:

    ServerDB();
    ~ServerDB();

    static const char *GenerateUID();

    void open(Fl_Preferences &);
    void save(Fl_Preferences &) const;

    struct ServerData *GetGlobal();
    void SetGlobal(struct ServerData *);

    static void LoadServer(struct ServerData *, Fl_Preferences &);
    inline static void LoadServer(ServerDataP &server, Fl_Preferences &prefs){
        LoadServer(server.get(), prefs);
    }

    struct ServerData *GenerateServer() const;
    void MarkDirty(const struct ServerData *) const;

    void lock() const;
    void unlock() const;

    typedef std::vector<ServerDataP>::iterator iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;

    iterator begin(void) const;
    iterator end(void) const;
    reverse_iterator rbegin(void) const;
    reverse_iterator rend(void) const;

    iterator erase(iterator);

    inline reverse_iterator erase(reverse_iterator iter){
        return reverse_iterator(erase(iter.base()));
    }

    void push_back(ServerDataP);
    void push_back(struct ServerData *);

    static void SaveServer(struct ServerData *, Fl_Preferences &);
    inline static void SaveServer(iterator iter, Fl_Preferences &prefs){
        SaveServer(iter->get(), prefs);
    }
    static void SaveServer(const ServerDataP &autoServer, Fl_Preferences &prefs){
        SaveServer(autoServer.get(), prefs);
    }

    void clear();

    size_t size() const;

    //! @brief Type of callback for use with ServerData::Callbacks
    typedef void(*ServerDataCallback)(const struct ServerData* that, void *arg);

    //! @brief Type of callback container of Callbacks
    //!
    //! The first value in the pair is the callback function pointer.
    //! The second value in the pair is the argument to call the callback with.
    typedef std::vector<std::pair<ServerDataCallback, void *> > CallBackVector;

    //! These are called when the ServerData is modified. They are all called
    //! when ServerDB::MarkDirty is called.
    CallBackVector CallBacks;

};

}
