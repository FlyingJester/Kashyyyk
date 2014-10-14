#include "pling.h"
#include "win32_getwindow.h"
#include <Windows.h>

#include "win32_guard.h"

void Kashyyyk_Pling(void *window){
    FlashWindow(Kashyyyk_Win32_GetWindow(window), FALSE);
}
