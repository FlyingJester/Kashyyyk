#pragma once

//! @file
//! @brief Definitions of @link Kashyyyk::TypedReciever @endlink and
//! @link Kashyyyk::MessageHandler @endlink classes and some basic derivatives.
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0
//!
//! If you want to see how the Kashyyyk IRC Client works
//! conceptually, you are in the right place.
//!

#include <memory>
#include <list>
#include "autolocker.hpp"

#ifdef SendMessage
#undef SendMessage
#endif

struct IRC_Message;

namespace Kashyyyk {

//!
//! @brief Base for Message Handling using TypedReciever objects
//!
//! @note Almost all behaviour in the Kashyyyk IRC client is implemented in
//! derivatives of this class.
//!
//! Strongly owned by a TypedReciever.
//! A MessageHandler examines a message, takes some action or no action based
//! on its contents and other information at hand, and then indicates if it
//! should be removed from the list of handlers for the owning TypedReciever.
//!
//! Conceptually, having a certain MessageHandler means that the TypedReciever
//! is expected to recieve corresponding message or series of messages via
//! TypedReciever::GiveMessage. The behaviour of HandleMessage is the action
//! that should be taken given the contents of the messate, and the return
//! value from HandleMessage indicates if more applicable messages are expected.
//!
//! Most MessageHandler objects will derive from either
//! ChannelMessage::Message_Handler or ServerMessage::Message_Handler.
class MessageHandler {
public:

    MessageHandler();
    virtual ~MessageHandler();

    //! @brief Examines a message, possibly takes some action, and indicates
    //! if the MessageHandler should be deleted after this call.
    //!
    //! This will commonly choose a child TypedReciever of the MessageHandler's
    //! owner to give the message to, or to modify the owning object itself.
    //! Some cases, such as PingHandler, just give responses through a Server
    //! when appropriate, but never modify any objects.
    //!
    //! @return true if the handler should be removed after this call, false if
    //! the MessageHandler should remain in the HandlerList.
    virtual bool HandleMessage(IRC_Message *msg) = 0;

};

//! @cond
typedef std::unique_ptr<MessageHandler> unique_MessageHandler;
//! @endcond

//! @brief Abstract event-driven class capable of sending and recieving
//! messages
//!
//! This is probably not what you are looking for. Check out TypedReciever.
//! @sa TypedReciever
class Reciever {
public:

    Reciever();


    virtual ~Reciever();


    //! Gives the Reciever a message
    virtual void GiveMessage(IRC_Message *msg) = 0;
    //! Instructs Reciever to send a message
    virtual void SendMessage(IRC_Message *msg) = 0;

};

//!
//! @brief Hierarchical, event-driven class capable of sending and recieving
//! messages
//!
//! A TypedReciever is an object that can both recieve and send messages. It
//! almost always has a parent that stricly maintains lifetime and ownership,
//! and will usually have children. Most important is that it can and will
//! recieve messages from its parent, and will react to them.
//!
//! Actual behaviour when a message is recieved is defined by a set of
//! MessageHandler objects held in the Handlers list. When a message is given
//! to a TypedReciever by GiveMessage, the list of MessageHandler objects will
//! be used to react to the message.
//!
//! For TypedReciever classes with children, this will often involve deciding
//! which child will recieve the message.
//!
//! In the Kashyyyk IRC Client, almost all content, including Servers,
//! Channels, and even Windows are implemented as TypedReciever objects.
//!
//! @tparam Parent_T class of parent object
//! @tparam C container type for Handlers
//! @sa MessageHandler
template <class Parent_T, class C = std::list<unique_MessageHandler> >
class TypedReciever : public Reciever {
public:

    //! Typedef of MessageHandler container
    typedef C HandlerList_t;

protected:

    //! Parent object. May be NULL.
    //! @sa HasParent
    Parent_T *Parent;

    //! MessageHandler list
    HandlerList_t Handlers;

public:
    //! Constructs a TypedReciever. Being purely hierarchical it @b needs a parent.
    TypedReciever(Parent_T *parent)
      : Parent(parent) {}


    virtual ~TypedReciever(){}

    //! Returns true if the Parent is not NULL.
    inline bool HasParent() {return Parent!=nullptr;}


    //! Gives the Reciever a message
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
    //! Instructs TypedReciever to send a message
    virtual void SendMessage(IRC_Message *msg) = 0;

};

//!
//! @brief Hierarchical, event-driven class capable of sending and recieving
//! messages with support for locking
//!
//! Extends TypedReciever by adding a Mutex and locking itself before calling
//! GiveMessage. Designed to work with std::mutex and AutoLocker.
//!
//! @tparam Parent_T class of parent object
//! @tparam Mutex Mutual exclusing class implementing lock and unlock
//! @tparam C container type for Handlers
//! @sa TypedReciever
//! @sa AutoLocker
//! @sa MessageHandler
template <class Parent_T, class Mutex, class C = std::list<unique_MessageHandler> >
class LockingReciever : public TypedReciever<Parent_T, C> {
protected:
    Mutex m; //!< Embedded Mutex

    //! Locks embedded Mutex
    virtual void lock(){
        m.lock();
    }
    //! Unlocks embedded Mutex
    virtual void unlock(){
        m.unlock();
    }

public:
    //! Parent type
    typedef TypedReciever<Parent_T, C> TypedReciever_T;
    //! Corresponding AutoLocker
    typedef AutoLocker<LockingReciever<Parent_T, Mutex, C> *> AutoLocker_T;
    friend class AutoLocker<LockingReciever<Parent_T, Mutex, C> *>;

    //! Constructs a TypedReciever. Being purely hierarchical it @b needs a parent.
    //! LockingRecievers are constructed with the default locking state of the Mutex.
    LockingReciever(Parent_T *parent)
      : TypedReciever_T(parent) {}

    virtual ~LockingReciever(){}

    //! Locks and calls GiveMessage
    //! @sa TypedReciever::GiveMessage
    void GiveMessage(IRC_Message *msg) override {

        AutoLocker_T locker(this);

        TypedReciever_T::GiveMessage(msg);

    }

};


}
