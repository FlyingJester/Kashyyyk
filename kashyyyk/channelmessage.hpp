#pragma once

//! @file
//! @brief @link Kashyyyk::MessageHandler @endlink derivatives for defining
//! @link Kashyyyk::Channel @endlink behaviour
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0
//! @sa servermessage.hpp
//! @sa message.hpp

#include "reciever.hpp"
#include "server.hpp"
#include "channel.hpp"
#include "message.hpp"
#include "message.h"
#include <string>
#include "platform/strcasestr.h"

#ifdef SendMessage
#undef SendMessage
#endif

namespace Kashyyyk{
namespace ChannelMessage{

//!
//! @brief Abstract base class for Channel-centric Message Handlers
//!
//! This class primarily ensures the channel member is consistent in
//! Channel Message Handlers.
class Message_Handler : public MessageHandler {
protected:

    //! @brief pointer to the channel recieving the message
    Channel *channel;

    //!
    //! @brief Channel Message Handlers override this to define their behavior.
    //!
    //! The message being handled can be assumed to be directed to this channel
    //! if the message includes destination channel (such as JOIN or PART).
    //! Other types that are sent to clients without specifying the applicable
    //! channel, such as QUIT, are given indiscrimitely to all Channels the
    //! Server owns. It is the job of the Message_Handler to determine whether
    //! or not to act on such messages.
    //!
    //! @param msg Message being handled.
    bool HandleMessage(IRC_Message *msg) = 0;

public:

    Message_Handler(Channel *c);
    virtual ~Message_Handler() {}

};

//!
//! @brief Repeating handler for QUIT messages
//!
//! This Handler must determine if the channel contains the user quitting.
class Quit_Handler : public Message_Handler {
public:
    Quit_Handler(Channel *c);
    ~Quit_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};

//!
//! @brief Repeating handler for TOPIC and 332 (IRC_topic_num) messages
//!
//! This acts on both TOPIC and 332 messages.
class Topic_Handler : public Message_Handler {
public:
    Topic_Handler(Channel *c);
    ~Topic_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};


//!
//! @brief Repeating handler for JOIN messages
//!
//! This should attempt to sort the user list display of the channel.
class Join_Handler : public Message_Handler {
    from_reader r;
public:
    Join_Handler(Channel *c);
    ~Join_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};

//!
//! @brief Repeating handler for 353 (IRC_namelist_num) messages
//!
//! Even though this is a stateful message type for servers, it can
//! successfully be interpreted without handling 366 (IRC_namelist_end_num)
//! messages.
class Namelist_Handler : public Message_Handler {
    param_reader<3> r;
public:
    Namelist_Handler(Channel *c);
    ~Namelist_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override;

};

//!
//! @brief Arbitrary Message printing Handler
//!
//! Checks if a message should be printed, and if so, prints it using
//! Kashyyyk::Channel::WriteLine
//!
//! The message's from will be preserved in any printed messages. If it is
//! NULL, then an empty string will be used for the from.
//!
//! Some helpful unary predicates for this class can be found in message.hpp.
//!
//! @tparam type Messages not matching this type will be ignored.
//! @tparam level Level to call Channel::Highlight with if the message is
//! printed.
//! @tparam T A unary predicate type that will be callled with messages of the
//! correct type to be considered. It must return the message that will be
//! printed.
//!
//! @sa Kashyyy::Channel::Highlight
//! @sa Kashyyy::Channel::WriteLine
//!
template <IRC_messageType type, Channel::HighlightLevel level, class T>
class ChannelMessage_Handler : public Message_Handler {
    T t;
    from_reader r;
public:
    ChannelMessage_Handler(Channel *c)
      : Message_Handler(c){}

    ~ChannelMessage_Handler() override {}

    bool HandleMessage(IRC_Message *msg) override {

        if((msg->type!=type) || (msg->num_parameters<1))
          return false;

        channel->last_msg_type = type;
        const char *from = r(msg);

        channel->WriteLine(from, t(msg));

        if(strcasestr(t(msg), channel->server()->nick.c_str())!=nullptr){
            channel->Highlight(Channel::HighlightLevel::High);
        }
        else{
            channel->Highlight(level);
        }

		t.Reset();

        return false;
    }

};

//! Prints PRIVMSG messages
typedef ChannelMessage_Handler<IRC_privmsg, Channel::HighlightLevel::Medium, param_reader<1> > PrivateMessage_Handler;
//! Prints PART messages
typedef ChannelMessage_Handler<IRC_part,    Channel::HighlightLevel::Low,    part_reader>      Part_Handler;
//! Prints NOTIVE messages.
//! @note The channel doesn't care if it is the Server's `server` channel or
//! not, so the message should have been correctly routed in the Server.
typedef ChannelMessage_Handler<IRC_notice,  Channel::HighlightLevel::Low,    param_reader<1> > Notice_Handler;
typedef ChannelMessage_Handler<IRC_your_host_num,  Channel::HighlightLevel::Low, param_reader<1> > YourHost_Handler;
typedef ChannelMessage_Handler<IRC_topic_extra_num,  Channel::HighlightLevel::Low, param_reader<1> > TopicExtra_Handler;
//! Prints information about JOIN messages. Does not modify the userlist.
typedef ChannelMessage_Handler<IRC_join,    Channel::HighlightLevel::Low,    join_reader>      JoinPrint_Handler;

}
}
