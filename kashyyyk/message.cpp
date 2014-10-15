#include "message.hpp"

namespace Kashyyyk{

const char * join_reader::operator () (const IRC_Message *m){
    r(m);
    s = r.Value() + " Has Joined " + ((m->num_parameters>0)?m->parameters[0]:"");
    return s.c_str();
}


const char * part_reader::operator () (const IRC_Message *m){
    r(m);
    s = r.Value() + " Has Left (" + p(m) + ")";
    return s.c_str();
}


const char * from_reader::operator () (const IRC_Message *m){

    assert(m);

    if(msg!=m){
        msg = m;

        if(m->from==nullptr){
            value = "";
        }
        else{
            value = (m->from[0]==':')?(m->from+sizeof(char)):m->from;
            value = value.substr(0, value.find('!'));
        }

    }
    return value.c_str();
}


bool Debug_Handler::HandleMessage(IRC_Message *msg){
    printf("[Debug] Message type (%s) from (%s) with %li parameter", IRC_GetMessageToken(msg->type), msg->from, msg->num_parameters);

    if(msg->num_parameters!=1)
        putchar('s');

    if(msg->num_parameters==0)
        putchar('.');
    else
        putchar(':');

    for(int i = 0; i<msg->num_parameters; i++){
        printf("(%s) ", msg->parameters[i]);
    }

    putchar('\n');

    return false;
}

}
