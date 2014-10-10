#pragma once

struct IRC_User {
    const char *name;
    const char *server;
    const char *mode;
};

struct IRC_UserHolder {
    struct IRC_User *user;
    void *next;
    void *prev;
    void *userdata;
};

struct IRC_UserHolder *IRC_UserNext(struct IRC_UserHolder *a);
struct IRC_UserHolder *IRC_UserPrev(struct IRC_UserHolder *a);
struct IRC_User *IRC_UserUnwrap(struct IRC_UserHolder *a);

