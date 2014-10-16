#include "init.h"
#include <stdio.h>

static const char * const About =
KASHYYY_NAME_STRING "\n" \
"Initilized" "\n";

static const char * const Close =
KASHYYY_NAME_STRING "\n" \
"Shutting Down" "\n";

void Kashyyyk_PlatformInit(){

    printf(About);

}

void Kashyyyk_PlatformClose(){

    printf(Close);

}
