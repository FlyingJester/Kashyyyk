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
    typedef ItemType(*ItemCallback)(ItemType in, void *arg);
    typedef void(*NumCallbackT)(int n, void *arg);
protected:


    int pad;

    Fl_Button AddButton;
    Fl_Button DelButton;
    Fl_Button EdtButton;

    T list;

    ItemCallback AddCallback;
    void *AddCallbackArg;
    ItemCallback DelCallback;
    void *DelCallbackArg;
    ItemCallback SelCallback;
    void *SelCallbackArg;
    NumCallbackT  NumCallback;
    void *NumCallbackArg;

    static void Edt_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);

        int i = that->list.value();

        const char *c = fl_input("Rename Server %s", that->list.text(i), that->list.text(i));
        if(!c)
          return;

        that->list.text(i, c);

    }

    static void Add_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);
        assert(that);

        char *n_text = strdup("New Item");

        ItemType proto = {n_text, nullptr};
        if(that->AddCallback)
          proto = that->AddCallback(proto, that->AddCallbackArg);

        if((proto.first==nullptr) && (proto.second==nullptr)){
            return;
        }

        that->AddItem(proto);

        free((void *)proto.first);

        that->NumItemsChanged();

        if(that->list.size()==1)
          that->list.select(1);

    }

    static void Del_CB(Fl_Widget *w, void *p){

        this_type *that = static_cast<this_type *>(p);

        int sel = that->list.value();

        if(that->DelCallback){
            that->DelCallback({that->list.text(sel), that->list.data(sel)}, that->DelCallbackArg);
        }

        that->list.remove(sel);
        that->list.redraw();

        that->NumItemsChanged();

        if(that->list.size()==0)
          return;

        sel = std::min<int>(sel, that->list.size());

        that->list.select(sel);

    }

    static void Sel_CB(Fl_Widget *w, void *p){

        this_type *that = static_cast<this_type *>(p);

        int sel = that->list.value();

        if(sel==0)
          return;

        if(that->SelCallback)
            that->SelCallback({that->list.text(sel), that->list.data(sel)}, that->SelCallbackArg);

    }

    void NumItemsChanged(){
        list.redraw();

        if(list.size()==0){
            DelButton.deactivate();
            EdtButton.deactivate();
        }
        else{
            DelButton.activate();
            EdtButton.activate();
        }

        if(NumCallback)
          NumCallback(list.size(), NumCallbackArg);

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
      , AddCallbackArg(nullptr)
      , DelCallback(nullptr)
      , DelCallbackArg(nullptr)
      , SelCallback(nullptr)
      , SelCallbackArg(nullptr)
      , NumCallback(nullptr)
      , NumCallbackArg(nullptr) {

        // Set the appearance as if the entire widget, except for
        // the add and del buttons, is a single big box.
        box(FL_DOWN_BOX);
        list.box(FL_NO_BOX);
        list.color(FL_BACKGROUND2_COLOR);

        assert(this);

        AddButton.callback(Add_CB, this);
        DelButton.callback(Del_CB, this);
        EdtButton.callback(Edt_CB, this);

        list.callback(Sel_CB, this);

        NumItemsChanged();

        begin();

    }

    virtual ~EditList(){}

    inline void SetAddCallback(ItemCallback cb, void *arg = nullptr){
        AddCallback = cb;
        AddCallbackArg = arg;
    }

    inline void SetDelCallback(ItemCallback cb, void *arg = nullptr){
        DelCallback = cb;
        DelCallbackArg = arg;
    }

    inline void SetSelCallback(ItemCallback cb, void *arg = nullptr){
        SelCallback = cb;
        SelCallbackArg = arg;
    }

    inline void SetNumCallback(NumCallbackT cb, void *arg = nullptr){
        NumCallback = cb;
        NumCallbackArg = arg;
    }

    inline int GetNumItems(){
        return list.size();
    }

    inline const ItemType GetItem(){
        const int sel = list->selected();
        return {list->text(sel), list->data(sel)};
    }

    inline void AddItem(ItemType item){
        list.add(item.first, item.second);
        if(list.size()==1){
            list.value(1);
            list.do_callback();
        }
        NumItemsChanged();
    }

    // If one of the parts of `item' is NULL, then only the other item is
    // looked up.
    // Returns true if the item actually was found.
    inline bool RemoveItem(ItemType item){
        bool found = false;

        for(int i = 1; i<list.size(); i++){

            if((item.first==nullptr) && (item.second==nullptr))
              break;

            if(((item.first==nullptr) || (strcmp(list.text(i), item.first)==0))
               && ((item.second==nullptr) || (item.second==list.data(i)))){
                  found = true;
                  list.remove(i);
                  break;
               }
        }

        NumItemsChanged();
        return found;
    }

    inline void Deactivate(){

        AddButton.deactivate();
        DelButton.deactivate();
        EdtButton.deactivate();
        list.deactivate();
    }

    inline void Activate(){

        AddButton.activate();

        if(list.size()>0){
            DelButton.activate();
            EdtButton.activate();
        }

        list.activate();

    }

    inline void Clear(){
        list.clear();
        NumItemsChanged();
    }

    inline void SetFocusCallback(Fl_Callback cb, void *arg = nullptr){
        list.callback(cb, arg);
    }

};

}
