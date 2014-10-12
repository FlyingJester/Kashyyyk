#include "win32_getwindow.h"
#include <Windows.h>
#include <Fl/x.H>

class Fl_Window;

#include "win32_guard.h"

HWND Kashyyyk_Win32_GetWindow(Fl_Window *window){
    return fl_xid(window);
}

}
