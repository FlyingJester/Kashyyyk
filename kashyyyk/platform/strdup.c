#include "strdup.h"
#include <stdlib.h>
#ifndef _MSC_VER
char *strdup(const char *a){
	const unsigned long len = strlen(a);
	char *b = malloc(len+1);
	memcpy(b, a, len+1);
	return b;
}

#endif
