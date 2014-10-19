#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "strdup.h"

#define SIZE 0xFF<<4

char * Kashyyyk_HomeDirectory(){

    char buffer[SIZE], *homedir;
    struct passwd *pw = malloc(sizeof(struct passwd));
    getpwuid_r(getuid(), pw, buffer, SIZE, &pw);
    homedir = strdup(pw->pw_dir);

    free(pw);

    return homedir;
}

#undef SIZE

void Kashyyyk_MakeDir(const char *a){
    struct stat s, home;
    const char *homedir = Kashyyyk_HomeDirectory();
    int err = stat(homedir, &home);
    free((void *)homedir);

    if(err==-1){
        perror("Could not stat home directory");
        return;
    }
    err = stat(a, &s);
    if(err==-1) {
        if(errno!=ENOENT) {
            perror("Could not stat directory");
        }
        mkdir(a, home.st_mode);
    }

}

const char * Kashyyyk_ConfigDirectory(){
    const char * const append = "/.conf";
    char * c = Kashyyyk_HomeDirectory();
    char *r = realloc(c, strlen(c)+strlen(append)+1);
    strcat(r, append);

    Kashyyyk_MakeDir(r);

    return r;
}
