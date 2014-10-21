#include "launcher.hpp"
#include "platform/launcher.h"
#include "window.hpp"
#include "prefs.hpp"
#include "background.hpp"
#include <cstdlib>
#include <cassert>
#include <forward_list>
#include <algorithm>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_RGB_Image.H>

#include <icons/index.h>

#ifdef _MSC_VER
#define constexpr const
#endif

namespace Kashyyyk {

struct Launcher::LauncherImpl{

    //! Task class for deleting a Kashyyyk::Launcher on a different thread.
    class LauncherDeleteTask : public Task {
        Launcher *launcher;
    public:
        LauncherDeleteTask(Launcher *l)
          : launcher(l) {

        }

        void Run() override {
            delete launcher;
            repeating = false;
        }

    };

    //! Wraps Launcher::NewWindow for use in an FLTK callback.
    static void NewWindow_CB(Fl_Widget *w, void *p){
        Launcher *launch = static_cast<Launcher *>(p);
        launch->NewWindow();
    }

    //! Wraps Launcher::DirectConnect for use in an FLTK callback.
    static void DirectConnect_CB(Fl_Widget *w, void *p){
        Launcher *launch = static_cast<Launcher *>(p);
        launch->DirectConnect();
    }

    //! Wraps Launcher::Quit for use in an FLTK callback.
    static void Quit_CB(Fl_Widget *w, void *p){
        Launcher *launch = static_cast<Launcher *>(p);
        launch->Quit();
    }

    //! Wraps Launcher::Preferences for use in an FLTK callback.
    static void Preferences_CB(Fl_Widget *w, void *p){
        Launcher *launch = static_cast<Launcher *>(p);
        launch->Preferences();
    }

    Launcher &launcher;
    Thread::TaskGroup *group;
    std::forward_list<Window *> windows;

    LauncherImpl(Launcher &l, Thread::TaskGroup *g)
      : launcher(l)
      , group(g) {

    }

    virtual ~LauncherImpl() {
    }

};


struct LauncherButtons {

    std::unique_ptr<Fl_Button> NewWindowButton;
    std::unique_ptr<Fl_Button> DirectConnectButton;
    std::unique_ptr<Fl_Button> JoinChannelButton;
    std::unique_ptr<Fl_Button> ServerListButton;
    std::unique_ptr<Fl_Button> PreferencesButton;
    std::unique_ptr<Fl_Button> QuitButton;

};

struct WindowLauncherImpl : public Launcher::LauncherImpl{


    Fl_Window window;

    Fl_Window *last_window;
    WindowLauncherImpl(Launcher &l, Thread::TaskGroup *g, unsigned window_width, unsigned window_height)
      : LauncherImpl(l, g)
      , window(window_width, window_height, "Kashyyyk")
      , last_window(nullptr) {

    }

    virtual ~WindowLauncherImpl() {
    }

};


template<unsigned icon_dimen_wT, unsigned icon_dimen_hT,  bool horozional=true>
struct IconLauncherSpacingImpl : public WindowLauncherImpl {

    struct LauncherButtons buttons;

    static constexpr int h = (horozional)?1:0;
    static constexpr int v = (!horozional)?1:0;
    static constexpr unsigned icon_dimen_w = icon_dimen_wT;
    static constexpr unsigned icon_dimen_h = icon_dimen_hT;
    static constexpr unsigned window_width  = (icon_dimen_w*(( horozional)?6:1))+(2*16);
    static constexpr unsigned window_height = (icon_dimen_h*((!horozional)?6:1))+(2*16);

    IconLauncherSpacingImpl(Launcher &l, Thread::TaskGroup *g)
      : WindowLauncherImpl(l, g, window_width, window_height)
    {
        buttons.NewWindowButton.reset(new Fl_Button(    16+(icon_dimen_w*0*h), 16+(icon_dimen_h*0*v), icon_dimen_w, icon_dimen_h));
        buttons.DirectConnectButton.reset(new Fl_Button(16+(icon_dimen_w*1*h), 16+(icon_dimen_h*1*v), icon_dimen_w, icon_dimen_h));
        buttons.JoinChannelButton.reset(new Fl_Button(  16+(icon_dimen_w*2*h), 16+(icon_dimen_h*2*v), icon_dimen_w, icon_dimen_h));
        buttons.ServerListButton.reset(new Fl_Button(   16+(icon_dimen_w*3*h), 16+(icon_dimen_h*3*v), icon_dimen_w, icon_dimen_h));
        buttons.PreferencesButton.reset(new Fl_Button(  16+(icon_dimen_w*4*h), 16+(icon_dimen_h*4*v), icon_dimen_w, icon_dimen_h));
        buttons.QuitButton.reset(new Fl_Button(         16+(icon_dimen_w*5*h), 16+(icon_dimen_h*5*v), icon_dimen_w, icon_dimen_h));

        buttons.NewWindowButton->callback(NewWindow_CB, &launcher);
        buttons.DirectConnectButton->callback(DirectConnect_CB, &launcher);
        buttons.QuitButton->callback(Quit_CB, &launcher);
        buttons.PreferencesButton->callback(Preferences_CB, &launcher);

    }

    ~IconLauncherSpacingImpl() override {}

};

#ifndef NO_ICONLAUNCHER

struct IconLauncher::IconLauncherImpl : public IconLauncherSpacingImpl<128, 128> {

    Fl_RGB_Image NewWindowImage;
    Fl_Image *NewWindowImage_Scale;

    IconLauncherImpl(Launcher &l, Thread::TaskGroup *g)
      : IconLauncherSpacingImpl<128, 128>(l, g)
      , NewWindowImage((uchar *)(kashyyyk_new_window_image.pixel_data), kashyyyk_new_window_image.width, kashyyyk_new_window_image.height, 4)
      , NewWindowImage_Scale(NewWindowImage.copy(128, 128)){
        buttons.NewWindowButton->image(NewWindowImage_Scale);
    }

    ~IconLauncherImpl() override {}

};

#endif

struct BoringLauncher::BoringLauncherImpl : public IconLauncherSpacingImpl<128, 24, false> {


    BoringLauncherImpl(Launcher &l, Thread::TaskGroup *g)
      : IconLauncherSpacingImpl<128, 24, false>(l, g){
        buttons.NewWindowButton->label("New Window");
        buttons.DirectConnectButton->label("Connect to Server");
        buttons.JoinChannelButton->label("Open Channel");
        buttons.ServerListButton->label("Server List");
        buttons.PreferencesButton->label("Preferences");
        buttons.QuitButton->label("Quit");
    }

    ~BoringLauncherImpl() override {}

};


EmptyLauncher::EmptyLauncher(Thread::TaskGroup *g)
  : Launcher(new Launcher::LauncherImpl(*this, g)){

}


EmptyLauncher::~EmptyLauncher() {}


#ifndef NO_ICONLAUNCHER


IconLauncher::IconLauncher(Thread::TaskGroup *g)
  : Launcher(new IconLauncher::IconLauncherImpl(*this, g)){

      guts_<WindowLauncherImpl>()->window.show();

}


IconLauncher::~IconLauncher(){}


#endif


BoringLauncher::BoringLauncher(Thread::TaskGroup *g)
  : Launcher(new BoringLauncher::BoringLauncherImpl(*this, g)){

    guts_<WindowLauncherImpl>()->window.show();

}


BoringLauncher::~BoringLauncher(){}

Launcher::Launcher(LauncherImpl *impl)
  : guts(impl){

}

Launcher::Launcher(Thread::TaskGroup *g)
  : guts(new LauncherImpl(*this, g)){

}

Launcher::~Launcher(){

	while(!guts->windows.empty())
	  Release(guts->windows.front());
	
    delete guts;
}


Window *Launcher::NewWindow(){

    Kashyyyk::Window *window_new = new Kashyyyk::Window(1024, 600, guts->group, this);
    window_new->Show();
    guts->windows.push_front(window_new);

    return window_new;

}


Launcher *Launcher::CreatePlatformLauncher(Thread::TaskGroup *g){
    return static_cast<Launcher *>(CreateLauncher(g));
}

void Launcher::DirectConnect(){

    Window *win = nullptr;

    if(!guts->windows.empty())
      win = guts->windows.front();
    else
      win = NewWindow();

    assert(win);

    WindowCallbacks::ConnectToServer(win);
}


void Launcher::JoinChannel(){

}


void Launcher::ServerList(){

}


void Launcher::Preferences(){
    OpenPreferencesWindow();
}


void Launcher::Quit(){

    // Forget the launcher so that they don't ask to be released.
    std::for_each(guts->windows.begin(), guts->windows.end(), std::mem_fn(&Window::ForgetLauncher));

    std::for_each(guts->windows.begin(), guts->windows.end(), [&](Window *w){Thread::AddTask(guts->group, new Window::WindowDeleteTask(w));});

    Thread::AddTask(guts->group, new LauncherImpl::LauncherDeleteTask(this));

}

void Launcher::Release(Window *window_check){

    bool deleted = false;
    int i = 0;

    for(std::forward_list<Window *>::iterator iter_at = guts->windows.begin(),
        iter_last= guts->windows.before_begin(); iter_at!=guts->windows.end();
        iter_at++, iter_last++){

        if(*iter_at == window_check){
            guts->windows.erase_after(iter_last);

            printf("Released window at %i.\n", i);
            deleted = true;

            break;
        }
        i++;
    }

    if(!deleted)
        printf("No Window released!\n");

}

}
