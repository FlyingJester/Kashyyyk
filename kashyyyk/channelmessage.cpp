#include "channelmessage.hpp"
#include "csv.h"
#include <string.h>
#include <stdio.h>

namespace Kashyyyk{
namespace ChannelMessage{

Message_Handler::Message_Handler(Channel *c)
  : channel(c){

}


bool Quit_Handler::HandleMessage(IRC_Message *msg){
    if((msg->type!=IRC_quit) || (msg->from==nullptr))
       return false;

    std::string pretty_name = (msg->from[0]==':')?(msg->from+sizeof(char)):msg->from;
    pretty_name = pretty_name.substr(0, pretty_name.find('!'));

    std::list<User>::const_iterator iter = std::find_if(channel->Users.cbegin(), channel->Users.cend(), Channel::find_user(pretty_name));
    if(iter!=channel->Users.cend()){
        std::string message = pretty_name + " Quit: " + ((msg->num_parameters>0)?msg->parameters[0]:"");
        channel->WriteLine("", message.c_str());
    }

    return false;
}

Topic_Handler::Topic_Handler(Channel *c)
  : Message_Handler(c){

}

bool Topic_Handler::HandleMessage(IRC_Message *msg){

    if(((msg->type==IRC_topic) || (msg->type==IRC_topic_num)) && (msg->num_parameters>2)){
        channel->SetTopic(msg->parameters[2]);
    }
    return false;

}


Join_Handler::Join_Handler(Channel *c)
  : Message_Handler(c){

}


bool Join_Handler::HandleMessage(IRC_Message *msg){
    if(msg->type==IRC_join){
        channel->AddUser_l(r(msg), "");
        channel->SortUsers_l();
    }
    return false;
}


Namelist_Handler::Namelist_Handler(Channel *c)
  : Message_Handler(c){

}


bool Namelist_Handler::HandleMessage(IRC_Message *msg){

    if(msg->type==IRC_namelist_num){
        const char **names = FJ::CSV::ParseString(r(msg), ' ');
        int i = 0;
        for(const char *iter = names[i++];iter!=nullptr; iter = names[i++]){
            if(strnlen(iter, 1)==0)
              continue;

            channel->AddUser_l({iter, ""});
        }

        channel->SortUsers_l();

        FJ::CSV::FreeParse(names);
    }
    return false;

}


}
}
