#include "channelmessage.hpp"
#include "csv.h"
#include <string.h>
#include <stdio.h>

namespace Kashyyyk{
namespace ChannelMessage{

Message_Handler::Message_Handler(Channel *c)
  : channel(c){

}


Quit_Handler::Quit_Handler(Channel *c)
  : Message_Handler(c){

}


bool Quit_Handler::HandleMessage(IRC_Message *msg){
    if((msg->type!=IRC_quit) || (msg->from==nullptr))
       return false;

    from_reader r;

    std::list<User>::const_iterator iter = std::find_if(channel->Users.cbegin(), channel->Users.cend(), Channel::find_user(r(msg)));
    if(iter!=channel->Users.cend()){
        std::string message = std::string(r(msg)) + " " + ((msg->num_parameters>0)?msg->parameters[0]:"");
        channel->WriteLine("", message.c_str());
    }

    channel->RemoveUser_l(r(msg));

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
            if(iter[0]=='\0')
              continue;

            channel->AddUser_l({iter, ""});
        }

        channel->SortUsers_l();

        FJ::CSV::FreeParse(names);
    }
    return false;

}


bool Part_Handler::HandleMessage(IRC_Message *msg){

    if((msg->type!=IRC_part) || (msg->num_parameters<1))
      return false;

    channel->last_msg_type = IRC_part;
    const char *from = r(msg);

    channel->WriteLine(from, t(msg));

    channel->Highlight(Channel::HighlightLevel::Low);

    channel->RemoveUser_l(from);

    t.Reset();
    r.Reset();

    return false;
}



}
}
