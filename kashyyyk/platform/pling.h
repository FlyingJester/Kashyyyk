#pragma once
/* Causes some form of alert to reach the user.
 For instance, on Windows the taskbar icon will flash.
 On OS X, the app icon will jump once.
*/
#ifdef __cplusplus

class Fl_Window;

extern "C" {
#else

typedef void Fl_Window;

#endif

void Kashyyyk_Pling(const Fl_Window *);

#ifdef __cplusplus
}

namespace Kashyyyk {

inline void Pling(const Fl_Window *window){
    Kashyyyk_Pling(window);
}

}

#endif
