#pragma once

class Fl_Widget;
class Fl_Preferences;

namespace Kashyyyk {

//! Use this instead of constructing new Fl_Preferences objects. It ensures the
//! same configuration file is used throughout, and that updates don't need to
//! be flushed to disk to be seen elsewhere in the program.
//! @returns the Kashyyyk preferences object.
Fl_Preferences &GetPreferences();

//! Opens the Preferences window.
void OpenPreferencesWindow();

//! @brief FLTK Callback wrapper for OpenPreferencesWindow
inline void OpenPreferencesWindow_CB(Fl_Widget *w, void *p){
    OpenPreferencesWindow();
}

}
