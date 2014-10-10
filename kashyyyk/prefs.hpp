#pragma once

class Fl_Widget;
class Fl_Preferences;

namespace Kashyyyk {

Fl_Preferences &GetPreferences();

void OpenPreferencesWindow();
inline void OpenPreferencesWindow_CB(Fl_Widget *w, void *p){
    OpenPreferencesWindow();
}

}
