#pragma once
#include <memory>
#include "background.hpp"

#include <FL/Fl_Window.H>

struct Fl_Menu_Item;

namespace Kashyyyk {

class Window;

class Launcher {
public:
    struct LauncherImpl;
protected:
    LauncherImpl *guts;
    Launcher(LauncherImpl*);
    Launcher(Thread::TaskGroup *);

    template<typename T>
    T *guts_(){
        return static_cast<T *>(guts);
    }

public:
    friend class WindowCallbacks;
    virtual ~Launcher();

    Window *NewWindow();
    void DirectConnect();
    void ServerList();
    void Preferences();
    void JoinChannel();
    void ChangeNick();
    void Disconnect();
    void Reconnect();
    void GDebugDisconnect();
    void GDebugReconnect();
    void Quit();

    void Release(Window *);

    static Launcher *CreatePlatformLauncher(Thread::TaskGroup *a);
    static const Fl_Menu_Item * const GetLauncherMenu(Launcher *l);

    //! Wraps Launcher::NewWindow for use in an FLTK callback.
    static void NewWindow_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::DirectConnect for use in an FLTK callback.
    static void DirectConnect_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::ServerList for use in an FLTK callback.
    static void ServerList_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::Quit for use in an FLTK callback.
    static void Quit_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::Preferences for use in an FLTK callback.
    static void Preferences_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::JoinChannel for use in an FLTK callback.
    static void JoinChannel_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::ChangeNick for use in an FLTK callback.
    static void ChangeNick_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::Disconnect for use in an FLTK callback.
    static void Disconnect_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::Reconnect for use in an FLTK callback.
    static void Reconnect_CB(Fl_Widget *w, void *p);
    
    //! Wraps Launcher::GDebugDisconnect for use in an FLTK callback.
    static void GDebugDisconnect_CB(Fl_Widget *w, void *p);

    //! Wraps Launcher::GDebugReconnect for use in an FLTK callback.
    static void GDebugReconnect_CB(Fl_Widget *w, void *p);


};


class EmptyLauncher : public Launcher {
public:
    EmptyLauncher(Thread::TaskGroup *);
    ~EmptyLauncher() override;
};


#ifndef NO_ICONLAUNCHER


class IconLauncher : public Launcher {
    struct IconLauncherImpl;
public:
    IconLauncher(Thread::TaskGroup *);
    ~IconLauncher() override;
};


#endif


class BoringLauncher : public Launcher {
    struct BoringLauncherImpl;
public:
    BoringLauncher(Thread::TaskGroup *);
    ~BoringLauncher() override;
};

}
