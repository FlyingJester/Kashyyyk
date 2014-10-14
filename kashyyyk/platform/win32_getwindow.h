#pragma once

/* Provides a this C-wrapper for a (few) FLTK x.H function(s).
*/

#include "win32_guard.h"

#include <Windows.h>

#ifdef __cplusplus

class Fl_Window;

extern "C" {
#else

typedef void Fl_Window;

#endif

HWND Kashyyyk_Win32_GetWindow(Fl_Window *);
HWND Kashyyyk_Win32_GetAnyWindow();

#ifdef __cplusplus
}
#endif
