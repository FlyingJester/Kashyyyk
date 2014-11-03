#pragma once

class Fl_Group;
class Fl_Button;
class Fl_Input;
class Fl_Choice;
class Fl_Widget;

namespace Kashyyyk{

struct IdentityFrame {
    Fl_Group *group;
    Fl_Button *global;
    Fl_Group *resize_group;
    Fl_Input *nick;
    Fl_Input *user;
    Fl_Input *real;
    Fl_Group *signon_group;
    Fl_Choice *signon_type;
    Fl_Input *username;
    Fl_Input *password;
};

struct IdentityFrame GenerateIdentityFrame(int x, int y, int w, int h, void (*Input_CB)(Fl_Widget *, long), void (*Button_CB)(Fl_Widget *, long), long NickArg = 0l, long UserArg = 0l, long RealArg = 0l, long GlobalArg = 0l);

}
