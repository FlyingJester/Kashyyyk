#include "message.h"
#include "parse.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>

static IRC_allocator Alloc;
static IRC_deallocator Dealloc;

struct IRC_ParseState{
    const char * const text;
    const char * cursor;
    const unsigned long len;

    char *buffer;
    unsigned long bufferlen;
    enum IRC_parseStatus status;
};

/* Return the next crlf, or 0 indicating that the end of the field comes first.
 You can be sure that it is Okay to scroll to it.
*/
static unsigned long IRC_GetNextLength(struct IRC_ParseState *state){
    int wascr = 0;
    unsigned long i = 0;
    while((state->cursor+i) - state->text < state->len){

        if((wascr!=0) && (state->cursor[i] == '\n')){
            return i;
        }

        if(state->cursor[i] == '\r')
          wascr = 1;
        else
          wascr = 0;

        i++;
    }

    state->status = IRC_unexpectedEnd;

    return 0;
}


struct IRC_ParseState *IRC_StitchParse(struct IRC_ParseState * old, const char *input){
    char *newinput;
    struct IRC_ParseState *state;

    unsigned len = old->len - (old->cursor - old->text) + strlen(input);
    IRC_GetAllocators(&Alloc, &Dealloc);

    newinput = Alloc(len+1);

    strcpy(newinput, old->cursor);
    strcat(newinput, input);

    state = IRC_InitParse(newinput);

    Dealloc(newinput);

    return state;

}

struct IRC_ParseState *IRC_InitParse(const char *input){
    struct IRC_ParseState *state = NULL;
    if(input == NULL)
      return state;

    IRC_GetAllocators(&Alloc, NULL);
    state = Alloc(sizeof(struct IRC_ParseState));

    *((const char **)(&(state->text))) = IRC_Strdup(input);
    state->cursor = state->text;
    *((unsigned long*)(&(state->len))) = strlen(input);
    state->status = IRC_inProgress;
    state->buffer = NULL;
    state->bufferlen = 0;
    return state;
}

unsigned long IRC_CountParameters(const char * const text){
    const char *a = text;

    /* Usually for the initial message, we need to trim the text.
    */
    while(*a==' ')
      a++;
    /* The rest of the message is a single parameter, regardless of ws.
    */
    if(*a==':')
      return 1;

    /* If we are in the middle/at the start of a word, move past it.
    */

    while((*a!='\0') && (*a!=' ') && (*a!=':'))
      a++;


    /* If we are now in between words, cut to the start of the next word.
    */
    while(*a==' ')
      a++;

    /* We are at the end of the message.
    */
    if(*a=='\0')
      return 1;

    /* Stop if we will hit /r/n
      We know that a[0] is not NULL from the previous check, so if the string
      really is null-terminated a[1] is fine to touch, too.
    */
    if((a[0]=='\r') && (a[1]=='\n'))
      return 1;

    /* Count this word and continue.
      If we have hit a ':', it will be picked up by this new call.
    */
    return 1+IRC_CountParameters(a);
}

void IRC_ParseParameter(const char * to[], const char * const text){
    const char *a = text, *b;
    while(*a==' ')
      a++;

    if(*a=='\0')
      return;

    /* The entire rest of text is a single parameter.
    */
    if(*a==':'){
        a++;
        to[0] = IRC_Strndup(a, strlen(a)-1);
        return;
    }

    b = a;

    while((*b!='\0') && (*b!=' ') && (*b!=':'))
      b++;

    to[0] = IRC_Strndup(a, b-a);

    a = b;

    IRC_ParseParameter(&(to[1]), a);

}

struct IRC_Message *IRC_ConsumeParse(struct IRC_ParseState *state){
    const char *from, *a;
    enum IRC_messageType msgtype;
    struct IRC_Message *msg;
    unsigned long l = IRC_GetNextLength(state);

    printf("Length is %lu\n", l);
    printf("Cursor is at %lu\n", state->cursor-state->text);

    if(state->status==IRC_unexpectedEnd)
      return NULL;

    if(l==0){
        /* Find out if we are just at the end, or if we have a fragment.
        */
        if(state->cursor-state->len>state->text+2)
          state->status = IRC_unexpectedEnd;
        else
          state->status = IRC_finished;

        /* Signal that we are done with this state, one way or another.
        */
        return NULL;
    }

    /* We will need these for a lot of this function.
     Just grab them no matter what for simplicity.
    */
    IRC_GetAllocators(&Alloc, &Dealloc);

    /* Be sure the buffer is large enough to hold our raw message.
    */
    if(l>state->bufferlen){

        /* We don't explicitly rely on Dealloc being capable of handling NULL.
        */
        if(state->buffer!=NULL)
          Dealloc(state->buffer);

        /* Make the buffer twice the length we need. This should make
         reallocations less common.
         IRC protocol specifies this should never be more than 512, but we
         just do as we are told, and handle any old string.
        */
        state->bufferlen = l << 1;
        state->buffer = Alloc(state->bufferlen);
    }

    /* Put the raw message into the buffer and NUL terminate it.
    */
    state->buffer[l] = '\0';
    memcpy(state->buffer, state->cursor, l);

    printf("Dumping message.\n%s\n", state->buffer);
    state->cursor+=l+1;

    a = state->buffer;

    /* Trim out any whitespace.
    */
    while((*a!='\0') && (*a==' '))
      a++;

    /* Get the sender if applicable.
     This is usually signified with a ':'
    */
    if(*a==':'){
        /* Find the length of the word. We drop the colon.
        */
        unsigned long len = 0;
        const char *b = a+1;
        while((*b!='\0') && (*b!=' '))
          b++;
        len = b-a;

        from = IRC_Strndup(a, len);

        /* Put `a' at the start of the next word.
        */
        a = b;
        a++;
    }
    else
      from = NULL;

    /* Get the message type.
    */
    {
        /* Prepare a buffer to hold the type and pass it for comparison.
        */
        char *type;
        unsigned long len;

        /* Find the length of the word. We drop the colon.
        */
        const char *b = a+1;
        while((*b!='\0') && (*b!=' ') && (*b!=':'))
          b++;

        len = b-a;

        type = IRC_Strndup(a, len);

        printf("Type: (%s)\n", type);

        msgtype = IRC_GetTokenEnum(type);

        Dealloc(type);
        /* Put `a' at the start of the next word.
        */
        a = b;

        /* Only skip ahead if we aren't at a ':'.
          This helps parsing the parameters.
        */
        if(*b!=':')
          a++;
    }

    /* Check if the message type is understood.
    */
    if(msgtype == IRC_mt_null){
        /* Message type we don't understand. Or it was malformed or something.
          Clean up after ourselves, set the status to reflect what is up, and
          return a NULL to notify the application that we don't know what we
          are looking at.
        */
        if(from!=NULL)
          Dealloc((void *)from);

        state->status = IRC_badMessage;
        return NULL;
    }

    /* We now have enough information to prepare a message.
    */
    msg = Alloc(sizeof(struct IRC_Message));

    msg->from = from;
    msg->type = msgtype;
    msg->num_parameters = IRC_CountParameters(a);
    msg->parameters = Alloc(msg->num_parameters*sizeof(const char *));

    IRC_ParseParameter(msg->parameters, a);

    state->status = IRC_inProgress;

    return msg;

}

enum IRC_parseStatus IRC_GetParseStatus(struct IRC_ParseState *state){
    return state->status;
}
/* Clean up the state.
*/
void IRC_DestroyParseState(struct IRC_ParseState *state){
    IRC_GetAllocators(NULL, &Dealloc);

    Dealloc((void *)state->text);
    Dealloc(state->buffer);
    Dealloc(state);

}
