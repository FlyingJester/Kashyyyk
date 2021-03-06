#pragma once
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <algorithm>
#include <memory>
#include <utility>
#include <cstdlib>
#include <cassert>


//! @file
//! @brief Definition of EditList FLTK Widget
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0

#include "platform/strdup.h"

#ifdef _WIN32
// This include is necessary for std::min and std::max with MSVC.
#include <algorithm>
#endif

namespace Kashyyyk {

//!
//! @brief A Fl_Browser derived widget with configurable Add, Delete,
//! and Edit buttons
//!
//! Provies a Fl_Browser widget (exact type specified in template parameter,
//! Fl_Hold_Browser default) with additional Add, Delete, and Edit buttons.
//! Each button has a configurable callback.
//!
//!  \image html kashyyyk_editlist.png
//!
//! All the widgets seen in the image are a single EditList, which responds
//! as expected to events like resizing.
//!
//! @tparam T Fl_Browser derived class to base the browser on
//! @tparam H Height of the section with buttons in it
template <class T = Fl_Hold_Browser, int H = 16>
class EditList : public Fl_Group {
public:
    //! Item tuple passed to callbacks
    //! @tparam \ Text value displayed in the browser
    //! @tparam \ User data of the associated item
    typedef std::pair<const char *, void *> ItemType;
    //! Callback type for list change events
    typedef ItemType(*ItemCallback)(ItemType in, void *arg);
    typedef void(*NoReturnItemCallback)(ItemType in, void *arg);
    //! Callback type for item number change events
    typedef void(*NumCallbackT)(int n, void *arg);

//! @cond
    typedef EditList<T, H> this_type;

    class Fl_Browser_iterator {

        const Fl_Browser &browser;
        int i;

    public:

        Fl_Browser_iterator(const Fl_Browser &b, int e = 0)
          : browser(b)
          , i(e) {

        }

        Fl_Browser_iterator& operator++ (){
            i++;
            return *this;
        }

        Fl_Browser_iterator& operator-- (){
            i--;
            return *this;
        }

        template<typename T2>
        void operator+ (T2 t){
            i+=t;
            return *this;
        }

        template<typename T2>
        void operator- (T2 t){
            i-=t;
            return *this;
        }

        ItemType operator -> () const {
            assert(i>0);
            assert(i<browser.size());
            return {browser.text(i), browser.data(i)};
        }

        bool operator == (const Fl_Browser_iterator& that) const {
            return ( (i==that->i) && (browser==that->browser));
        }
    };

protected:

    int pad;

    Fl_Button AddButton;
    Fl_Button DelButton;
    Fl_Button EdtButton;

    T list;

    struct CallBacks_T {
            ItemCallback AddCallback;
            void *AddCallbackArg;
            ItemCallback EdtCallback;
            void *EdtCallbackArg;
            NoReturnItemCallback DelCallback;
            void *DelCallbackArg;
            NoReturnItemCallback SelCallback;
            void *SelCallbackArg;
            NumCallbackT  NumCallback;
            void *NumCallbackArg;
    } CallBacks;

    static void Edt_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);

        int i = that->list.value();
        ItemType proto = {that->list.text(i), that->list.data(i)};

        if(!that->CallBacks.EdtCallback)
          proto.first = fl_input("Rename Item %s", that->list.text(i), that->list.text(i));
        else
          proto = that->CallBacks.EdtCallback(proto, that->CallBacks.EdtCallbackArg);
        if(!proto.first)
            return;

        that->list.text(i, proto.first);
        that->list.data(i, proto.second);
        free((void *)proto.first);

    }

    static void Add_CB(Fl_Widget *w, void *p){
        this_type *that = static_cast<this_type *>(p);
        assert(that);

        char *n_text = strdup("New Item");

        ItemType proto = {n_text, nullptr};
        if(that->CallBacks.AddCallback)
          proto = that->CallBacks.AddCallback(proto, that->CallBacks.AddCallbackArg);

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

        if(that->CallBacks.DelCallback){
            that->CallBacks.DelCallback({that->list.text(sel), that->list.data(sel)}, that->CallBacks.DelCallbackArg);
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

        if(that->CallBacks.SelCallback)
            that->CallBacks.SelCallback({that->list.text(sel), that->list.data(sel)}, that->CallBacks.SelCallbackArg);

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

        if(CallBacks.NumCallback)
          CallBacks.NumCallback(list.size(), CallBacks.NumCallbackArg);

    }
//! @endcond

public:

    //! Constructs a new EditList
    //! @param x X location
    //! @param y X location
    //! @param w Width
    //! @param h Height
    //! @param label Label text. Can be NULL for a blank label.
    //! @param p Padding, in pixels, around the outer frame, the buttons, and
    //! the browser.
    EditList(int x, int y, int w, int h, const char *label = 0, int p = 4)
      : Fl_Group(x, y, w, h, label)
      , pad(p)
      , AddButton(x+pad, y+h-H-(pad*2), (w-(pad*3))/3, H, "Add New")
      , DelButton(((w-pad)/3)+x+pad, y+h-H-(pad*2), (w-(pad*3))/3, H, "Remove")
      , EdtButton(((w-pad)*2/3)+x+pad, y+h-H-(pad*2), (w-(pad*3))/3, H, "Edit")
      , list(x+pad, y+pad, w-(pad*2), h-H-(pad*4)) {

        memset(&CallBacks, 0, sizeof(struct CallBacks_T));

        // Set the appearance as if the entire widget, except for
        // the add and del buttons, is a single big box.
        box(FL_DOWN_BOX);
        list.box(FL_NO_BOX);
        list.color(FL_BACKGROUND2_COLOR);

        assert(this);

        begin();

        AddButton.callback(Add_CB, this);
        DelButton.callback(Del_CB, this);
        EdtButton.callback(Edt_CB, this);

        this->add(AddButton);
        this->add(DelButton);
        this->add(EdtButton);

        list.callback(Sel_CB, this);

        NumItemsChanged();


    }

    virtual ~EditList(){}

    //! @brief Set callback for the Add button
    //! @param cb Callback for Add button
    //! @param arg Argument for @p cb when used.
    inline void SetAddCallback(ItemCallback cb, void *arg = nullptr){
        CallBacks.AddCallback = cb;
        CallBacks.AddCallbackArg = arg;
    }

    //! @brief Set callback for the Delete button
    //! @param cb Callback for Delete button
    //! @param arg Argument for @p cb when used.
    inline void SetDelCallback(NoReturnItemCallback cb, void *arg = nullptr){
        CallBacks.DelCallback = cb;
        CallBacks.DelCallbackArg = arg;
    }

    //! @brief Set callback for when an item is edited with the edit button
    //! @param cb Callback for the browser
    //! @param arg Argument for @p cb when used.
    inline void SetEdtCallback(ItemCallback cb, void *arg = nullptr){
        CallBacks.EdtCallback = cb;
        CallBacks.EdtCallbackArg = arg;
    }

    //! @brief Set callback for when an item is selected
    //! @param cb Callback for the browser
    //! @param arg Argument for @p cb when used.
    inline void SetSelCallback(NoReturnItemCallback cb, void *arg = nullptr){
        CallBacks.SelCallback = cb;
        CallBacks.SelCallbackArg = arg;
    }

    //! @brief Set callback for when the number of items in the browser changes
    //! @param cb Callback taking the new number of items
    //! @param arg Argument for @p cb when used.
    inline void SetNumCallback(NumCallbackT cb, void *arg = nullptr){
        CallBacks.NumCallback = cb;
        CallBacks.NumCallbackArg = arg;
    }

    //! @return Number of items in the browser
    inline int GetNumItems() const {
        return list.size();
    }

    //! @return Selected item in the browser
    inline const ItemType GetItem() const {
        const int sel = list.value();
        return {list.text(sel), list.data(sel)};
    }


    Fl_Browser_iterator begin() const {
        return Fl_Browser_iterator(list, 0);
    }

    Fl_Browser_iterator end() const {
        return Fl_Browser_iterator(list, list.size());
    }


    const size_t Size(){
        return list.size();
    }


    inline void SetText(const Fl_Browser_iterator &iter, const char *text){
        list.text(iter.i, text);
    }


    inline void SetText(const char *text){
        const int sel = list.value();
        list.text(sel, text);
    }

    //! @brief  Add an item to the browser
    //! This will call NumItemsChanged
    inline void AddItem(ItemType item){
        list.add(item.first, item.second);
        if(list.size()==1){
            list.value(1);
            list.do_callback();
        }
        NumItemsChanged();
    }

    //! @brief Removes an item if it exists in the browser
    //!
    //! Can be based on value, user_data, or both.
    //! If one of the parts of `item' is NULL, then only the other item is
    //! used to compare.
    //! @returns Whether or not item actually was found.
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

    //! Deactivates all contained widgets, making the entire EditList
    //! unresponsive.
    inline void Deactivate(){

        AddButton.deactivate();
        DelButton.deactivate();
        EdtButton.deactivate();
        list.deactivate();
    }

    //! Activates appropriate widgets
    //! @note The Edit and Delete buttons may or may not reactivate, since they
    //! require at least one element in the EditList.
    inline void Activate(){

        AddButton.activate();

        if(list.size()>0){
            DelButton.activate();
            EdtButton.activate();
        }

        list.activate();

    }

    //! Clears content browser
    inline void Clear(){
        list.clear();
        NumItemsChanged();
    }

    //! @brief Sets the callback when the EditList changes focus.
    //!
    //! This is useful as a callback to examine the EditList when the user
    //! clicks into or out of the list
    inline void SetFocusCallback(Fl_Callback cb, void *arg = nullptr){
        list.callback(cb, arg);
    }

};

}
