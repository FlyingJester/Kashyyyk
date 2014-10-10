#pragma once
#include "message.h"
#include "state.h"

/* Checks if an incoming message is applicable to the given state
*/
int IRC_CanApplyMessage(struct IRC_State *state, struct IRC_Message *msg);

void IRC_ApplyMessage(struct IRC_State *state, struct IRC_Message *msg);
