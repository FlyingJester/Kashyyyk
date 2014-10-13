#include "serverlist.hpp"
#include "window.hpp"
#include "editlist.hpp"
#include <cassert>
#include <cstdio>
#include <string>

#include <memory>

#include <FL/Fl_Window.H>

namespace Kashyyyk {

// The window is associated with the last window passed as `p' into ServerList.
// ANy attempt to join a server will add it to that window.

static std::unique_ptr<Fl_Window> serverlist_window;
static Window *serverlist_associated_window;

static void Nullify(Fl_Input *in, EditList<> *){

  static int i = 0;
  in->value((std::string("Nothing!")+std::to_string(i)).c_str());
  printf("%s\n", (std::string("Nothing!")+std::to_string(i)).c_str());

  i++;

}

void ServerList(Fl_Widget *w, void *p){

    assert(p);

    serverlist_associated_window = static_cast<Window *>(p);

    static bool first = true;
    if(first){
        first = false;

        const unsigned H = (24*6)+(16*7)+(8*8);

        Fl_Window *serverlist = new Fl_Window((256*2)+(8*3), H);
        serverlist_window.reset(serverlist);

        EditList<> *list = new EditList<>(8, 24, 256, H-32, "Servers");
        list->SetAddCallback(Nullify);

    }

    serverlist_window->show();

}

}
