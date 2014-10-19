#pragma once

#include "prefs.hpp"

#include <memory>
#include <iterator>
#include <vector>
#include <list>

#include <FL/Fl_Preferences.H>

namespace Kashyyyk {

struct ServerData;
typedef std::unique_ptr<struct ServerData> ServerDataP;

class ServerDB;
struct ServerData;

//! @brief The data about a server stored in the server database
//!
//! This is used together with a ServerDB to record and retrieve data about
//! servers.
//! @sa ServerDB::GenerateServer
struct ServerData {
    //! Universal identifier for this server data. Because it is possible for
    //! two servers to have the same name or address, this is the recommended
    //! way to identify the server uniquely.
    const char *UID;
    char *Name;             //!< Human-readable name
    char *Address;          //!< Network address
    int  port;              //!< Port to connect on
    int SSL;                //!< Use SSL
    int UserGlobalIdentity; //!< Use global identity information
    char *Nick;             //!< Default nickname on connecting
    char *User;             //!< Default username on connecting
    char *Real;             //!< Default realname on connecting
    //! Vector of channel names to join automatically on connecting
    std::vector<std::string> AutoJoins;
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
