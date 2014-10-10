#include "usermessage.h"
#include "message.h"

struct IRC_Message *IRC_CreateUserPrivMessage(struct IRC_User *a, const char *message){
    return IRC_CreatePrivateMessage(a->name, message);
}
