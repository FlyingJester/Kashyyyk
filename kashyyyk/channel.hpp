#pragma once

#include "reciever.hpp"
#include "autolocker.hpp"

#include <list>
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <cmath>
#include <cassert>

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

class Channel : public LockingReciever<Server, std::mutex>{

    struct StyleTable;
    static struct StyleTable table;

    std::unique_ptr<Fl_Group> widget;

    Fl_Output *topiclabel;
    Fl_Browser *userlist;
    Fl_Text_Display *chatlist;
    Fl_Text_Buffer *buffer;
    Fl_Text_Buffer *stylebuffer;

    bool focus;
    int font;

    Fl_Tree_Item *GetWindowItem();

    // Used for aligning usernames with messages in the chat box.
    unsigned alignment;

public:
    friend class Server;
    friend class AutoLocker<Channel *>;

    Channel(Server *, const std::string &channel_name);
    ~Channel();

    std::string name;

    std::list<User> Users;

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

    void WriteLine(const char *from, const char *msg);

    void AddUser(const char *user, const char *mode);
    void AddUser(const struct User &user);
    void AddUser_l(const char *user, const char *mode);
    void AddUser_l(const struct User &user);

    void SortUsers();
    void SortUsers_l();

    void RemoveUser(const char *user);

    const char *Nick();

    static void TextModify_CB(int, int, int, int, const char*, void*);

     // Functional-style object for finding certain Users in a Channel
    class find_user {
        const std::string &n;
    public:
        find_user(const std::string &s);
        find_user(const User *);
        bool operator () (const User &);
    };

    int last_msg_type;

};

}
