#pragma once

/* Be careful.
*/

#define ALLOC_MSG Alloc(sizeof(struct IRC_Message))

#define NEW_MSG(X) struct IRC_Message * X = ALLOC_MSG

#define ALLOC_PARAMS(X, N)\
    X->num_parameters = N;\
    X->parameters = Alloc(sizeof(const char *)*((N>=1)?N:1));\

#define GENERATE_MSG(X, N, T)\
    NEW_MSG(X);\
    X->from = NULL;\
    ALLOC_PARAMS(X, N);\
    X->type = T

#define SET_EMPTY_PARAM(X, N)\
    X->parameters[N] = Alloc(1);\
    *(char *)(X->parameters[N]) = '\0'

#define SET_PARAM(X, N, TO)\
    do{\
    unsigned long len = strlen(TO)+1;\
    char * f = Alloc(len);\
    memcpy(f, TO, len);\
    X->parameters[N] = f;\
    }while(0)


#define SET_PARAM_WITH_PRECEDING_COLON(X, N, TO)\
    do{\
    unsigned long len = strlen(TO)+2;\
    char * f = Alloc(len);\
    f[0] = ':';\
    memcpy(f+1, TO, len);\
    X->parameters[N] = f;\
    }while(0)

/* Set's X->parameters[N] to TO if TO is not NULL.
 Otherwise, sets it to "".
*/
#define SET_PARAM_DEFAULT(X, N, TO)\
    if(TO!=NULL)\
      SET_PARAM(X, N, TO);\
    else\
      SET_EMPTY_PARAM(X, N)
