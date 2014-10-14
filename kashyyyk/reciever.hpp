#pragma once

#include <memory>
#include <vector>

struct IRC_Message;

namespace Kashyyyk {

class MessageHandler {
public:

    MessageHandler();
    virtual ~MessageHandler();

    // Returns true if the handler should be removed after this call.
    virtual bool HandleMessage(IRC_Message *msg) = 0;

};

typedef std::unique_ptr<MessageHandler> unique_MessageHandler;

class Reciever {
public:

    Reciever();


    virtual ~Reciever();


    virtual void GiveMessage(IRC_Message *msg) = 0;
    virtual void SendMessage(IRC_Message *msg) = 0;

};

template <class Parent_T, class C = std::vector<unique_MessageHandler> >
class TypedReciever : public Reciever {
public:

    typedef C HandlerList_t;

protected:

    Parent_T *Parent;
    HandlerList_t Handlers;

public:

    TypedReciever(Parent_T *parent)
      : Parent(parent) {}


    virtual ~TypedReciever(){}


    inline bool HasParent() {return Parent!=nullptr;}

};

}
