#pragma once

//! @file
//! @brief Definition of @link Kashyyyk::Channel @endlink
//! @author    FlyingJester
//! @date      2014
//! @copyright GNU Public License 2.0

#include "reciever.hpp"
#include "autolocker.hpp"
#include "monitor.hpp"

#include <list>
#include <vector>
#include <memory>
#include <string>
#include <cmath>
#include <cassert>

class Fl_Group;
class Fl_Browser;
class Fl_Text_Display;
class Fl_Text_Buffer;
class Fl_Tree_Item;
class Fl_Output;

#ifdef High
#undef High
#endif

#ifdef Medium
#undef Medium
#endif

#ifdef Low
#undef Low
#endif

#ifdef None
#undef None
#endif

#ifdef SendMessage
#undef SendMessage
#endif


namespace Kashyyyk{

//! @brief User Structure
//!
//! Stores a user name and mode.
//! Currently, Mode is meaningless.
struct User {
    //! User's name. This includes the Mode character, if applicable.
    std::string name;
    //! Currently, Mode is meaningless, and will just be an empty string.
    //! Mode signifiers are prepended to the Name.
    std::string mode;
};

class Server;

//!
//! @brief IRC Channel
//!
//! Represents an IRC Channel. It is strongly owned by a Kashyyyk::Server.
//! Messages sent to this class using SendMessage are passed on to the
//! owning Kashyyyk::Server.
//! Channels should be constructed to show that the server has accepted the
//! client joining the channel. As such, Channels manage the GUI elements that
//! they use, and can recieve messages from the Kashyyyk::Server.
//! They do not maintain or act upon the status of the clients membership to
//! the channel, which is the responsibility of the owning Kashyyyk::Server.
//!
//! @sa Kashyyyk::User
//! @sa Kashyyyk::Server
//! @sa Kashyyyk::Window
//! @sa Kashyyyk::Reciever
class Channel : public LockingReciever<Server, Monitor>{

    struct StyleTable;
    //! Used internally to format text for the chat box
    static struct StyleTable table;

    //! Containing group for all related widgets to the channel.
    //! The lifetime of this widget controls the lifetime of
    std::unique_ptr<Fl_Group> widget;

    //! Topic for the channel
    Fl_Output *topiclabel;
    //! User list for the channel
    Fl_Browser *userlist;
    //! The main chat box
    Fl_Text_Display *chatlist;
    //! Text buffer for the chat box
    Fl_Text_Buffer *buffer;
    //! Style buffer for the chat box
    Fl_Text_Buffer *stylebuffer;

    bool focus;
    int font;
/*
    //! @brief Gets the item that represents the channel
    //!
    //! Gets the item from the owning Server's owning Window's channel_list.
    Fl_Tree_Item *GetWindowItem();
*/
    //! @brief Used for aligning usernames with messages in the chat box
    unsigned alignment;

public:
    friend class Server;
    friend class AutoLocker<Channel *>;

    //! Constructs a Channel and adds it to a server.
    Channel(Server *server, const std::string &channel_name);
    ~Channel();

    //! Server's name. Includes any type (#, &, etc.) prefix.
    std::string name;

    //! @brief All active users.
    //! @warning You must lock this Channel before modifying or iterating this member.
    std::list<User> Users;
    
    void GiveMessage(IRC_Message *msg) override;
    
    //!
    //! @brief Send a Message from this Channel, through the owning Server.
    //!
    //! Results in @p msg being written to the owning server's socket.
    //! Can affect the Channel and the owning Server. For instance, a JOIN
    //! message causes the owning Server to begin listening for responding
    //! JOIN messages. A message of PART will indirectly cause the Channel
    //! to be destroyed, assuming the PART response from the Server is
    //! recieved.
    //!
    //! @param msg Message to write, freed by caller
    virtual void SendMessage(IRC_Message *msg) override;

    //! The method that is called when Show occurs. Can be called independantly
    //! to simulate a Show event.
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    void GiveFocus();
    //! The method that is called when Hide occurs. Can be called independantly
    //! to simulate a Show event.
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    void LoseFocus();

    //! Bring the chatbox to the front for the Server
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    void Show();
    //! Hide the chatbox
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    void Hide();

    //! Get owning Server
    Server *server(){return Parent;}

    //! Get owning Server
    inline bool HasFocus(){return focus;}

    //! @enum HighlightLevel
    //! @brief Highlight options
    //! @sa Highlight
    enum HighlightLevel {
      None,   //!< No highlighting
      Low,    //!< Some highlighting
      Medium, //!< More highlighting
      High    //!< All the highlighting
    };

    //!
    //! @brief Highlights the Channel in the channel list of the server
    //!
    //! The @p level controls the behaviour.
    //! @li A level of @link None @endlink causes the method to do nothing.
    //! @li @link Low @endlink and @link Medium @endlink cause the channel to
    //! be  highlighted in different colors in the Server's channel tree, the
    //! color indicating the level.
    //! @li A level of @link High @endlink is the same as @link Medium @endlink
    //! except that it also causes the owning Server's owning Window to flash
    //! for the user's attention.
    //!
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    //!
    //! @param level Level of highlighting.
    //!
    //! @sa Kashyyyk::Server::Highlight
    void Highlight(HighlightLevel level = Low);
    //! Called to indicate that the focus has changed. Results in the chat GUI
    //! redrawing if it is active.
    void FocusChanged();

    //! @brief Get '/' delimited Channel/Server hierarchy
    //!
    //! Gets the Server/Channel hierarchy, in a '/' delimited string. This will
    //! usuall be in the format '/irc.server.net/#channel', and is suitable
    //! for use with the owning Server's owning Window's channel list tree.
    //!
    //! @param [out] path Resulting path is placed here
    void GetPath(std::string &path) const;

    //! Sets the Topic in the topic widget.
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    void SetTopic(const char *topic);
    //! @overload
    inline void SetTopic(const std::string &topic){SetTopic(topic.c_str());}

    void WriteLine(const char *from, const char *msg);

    //! @brief Adds a User to the Channel
    //!
    //! Adds a user to the channel. This does not generate a message in the
    //! chatbox about the user joining, although it does add the user to the
    //! user list widget.
    //!
    //! It is recommended to call SortUsers after adding Users.
    //!
    //! @warning This function locks the Channel! If you already have locked
    //! the channel, used the the nonlocking variant @link AddUsers_l @endlink
    //!
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    //!
    //! @sa SortUsers
    //! @sa AddUser_l
    void AddUser(const struct User &user);
    //! @overload
    //! @brief Equivalent to @link AddUser @endlink @p ({ @p user, @p mode})
    void AddUser(const char *user, const char *mode);

    //! @brief Nonlocking version of AddUser
    //!
    //! This method should only be used if the Channel has previously been locked.
    //!
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    //!
    //! @sa AddUser
    void AddUser_l(const struct User &user);
    //! @overload
    //! @brief Equivalent to @link AddUser_l @endlink @p ({ @p user, @p mode})
    void AddUser_l(const char *user, const char *mode);

    //! @brief Sorts the usernames in the userlist alphabetically
    //!
    //! Sorts the usernames in the userlist alphabetically. Generally this
    //! should be done after calls to @link AddUser @endlink or
    //! @link AddUser_l @endlink . If many users will be added at once, it is
    //! better to add all the users and then call this method only once when
    //! you are done adding users. This function does not change the order of
    //! Users.
    //!
    //! @warning This function locks the Channel! If you already have locked
    //! the channel, used the the nonlocking variant @link SortUsers_l @endlink
    //!
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread! See
    //! http://fltk.org/doc-1.3/group__fl__multithread.html
    //!
    //! @note Sorting follows UTF code points. This puts ~ at the end of the
    //! basic ASCII code points. This has the unfortunate side effect of making
    //! channel owners show up after all users with names starting with an
    //! ASCII character, including all uses with explicit mode characters in
    //! their names.
    //! @sa SortUsers
    void SortUsers();

    //! @brief Nonlocking version of SortUsers
    //!
    //! This method should only be used if the Channel has previously been locked.
    //!
    //! @warning You must use Fl::lock if you are before calling this if you
    //! not on the main thread!
    //!
    //! @sa SortUsers
    void SortUsers_l();

    //! @brief Removes a user from the Channel
    void RemoveUser_l(const char *user);

    //! @brief Removes a user from the Channel
    void RemoveUser(const char *user);

    //! @brief Get the nickname for this Channel
    const char *GetNick();

    //! @brief Disables the channel's widgets
    //! 
    //! This is primarily used when a server has disconnected.
    //! @sa Enable
    void Disable();
    
    //! @brief Enables the channel's widgets
    //! 
    //! This is primarily used when a server has been disconnected, and is now reconnected.
    //! @sa Disable
    void Enable();
    
    static void TextModify_CB(int, int, int, int, const char*, void*);

    //! @brief Functional-style object for finding certain Users in a Channel
    //!
    //! This class is intended to be used with std::find_if and Users.
    //! Used to search for a User in UserList by name. Does not consider Mode.
    //! Should not be used after the string or User used to construct the
    //! find_user is freed.
    //! @sa Kashyyyk::User
    class find_user {
        const std::string &n;
    public:
        find_user(const std::string &s);
        find_user(const User *);
        bool operator () (const User &);
    };

    //! @brief Send a Pling to the Parent
    //!
    //! This will cause the owning Window to Pling.
    //! @sa Server::Pling
    //! @sa Window::Pling
    void Pling();

    int last_msg_type;

};

}
