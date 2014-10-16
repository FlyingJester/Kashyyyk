#include "init.h"
#include "cocoa_about.h"
#include <cassert>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Sys_Menu_Bar.H>

static void Null_CB(Fl_Widget *w, void *p){}

static void About_CB(Fl_Widget *w, void *p){

    assert(w==nullptr);

    Kashyyyk_Cocoa_AboutWindow();

}

void Kashyyyk_PlatformInit(){

    Fl_Menu_Bar *m = new Fl_Sys_Menu_Bar(0, 0, 100, 24);
    m->add("&File/Connect To...", 0, Null_CB, nullptr);
    fl_mac_set_about(About_CB, nullptr);

}

void Kashyyyk_PlatformClose(){

}
