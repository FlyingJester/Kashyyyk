#pragma once
#include <string>
#include <cstdlib>
#include <FL/Fl_Preferences.H>

class Fl_Widget;

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

void LoadScheme(const char *s);

template <typename T1, typename T2>
inline void GetAndExist(Fl_Preferences &prefs, const std::string &name, T1 &item, const T2 def){
    if(!prefs.get(name.c_str(), item, def))
      prefs.set(name.c_str(), def);
}


template <>
inline void GetAndExist<std::string, const char *>(Fl_Preferences &prefs, const std::string &name, std::string &item, const char * const def){
    char *c = nullptr;
    if(!prefs.get(name.c_str(), c, def))
      prefs.set(name.c_str(), def);

    item = c;
    free(c);
}


template <>
inline void GetAndExist<std::string &, const std::string>(Fl_Preferences &prefs, const std::string &name, std::string &item, const std::string def){
    GetAndExist(prefs, name, item, def.c_str());
}

template <>
inline void GetAndExist<bool, bool>(Fl_Preferences &prefs, const std::string &name, bool &item, const bool def){
    int d = def, out = 0;
    if(!prefs.get(name.c_str(), out, d))
      prefs.set(name.c_str(), d);

    item = out;
}

}
