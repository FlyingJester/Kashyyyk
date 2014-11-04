#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Kashyyyk_ShortLog_WriteLine(const char *str){
    static FILE *file = NULL;

    if(file==NULL)
      file = tmpfile();

    fwrite(str, 1, strlen(str), file);
    fflush(file);
}
