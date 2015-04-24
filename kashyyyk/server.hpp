#pragma once

//! @file
//! @brief Definition of @link Kashyyyk::Server @endlink
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0

#include "window.hpp"
#include "reciever.hpp"
#include "promise.hpp"
#include "autolocker.hpp"

#include <list>
#include <memory>
#include <atomic>
#include <string>
#include <algorithm>
#include <FL/Fl_Tree_Prefs.H>

#ifdef SendMessage
#undef SendMessage
#endif

class Fl_Group;
class Fl_Tree_Item;
struct WSocket;

namespace std {class atomic_flag;}

namespace Kashyyyk{

class Channel;
class ServerTask;

class ServerConnectTask : public Task {

    Server *const server;
    WSocket * const socket;
    bool reconnect_channels;
    long port;
public:

    ServerConnectTask(Server *aServer, WSocket *aSocket, long prt = 6665, bool reconnect_chans = true, bool SSL = false);

    void Run() override;

    std::shared_ptr<PromiseValue<bool> > promise;

};


//!
//! @brief IRC Server
//!
//! Represents an IRC Server, which is strongly owned by a Kashyyyk::Window and
//! strongly owns a set of Kashyyyk::Channels.
//! 
//! The MessageHandlers in a Server should only very rarely need to actually see
//! much about the Server's Channels. In the case of server-only messages, such
//! as PING or NOTICE messages, the Handler won't even touch any Channels. If
//! the message has a clear target, such as a PRIVMSG, it will be sent to the
//! appropriate channel. Other messages, such as QUITs, will be sent to all
//! Channels. It will be up to the channels to decide if such Channel-specific
//! messages with no codified target are relevant or not.
//!
//! Other MessageHandlers do exist, such as listeners for JOIN success, or
//! one-shot reactionary responses for registration.
//!
//! @sa Kashyyyk::Server::ServerState
//! @sa Kashyyyk::User
//! @sa Kashyyyk::Channel
//! @sa Kashyyyk::Window
//! @sa Kashyyyk::Reciever

class Server : public LockingReciever<Window, Monitor> {
public:
    //! Container class for storing channels in
    typedef std::list<std::unique_ptr<Channel> > ChannelList;
    
    //! Callback for reconnecting server.
    static void ReconnectServer_CB(Fl_Widget *, void *p);
    
    //!
    //! @brief IRC Server Information
    //! 
    //! Contains all the data that would be required to recreate the current state
    //! of a Kashyyyk::Server. 
    //!
    struct ServerState{
        std::string name;
        std::string nick;
        std::string user;
        std::string real;
        
        //! A socket that may be used to connect with, or may have been disconnected.
        //! It is guaranteed that this will be a valid return of Create_Socket, and
        //! not NULL.
        WSocket *socket;
        long port;
        bool SSL;
        std::list<std::string> channels;
    };
    
protected:
    
    Channel *last_channel;

    std::unique_ptr<Fl_Group> widget;
    mutable std::unique_ptr<Fl_Tree_Item> channel_list;

    Fl_Tree_Prefs tree_prefs;

    bool task_died;
    ServerTask * const network_task;

    void Show(Channel *chan);

    void FocusChanged() const;

    ChannelList channels;

    struct ServerState state;
    
public:

    friend class Channel;
    friend class Window;
    friend class ServerConnectTask;
    friend class AutoLocker<Server *>;

    //! Constructs a server using an initial state
    //! @param init_state Initial state to construct the server with
    //! @param w Window to place the server in
    Server(const struct ServerState &init_state, Window *w);
    ~Server();
    
    //! Returns the username used on this server.
    const std::string &GetName() const {return state.name;}
    //! Returns the nickname used on this server.
    const std::string &GetNick() const {return state.nick;}
    //! Returns the realname used on this server.
    const std::string &GetReal() const {return state.real;}
    
    //! Retrieves a list of channels that are currently joined on this server. 
    const ChannelList &GetChannels() const{return channels;}

    //! @brief Returns if this server is connected
    bool IsConnected() const;
    
    //! Maintains the state of the last attempt to connect (in progress, succeeded, failed)
    mutable std::shared_ptr<PromiseValue<bool> > last_connection;
    
    //! @brief Add a new chat widget group.
    //! This whill resize the group given to the correct proportions.
    void AddChild(Fl_Group *);

    //! Sends the message out the server's socket socket.
    virtual void SendMessage(IRC_Message *msg) override;

    //! Attempts to join the specified channel.
    //! Returns a promise that represents the join.
    std::shared_ptr<PromiseValue<Channel *> > JoinChannel(const std::string &channel, int dummy_);
    
    //! Informs server that we will be recieving a JOIN message for this channel.
    //!
    //! @warning This message should only be used if you have sent a JOIN message out this Server's socket.
    std::shared_ptr<PromiseValue<Channel *> > JoinChannel(const std::string &channel);
    
    //! Attempt to rejoin
    std::shared_ptr<PromiseValue<bool> >  Reconnect();
    //! Attempt to rejoin with a new state
    std::shared_ptr<PromiseValue<bool> >  Reconnect(const struct ServerState &init_state);
    //! Disconnect this server.
    //! @todo Make this better than just dropping the connection.
    void Disconnect() const;

    //! Used to test the graphical states that signify disconnection.
    //! @sa GDebugDisconnect
    void GDebugReconnect(){
        Disable();
    }
    
    //! Used to test the graphical states that signify disconnection.
    //! @sa GDebugReconnect
    void GDebugDisconnect(){
        Enable();
    }
    
    void AddChannel(Channel *);
    void AddChannel_l(Channel *);

    void Show();
    void Hide();

    void Highlight() const;

    //! Functional-style object for finding a certain Channel in a Server
    class find_channel {
        const std::string &n;
    public:
        find_channel(const std::string &s);
        find_channel(const Channel *);
        bool operator () (const std::unique_ptr<Channel> &);
    };
    
    //! Plings the parent window to indicate some interaction
    void Pling(){
        Parent->Pling();
    }
    
    //! Returns true of the socket is usable for sending messages or may currently recieve messages,
    //! and false otherwise.
    bool SocketStatus();
    
    //! Puts this server into Disabled mode.
    //! No new input will be accepted until it is enabled.
    //! @sa Enable()
    void Disable();
    
    //! Puts this server into Enabled mode.
    //! @sa Disable()
    void Enable();
    
    //! Copies a server state
    //! @param to ServerState to copy server state to
    //! @param from ServerState to copy server state from
    static bool CopyState(struct ServerState &to, const struct ServerState &from);
    //! Copies this server's state to \p to.
    inline bool EnumerateState(struct ServerState &to) const{return CopyState(to, state);}
    //! Returns this server's state.
    inline struct ServerState EnumerateState() const{struct ServerState out; CopyState(out, state); return out;}

};

}
