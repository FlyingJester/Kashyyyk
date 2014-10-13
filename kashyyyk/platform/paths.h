#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Free values with free();
*/

const char * Kashyyyk_HomeDirectory();
const char * Kashyyyk_ConfigDirectory();
void Kashyyyk_MakeDir(const char *);

#ifdef __cplusplus
}
#endif
