#pragma once

#include "reciever.hpp"
#include "message.h"
#include <string>
#include <cassert>

namespace Kashyyyk {


// Functional-style objects for message handlers.
template<IRC_messageType type>
class OnMsgType {
public:
    bool operator() (IRC_Message *msg){
        if(msg->type==type)
          return true;
        return false;
    }

};


template<bool b>
class Always{
public:
    bool operator() (IRC_Message *msg){
      return b;
    }

};


typedef Always<true> OnMsgAlways;
typedef Always<false> OnMsgNever;



// Functiona-style objects for generating strings for message handlers.
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

    inline const std::string &Value() const{return value;}
    inline const char *ValueC() const{return valuec;}

};

class from_reader {
    const IRC_Message *msg;
    std::string value;
public:
    from_reader()
      : msg(nullptr){

    }

    const char * operator () (const IRC_Message *m);

    inline const std::string &Value() const{return value;}
    inline void Reset(){msg=nullptr;}

};


class join_reader {
    from_reader r;
    std::string s;
public:

    const char * operator () (const IRC_Message *m);

};


class part_reader {
    from_reader r;
    param_reader<1> p;
    std::string s;
public:

    const char * operator () (const IRC_Message *m);
};


// A simple handler that prints annotated versions of every message it sees
// to stdout.
class Debug_Handler : public MessageHandler {
public:
    bool HandleMessage(IRC_Message *msg) override;
};

}
