#pragma once
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Pack.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <memory>
#include <cstdlib>

namespace Kashyyyk {

template <class T = Fl_Pack, class I = Fl_Input, int H = 16>
class EditList : public Fl_Group {
public:
    typedef void(*ItemCallback)(I *in, EditList<T, I, H> *that);
protected:

    Fl_Button AddButton;
    Fl_Button DelButton;

    T list;

    I *last_item;

    ItemCallback AddCallback;
    ItemCallback DelCallback;

    static void Add_CB(Fl_Widget *w, void *p){
        EditList<T, I, H> *that = static_cast<EditList<T, I, H> *>(p);

        I *i = new I(0, 0, that->list.w(), H);

        if(that->AddCallback)
          that->AddCallback(i, that);

        that->list.add(i);
        that->last_item = i;

    }

    static void Del_CB(Fl_Widget *w, void *p){

        EditList<T, I, H> *that = static_cast<EditList<T, I, H> *>(p);

        if(!that->last_item)
          return;

        if(that->DelCallback)
          that->DelCallback(that->last_item, that);

        that->list.remove(that->last_item);
        delete that->last_item;
        that->last_item = nullptr;

    }

public:

    EditList(int x, int y, int w, int h, const char *label = 0, int pad = 4)
      : Fl_Group(x, y, w, h-H, label)
      , AddButton(x+pad, h-H+pad, (w-(pad*2))/2, H, "Add New")
      , DelButton(((w-pad)/2)+x+pad, h-H+pad, (w-(pad*2))/2, H, "Remove")
      , list(x, y, w, h-H)
      , last_item(nullptr)
      , AddCallback(nullptr)
      , DelCallback(nullptr) {

        box(FL_DOWN_BOX);

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
