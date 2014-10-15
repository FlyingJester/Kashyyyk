#pragma once

#include "reciever.hpp"
#include "server.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "message.h"
#include <string>

namespace Kashyyyk{
namespace ChannelMessage{

// Pure virtual base class. Primarily to ensure the consistency of 'channel'.
//
//
class Message_Handler : public MessageHandler {
protected:
    Channel *channel;
public:
    Message_Handler(Channel *s);
    virtual ~Message_Handler() {}
};

class Quit_Handler : public Message_Handler {
public:
    Quit_Handler(Channel *c)
      : Message_Handler(c){

    }

    bool HandleMessage(IRC_Message *msg) override;

};


class Topic_Handler : public Message_Handler {
    from_reader r;
public:
    Topic_Handler(Channel *c);
    ~Topic_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};


class Join_Handler : public Message_Handler {
    from_reader r;
public:
    Join_Handler(Channel *c);
    ~Join_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};


class Namelist_Handler : public Message_Handler {
    Channel *channel;
    param_reader<3> r;
public:
    Namelist_Handler(Channel *c);
    ~Namelist_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};


template <IRC_messageType type, Channel::HighlightLevel level, class T>
class ChannelMessage_Handler : public Message_Handler {
    T t;
public:
    ChannelMessage_Handler(Channel *c)
      : Message_Handler(c){

    }

    ~ChannelMessage_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override {

        if((msg->type!=type) || (msg->num_parameters<1))
          return false;

        channel->last_msg_type = type;

        channel->WriteLine((msg->from==nullptr)?msg->from:"", t(msg));

        if(strcasestr(t(msg), channel->server()->name.c_str())!=nullptr){
            channel->Highlight(Channel::HighlightLevel::High);
        }
        else{
            channel->Highlight(level);
        }

        return false;
    }

};


typedef ChannelMessage_Handler<IRC_privmsg, Channel::HighlightLevel::Medium, param_reader<1> > PrivateMessage_Handler;
typedef ChannelMessage_Handler<IRC_part,    Channel::HighlightLevel::Low,    part_reader>      Part_Handler;
typedef ChannelMessage_Handler<IRC_notice,  Channel::HighlightLevel::Low,    param_reader<1> > Notice_Handler;
typedef ChannelMessage_Handler<IRC_join,    Channel::HighlightLevel::Low,    join_reader>      JoinPrint_Handler;

}
}
