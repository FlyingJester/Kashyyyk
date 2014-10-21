#include "pling.h"

#include <FL/fl_ask.H>
#include <cstdio>

inline void GenericPling(void){
#ifndef __CYGWIN__
    fl_beep(FL_BEEP_NOTIFICATION);
#endif
}

// For when we don't know where we are.

void Kashyyyk_Pling(const Fl_Window *){
    printf("\nPLINGING!\n\n");
    GenericPling();
}
