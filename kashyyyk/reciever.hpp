#pragma once

#include <memory>
#include <list>
#include "autolocker.hpp"

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

template <class Parent_T, class C = std::list<unique_MessageHandler> >
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


    void GiveMessage(IRC_Message *msg) override {

        typename HandlerList_t::iterator iter = Handlers.begin();
        while(iter!=Handlers.end()){

            if(iter->get()->HandleMessage(msg)){
                iter = Handlers.erase(iter);
            }
            else{
                ++iter;
            }
        }
    }

};

template <class Parent_T, class Mutex, class C = std::list<unique_MessageHandler> >
class LockingReciever : public TypedReciever<Parent_T, C> {
protected:
    Mutex m;

    virtual void lock(){
        m.lock();
    }
    virtual void unlock(){
        m.unlock();
    }

public:
    typedef TypedReciever<Parent_T, C> TypedReciever_T;
    typedef AutoLocker<LockingReciever<Parent_T, Mutex, C> *> AutoLocker_T;
    friend class AutoLocker<LockingReciever<Parent_T, Mutex, C> *>;


    LockingReciever(Parent_T *parent)
      : TypedReciever_T(parent) {}

    virtual ~LockingReciever(){}

    void GiveMessage(IRC_Message *msg) override {

        AutoLocker_T locker(this);

        TypedReciever_T::GiveMessage(msg);

    }

};


}
