#pragma once

/* Provides a this C-wrapper for a (few) FLTK x.H function(s).
*/

#include "win32_guard.h"

#include <Windows.h>

#ifdef __cplusplus

class Fl_Window;

extern "C" {
#else

typedef Fl_Window void;

#endif

HWND Kashyyyk_Win32_GetWindow(Fl_Window *);

#ifdef __cplusplus
}

