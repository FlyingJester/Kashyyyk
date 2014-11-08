#include <unistd.h>
#include <string.h>

static int file = -1;

void Kashyyyk_ShortLog_WriteLine(const char *str){

    char s[] = {'k', 'a', 's', 'h', 'y', 'y', 'y', 'k', '_', 's', 'h',
        'o', 'r', 't', '_', 'l', 'o', 'g', '_', 'X', 'X', 'X', 'X', 'X', 'X'};

    if(file<0)
      file = mkstemp(s);

    write(file, str, strlen(str));

}

