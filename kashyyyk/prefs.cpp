#include "prefs.hpp"
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>

/*
 const Fl_Font FL_HELVETICA              = 0;
 const Fl_Font FL_HELVETICA_BOLD         = 1;
 const Fl_Font FL_HELVETICA_ITALIC       = 2;
 const Fl_Font FL_HELVETICA_BOLD_ITALIC  = 3;
 const Fl_Font FL_COURIER                = 4;
 const Fl_Font FL_COURIER_BOLD           = 5;
 const Fl_Font FL_COURIER_ITALIC         = 6;
 const Fl_Font FL_COURIER_BOLD_ITALIC    = 7;
 const Fl_Font FL_TIMES                  = 8;
 const Fl_Font FL_TIMES_BOLD             = 9;
 const Fl_Font FL_TIMES_ITALIC           = 10;
 const Fl_Font FL_TIMES_BOLD_ITALIC      = 11;
 const Fl_Font FL_SYMBOL                 = 12;
 const Fl_Font FL_SCREEN                 = 13;
 const Fl_Font FL_SCREEN_BOLD            = 14;
 const Fl_Font FL_ZAPF_DINGBATS          = 15;
*/

#define ADD_FONT_ITEM(TO, NAME, VALUE)\
{\
  int _i = TO->add(NAME);\
  int *_p = new int; *_p = VALUE;\
  const_cast<Fl_Menu_Item *>(TO->menu())[_i].user_data(_p);\
}

struct FontInfo {
  bool Italic;
  bool Bold;
};


static const char * const fontNames[7] = {
    "Helvetica",
    "Courier",
    "Times",
    "Symbol",
    "Screen",
    "Zapf Dingbats",
    "(Bad Font Index)"
};

static inline const char *GetFontName(int font){

    int i = 0;
    if(font<12){
        i = font>>2;
    }
    else if (font==12)
      i = 3;
    else if (font==15)
      i = 5;
    else if (font<15)
      i = 4;
    else
      i = 6;

    return fontNames[i];

}

static void Prefs_OKButton_CB(Fl_Widget *w, void *p){
    static_cast<Fl_Window *>(p)->hide();
}


static void Prefs_Theme_CB(Fl_Widget *w, void *p){
    Fl_Choice *choice = static_cast<Fl_Choice *>(w);

    const Fl_Menu_Item *item = choice->mvalue();

    if(item!=nullptr){
        Kashyyyk::GetPreferences().set("sys.appearance.theme", item->label());
        Kashyyyk::GetPreferences().flush();
        printf("Set theme to %s\n", item->label());
        Fl::scheme(item->label());
    }
}


static void Prefs_Font_CB(Fl_Widget *w, void *p){
    Fl_Choice *choice = static_cast<Fl_Choice *>(w);
    FontInfo *info   = static_cast<FontInfo *>(p);

    const Fl_Menu_Item *item = choice->mvalue();

    if(item!=nullptr){

        int *Font_p = static_cast<int *>(item->user_data());
        int Font = Font_p[0];

        if(info!=nullptr){

            if((Font!=FL_ZAPF_DINGBATS) && (Font!=FL_SYMBOL)){
                if(info->Bold)
                  Font+=1;

                if(Font!=FL_SCREEN){
                    if(info->Italic)
                      Font+=2;
                }
            } // Not dingbats or symbol

        } // info not null

        Kashyyyk::GetPreferences().set("sys.appearance.font", Font);
        printf("Set font to %s (%i)\n", GetFontName(Font), Font);
    }

}


Fl_Preferences &Kashyyyk::GetPreferences(){
    static Fl_Preferences prefs("conf", "FlyingJester", "Kashyyyk");
    return prefs;
}

void Kashyyyk::OpenPreferencesWindow(){

    static bool first = true;

    static Fl_Window *window = new Fl_Window(800, 600, "Preferences");
    if(first){

        Fl_Preferences &prefs = Kashyyyk::GetPreferences();

        window->end();
        first = false;

        Fl_Button * OK = new Fl_Button(8, 600-40, 128, 32, "OK");
        OK->callback(Prefs_OKButton_CB, window);
        window->add(OK);

        Fl_Group *gfx_group = new Fl_Group(8, 32, 200, 128, "Appearance");
        gfx_group->box(FL_EMBOSSED_FRAME);

        gfx_group->add(new Fl_Box(16, 36, 200-16, 24, "Theme"));
        //theme_label->box(FL_FLAT_BOX);
        Fl_Choice * theme_input = new Fl_Choice(16, 34+32, 200-16, 24);
        theme_input->callback(Prefs_Theme_CB, nullptr);
        theme_input->add("none");
        theme_input->add("gtk+");
        theme_input->add("plastic");
        {
            char *theme = nullptr;
            prefs.get("sys.appearance.theme", theme, "gtk+");
            const Fl_Menu_Item *selected = theme_input->find_item(theme);
            free(theme);

            if(selected!=nullptr)
              theme_input->picked(selected);

        }

        gfx_group->add(new Fl_Box(16, 34+32+24, 200-16, 24, "Font (Requires Restart)"));
        Fl_Choice * font_input = new Fl_Choice(16, 34+32+48, 200-16, 24);
        font_input->callback(Prefs_Font_CB, nullptr);

        ADD_FONT_ITEM(font_input, fontNames[0], FL_HELVETICA);
        ADD_FONT_ITEM(font_input, fontNames[1], FL_COURIER);
        ADD_FONT_ITEM(font_input, fontNames[2], FL_TIMES);
        ADD_FONT_ITEM(font_input, fontNames[3], FL_SYMBOL);
        ADD_FONT_ITEM(font_input, fontNames[4], FL_SCREEN);
        ADD_FONT_ITEM(font_input, fontNames[5], FL_ZAPF_DINGBATS);

        {
            int font = 0;
            prefs.get("sys.appearance.font", font, FL_SCREEN);
            const Fl_Menu_Item *selected = theme_input->find_item(GetFontName(font));
            printf("Last font setting was %s (%p)\n", GetFontName(font), selected);

            if(selected!=nullptr)
              font_input->picked(selected);

        }

        gfx_group->end();
        window->add(gfx_group);

    }

    window->show();
}
