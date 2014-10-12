#include "channel.hpp"
#include "server.hpp"
#include "prefs.hpp"
#include "message.h"
#include "input.h"
#include "platform/pling.h"
#include "platform/notification.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tile.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Multi_Browser.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_ask.H>

#ifdef _WIN32
#undef SendMessage
#endif

namespace Kashyyyk{

static IRC_allocator Alloc;
static IRC_deallocator Dealloc;

struct Channel::StyleTable{
public:
    static const int NumEntries = 5;

    Fl_Text_Display::Style_Table_Entry styletable[Channel::StyleTable::NumEntries];

    inline void ChangeFont(Fl_Font font){
        for(int i = 0; i<Channel::StyleTable::NumEntries; i++)
         styletable[i].font = font;
    }

};

Channel::StyleTable Channel::table = {{
  {FL_FOREGROUND_COLOR, FL_COURIER, FL_NORMAL_SIZE}, // A - Default
  {FL_DARK_RED,         FL_COURIER, FL_NORMAL_SIZE}, // B - Joins
  {FL_DARK_YELLOW,      FL_COURIER, FL_NORMAL_SIZE}, // C - Quits
  {FL_DARK_CYAN,        FL_COURIER, FL_NORMAL_SIZE}, // D - Nick changes
  {FL_RED,              FL_COURIER, FL_NORMAL_SIZE}, // E - Directed Messages
}};


// Based the FLTK.org text editor example.
// Very much overkill, this can handle text insertions as well as deletions.
// Which is good just in case?
void Channel::TextModify_CB(int pos, int nInserted, int nDeleted, int nRestyled, const char* deletedText, void *p){

        Channel *that = static_cast<Channel *>(p);
        assert(that);

        if ((!nInserted) && (!nDeleted)){
            that->stylebuffer->unselect();
            return;
        }

        if (nInserted>0) {
          char style = 'A';

          switch((IRC_messageType)(that->last_msg_type)){
              case IRC_join:
              style = 'B';
              break;
              case IRC_quit:
              style = 'C';
              break;
              case IRC_nick:
              style = 'D';
              break;
              case IRC_notice:
              style = 'E';
              break;
              case IRC_privmsg:
              default:
              break;
          }

          std::string style_str(nInserted, style);
          that->stylebuffer->replace(pos, pos+nDeleted, style_str.c_str());
        }
        else {
          that->stylebuffer->remove(pos, pos+nDeleted);
        }

        // Avoids callbacks?
        that->stylebuffer->select(pos, pos+nInserted-nDeleted);
    }



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
  , alignment(8)
  , name(channel_name) {

    Fl_Preferences &prefs = GetPreferences();

    prefs.get("sys.appearance.font", font, FL_SCREEN);
    table.ChangeFont(font);

    fl_font(font, fl_size());

    Fl_Tile *tiler = new Fl_Tile(0, 0, 192, 112);

    widget.reset(tiler);

    topiclabel = new Fl_Output(0, 0, 64, 24);

    chatlist =  new Fl_Text_Display(0,  24,  64, 64);

    buffer = new Fl_Text_Buffer();
    buffer->add_modify_callback(Channel::TextModify_CB, this);
    stylebuffer = new Fl_Text_Buffer();

    chatlist->buffer(buffer);
    chatlist->highlight_data(stylebuffer, table.styletable, table.NumEntries, 'A', nullptr, 0);
    chatlist->color(FL_BACKGROUND2_COLOR);

    tiler->begin();

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


    Fl_Preferences &prefs = GetPreferences();

    int do_pling = 1;
    prefs.get("sys.pling.enabled", do_pling, 1);

    if(i)
      switch(level){
      case High:
        i->labelcolor(FL_RED);
        if(do_pling==1){
          Pling(Parent->Parent->Handle());
          break;
        }
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

    last_msg_type = msg->type;

    HighlightLevel level;
    switch(msg->type){
      case IRC_notice:
      case IRC_privmsg:
        level = Medium;
        break;
      default:
        level = Low;
    }

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

        /*
        if(!(alignment>=str_s.size()))
          alignment=str_s.size();
        */
        if(strcasestr(msg->parameters[1], Nick())){
          last_msg_type = IRC_notice;
          level = High;

          std::string str_note = "<";
          str_note+=str_s;
          str_note+="> ";
          str_note+=msg->parameters[1];

          Kashyyyk::GiveNotification("Private Message", str_note);

        }

        str_s+=std::string(alignment-str_s.size(), ' ');

        str_s.push_back('|');
        str_s+=msg->parameters[1];

        str = (char *)Alloc(str_s.size()+1);
        strcpy(str, str_s.c_str());

    }
    else if(msg->type==IRC_join){
        if(std::string(msg->from)==Parent->nick){
            Parent->JoinChannel(msg->parameters[0]);
            str = (char *)Alloc(1);
            str[0] = '\0';
        }
        else {
            std::string nick = msg->from;

            size_t colon = nick.find(':');
            if(colon==std::string::npos)
              colon = 0;
            else
              nick = nick.substr(colon+1);


            std::string join_s = std::string(alignment, ' ');
            join_s[0] = '*';
            join_s[1] = '*';
            join_s[2] = '*';

            join_s.push_back('|');

            assert(join_s.size()==alignment+1);

            str = IRC_Strdup(((join_s+nick.substr(0, nick.find('!'))).append(" Joined ")+name).c_str());
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

        str = IRC_Strdup((std::string("***")+std::string(alignment-3, ' ').append("|")+nick).append(" Quit (").append(msg->parameters[0]).append(")").c_str());


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

        str = IRC_Strdup((std::string("***")+std::string(alignment-3, ' ').append("|")+from_nick).append(" is now knwown as ").append(to_nick).append(".").c_str());
    }
    else{

      str = IRC_ParamsToString(msg);

      std::string str_s(alignment, ' ');
      str_s.push_back('|');
      str_s.append(str);

      Dealloc(str);
      str = IRC_Strdup(str_s.c_str());

    }

    buffer->append(str);
    buffer->append("\n");
    chatlist->redraw();
    widget->redraw();
    Parent->widget->redraw();
    Dealloc(str);

    Highlight(level);

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

    alignment = std::max<unsigned>(user.Name.size(), alignment);

    unlock();

}


void Channel::AddUser(const char *user, const char *mode){
    AddUser({user, mode});
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
