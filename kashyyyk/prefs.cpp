#include "prefs.hpp"
#include <FL/Fl_Window.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>

#include <FL/Fl_Preferences.H>
#include <FL/Fl.H>
#include <FL/Fl_Color_Chooser.H>

#include "platform/paths.h"

#define ADD_FONT_ITEM(TO, NAME, VALUE)\
{\
  int _i = TO->add(NAME);\
  int *_p = new int; *_p = VALUE;\
  const_cast<Fl_Menu_Item *>(TO->menu())[_i].user_data(_p);\
}

//! Meta-data for a font face
struct FontInfo {
  bool Italic; //!< Is bold
  bool Bold;   //!< Is italic
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

template<void(*func)(uchar, uchar, uchar)>
void Prefs_color_CB(Fl_Widget *w, void *p){
    static uchar r, g, b;
    int a = fl_color_chooser("Select a Color", r, g, b);

    if(a==0)
      return;

    func(r, g, b);

    Fl::flush();

}


static void Pling_Button_CB(Fl_Widget *w, void *p){

    Fl_Button *b = static_cast<Fl_Button *>(w);

    int do_pling = b->value();

    Kashyyyk::GetPreferences().set("sys.pling.enabled", do_pling);

}


Fl_Preferences &Kashyyyk::GetPreferences(){
    printf("Using config directory %s.\n", Kashyyyk_ConfigDirectory());
    static Fl_Preferences prefs(Kashyyyk_ConfigDirectory(), "FlyingJester", "Kashyyyk");
    return prefs;
}

void Kashyyyk::OpenPreferencesWindow(){

    static bool first = true;

    static Fl_Window *window = new Fl_Window(800, 600, "Preferences");
    if(first){
        Fl_Preferences &prefs = Kashyyyk::GetPreferences();
        Fl_Pack *Left_Column;
        { // window is active

            first = false;

            Fl_Button * OK = new Fl_Button(8, 600-40, 128, 32, "OK");
            OK->callback(Prefs_OKButton_CB, window);

            Left_Column = new Fl_Pack(8, 32, 200, 600-40-16);
            Left_Column->spacing(24);
            Left_Column->end();
            window->end();
        }// window is active

        Fl_Pack *gfx_group = new Fl_Pack(0, 0, 32, 128, "Appearance");
        gfx_group->box(FL_EMBOSSED_FRAME);

        gfx_group->add(new Fl_Box(0, 0, 0, 24, "Theme"));
        //theme_label->box(FL_FLAT_BOX);
        Fl_Choice * theme_input = new Fl_Choice(0, 0, 0, 24);
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

        new Fl_Box(0, 0, 0, 24, "Font (Requires Restart)");
        Fl_Choice * font_input = new Fl_Choice(0, 0, 0, 24);
        font_input->callback(Prefs_Font_CB, nullptr);
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

        {
            new Fl_Box(0, 0, 0, 24);
            new Fl_Pack(0, 0, 1, 24, "Color Scheme");

            Fl_Button * bkg1 = new Fl_Button(0, 0, 1, 24, "Background 1");
            bkg1->callback(Prefs_color_CB<Fl::background>, nullptr);
            bkg1->color(FL_BACKGROUND_COLOR);

            Fl_Button * bkg2 = new Fl_Button(0, 0, 1, 24, "Background 2");
            bkg2->callback(Prefs_color_CB<Fl::background2>, nullptr);
            bkg2->color(FL_BACKGROUND2_COLOR);

            Fl_Button * fgnd = new Fl_Button(0, 0, 1, 24, "Foreground");
            fgnd->callback(Prefs_color_CB<Fl::foreground>, nullptr);
            fgnd->color(FL_FOREGROUND_COLOR);
            fgnd->labelcolor(FL_BACKGROUND2_COLOR);

            gfx_group->begin();
        }
        gfx_group->end();

        Fl_Pack *note_group = new Fl_Pack(0, 0, 32, 128, "Notifications");
        note_group->box(FL_EMBOSSED_FRAME);
        note_group->begin();
        {
            Fl_Light_Button *pling_button = new Fl_Light_Button(0, 0, 80, 24, "Plings");
            int do_pling = 1;
            prefs.get("sys.pling.enabled", do_pling, do_pling);
            pling_button->value(do_pling);
            pling_button->align(FL_ALIGN_CENTER);
            pling_button->callback(Pling_Button_CB, nullptr);
        }
        note_group->end();
        Left_Column->add(gfx_group);
        Left_Column->add(note_group);

    }
    window->show();
}
