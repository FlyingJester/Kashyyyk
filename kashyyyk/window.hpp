#pragma once

#include <list>
#include <vector>
#include <memory>
#include <mutex>

#include "autolocker.hpp"

#include <FL/Fl_Tree_Prefs.H>

class Fl_Window;
class Fl_Group;
class Fl_Tree;
class Fl_Tree_Item;

namespace Kashyyyk {

  class Server;
  class Channel;
  class TaskGroup;

  class WindowCallbacks{
public:

      static void ChangeNick_CB(Fl_Widget *, void *);
      static void JoinChannel_CB(Fl_Widget *, void *);
      static void ChannelList_CB(Fl_Widget *, void *);

  };

  class Window {
public:
      TaskGroup *task_group;

protected:

      std::unique_ptr<Fl_Window> widget;
      std::unique_ptr<Fl_Group>  chat_holder;
      std::unique_ptr<Fl_Tree>   channel_list;

      std::mutex mutex;

      inline void lock(){mutex.lock();}
      inline void unlock(){mutex.unlock();}

      bool osx_style;

      Fl_Tree_Prefs prefs;

      Server *last_server;

public:
      friend class AutoLocker<Window *>;
      friend class WindowCallbacks;

      Window();
      Window(int w, int h, TaskGroup *g, bool osx = false);
      ~Window();

      std::list<Channel *> Channels;
      std::list<std::unique_ptr<Server> > Servers;

      void AddServer(Server *);
      void RemoveChannel(Channel *);

      void SetChannel(Channel *);

       // Only call on the main thread.
      void Show();
      void Hide();

       // Remember to call Fl::lock() before calling these on other threads.
      void RedrawChannels();
      void RedrawChat();
      void Redraw();

      Fl_Tree_Item *FindChannel(const char *);

      inline const Fl_Window *Handle(){
          return widget.get();
      }

  };

}
