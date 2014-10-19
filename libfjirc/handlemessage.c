#include "handlemessage.h"
#include <stdlib.h>

int IRC_CanApplyMessage(struct IRC_State *state, struct IRC_Message *msg){
    switch(msg->type){

      case IRC_mt_null:

        /* These must be recieved by a server.
        */
      case IRC_join:
      case IRC_part:
      case IRC_quit:
      case IRC_squit:
      case IRC_topic:
        return 0;

      case IRC_privmsg:
        if(msg->from==NULL)
          return 0;
        if(IRC_StateContainsUser(state, msg->from))
          return 1;

      default:
        return 0;

    }

}
