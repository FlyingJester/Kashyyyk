#pragma once

#include "window.hpp"
#include "reciever.hpp"
#include "promise.hpp"
#include "autolocker.hpp"

#include <list>
#include <memory>
#include <string>
#include <mutex>
#include <FL/Fl_Tree_Prefs.H>

class Fl_Group;
class Fl_Tree_Item;
struct WSocket;

namespace Kashyyyk{

class Channel;

class Server : public TypedReciever<Window> {
public:
    typedef std::list<std::unique_ptr<MessageHandler> > HandlerList;
    typedef std::list<std::unique_ptr<Channel> >        ChannelList;

protected:

    Channel *last_channel;

    WSocket *socket;
    HandlerList Handlers;

    std::mutex mutex;

    inline void lock(){mutex.lock();}
    inline void unlock(){mutex.unlock();}

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

     // Will give the message to the appropriate channel.
    virtual void GiveMessage(IRC_Message *msg) override;
     // Sends the message out the socket.
    virtual void SendMessage(IRC_Message *msg) override;

    // Informs us that we will be recieving a JOIN message for this channel.
    std::shared_ptr<PromiseValue<Channel *> > JoinChannel(const std::string &channel);

    void AddChannel(Channel *);
    void AddChannel_l(Channel *);

    void Show();
    void Hide();

    void Highlight();

};

}