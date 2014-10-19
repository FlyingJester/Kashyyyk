#include "input.h"
#include "message.h"
#include "messageinternal.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static IRC_allocator Alloc;
static IRC_deallocator Dealloc;

/* `params' does not include the `to' parameter.
 Therefor, almost all messages will have one more
 parameters than `params' because of this.
*/
struct Keyword {
  const char *literal;
  enum IRC_messageType type;
};

static struct Keyword Keywords[] = {
  {"nick", IRC_nick},
  {"join", IRC_join},
  {"leave", IRC_part},
  {"topic", IRC_topic},
  {NULL, IRC_mt_null}
};

struct IRC_Message *IRC_GenerateMessage(const char *to, const char *input){
    const char *type_str, *msg_start = input;
    enum IRC_messageType type = IRC_privmsg;
    struct IRC_Message *r_msg;

    IRC_GetAllocators(&Alloc, &Dealloc);

    while(isspace((int)msg_start[0]))
      msg_start++;

    if(msg_start[0]=='\0')
      return NULL;

    if((msg_start[0]=='!') || (msg_start[0]=='/')){
        const char *e, *type_start = msg_start+1;

        e = strchr(type_start, ' ');
        if(e==NULL)
          e = strchr(type_start, '\0')-1;

        type_str = IRC_Strndup(type_start, e-type_start);

        if(msg_start[0]=='!'){
            type = IRC_GetTokenEnum(type_str);
        }
        else{
            int i = 0;
            while(Keywords[i].literal!=NULL){
                if(strcmp(type_str, Keywords[i].literal)==0){
                    type = Keywords[i].type;
                    input=e+1;
                    break;
                }
                i++;
            }
        }

        printf("Processing %s (%s) message %s.\n", type_str, IRC_GetMessageToken(type), input);

        Dealloc((void *)type_str);

        if(type==IRC_mt_null)
          type = IRC_privmsg;

    }
    else
      printf("Processing %s message %s.\n", IRC_GetMessageToken(type), input);


    if((type==IRC_nick) || (type==IRC_join)){
        GENERATE_MSG(msg, 1, type);
        SET_PARAM(msg, 0, input);
        r_msg = msg;
    }
    else{
        GENERATE_MSG(msg, 2, type);
        SET_PARAM(msg, 0, to);
        SET_PARAM(msg, 1, input);
        r_msg = msg;
    }

    return r_msg;

}
