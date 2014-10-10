#pragma once

#include "user.h"
#include "message.h"

struct IRC_Message *IRC_CreateUserPrivMessage(struct IRC_User *a, const char *message);
