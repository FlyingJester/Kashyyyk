#include "init.h"
#include "cocoa_about.h"
#include <cassert>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>

static void About_CB(Fl_Widget *w, void *p){

    assert(w==nullptr);

    Kashyyyk_Cocoa_AboutWindow();

}

void Kashyyyk_PlatformInit(){

    Fl_Mac_App_Menu::print = "";

    fl_mac_set_about(About_CB, nullptr);

}

void Kashyyyk_PlatformClose(){

}
