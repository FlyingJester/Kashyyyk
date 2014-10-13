#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <direct.h>
#include <Windows.h>
#include "win32_getwindow.h"

void Kashyyyk_MakeDir(const char *a){
    _makedir(a);
}

char *Kashyyyk_Win32_GetDirectory(REFKNOWNFOLDERID id){
    size_t i;
    PWSTR val;
    HRESULT r = SHGetKnownFolderPath(id, 0, NULL, &val);
    const size_t s = wcslen(val);
    char *ret = malloc(s+1);
    memset(ret, 0, s);

    wcstombs_s(&i, ret, s, val, _TRUNCATE);
    CoTaskMemFree(val);

    return r;
}

const char * Kashyyyk_HomeDirectory(){
    return Kashyyyk_Win32_GetDirectory(FOLDERID_Profile);
}

const char * Kashyyyk_ConfigDirectory(){

    const char * const Kashyyyk_literal = "\\Kashyyyk";

    mbstate_t m;
    mbrlen(NULL, 0, &m);
    {

        const size_t s = mbrlen(c, strlen(c), &m);

        char * c = Kashyyyk_Win32_GetDirectory(FOLDERID_LocalAppData);
        c = realloc(c, +1);
        memcpy(c+s, Kashyyyk_literal, strlen(Kashyyyk_literal));

    }

    return c;
}
