#pragma once

struct IRC_Message;

#ifdef __cplusplus
extern "C" {
#endif

struct IRC_Message *IRC_GenerateMessage(const char *to, const char *input);

#ifdef __cplusplus
}
#endif
