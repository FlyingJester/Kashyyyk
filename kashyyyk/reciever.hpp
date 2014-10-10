#pragma once

struct IRC_Message;

namespace Kashyyyk {

class MessageHandler {
public:

    MessageHandler();
    virtual ~MessageHandler();

    // Returns true if the handler should be removed after this call.
    virtual bool HandleMessage(IRC_Message *msg) = 0;

};

class Reciever {
public:

    Reciever();


    virtual ~Reciever();


    virtual void GiveMessage(IRC_Message *msg) = 0;
    virtual void SendMessage(IRC_Message *msg) = 0;

};

template <class Parent_T>
class TypedReciever : public Reciever {
protected:

    Parent_T *Parent;

public:

    TypedReciever(Parent_T *parent)
      : Parent(parent) {}


    virtual ~TypedReciever(){}


    inline bool HasParent() {return Parent!=nullptr;}

};


}
