#include "identityframe.hpp"
#include "serverdatabase.hpp"
#include <array>
#include <FL/Fl_Group.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Choice.H>


namespace Kashyyyk{


std::array<Fl_Menu_Item, AuthType::NumAuthTypes> AuthTypeMenuItems;

struct IdentityFrame GenerateIdentityFrame(int x, int y, int w, int h, void (*Input_CB)(Fl_Widget *, long), void (Button_CB)(Fl_Widget *, long), long a, long b, long c, long d){

    struct IdentityFrame frame;
    frame.group  = new Fl_Group(x, y+24, w, h-24, "Identity");
    w-=8;
    x+=4;
    Fl_Pack *pack = new Fl_Pack(x, y+28, w, h-24);
    pack->spacing(4);

    frame.global = new Fl_Check_Button(0, 0, 32, 24, "Use Global Identity");
    frame.global->callback(Button_CB, d);

    frame.resize_group = new Fl_Group(0, 0, 32, 28*3);
    frame.resize_group->resizable(nullptr);

    frame.nick = new Fl_Input(44,  0, w-48, 24, "Nick");
    frame.user = new Fl_Input(44, 28, w-48, 24, "User");
    frame.real = new Fl_Input(44, 56, w-48, 24, "Real");

    frame.nick->callback(Input_CB, a);
    frame.user->callback(Input_CB, b);
    frame.real->callback(Input_CB, c);

    frame.resize_group->end();

    frame.signon_group = new Fl_Group(0, 0, 32, 28*3);
    frame.signon_group->resizable(nullptr);
    frame.signon_type = new Fl_Choice(0, 0, w, 24);

    {
        int i = -1;
        Fl_Menu_Item *items = new Fl_Menu_Item[64];
            AuthTypeMenuItems[AuthType::Nothing]        = items[++i] = {"No Password", 0,0,new int(NoPassNoName)};
            AuthTypeMenuItems[AuthType::NickServ]       = items[++i] = {"NickServ",0,0,new int(PassOnly),FL_MENU_INACTIVE};
            AuthTypeMenuItems[AuthType::ServerPassword] = items[++i] = {"Server Password",0,0,new int(PassOnly),FL_MENU_INACTIVE};
            items[++i] = {"SASL",0,0,new int(PassAndName),FL_MENU_INACTIVE};
            items[++i] = {"SASL with Certificate",0,0,new int(NoPassNoName),FL_MENU_INACTIVE};
            items[++i] = {"SASL with SSL or TLS",0,0,new int(NoPassNoName),FL_MENU_INACTIVE};
            items[++i] = {"SASL with Mozilla Persona",0,0,new int(NoPassNoName),FL_MENU_INACTIVE};

        items[++i] = {0};
        frame.signon_type->menu(items);
    }

    frame.username = new Fl_Input(44, 28, w-48, 24, "Name");
    frame.password = new Fl_Secret_Input(44, 56, w-48, 24, "Key");

    frame.signon_group->end();
    frame.group->end();

    return frame;
}


}
