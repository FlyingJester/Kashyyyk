#include "servermessage.hpp"

namespace Kashyyyk{
namespace ServerMessage{

Message_Handler::Message_Handler(Server *s)
  : server(s){

}
Message_Handler::~Message_Handler(){

}

Ping_Handler::Ping_Handler(Server *s)
  : Message_Handler(s) {

}

bool Ping_Handler::HandleMessage(IRC_Message *msg){
    if(msg->type==IRC_ping){

        IRC_Message *r_msg = IRC_CreatePongFromPing(msg);
        server->SendMessage(r_msg);
        IRC_FreeMessage(r_msg);

    }

    return false;
}


const std::string Notice_Handler::server_s = "server";


Notice_Handler::Notice_Handler(Server *s)
  : ServerToAllChannels_Handler<IRC_notice> (s){

}


bool Notice_Handler::HandleMessage(IRC_Message *msg){

    if((msg->type==IRC_notice) || (msg->type==IRC_your_host_num) || (msg->type==IRC_topic_extra_num)){
        Server::ChannelList::const_iterator server_chan =
          std::find_if(server->Channels.cbegin(), server->Channels.cend(), Server::find_channel(server_s));

        if(server_chan==server->Channels.cend()){
            fprintf(stderr, "Warning: server channel not found for Server %s.\n", server->name.c_str());
            return false; // Wait, what?
        }

        server_chan->get()->GiveMessage(msg);
    }

    return false;
}

}

}
