#pragma once

#include "reciever.hpp"
#include "autolocker.hpp"

#include <list>
#include <vector>
#include <memory>
#include <string>
#include <mutex>

class Fl_Group;
class Fl_Browser;
class Fl_Text_Display;
class Fl_Text_Buffer;
class Fl_Tree_Item;
class Fl_Output;

namespace Kashyyyk{

struct User {
  std::string Name;
  std::string Mode;
};

class Server;

class Channel : public TypedReciever<Server>{

    struct StyleTable;
    static struct StyleTable table;

    std::unique_ptr<Fl_Group> widget;

    Fl_Output *topiclabel;
    Fl_Browser *userlist;
    Fl_Text_Display *chatlist;
    Fl_Text_Buffer *buffer;
    Fl_Text_Buffer *stylebuffer;

    bool focus;
    int font, last_msg_type;

    Fl_Tree_Item *GetWindowItem();

    std::mutex mutex;

    // Used for aligning usernames with messages in the chat box.
    unsigned alignment;

    inline void lock(){mutex.lock();}
    inline void unlock(){mutex.unlock();}

public:
    friend class Server;
    friend class AutoLocker<Channel *>;


    Channel(Server *, const std::string &channel_name);
    ~Channel();

    std::string name;

    std::list<User> Users;

    virtual void GiveMessage(IRC_Message *msg) override;
    virtual void SendMessage(IRC_Message *msg) override;

    void GiveFocus();
    void LoseFocus();

    void Show();
    void Hide();

    Server *server(){return Parent;}

    inline bool HasFocus(){return focus;}

    enum HighlightLevel {Low, Medium, High};

    void Highlight(HighlightLevel = Low);

    void FocusChanged();

    void GetPath(std::string &path) const;

    void SetTopic(const char *topic);
    inline void SetTopic(const std::string &topic){SetTopic(topic.c_str());}

    void AddUser(const char *user, const char *mode);
    void AddUser(const struct User &user);

    void RemoveUser(const char *user);

    const char *Nick();

    static void TextModify_CB(int, int, int, int, const char*, void*);

};

}
