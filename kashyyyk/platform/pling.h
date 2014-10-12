#pragma once
/* Causes some form of alert to reach the user.
 For instance, on Windows the taskbar icon will flash.
 On OS X, the app icon will jump once.
*/
#ifdef __cplusplus
extern "C" {
#endif

void Kashyyyk_Pling(void);

#ifdef __cplusplus
}

namespace Kashyyyk {

inline void Pling(void){
    void Kashyyyk_Pling();
}

}

#endif
