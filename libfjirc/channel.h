#pragma once
#include "user.h"

struct IRC_Channel;
struct IRC_MessageHolder;

typedef void (*IRC_ChannelUserCallback)(struct IRC_Channel *, struct IRC_User *);

struct IRC_Channel {

    const char *name;

    unsigned long num_members;
    struct IRC_UserHolder *users;

    const char *mode;
    const char *umode;

    IRC_ChannelUserCallback OnUserAdd;
    IRC_ChannelUserCallback OnUserRemove;

    void *userdata;
};

/* In a multithreaded environment it is up to the application to
  perform appropriate locking before calling these functions.
*/
void IRC_ChannelAddUser(struct IRC_Channel *a, struct IRC_User *user);
void IRC_ChannelRemoveUser(struct IRC_Channel *a, struct IRC_User *user);
void IRC_ChannelRemoveUserR(struct IRC_Channel *a, struct IRC_UserHolder *user);
void IRC_ChannelRemoveUserName(struct IRC_Channel *a, const char *name);

/* Linked-list containers.
*/
struct IRC_ChannelHolder{
    struct IRC_Channel *channel;
    void *next;
    void *prev;

    struct IRC_MessageHolder *history;

    void *userdata;

};

struct IRC_ChannelHolder *IRC_ChannelNext(struct IRC_ChannelHolder *a);
struct IRC_ChannelHolder *IRC_ChannelPrev(struct IRC_ChannelHolder *a);
struct IRC_Channel *IRC_ChannelUnwrap(struct IRC_ChannelHolder *a);
