#include "channel.hpp"
#include "server.hpp"
#include "prefs.hpp"
#include "message.hpp"
#include "channelmessage.hpp"
#include "message.h"
#include "input.h"
#include "csv.h"
#include "platform/pling.h"
#include "platform/notification.h"
#include "platform/strcasestr.h"

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
#include <FL/fl_ask.H>

#ifdef _WIN32
// This include is necessary for std::min and std::max with MSVC.
#include <algorithm>
#endif

#ifdef SendMessage
#undef SendMessage
#endif

#ifdef None
#undef None
#endif

using namespace Kashyyyk::ChannelMessage;

namespace Kashyyyk{

Channel::find_user::find_user(const std::string &s)
  : n(s){

}


Channel::find_user::find_user(const User *a)
  : n(a->Name) {

}


bool Channel::find_user::operator () (const User &a){
    return a.Name==n;
}

//! @cond
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

//! @endcond

//! Based the FLTK.org text editor example.
//! Very much overkill, this can handle text insertions as well as deletions.
//! Which is good just in case.
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

    channel->SendMessage(msg);

    msg->from = IRC_Strdup(channel->Nick());
    channel->GiveMessage(msg);

    IRC_FreeMessage(msg);

}


Channel::Channel(Server *s, const std::string &channel_name)
  : LockingReciever<Server, std::mutex>(s)
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
    userlist->format_char(0);

     // Set the chat box to be the auto-resizable portion.
    Fl_Box *resize_box = new Fl_Box(FL_NO_BOX, 24, 24, 40, 64, "");
    resize_box->hide();
    tiler->resizable(resize_box);
    tiler->end();

    Parent->AddChild(tiler);

    Handlers.push_back(std::unique_ptr<MessageHandler>(new PrivateMessage_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Part_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new JoinPrint_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Join_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Quit_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Namelist_Handler(this)));
    Handlers.push_back(std::unique_ptr<MessageHandler>(new Topic_Handler(this)));

}

Channel::~Channel(){
 // Server->Window->...
    Parent->Parent->RemoveChannel(this);
}


void Channel::SetTopic(const char *topic){

    topiclabel->value(topic);

}


void Channel::WriteLine(const char *from, const char *msg){
    const unsigned from_len = strlen(from);
    alignment = std::max<unsigned>(from_len, alignment);

    std::string line = from + std::string(alignment-from_len, ' ');

    line.push_back('|');

    line+=msg;

    line.push_back('\n');

    buffer->append(line.c_str());

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

    if(level==HighlightLevel::None)
      return;

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
          Pling();
        }
        break;
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

void Channel::SendMessage(IRC_Message *msg){
    if(msg->type==IRC_join && msg->num_parameters>0){
        Parent->JoinChannel(msg->parameters[0]);
        printf("Pushing a join message for %s.\n", msg->parameters[0]);
    }

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

void Channel::AddUser_l(const char *user, const char *mode){
    AddUser_l({user, mode});
}

void Channel::AddUser_l(const struct User &user){

    Users.push_back(user);
    userlist->add(user.Name.c_str());

    alignment = std::max<unsigned>(user.Name.size(), alignment);
}


void Channel::AddUser(const struct User &user){
    lock();

    AddUser_l(user);

    unlock();

}


void Channel::SortUsers(){
    lock();
    SortUsers_l();
    unlock();
}


void Channel::SortUsers_l(){
    userlist->sort(FL_SORT_ASCENDING);
    userlist->redraw();
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

void Channel::Pling(){
    Parent->Pling();
}

}
