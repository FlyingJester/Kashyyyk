#include "../launcher.hpp"

extern "C"
Kashyyyk::Launcher *CreateLauncher(void /*Thread::TaskGroup*/ *a){
    return new Kashyyyk::BoringLauncher(static_cast<Kashyyyk::Thread::TaskGroup *>(a));
}
