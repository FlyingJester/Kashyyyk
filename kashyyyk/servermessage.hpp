#pragma once

#include "reciever.hpp"
#include "server.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "message.h"

namespace Kashyyyk{
namespace ServerMessage{

// Pure virtual base class. Primarily to ensure the
// consistency of 'server'.
class Message_Handler : public MessageHandler {
protected:
    Server *server;
public:
    Message_Handler(Server *s);
    virtual ~Message_Handler();
};


// Honds onto a message until a message that causes T::(msg) to return true.
// It then sends the message, frees the message, and dies.
template <class T>
class SendMessageOn_Handler : public Message_Handler {
IRC_Message *r_msg;
public:
    SendMessageOn_Handler(Server *s, IRC_Message *m)
      : Message_Handler(s)
      , r_msg(m) {

    }

    ~SendMessageOn_Handler() override{
        IRC_FreeMessage(r_msg);
    }

    bool HandleMessage(IRC_Message *msg) override {

        T t;

        if(t(msg)){
            server->SendMessage(r_msg);
            return true;
        }
        return false;
    }

};


// Sends the message in response to any message at all.
// A SendMessageOn with a T::(msg) that always returns true.
typedef SendMessageOn_Handler<OnMsgAlways> SendMessage_Handler;


// Checks if a certain message parameter equals the channel name for
// a certain message type, and GiveMessage's the channel if it does.
// Most Handlers should be of this type.
template<IRC_messageType type, int n = 0>
class ChannelChecker_Handler : public Message_Handler {
public:
    ChannelChecker_Handler(Server *s)
      : Message_Handler(s) {

    }

    ~ChannelChecker_Handler() override {};

    bool HandleMessage(IRC_Message *msg) override {
        if( (msg->type==type) && (msg->num_parameters>n)){

            Server::ChannelList::const_iterator iter =
              std::find_if(server->Channels.cbegin(), server->Channels.cend(), Server::find_channel(msg->parameters[n]));

            if(iter!=server->Channels.cend())
              iter->get()->GiveMessage(msg);
        }

        return false;
    }
};

// Known specializations
typedef ChannelChecker_Handler<IRC_join>  Join_Handler;
typedef ChannelChecker_Handler<IRC_part>  Part_Handler;
typedef ChannelChecker_Handler<IRC_topic> Topic_Handler;
typedef ChannelChecker_Handler<IRC_topic_num, 1> NumericTopic_Handler;
typedef ChannelChecker_Handler<IRC_no_topic_num, 1> NumericNoTopic_Handler;
typedef ChannelChecker_Handler<IRC_namelist_num, 2> Namelist_Handler;

// These types of messages are broad to all channels, and are up to the channel
// whether or not to act on them.
template<IRC_messageType type>
class ServerToAllChannels_Handler : public Message_Handler {
protected:
public:
    ServerToAllChannels_Handler(Server *s)
      : Message_Handler(s) {

    }

    ~ServerToAllChannels_Handler() override {};

    bool HandleMessage(IRC_Message *msg) override {
        if(msg->type==type) {
            for(Server::ChannelList::iterator iter = server->Channels.begin(); iter!=server->Channels.end(); iter++)
              iter->get()->GiveMessage(msg);
        }

        return false;
    }

};

// Known specializations
typedef ServerToAllChannels_Handler<IRC_quit> Quit_Handler;
typedef ServerToAllChannels_Handler<IRC_nick> Nick_Handler;


// Delivers notifications to
class Notice_Handler : public ServerToAllChannels_Handler<IRC_notice> {
    static const std::string server_s;
public:
    Notice_Handler(Server *s);

    ~Notice_Handler() override {};

    bool HandleMessage(IRC_Message *msg) override;

};


// Listens for an expected channel JOIN, and adds the channel when
// it is recieved.
class JoinChannel_Handler : public Message_Handler {
    std::string channel_name;
public:
    JoinChannel_Handler(Server *s, const std::string &n)
      : Message_Handler(s)
      , channel_name(n)
      , promise(new PromiseValue<Channel *>(nullptr)) {

    }

    ~JoinChannel_Handler() override {};

    bool HandleMessage(IRC_Message *msg) override {

        if(msg->type!=IRC_join)
          return false;

        char *str = IRC_MessageToString(msg);
        printf("Join handler examining message %s\nWe are looking for %s.\n", str, channel_name.c_str());
        free(str);

        if(std::string(msg->parameters[0])!=channel_name)
          return false;

        Fl::lock();
        Channel * channel = new Channel(server, channel_name);

        promise->Finalize(channel);
        server->AddChannel_l(channel);
        Fl::unlock();

        promise->SetReady();
        return true;
    }

    std::shared_ptr<PromiseValue<Channel *> > promise;

};

class Ping_Handler : public Message_Handler {
public:
    Ping_Handler(Server *s);
    ~Ping_Handler() override {};
    bool HandleMessage(IRC_Message *msg) override;

};



}
}
