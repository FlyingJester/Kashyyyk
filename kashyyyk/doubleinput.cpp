#include "doubleinput.hpp"

#include <cassert>
#include <cstring>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Box.H>
#include <FL/Fl.H>
#include <FL/fl_ask.H>

#include "platform/strdup.h"

namespace Kashyyyk {

static void No_Callback(Fl_Widget *w, void *p){}

static void OK_Callback(Fl_Widget *w, void *p){
    struct DoubleInput_Return *d = static_cast<struct DoubleInput_Return *>(p);
    d->value = 1;
}

static void Cancel_Callback(Fl_Widget *w, void *p){
    struct DoubleInput_Return *d = static_cast<struct DoubleInput_Return *>(p);
    d->value = 0;
}

struct DoubleInput_Return DoubleInput(const char *msg, const char *label1, const char *defstr1, const char *label2, const char *defstr2, std::vector<Fl_Widget *> *extra, bool cancellable){
    struct DoubleInput_Return r = {-1, defstr1, defstr2};

    Fl_Window w(400, 136);

    int extra_height = 0;

    Fl_Pack p(8, 8, 400-16, 200-16);
    p.spacing(8);

    Fl_Pack labelp(0, 0, 200, 24);
    labelp.type(Fl_Pack::HORIZONTAL);
    Fl_Box label(0, 0, 400-80-16, 24);
    label.label(msg);


    Fl_Pack in1p(0, 0, 200, 24);
    in1p.type(Fl_Pack::HORIZONTAL);
    Fl_Box in1b(0, 0, 80, 24, label1);
    Fl_Input in1(80, 64, 400-80-16, 24);
    in1p.end();
    in1.value(defstr1);

    p.add(in1p);

    Fl_Pack in2p(0, 0, 200, 24);
    in2p.type(Fl_Pack::HORIZONTAL);
    Fl_Box in2b(0, 0, 80, 24, label2);
    Fl_Input in2(80, 96, 400-80-16, 24);
    in2p.end();
    in2.value(defstr2);
    p.add(in2p);

    if(extra)
      for(std::vector<Fl_Widget *>::iterator iter = extra->begin(); iter!=extra->end(); iter++){
          p.add(*iter);
          extra_height+=(*iter)->h();
      }

    Fl_Group g(0, 0, 400, 24);
    p.add(g);

    Fl_Return_Button ok(400-160-8, g.y(), 80, 24, fl_ok);
    ok.callback(OK_Callback, &r);
    g.add(ok);
    Fl_Button cancel(400-80, g.y(), 80, 24, fl_cancel);
    cancel.callback(Cancel_Callback, &r);
    g.add(cancel);

    if(!cancellable){
        w.callback(No_Callback, nullptr);
        cancel.deactivate();
    }

    if(extra_height)
      w.resize(w.x(), w.y(), w.w(), w.h()+extra_height);

    w.show();

    do{
    
        while(Fl::wait()){
          if(r.value!=-1)
            break;
        }

        r.one = in1.value();
        r.two = in2.value();

        if(!((r.value==1) || (r.value==0))){
          if(!cancellable)
            continue;

          r.value = 0;

        }
        
    }while(0);
    
    assert(r.one);
    assert(r.two);

    r.one = strdup(r.one);
    r.two = strdup(r.two);

    return r;

}

}
