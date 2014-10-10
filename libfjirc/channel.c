#include "channel.h"
#include "message.h"
#include <string.h>

struct IRC_ChannelHolder *IRC_ChannelNext(struct IRC_ChannelHolder *a){
    return a->next;
}


struct IRC_ChannelHolder *IRC_ChannelPrev(struct IRC_ChannelHolder *a){
    return a->prev;
}


struct IRC_Channel *IRC_ChannelUnwrap(struct IRC_ChannelHolder *a){
    return a->channel;
}


static IRC_allocator Alloc;
static IRC_deallocator Dealloc;

void IRC_ChannelAddUser(struct IRC_Channel *a, struct IRC_User *user){

    IRC_GetAllocators(&Alloc, NULL);

    {
        struct IRC_UserHolder *h = Alloc(sizeof(struct IRC_UserHolder));
        struct IRC_UserHolder *first = a->users;
        struct IRC_UserHolder *last  = IRC_UserPrev(a->users);

        first->prev = h;
        last->next  = h;
        h->next = first;
        h->prev = last;

    }

    if(a->OnUserAdd!=NULL)
      a->OnUserAdd(a, user);

}

void IRC_ChannelRemoveUserR(struct IRC_Channel *a, struct IRC_UserHolder *user){

    IRC_GetAllocators(NULL, &Dealloc);

    IRC_UserNext(user)->prev = IRC_UserPrev(user);
    IRC_UserPrev(user)->next = IRC_UserNext(user);

    if(a->OnUserRemove!=NULL)
      a->OnUserRemove(a, user->user);

    Dealloc(user->user);
    Dealloc(user);

}

void IRC_ChannelRemoveUser(struct IRC_Channel *a, struct IRC_User *user){

    struct IRC_UserHolder *h = a->users;
    do{
        if(h->user==user){
            IRC_ChannelRemoveUserR(a, h);
            return;
        }
        h = IRC_UserNext(h);
    }while(h!=a->users);

}

void IRC_ChannelRemoveUserName(struct IRC_Channel *a, const char *name){

    struct IRC_UserHolder *h = a->users;
    do{
        if(strcmp(h->user->name, name)==0){
            IRC_ChannelRemoveUserR(a, h);
            return;
        }
        h = IRC_UserNext(h);
    }while(h!=a->users);

}
