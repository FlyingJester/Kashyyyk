#include "../window.hpp"
#include "../launcher.hpp"

extern "C"
Kashyyyk::Launcher *CreateLauncher(void /*Thread::TaskGroup*/ *a){
    new Kashyyyk::Window(800, 600, static_cast<Kashyyyk::Thread::TaskGroup *>(a));
    return new Kashyyyk::EmptyLauncher(static_cast<Kashyyyk::Thread::TaskGroup *>(a));
}
