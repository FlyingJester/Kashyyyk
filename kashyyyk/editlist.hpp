#pragma once
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Pack.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Scroll.H>
#include <Fl/Fl.H>
#include <memory>
#include <cstdlib>

namespace Kashyyyk {

template <class T = Fl_Pack, class I = Fl_Input, int H = 16>
class EditList : public Fl_Group {
public:
    typedef void(*ItemCallback)(I *in, EditList<T, I, H> *that);
protected:

    int pad;

    Fl_Button AddButton;
    Fl_Button DelButton;

    Fl_Scroll scroller;
    T list;

    I *last_item;

    ItemCallback AddCallback;
    ItemCallback DelCallback;

    static void Add_CB(Fl_Widget *w, void *p){
        EditList<T, I, H> *that = static_cast<EditList<T, I, H> *>(p);

        that->list.resize(that->x()+1, that->y()+1,
                          that->w()-2-that->scroller.scrollbar_size(),
                          that->h()-H-2-that->pad);

        I *i = new I(0, 0, that->list.w(), H);

        if(that->AddCallback)
          that->AddCallback(i, that);

        that->list.add(i);
        that->list.redraw();
        that->redraw();

        that->last_item = i;

    }

    static void Del_CB(Fl_Widget *w, void *p){

        EditList<T, I, H> *that = static_cast<EditList<T, I, H> *>(p);

        if(!that->last_item)
          return;

        if(that->DelCallback)
          that->DelCallback(that->last_item, that);

        that->list.remove(that->last_item);
        that->list.redraw();
        that->redraw();

        delete that->last_item;
        that->last_item = nullptr;

    }

public:

    EditList(int x, int y, int w, int h, const char *label = 0, int p = 4)
      : Fl_Group(x, y, w, h, label)
      , pad(p)
      , AddButton(x+pad, h+pad, (w-(pad*2))/2, H, "Add New")
      , DelButton(((w-pad)/2)+x+pad, h+pad, (w-(pad*2))/2, H, "Remove")
      , scroller(x+1, y+1, w-2, h-H-2-pad)
      , list(x+1, y+1, w-2, h-H-2-pad)
      , last_item(nullptr)
      , AddCallback(nullptr)
      , DelCallback(nullptr) {

        box(FL_DOWN_BOX);
        list.box(FL_DOWN_FRAME);

        scroller.scrollbar_size(Fl::scrollbar_size());
        scroller.type(Fl_Scroll::VERTICAL_ALWAYS);
        list.resize(x+1, y+1, w-2-scroller.scrollbar_size(), h-H-2-pad);

        AddButton.callback(Add_CB, this);
        DelButton.callback(Del_CB, this);

    }

    virtual ~EditList(){}

    void SetAddCallback(ItemCallback a){
        AddCallback = a;
    }

    void SetDelCallback(ItemCallback a){
        DelCallback = a;
    }

    size_t GetNumItems(){
        return list.children();
    }

    const I *GetItems(){
        return list.array();
    }
    const I *GetItem(size_t index){
        return list.child(index);
    }

};

}
