#include "win32_getwindow.h"
#include <Windows.h>
#include <Fl/Fl_Window.H>
#include <Fl/x.H>

class Fl_Window;

#include "win32_guard.h"

HWND Kashyyyk_Win32_GetWindow(Fl_Window *window){
    return fl_xid(window);
}

Kashyyyk_Win32_GetAnyWindow(){
    return Kashyyyk_Win32_GetWindow(Fl_Window::current());
}

}
