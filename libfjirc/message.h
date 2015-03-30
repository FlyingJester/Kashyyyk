#pragma once

typedef void *(*IRC_allocator)(unsigned long);
typedef void (*IRC_deallocator)(void *);

#ifdef __cplusplus
extern "C" {
#endif

/* Pass a NULL for one of these to not change it.
*/
void IRC_SetAllocators(IRC_allocator, IRC_deallocator);
/* Pass a NULL for one of these to not retrieve it.
*/
void IRC_GetAllocators(IRC_allocator *, IRC_deallocator *);
void IRC_ResetAllocators();

char *IRC_Strdup(const char * a);
char *IRC_Strndup(const char * a, unsigned long n);

enum IRC_messageType {IRC_mt_null, IRC_error_m, IRC_ping, IRC_pong, IRC_pass,
  IRC_nick, IRC_user, IRC_oper, IRC_mode, IRC_service, IRC_quit, IRC_squit,
  IRC_join, IRC_part, IRC_topic, IRC_privmsg, IRC_notice,
  IRC_namelist_start_num, IRC_namelist_end_num, IRC_topic_num,
  IRC_no_topic_num, IRC_not_registered_num, IRC_welcome_num, IRC_your_host_num,
  IRC_topic_extra_num, IRC_join_ban_num, IRC_join_invite_only_num,
/*Aliases*/
  IRC_namelist_num = IRC_namelist_start_num
  };

const char *IRC_GetMessageToken(enum IRC_messageType a);
enum IRC_messageType IRC_GetTokenEnum(const char * a);

/* `from' can be NULL.
*/
struct IRC_Message {
  enum IRC_messageType type;
  const char *from;
  long num_parameters;
  const char **parameters;
};

struct IRC_MessageHolder{
    struct IRC_Message *msg;
    void *next;
    void *prev;
    void *userdata;
};

char * IRC_MessageToString(struct IRC_Message *msg);
/* For printing messages such as PRIVMSG, NOTICE, etc.
*/
char * IRC_ParamsToString(struct IRC_Message *msg);

struct IRC_MessageHolder *IRC_MessageNext(struct IRC_MessageHolder *a);
struct IRC_MessageHolder *IRC_MessagePrev(struct IRC_MessageHolder *a);
struct IRC_Message *IRC_MessageUnwrap(struct IRC_MessageHolder *a);


/* Value of -1 indicates variable number of paramters.
  The `num_paramters' part of the IRC_Message still has to be accurate.
*/
#define IRC_PASS_PARAM_NUM 1
#define IRC_NICK_PARAM_NUM 1
#define IRC_USER_PARAM_NUM 5
#define IRC_OPER_PARAM_NUM 2
#define IRC_MODE_PARAM_NUM 2
#define IRC_SERVICE_PARAM_NUM 6
#define IRC_QUIT_PARAM_NUM -1
#define IRC_SQUIT_PARAM_NUM 2
#define IRC_JOIN_PARAM_NUM -1
#define IRC_PART_PARAM_NUM -1
#define IRC_TOPIC_PARAM_NUM -1
#define IRC_PRIVMSG_PARAM_NUM 2
#define IRC_CHANNELFAIL_PARAM_NUM 1

#define IRC_SetMessageFrom(s, a)\
    s->from = IRC_Strdup(a);

void IRC_FreeMessage(struct IRC_Message*);

/* Construct a message
*/
struct IRC_Message *IRC_CreatePass(const char *a);
struct IRC_Message *IRC_CreatePing(const char *a);
struct IRC_Message *IRC_CreatePong(const char *a);
struct IRC_Message *IRC_CreatePongFromPing(struct IRC_Message *a);
struct IRC_Message *IRC_CreateNick(const char *a);
struct IRC_Message *IRC_CreateUser(const char *name, const char *host, const char *server, const char *realname);
struct IRC_Message *IRC_CreateOper(const char *name, const char *pass);
struct IRC_Message *IRC_CreateMode(const char *name, const char *mode);
struct IRC_Message *IRC_CreateService(const char *name, const char *visibility, const char *id);
struct IRC_Message *IRC_CreateQuit(const char *a); /* Can take NULL */
struct IRC_Message *IRC_CreateSQuit(const char *s, const char *comment);
struct IRC_Message *IRC_CreateJoin(long n, ...);
 /* Shorthand. Used to make a JOIN message about a single channel. */
struct IRC_Message *IRC_CreateJoinSingle(const char *s);
 /* Shorthand. Create a JOIN message from an array of char *'s. `v' can be
   NULL-terminated. If not, `n' must be set to its length. `n' = 0 means look
   until NULL in `v'.
   Will not look into `v' past `n' elements.
 */
struct IRC_Message *IRC_CreateJoinV(long n, const char **v);
struct IRC_Message *IRC_CreatePart(long n, ...);
 /* Shorthand. Used to make a PART message about a single channel. */
struct IRC_Message *IRC_CreatePartSingle(const char *s);
 /* Shorthand. Create a PART message from an array of char *'s. `v' can be
   NULL-terminated. If not, `n' must be set to its length. `n' = 0 means look
   until NULL in `v'.
   Will not look into `v' past `n' elements.
 */
struct IRC_Message *IRC_CreatePartV(long n, const char **v);
 /* Can take NULL for n */
struct IRC_Message *IRC_CreateTopic(const char *channel, const char *newtopic);
struct IRC_Message *IRC_CreatePrivateMessage(const char *to, const char *msg);

#ifdef __cplusplus
}
#endif
