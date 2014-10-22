#include "message.h"
#include "message.h"
#include "message.h"
#include "messageinternal.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

static IRC_allocator Alloc = (void *(*)(unsigned long ))malloc;
static IRC_deallocator Dealloc = free;

void IRC_SetAllocators(IRC_allocator a, IRC_deallocator b){
    if(a!=NULL)
      Alloc = a;
    if(b!=NULL)
      Dealloc = b;
}

void IRC_GetAllocators(IRC_allocator *a, IRC_deallocator *b){
    if(a!=NULL)
      *a = Alloc;
    if(b!=NULL)
      *b = Dealloc;
}

void IRC_ResetAllocators(){
    Alloc = (void *(*)(unsigned long ))malloc;
    Dealloc = free;
}

char *IRC_Strdup(const char * a){
    unsigned long len = strlen(a)+1;
    char * r = Alloc(len);
    memcpy(r, a, len);
    return r;
}

char *IRC_Strndup(const char * a, unsigned long n){
    unsigned long len = 0;
    char * r;
    while(len < n){
        if(a[len]=='\0')
          break;
        len++;
    }
    r = Alloc(len+1);
    memcpy(r, a, len);
    r[len] = '\0';

    return r;
}

struct IRC_MessageHolder *IRC_MessageNext(struct IRC_MessageHolder *a){
    return a->next;
}


struct IRC_MessageHolder *IRC_MessagePrev(struct IRC_MessageHolder *a){
    return a->prev;
}


struct IRC_Message *IRC_MessageUnwrap(struct IRC_MessageHolder *a){
    return a->msg;
}

char * IRC_ParamsToString(struct IRC_Message *msg){
    unsigned long len = 0;
    char *m;
    int i;
    if(msg->from!=NULL){
        len = strlen(msg->from)+2;
    }

    for(i = 1;i<msg->num_parameters; i++){
        len+= 2+strlen(msg->parameters[i]);
    }

    len+=4; /* NUL */

    m = calloc(len, 1);

    if(msg->from!=NULL)
      strcat(m, msg->from);


    for(i = 1;i<msg->num_parameters; i++)
      strcat(m, msg->parameters[i]);


    return m;

}

char * IRC_MessageToString(struct IRC_Message *msg){

    unsigned long len = 0;
    int i = 0;
    char *m;
    const char *type = IRC_GetMessageToken(msg->type);

    if(msg->from!=NULL)
      len = 1+strlen(msg->from);

    len+=strlen(type)+1;

    for(;i<msg->num_parameters; i++){
        len+= 1+strlen(msg->parameters[i]);
    }

    i = 0;

    len+=3; /* cr+lf+NUL */


    m = calloc(len, 1);
    if(msg->from!=NULL){
        strcat(m, msg->from);
        strcat(m, " ");
    }

    strcat(m, type);
    strcat(m, " ");


    for(;i<msg->num_parameters; i++){
        strcat(m, msg->parameters[i]);
        strcat(m, " ");
    }

    m[len-3] = '\r';
    m[len-2] = '\n';

    return m;

}

/*
enum IRC_messageType {IRC_mt_null, IRC_pass, IRC_nick, IRC_user, IRC_oper, IRC_mode, IRC_service, };
*/
const char *IRC_GetMessageToken(enum IRC_messageType a){
    switch(a){
      case IRC_error_m:
        return "ERROR";
      case IRC_ping:
        return "PING";
      case IRC_pong:
        return "PONG";
      case IRC_pass:
        return "PASS";
      case IRC_nick:
        return "NICK";
      case IRC_user:
        return "USER";
      case IRC_oper:
        return "OPER";
      case IRC_mode:
        return "MODE";
      case IRC_service:
        return "SERVICE";
      case IRC_quit:
        return "QUIT";
      case IRC_squit:
        return "SQUIT";
      case IRC_join:
        return "JOIN";
      case IRC_part:
        return "PART";
      case IRC_topic:
        return "TOPIC";
      case IRC_privmsg:
        return "PRIVMSG";
      case IRC_notice:
        return "NOTICE";
      case IRC_namelist_start_num:
        return "353";
      case IRC_namelist_end_num:
        return "366";
      case IRC_topic_num:
        return "332";
      case IRC_no_topic_num:
        return "333";
      case IRC_topic_extra_num:
        return "372";
      case IRC_not_registered_num:
        return "451";
      case IRC_welcome_num:
        return "001";
      case IRC_your_host_num:
        return "002";
      default:
        return NULL;
    }
}

enum IRC_messageType IRC_GetTokenEnum(const char * a){

    int i = 1;
    const char *token = IRC_GetMessageToken(1);
    do{
        if(strncmp(a, token, 8)==0){
            return i;
        }
        i++;
        token = IRC_GetMessageToken(i);
    } while(token!=NULL);

    return IRC_mt_null;
}


void IRC_FreeMessage(struct IRC_Message *a){
    int i = 0;
    for(; i<a->num_parameters; i++)
      free((void *)a->parameters[i]);
    free((void *)a->from);
    free((void *)a);
}

struct IRC_Message *IRC_CreatePass(const char *a){
    GENERATE_MSG(msg, 1, IRC_pass);
    SET_PARAM(msg, 0, a);
    return msg;
}

struct IRC_Message *IRC_CreatePing(const char *a){
    GENERATE_MSG(msg, 1, IRC_ping);
    SET_PARAM(msg, 0, a);
    return msg;
}

struct IRC_Message *IRC_CreatePong(const char *a){
    GENERATE_MSG(msg, 1, IRC_pong);
    SET_PARAM(msg, 0, a);
    return msg;
}

struct IRC_Message *IRC_CreatePongFromPing(struct IRC_Message *a){
    GENERATE_MSG(msg, 1, IRC_pong);
    SET_PARAM(msg, 0, a->parameters[0]);
    return msg;
}

struct IRC_Message *IRC_CreateNick(const char *a){
    GENERATE_MSG(msg, IRC_NICK_PARAM_NUM, IRC_nick);
    SET_PARAM(msg, 0, a);
    return msg;
}


struct IRC_Message *IRC_CreateUser(const char *name, const char *host, const char *server, const char *realname){
    GENERATE_MSG(msg, IRC_USER_PARAM_NUM, IRC_user);
    SET_PARAM(msg, 0, name);
    SET_PARAM(msg, 1, host);
    SET_PARAM(msg, 2, server);
    SET_PARAM(msg, 3, ":");
    SET_PARAM(msg, 4, realname);
    return msg;
}


struct IRC_Message *IRC_CreateOper(const char *name, const char *pass){
    GENERATE_MSG(msg, IRC_OPER_PARAM_NUM, IRC_oper);
    SET_PARAM(msg, 0, name);
    SET_PARAM(msg, 1, pass);
    return msg;
}


struct IRC_Message *IRC_CreateMode(const char *name, const char *mode){
    GENERATE_MSG(msg, IRC_MODE_PARAM_NUM, IRC_mode);
    SET_PARAM(msg, 0, name);
    SET_PARAM(msg, 1, mode);
    return msg;
}


struct IRC_Message *IRC_CreateService(const char *name, const char *visibility, const char *id){
    GENERATE_MSG(msg, IRC_SERVICE_PARAM_NUM, IRC_service);
    SET_PARAM(msg, 0, name);
    SET_EMPTY_PARAM(msg, 1);
    SET_PARAM(msg, 2, visibility);
    SET_EMPTY_PARAM(msg, 3);
    SET_EMPTY_PARAM(msg, 4);
    SET_PARAM(msg, 5, id);
    return msg;
}


struct IRC_Message *IRC_CreateQuit(const char *a){ /* Can take NULL */
    GENERATE_MSG(msg, IRC_QUIT_PARAM_NUM, IRC_quit);
    SET_PARAM_DEFAULT(msg, 0, a);
    return msg;
}


struct IRC_Message *IRC_CreateSQuit(const char *s, const char *comment){
    GENERATE_MSG(msg, IRC_QUIT_PARAM_NUM, IRC_squit);
    SET_PARAM(msg, 0, s);
    SET_PARAM(msg, 1, comment);
    return msg;
}

struct IRC_Message *IRC_CreateJoin(long n, ...){
    long i;
    const char *a;
    va_list channels;
    GENERATE_MSG(msg, n, IRC_join);
    if(n==0)
      return msg;

    va_start(channels, n);

    for(i=0; i<n; i++){
        a = va_arg(channels, const char *);
        SET_PARAM(msg, i, a);
    }
    va_end(channels);
    return msg;
}


struct IRC_Message *IRC_CreatePart(long n, ...){
    long i;
    const char *a;
    va_list channels;
    GENERATE_MSG(msg, n, IRC_part);
    if(n==0)
      return msg;

    va_start(channels, n);

    for(i=0; i<n; i++){
        a = va_arg(channels, const char *);
        SET_PARAM(msg, i, a);
    }
    va_end(channels);
    return msg;
}


struct IRC_Message *IRC_CreateJoinSingle(const char *s){
    GENERATE_MSG(msg, 1, IRC_join);
    SET_PARAM(msg, 1, s);
    return msg;
}


struct IRC_Message *IRC_CreatePartSingle(const char *s){
    GENERATE_MSG(msg, 1, IRC_part);
    SET_PARAM(msg, 1, s);
    return msg;
}


struct IRC_Message *IRC_CreateV(long n, const char **v, enum IRC_messageType t){
    int i = 0, e = 0;

    if(n==0)
      n = 0xFFFFFFFF;

    while((i<n) && (v[i]!=NULL)){
      i++;
    }

    {
        GENERATE_MSG(msg, i, t);
        while(e<i){
          SET_PARAM(msg, e, v[e]);
          e++;
        }
        return msg;
    }
}


struct IRC_Message *IRC_CreateJoinV(long n, const char **v){
    return IRC_CreateV(n, v, IRC_join);
}


struct IRC_Message *IRC_CreatePartV(long n, const char **v){
    return IRC_CreateV(n, v, IRC_part);
}

struct IRC_Message *IRC_CreateTopic(const char *channel, const char *newtopic){
    if(newtopic==NULL){
        GENERATE_MSG(msg, 1, IRC_topic);
        SET_PARAM(msg, 0, channel);
        return msg;
    }

    {
        GENERATE_MSG(msg, 2, IRC_topic);
        SET_PARAM(msg, 0, channel);
        SET_PARAM(msg, 1, newtopic);
        return msg;
    }
}

struct IRC_Message *IRC_CreatePrivateMessage(const char *to, const char *amsg){
    GENERATE_MSG(msg, 2, IRC_privmsg);
    SET_PARAM(msg, 0, to);
    SET_PARAM(msg, 1, amsg);
    return msg;
}
