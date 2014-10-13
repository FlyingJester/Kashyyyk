#include <string.h>

const char * Kashyyyk_HomeDirectory(){
    char *c = malloc(3);
    c[0] = '.';
    c[1] = '/';
    c[2] = '\0';

    return c;
}

const char * Kashyyyk_ConfigDirectory(){
    char *c = malloc(7);
    c[0] = '.';
    c[1] = '/';
    c[2] = 'c';
    c[3] = 'o';
    c[4] = 'n';
    c[5] = 'f';
    c[6] = '\0';

    return c;
}

void Kashyyyk_MakeDir(const char *a){

}
