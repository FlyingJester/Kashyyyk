#pragma once
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Hold_Browser.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl.H>
#include <Fl/fl_ask.H>
#include <memory>
#include <utility>
#include <cstdlib>

namespace Kashyyyk {

template <class T = Fl_Hold_Browser, int H = 16>
class EditList : public Fl_Group {
public:
    typedef EditList<T, H> this_type;
    typedef std::pair<const char *, void *> ItemType;
    typedef ItemType(*ItemCallback)(ItemType in);
protected:


    int pad;

    Fl_Button AddButton;
    Fl_Button DelButton;
    Fl_Button EdtButton;

    T list;

    ItemCallback AddCallback;
    ItemCallback DelCallback;

    static void Edt_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);

        int i = that->list.value();

        const char *c = fl_input("Rename Server %s", that->list.text(i), that->list.text(i));
        if(c)
          that->list.text(i, c);

    }

    static void Add_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);

        ItemType t = {"New Item", nullptr};

        if(that->AddCallback)
          t = that->AddCallback(t);

        that->list.add(t.first, t.second);

        if(that->list.size()==1)
          that->list.select(1);

        that->list.redraw();

    }

    static void Del_CB(Fl_Widget *w, void *p){

        this_type *that = static_cast<this_type *>(p);

        int sel = that->list.value();

        if(that->DelCallback){
            that->DelCallback({that->list.text(sel), that->list.data(sel)});
        }

        that->list.remove(sel);
        that->list.redraw();

        if(that->list.size()==0)
          return;

        sel = std::min<int>(sel, that->list.size());

        that->list.select(sel);

    }

public:

    EditList(int x, int y, int w, int h, const char *label = 0, int p = 4)
      : Fl_Group(x, y, w, h, label)
      , pad(p)
      , AddButton(x+pad, h+pad, (w-(pad*3))/3, H, "Add New")
      , DelButton(((w-pad)/3)+x+pad, h+pad, (w-(pad*3))/3, H, "Remove")
      , EdtButton(((w-pad)*2/3)+x+pad, h+pad, (w-(pad*3))/3, H, "Edit")
      , list(x+pad, y+pad, w-(pad*2), h-H-2-(pad*2)-2)
      , AddCallback(nullptr)
      , DelCallback(nullptr) {

        // Set the appearance as if the entire widget, except for
        // the add and del buttons, is a single big box.
        box(FL_DOWN_BOX);
        list.box(FL_NO_BOX);
        list.color(FL_BACKGROUND2_COLOR);

        AddButton.callback(Add_CB, this);
        DelButton.callback(Del_CB, this);
        EdtButton.callback(Edt_CB, this);

    }

    virtual ~EditList(){}

    void SetAddCallback(ItemCallback a){
        AddCallback = a;
    }

    void SetDelCallback(ItemCallback a){
        DelCallback = a;
    }

    const ItemType GetItem(){
        const int sel = list->selected();
        return {list->text(sel), list->data(sel)};
    }

};

}
