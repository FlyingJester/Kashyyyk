#pragma once

#include "reciever.hpp"
#include "server.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "message.h"
#include <atomic>

#ifdef SendMessage
#undef SendMessage
#endif

//! @file
//! @brief @link Kashyyyk::MessageHandler @endlink derivatives for defining
//! @link Kashyyyk::Server @endlink behaviour
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0
//! @sa channelmessage.hpp
//! @sa message.hpp

namespace Kashyyyk{
namespace ServerMessage{

//! @brief Base class for Server-based MessageHandler objects
//! @sa MessageHandler
class Message_Handler : public MessageHandler {
protected:
    //! Owning Server
    Server *server;
public:
    //! Construct with specified Server
    Message_Handler(Server *s);
    virtual ~Message_Handler();
};

//!
//! @brief Message_Handler to hold on to a message until some later time, and
//! send it in response to a certain message
//!
//! Holds on to a message until the unary predicate T of a recieved message
//! evaluates to true. The SendMessageOn_Handler then sends its message and
//! signals its owner that it has complete its task and so shall be deleted.
//! @tparam T unary predicate to evaluate messages
template <class T>
class SendMessageOn_Handler : public Message_Handler {
//! Message that will be sent. Will be freed when SendMessageOn_Handler is
//! deleted.
IRC_Message *r_msg;
//! Unary predicate to evaluate incoming messages using
T t;
public:

    //! Constructs a SendMessageOn_Handler. The given message will be freed
    //! then this object is deleted.
    //! @param s Server to send message to
    //! @param msg Message to send
    SendMessageOn_Handler(Server *s, IRC_Message *msg)
      : Message_Handler(s)
      , r_msg(msg) {

    }

    ~SendMessageOn_Handler() override{
        IRC_FreeMessage(r_msg);
    }

    //! Check @p msg against a @p T predicate, and if it evaluates to true then
    //! send r_msg.
    //! @param msg Message to evaluates
    bool HandleMessage(IRC_Message *msg) override {

        if(t(msg)){
            server->SendMessage(r_msg);
            return true;
        }
        return false;
    }

};


//! @brief Sends the message in response to any message at all.
//!
//! A SendMessageOn with a T::(msg) that always returns true.
typedef SendMessageOn_Handler<OnMsgAlways> SendMessage_Handler;

//! Checks if a certain message parameter equals the channel name for
//! a certain message type, and GiveMessage's the channel if it does.
//! Most Handlers should be of this type.
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

            if(iter!=server->Channels.cend()){
                iter->get()->GiveMessage(msg);
            }
        }

        return false;
    }
};

// Known specializations
typedef ChannelChecker_Handler<IRC_join>  Join_Handler;
typedef ChannelChecker_Handler<IRC_part>  Part_Handler;
typedef ChannelChecker_Handler<IRC_topic> Topic_Handler;
typedef ChannelChecker_Handler<IRC_privmsg, 0> PrivateMessage_Handler;
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
            for(Server::ChannelList::iterator iter = server->Channels.begin(); iter!=server->Channels.end(); iter++){
                iter->get()->GiveMessage(msg);
            }
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

        printf("strstr: %s\tstrcasestr: %s\n", strstr(msg->parameters[0], channel_name.c_str()), strcasestr(msg->parameters[0], channel_name.c_str()));

        if(strcasestr(msg->parameters[0], channel_name.c_str())!=msg->parameters[0]){
            printf("Found %s. Not %s.\n", msg->parameters[0], channel_name.c_str());
            return false;
        }
        else
          printf("Found name\n");
        Fl::lock();
        Channel * channel = new Channel(server, msg->parameters[0]);

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
