#pragma once
#include <memory>
#include "background.hpp"

#include <FL/Fl_Window.H>

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

    void NewWindow();
    void DirectConnect();
    void JoinChannel();
    void ServerList();
    void Preferences();
    void Quit();

    void Release(Window *);

    static Launcher *CreatePlatformLauncher(Thread::TaskGroup *a);

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
