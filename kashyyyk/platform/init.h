#pragma once

#define KASHYYY_NAME_STRING "Kashyyyk IRC Client"

#ifdef __cplusplus
extern "C" {
#endif

void Kashyyyk_PlatformInit();
void Kashyyyk_PlatformClose();

#ifdef __cplusplus
}

namespace Kashyyyk {

    inline void Init(){
        Kashyyyk_PlatformInit();
    }

    inline void Close(){
        Kashyyyk_PlatformClose();
    }

}

#endif
