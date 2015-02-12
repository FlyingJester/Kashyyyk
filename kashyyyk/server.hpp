#pragma once

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

    const Server *server;
    WSocket *socket;
    bool reconnect_channels;
    long port;
public:

    ServerConnectTask(const Server *aServer, WSocket *aSocket, long prt = 6665, bool reconnect_chans = true, bool SSL = false);

    void Run() override;

    std::shared_ptr<PromiseValue<bool> > promise;

};

//
// The MessageHandlers in a Server should only very rarely need to actually see
// much about the Server's Channels. In the case of server-only messages, such
// as PING or NOTICE messages, the Handler won't even touch any Channels. If
// the message has a clear target, such as a PRIVMSG, it will be sent to the
// appropriate channel. Other messages, such as QUITs, will be sent to all
// Channels. It will be up to the channels to decide if such Channel-specific
// messages with no codified target are relevant or not.
//
// Other MessageHandlers do exist, such as listeners for JOIN success, or
// one-shot reactionary responses for registration.
//

class Server : public LockingReciever<Window, Monitor> {
public:

    typedef std::list<std::unique_ptr<Channel> > ChannelList;

    static void ReconnectServer_CB(Fl_Widget *, void *p);
    
    struct ServerState{
        std::string name;
        std::string nick;
        std::string user;
        std::string real;
        WSocket *socket;
        long port;
        bool SSL;
        std::list<std::string> channels;
    };
    
protected:
    
    Channel *last_channel;

    std::unique_ptr<Fl_Group> widget;
    std::unique_ptr<Fl_Tree_Item> channel_list;

    Fl_Tree_Prefs tree_prefs;

    bool task_died;
    ServerTask * const network_task;

    const std::string UID;

    void Show(Channel *chan);

    void FocusChanged();

    mutable std::shared_ptr<PromiseValue<bool> > last_reconnect;

    ChannelList Channels;

    struct ServerState state;
    
public:

    friend class Channel;
    friend class Window;
    friend class ServerConnectTask;
    friend class AutoLocker<Server *>;

    Server(const struct ServerState &init_state, Window *w);
    //Server(WSocket *socket, const std::string &name, Window *w, long prt, bool SSL=false);
    ~Server();

   const std::string &GetName() const {return state.name;}
   const std::string &GetNick() const {return state.nick;}

    const ChannelList &GetChannels() const{return Channels;}

    bool IsConnected() const;

     // Registers a new chat group.
     // Resizes the group given to the correct proportions.
    void AddChild(Fl_Group *);

     // Sends the message out the socket.
    virtual void SendMessage(IRC_Message *msg) override;

    // Informs us that we will be recieving a JOIN message for this channel.
    std::shared_ptr<PromiseValue<Channel *> > JoinChannel(const std::string &channel);
    std::shared_ptr<PromiseValue<bool> >  Reconnect(bool reconnect_channels = true) const;
    // TODO: Make this better than just dropping the connection.
    std::shared_ptr<PromiseValue<bool> >  Disconnect() const;

    void AddChannel(Channel *);
    void AddChannel_l(Channel *);

    void Show();
    void Hide();

    void Highlight();

     // Functional-style object for finding a certain Channel in a Server
    class find_channel {
        const std::string &n;
    public:
        find_channel(const std::string &s);
        find_channel(const Channel *);
        bool operator () (const std::unique_ptr<Channel> &);
    };

    void Pling(){
        Parent->Pling();
    }

    bool SocketStatus();
    
    static bool CopyState(struct ServerState &to, const struct ServerState &from);
    inline bool EnumerateState(struct ServerState &to) const{return CopyState(to, state);}

};

}
