#include "pling.h"

#include <Fl/fl_ask.H>
#include <cstdio>

inline void GenericPling(void){
    fl_beep(FL_BEEP_NOTIFICATION);
}

// For when we don't know where we are.

void Kashyyyk_Pling(void){
    printf("\nPLINGING!\n\n");
    GenericPling();
}
