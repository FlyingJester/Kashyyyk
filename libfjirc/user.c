#include "user.h"

struct IRC_UserHolder *IRC_UserNext(struct IRC_UserHolder *a){
    return a->next;
}


struct IRC_UserHolder *IRC_UserPrev(struct IRC_UserHolder *a){
    return a->prev;
}


struct IRC_User *IRC_UserUnwrap(struct IRC_UserHolder *a){
    return a->user;
}
