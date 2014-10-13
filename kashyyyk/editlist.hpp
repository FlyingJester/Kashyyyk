#pragma once
#include <Fl/Fl_Group.H>
#include <Fl/Fl_Pack.H>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Scroll.H>
#include <Fl/Fl.H>
#include <memory>
#include <list>
#include <utility>
#include <cstdlib>

namespace Kashyyyk {

template <class T = Fl_Pack, class I = Fl_Input, int H = 16>
class EditList : public Fl_Group {
public:
    typedef EditList<T, I, H> this_type;
    typedef void(*ItemCallback)(I *in, this_type *that);
protected:


    int pad;

    Fl_Button AddButton;
    Fl_Button DelButton;

    Fl_Scroll scroller;
    T list;

    I *last_item;

    ItemCallback AddCallback;
    ItemCallback DelCallback;

    Fl_Callback *ListCallback;

    static void ListCallback_Wrappers(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);

        if(that->last_item){
          that->last_item->color(FL_YELLOW);
          that->last_item->position(0, 0);
        }

        printf("Old %s, new %s\n",
            (that->last_item)?that->last_item->value():"NULL",
            static_cast<I *>(w)->value());

        that->last_item = static_cast<I *>(w);
        w->color(FL_RED);

        if(that->ListCallback)
          that->ListCallback(w, p);

        that->list.redraw();

    }

    static void Add_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);

        I *i = new I(0, 0, that->list.w(), H);

        i->box(FL_FLAT_BOX);
        i->color(FL_BACKGROUND2_COLOR);

        i->callback(ListCallback_Wrappers, that);

        i->when(FL_WHEN_NOT_CHANGED|FL_WHEN_CHANGED|FL_WHEN_RELEASE_ALWAYS|FL_WHEN_ENTER_KEY_ALWAYS);

        if(that->AddCallback)
          that->AddCallback(i, that);

        that->list.add(i);
        that->list.redraw();

        that->last_item = i;

    }

    static void Del_CB(Fl_Widget *w, void *p){

        this_type *that = static_cast<this_type *>(p);

        if(!that->last_item)
          return;

        if(that->DelCallback)
          that->DelCallback(that->last_item, that);

        int i = that->list.find(that->last_item);

        that->list.remove(i);
        that->list.redraw();

        delete that->last_item;

        if(that->list.children()==0){
            that->last_item = nullptr;
            return;
        }

        i = std::max<int>(i-1, 0);
        i = std::min<int>(i, that->list.children());

        that->last_item = static_cast<Fl_Input *>(that->list.array()[i]);

    }

public:

    EditList(int x, int y, int w, int h, const char *label = 0, int p = 4)
      : Fl_Group(x, y, w, h, label)
      , pad(p)
      , AddButton(x+pad, h+pad, (w-(pad*2))/2, H, "Add New")
      , DelButton(((w-pad)/2)+x+pad, h+pad, (w-(pad*2))/2, H, "Remove")
      , scroller(x+pad, y+pad, w-(pad*2), h-H-2-(pad*2))
      , list(x+pad, y+pad, w-(pad*2), h-H-2-(pad*2)-2)
      , last_item(nullptr)
      , AddCallback(nullptr)
      , DelCallback(nullptr)
      , ListCallback(nullptr) {

        // Set the appearance as if the entire widget, except for
        // the add and del buttons, is a single big box.
        box(FL_DOWN_BOX);
        list.box(FL_NO_BOX);
        list.color(FL_BACKGROUND2_COLOR);
        scroller.box(FL_DOWN_BOX);
        scroller.color(FL_BACKGROUND2_COLOR);

        // Setup the scroller to always have a scroll bar, and resize
        // the T to make room for the scroll bar.
        scroller.scrollbar_size(Fl::scrollbar_size());
        scroller.type(Fl_Scroll::VERTICAL_ALWAYS);
        list.resize(x+pad, y+pad+1, w-scroller.scrollbar_size()-(pad*2), h-H-(pad*4));
        list.spacing(pad>>2);

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
