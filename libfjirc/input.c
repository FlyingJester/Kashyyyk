#include "input.h"
#include "message.h"
#include "messageinternal.h"

#include <stdlib.h>
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
    const char *type_str;
    enum IRC_messageType type = IRC_privmsg;
    struct IRC_Message *r_msg;

    IRC_GetAllocators(&Alloc, &Dealloc);

    if((input[0]=='!') || (input[0]=='/')){
        const char *e = input+1, *type_start = input+1;
        while((*e!=' ') && (*e=='\0'))
          e++;
        e--;

        type_str = IRC_Strndup(type_start, e-type_start);

        if(input[0]=='!'){
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

        Dealloc((void *)type_str);

        if(type==IRC_mt_null)
          type = IRC_privmsg;

    }


    if((type==IRC_nick) || (type==IRC_join)){
        GENERATE_MSG(msg, 1, type);
        SET_PARAM(msg, 1, input);
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
