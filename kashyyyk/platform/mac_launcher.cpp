#include "../window.hpp"
#include "../launcher.hpp"
#include "cocoa_launcher.h"

extern "C"
Kashyyyk::Launcher *CreateLauncher(void /*Thread::TaskGroup*/ *a){

    Kashyyyk::Launcher *launcher = new Kashyyyk::EmptyLauncher(static_cast<Kashyyyk::Thread::TaskGroup *>(a));

    Kashyyyk_CreateOSXMenu(launcher);

    return launcher;

}
