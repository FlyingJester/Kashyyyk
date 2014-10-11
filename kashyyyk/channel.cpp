#include "channel.hpp"
#include "server.hpp"
#include "prefs.hpp"
#include "message.h"
#include "input.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Preferences.H>

#ifdef _WIN32
#undef SendMessage
#endif

namespace Kashyyyk{

static IRC_allocator Alloc;
static IRC_deallocator Dealloc;

void Input_CB(Fl_Widget *w, void *p){
    Fl_Input *input   = static_cast<Fl_Input *>(w);
    Channel  *channel = static_cast<Channel *> (p);

    std::string str = input->value();
    if(str.empty())
      return;

    std::string::iterator iter = str.begin();
    while(iter!=str.end()){
        if(!isspace(*iter))
          break;
    }

    if(iter==str.end())
      return;

    IRC_Message *msg = IRC_GenerateMessage(channel->name.c_str(), input->value());

    input->value("");

    AutoLocker<Channel *> locker(channel);

    channel->SendMessage(msg);

    msg->from = IRC_Strdup(channel->Nick());
    channel->GiveMessage(msg);

    IRC_FreeMessage(msg);

}

Channel::Channel(Server *s, const std::string &channel_name)
  : TypedReciever<Server>(s)
  , widget()
  , name(channel_name) {

    Fl_Preferences &prefs = GetPreferences();

    int font = 0;
    prefs.get("sys.appearance.font", font, FL_SCREEN);

    fl_font(font, fl_size());

    Fl_Tile *tiler = new Fl_Tile(0, 0, 192, 112);

    widget.reset(tiler);

    topiclabel = new Fl_Output(0, 0, 64, 24);

    static const int column_widths[] = {128, 10, 0};

    chatlist =  new Fl_Multi_Browser(0,  24,  64, 64);
    chatlist->textfont(font);
    chatlist->column_widths(column_widths);
    chatlist->column_char('\a');

    Fl_Input *inputer = new Fl_Input(0, 88, 64, 24);
    inputer->textfont(font);
    inputer->callback(Input_CB, this);
    inputer->when(FL_WHEN_ENTER_KEY|FL_WHEN_NOT_CHANGED);

    userlist = new Fl_Browser(64, 0, 128, 112);
    userlist->textfont(font);

     // Set the chat box to be the auto-resizable portion.
    Fl_Box *resize_box = new Fl_Box(FL_NO_BOX, 24, 24, 40, 64, "");
    resize_box->hide();
    tiler->resizable(resize_box);
    tiler->end();

    Parent->AddChild(tiler);


}

Channel::~Channel(){
 // Server->Window->...
    Parent->Parent->RemoveChannel(this);
}


void Channel::SetTopic(const char *topic){

    topiclabel->value(topic);

}


void Channel::GetPath(std::string &path) const{
    path = Parent->name;
    path.push_back('/');
    path+=name;
}

Fl_Tree_Item *Channel::GetWindowItem(){

    std::string path;
    GetPath(path);

    Fl_Tree_Item * i = Parent->Parent->FindChannel(path.c_str());

    return i;
}


void Channel::Highlight(HighlightLevel level){

    Parent->Highlight();

    Fl_Tree_Item *i = GetWindowItem();


    if(i)
      switch(level){
      case High:
      case Medium:
        i->labelcolor(FL_DARK_RED);
      break;
      case Low:
      default:
        i->labelcolor(FL_DARK_BLUE);
      }

}


void Channel::FocusChanged(){
    Fl_Tree_Item *i = GetWindowItem();

    if(i)
      i->labelcolor(FL_FOREGROUND_COLOR);

}

void Channel::GiveMessage(IRC_Message *msg){

    char *str = nullptr;

    assert(msg);

    IRC_GetAllocators(&Alloc, &Dealloc);

    if(msg->type==IRC_error_m){
      str = IRC_MessageToString(msg);

      std::string str_s = "\a";
      str_s.append(str);

      Dealloc(str);
      str = IRC_Strdup(str_s.c_str());

    }
    else if(msg->type==IRC_privmsg){
        std::string str_s = msg->from?msg->from:"***";
        str_s = str_s.substr(str_s[0]==':'?1:0);
        str_s = str_s.substr(0, str_s.find('!'));

        str_s.push_back('\a');
        str_s+=msg->parameters[1];

        str = (char *)Alloc(str_s.size()+1);
        strcpy(str, str_s.c_str());

    }
    else if(msg->type==IRC_join){
        if(std::string(msg->from)==Parent->nick){
            Parent->JoinChannel(msg->parameters[0]);
            str = (char *)Alloc(2);
            str[0] = '\a';
            str[1] = '\0';
        }
        else {
            std::string nick = msg->from;

            size_t colon = nick.find(':');
            if(colon==std::string::npos)
              colon = 0;
            else
              nick = nick.substr(colon+1);

            str = IRC_Strdup(((std::string("***\a")+nick.substr(0, nick.find('!'))).append(" Joined ")+name).c_str());
        }



    }
    else if(msg->type==IRC_quit){

        std::string nick   = msg->from;

        size_t colon = nick.find(':');
        if(colon==std::string::npos)
          colon = 0;

        nick = nick.substr(colon, nick.find('!'));

        std::list<User>::iterator user_iter = Users.begin();
        while(user_iter!=Users.end()){
            if(user_iter->Name==nick)
              break;

            user_iter++;
        }

        if(user_iter==Users.end())
          return;

        RemoveUser(nick.c_str());

        str = IRC_Strdup((std::string("***\a")+nick).append(" Quit (").append(msg->parameters[0]).append(")").c_str());


    }
    else if((msg->type==IRC_nick) && (msg->from==nullptr) && (msg->num_parameters<1)){

        std::string from_nick = msg->from;
        std::string to_nick   = msg->parameters[0];

        size_t colon = from_nick.find(':');
        if(colon==std::string::npos)
          colon = 0;
        from_nick = from_nick.substr(colon, from_nick.find('!'));

        std::list<User>::iterator user_iter = Users.begin();
        while(user_iter!=Users.end()){
            if(user_iter->Name==from_nick){
                user_iter->Name = to_nick;
                for(int i = 0; i<userlist->size(); i++) {
                      if(std::string(userlist->text(i))==from_nick){
                          userlist->text(i, to_nick.c_str());
                          break;
                      }
                }

                break;
            }
            user_iter++;

        }

        str = IRC_Strdup((std::string("***\a")+from_nick).append(" is now knwown as ").append(to_nick).append(".").c_str());
    }
    else{

      str = IRC_ParamsToString(msg);

      std::string str_s = "\a";
      str_s.append(str);

      Dealloc(str);
      str = IRC_Strdup(str_s.c_str());

    }

    chatlist->add(str);
    widget->redraw();
    Parent->widget->redraw();
    Dealloc(str);

    Highlight(((msg->type==IRC_privmsg)||(msg->type==IRC_notice))?Medium:Low);

}


void Channel::SendMessage(IRC_Message *msg){
    Parent->SendMessage(msg);
}

void Channel::GiveFocus(){
    widget->show();
    focus = true;
}


void Channel::LoseFocus(){
    widget->hide();
    focus = false;
}


void Channel::Show(){

    Parent->Show(this);

}

void Channel::Hide(){

    Parent->Hide();

}


void Channel::AddUser(const struct User &user){
    lock();
    Users.push_back(user);
    userlist->add(user.Name.c_str());

    unlock();

}


void Channel::AddUser(const char *user, const char *mode){
    lock();
    Users.push_back({user, mode});
    userlist->add(user);

    unlock();
}


const char *Channel::Nick(){
    return Parent->nick.c_str();
}


void Channel::RemoveUser(const char *user_c){

    AutoLocker<Channel *> locker(this);

    std::string user(user_c);

    std::list<User>::iterator iter = Users.begin();
    while(iter!=Users.end()){

        if(iter->Name==user){
            Users.erase(iter);
            return;
        }

        iter++;
    }

}

}
