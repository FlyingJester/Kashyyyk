#pragma once

/* A state generally applies to a single server connection.
*/

struct IRC_ChannelHolder;

struct IRC_State {
    const char *name;
    struct IRC_ChannelHolder *channels;

    void *user; /* applicatin-defined data to associate with this state. */
};

int IRC_StateContainsUser(struct IRC_State *state, const char *name);
