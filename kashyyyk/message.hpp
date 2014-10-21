#pragma once

//! @file
//! @brief Message Handler helper classes and Reciever-agnostic Handlers
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0

#include "reciever.hpp"
#include "message.h"
#include <string>
#include <cassert>

#ifdef Always
#undef Always
#endif

namespace Kashyyyk {


//! @brief A Functional-style object intended to for Channel Message Readers
//! that inspects the messages type
//!
//! Returns true if the given message's type matches the template parameter.
//! @tparam type Type to check for
template<IRC_messageType type>
class OnMsgType {
public:
    bool operator() (IRC_Message *msg){
        if(msg->type==type)
          return true;
        return false;
    }

};


//! @brief A Functional-style object intended to for Channel Message Readers
//! that always returns the same value
//!
//! @tparam b type Value returned
template<bool b>
class Always{
public:
    //! Returns @p b
    bool operator() (IRC_Message *msg){
      return b;
    }

};

//! Always returns true
typedef Always<true> OnMsgAlways;
//! Always returns false
typedef Always<false> OnMsgNever;

//! @brief Functional-style object for generating strings in message handlers
//! based on message parameters
//!
//! Returns a specific parameter number from messages, or an empty string if
//! the message doesn't have enough parameters.
//! Copies the parameter to an internal string that is freed on deletion.
//! @note This object is stateful, and if it is called in succession on the
//! same message a memoized result is returned, reducing overhead.
//! @tparam i Parameter @b index to copy
template<int i>
class param_reader {
    const IRC_Message *msg;
    const char * valuec;
    std::string value;
public:
    param_reader()
      : msg(nullptr){

    }

    const char * operator () (const IRC_Message *m){

        assert(m);

        if(msg!=m){
            msg = m;
            if(m->num_parameters>i){
                valuec = m->parameters[i];
            }
            else{
                valuec = "";
            }
            value = valuec;
        }
        return value.c_str();
    }

    //! Get the memoized value as a std::string
    inline const std::string &Value() const{return value;}
    //! Get the memoized value as a C string
    inline const char *ValueC() const{return valuec;}

};

//! @brief Functional-style object for generating strings in message handlers
//! based on the message's origin
//!
//! Returns the from field of a message, or a blank string if the from field is
//! NULL.
class from_reader {

    const IRC_Message *msg;
    std::string value;

public:
    from_reader()
      : msg(nullptr){

    }

    const char * operator () (const IRC_Message *m);

    //! Get the memoized value as a std::string
    inline const std::string &Value() const{return value;}
    //! Get the memoized value as a C string
    inline const char *ValueC() const{return value.c_str();}
    //! Clears last state (but not result), forcing next call to initialze a
    //! new state.
    inline void Reset(){msg=nullptr;}

};

//! @brief Functional-style object for generating Join messages to be displayed
//! in chat boxes
class join_reader {
    from_reader r;
    std::string s;
public:

    const char * operator () (const IRC_Message *m);

};


//! @brief Functional-style object for generating Part messages to be displayed
//! in chat boxes
class part_reader {
    from_reader r;
    param_reader<1> p;
    std::string s;
public:

    const char * operator () (const IRC_Message *m);
};


//! @brief A simple Message Handlers that prints annotated versions of every
//! message it sees to stdout in a slightly prettified way.
class Debug_Handler : public MessageHandler {
public:
    bool HandleMessage(IRC_Message *msg) override;
};

}
