#include "launcher.h"
#include "../launcher.hpp"

#define DEFINE_C_LAUNCHER_FUNCTION(NAME)\
void Kashyyyk_Launcher##NAME(void *a){\
    Kashyyyk::Launcher *launcher = static_cast<Kashyyyk::Launcher *>(a);\
    launcher->NAME();\
}

DEFINE_C_LAUNCHER_FUNCTION(NewWindow)
DEFINE_C_LAUNCHER_FUNCTION(DirectConnect)
DEFINE_C_LAUNCHER_FUNCTION(JoinChannel)
DEFINE_C_LAUNCHER_FUNCTION(ServerList)
DEFINE_C_LAUNCHER_FUNCTION(Preferences)
DEFINE_C_LAUNCHER_FUNCTION(Quit)
