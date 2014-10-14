#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <assert.h>

unsigned CSV_CountElements(const char *a){
    unsigned r = 0;

    assert(a);

    if(a[0]!='\0'){
        const char *b = a;
        r=1;
        do{
            if(b[0]==',')
              r++;
        } while(*b++!='\0');

    }
    return r;
}

const char **CSV_ParseString(const char *a){
    int i, elements = CSV_CountElements(a);
    char **r = malloc(sizeof(const char*)*(elements+1));
    const char *b = a;

    assert(a);
    assert(r);

    for(i = 0; i<elements; i++){
        const char *c = strchr(b, ',');

        if(c==NULL){
            assert(i==elements-1);

            r[i] = strdup(b);

            assert(r[i]);
        }
        else{
            /* Minus one to push back before the comma.
            */
            unsigned len = (c-b);
            assert(c>b);

            r[i] = malloc(len+1);

            assert(r[i]);

            r[i][len]='\0';

            memcpy(r[i], b, len);

        }
        /* Plus one to outrun the comma.
        */
        b = c+1;
    }

    r[i] = NULL;

    return (const char **)r;
}

const char *CSV_ConstructString(const char **a){

    char *r = NULL, *c = NULL;
    int i = 0, len = 0;
    assert(a);

    {
        const char *b = a[i++];

        /* the `len+=1' provides room for the comma, except on the last iteration.
         Then, it provides room for the NULL.
        */
        while(b!=NULL){
            printf("Parse 1: %s\n", b);
            len+=strlen(b)+1;
            b = a[i++];
        }

    }

    printf("Parse n: %i\n", len);
    r = malloc(len);
    r[len-1] = '\0';

    c = r;

    i = 0;

    if(len){
        const char *b = a[i++];
        while(b!=NULL){
            int olen = strlen(b);

            memcpy(c, b, olen);

            printf("Parse 2: %s\n", c);

            c[olen] = ',';

            c+=olen+1;


            b = a[i++];

            assert(c<=r+len);

        }
    }
    r[len-1] = '\0';

    return r;

}

void CSV_FreeParse(const char **a){

    assert(a);

    {
        const char *b = a[0];
        int i = 0;

        while(b!=NULL){

            assert(b);

            free((void *)b);
            b = a[++i];
        }
    }

    free(a);

}
