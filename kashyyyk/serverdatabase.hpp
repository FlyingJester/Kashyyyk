#pragma once

#include "prefs.hpp"

#include <memory>
#include <iterator>
#include <vector>
#include <list>
#include <string>
#include <cstring>

#include <FL/Fl_Preferences.H>

namespace Kashyyyk {

struct ServerData;
struct GroupData;
typedef std::unique_ptr<struct ServerData> ServerDataP;
typedef std::unique_ptr<struct GroupData>  GroupDataP;

class ServerDB;
struct ServerData;

//! @brief A predicate class to check UID of a ServerDataP
//!
//! This is intended for use with ServerDataP and GroupDataP, specifically when
//! using a ServerDB iterator. Useful for find_if and foreach.
struct equals_uid{
    const char * &uid;
    equals_uid(const char * &target);

    template<class T>
    bool operator() (const T &data){
        return strcmp(data->UID, uid)==0;
    }
};

//! @brief A data holder that represents an optional piece of data
//!
//! Represents a piece of optional data that might not be used.
//! This is used to override the settings of a server from a group.
template<typename T>
struct OptData {
    bool apply; //!< Whether the data applies or not
    T value; //!< Value to override with
};

enum AuthType {Nothing, NickServ, ServerPassword, NumAuthTypes};
enum AuthStyle {NoPassNoName, PassOnly, NameOnly, PassAndName, NumAuthOptions};


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

    enum AuthType auth_type; //!< Authetication type
    std::string username, password; //!< Authentication info

    const ServerDB *owner;
};


struct GroupData {

    const char *UID;
    std::string name;

    struct OptData<bool> SSL;
    struct OptData<bool> global;
    struct OptData<std::string> nick;
    struct OptData<std::string> user;
    struct OptData<std::string> real;

    struct OptData<enum AuthType> auth_type;
    struct OptData<std::string[2]> auth; //!< 0: Username, 1: Password

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

    class GroupDB {
        struct GroupDB_Impl;
        std::unique_ptr<struct GroupDB_Impl> guts;

    public:

        void lock() const;
        void unlock() const;

        typedef std::vector<GroupDataP>::iterator iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;

        static void LoadGroup(struct GroupData *, Fl_Preferences &);
        inline static void LoadGroup(GroupDataP &group, Fl_Preferences &prefs){
            LoadGroup(group.get(), prefs);
        }

        static void SaveGroup(struct GroupData *, Fl_Preferences &);
        inline static void SaveGroup(iterator iter, Fl_Preferences &prefs){
            SaveGroup(iter->get(), prefs);
        }
        static void SaveGroup(const GroupDataP &autoServer, Fl_Preferences &prefs){
            SaveGroup(autoServer.get(), prefs);
        }

        struct GroupData *GenerateGroup() const;
        void push_back(GroupDataP);
        void push_back(struct GroupData *);

        void clear();

        size_t size() const;

        iterator begin(void) const;
        iterator end(void) const;
        reverse_iterator rbegin(void) const;
        reverse_iterator rend(void) const;

        iterator erase(iterator);

        inline reverse_iterator erase(reverse_iterator iter){
            return reverse_iterator(erase(iter.base()));
        }



    } group;

};

}
