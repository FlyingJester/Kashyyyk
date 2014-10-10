#include "state.h"
#include "channel.h"
#include <stdlib.h>
#include <string.h>

int IRC_StateContainsUser(struct IRC_State *state, const char *name){
    struct IRC_ChannelHolder *h = state->channels;
    if(h!=NULL)
      do{
          struct IRC_UserHolder *u = h->channel->users;
          if(u!=NULL)
            do{
                if(strcmp(u->user->name, name)==0)
                  return 1;
                u = IRC_UserNext(u);
            }while(u!=h->channel->users);

          h = IRC_ChannelNext(h);
      }while(h!=state->channels);

    return 0;
}
