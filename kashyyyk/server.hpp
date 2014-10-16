#pragma once

#include "window.hpp"
#include "reciever.hpp"
#include "promise.hpp"
#include "autolocker.hpp"

#include <list>
#include <memory>
#include <string>
#include <mutex>
#include <algorithm>
#include <FL/Fl_Tree_Prefs.H>

class Fl_Group;
class Fl_Tree_Item;
struct WSocket;

namespace Kashyyyk{

class Channel;

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

class Server : public LockingReciever<Window, std::mutex> {
public:

    typedef std::list<std::unique_ptr<Channel> >        ChannelList;

protected:

    Channel *last_channel;

    WSocket *socket;

    std::unique_ptr<Fl_Group> widget;
    std::unique_ptr<Fl_Tree_Item> channel_list;

    Fl_Tree_Prefs tree_prefs;
    void Show(Channel *chan);

    void FocusChanged();

public:
    friend class Channel;
    friend class Window;
    friend class AutoLocker<Server *>;

    Server(WSocket *socket, const std::string &name, Window *w);
    ~Server();

    std::string name;
    std::string nick;

    ChannelList Channels;

     // Registers a new chat group.
     // Resizes the group given to the correct proportions.
    void AddChild(Fl_Group *);

     // Sends the message out the socket.
    virtual void SendMessage(IRC_Message *msg) override;

    // Informs us that we will be recieving a JOIN message for this channel.
    std::shared_ptr<PromiseValue<Channel *> > JoinChannel(const std::string &channel);

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

};

}
