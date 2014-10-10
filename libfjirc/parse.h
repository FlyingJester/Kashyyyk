#pragma once

struct IRC_ParseState;
struct IRC_Message;

/* IRC_inProgress means all is well, plox continue.
 IRC_error means an unrecoverable issue has occurred.
 IRC_unexpectedEnd means the text has ended without a real message ending.
 IRC_badMessage means the message couldn't be read (maybe it was gibberish,
   maybe we just don't know what it means). You can still continue calling
   consume.
 IRC_finished means the text has been fully processed successfully.
*/
enum IRC_parseStatus {IRC_inProgress, IRC_error, IRC_unexpectedEnd,
  IRC_badMessage, IRC_finished};

#ifdef __cplusplus
extern "C" {
#endif

/* Creates a parse state from an input string.
*/
struct IRC_ParseState *IRC_InitParse(const char *input);

/* Needed to stitch together an unexpected ending and new input.
*/
struct IRC_ParseState *IRC_StitchParse(struct IRC_ParseState * old, const char *input);
/* This will return messages until it has consumed the entire string.
 It will return NULL when it has completed. NULL can also indicate an error.

 It is recommended you check the status of the ParseState whenever you get a
 NULL from ConsumeParse. If you get anythign except a NULL, the status should
 be IRC_inProgress.
 Anything else indicates a bug.

 In a perfect world, you would just keep getting IRC_Messages from consume,
 the status would just keep being IRC_inProgress, until a NULL finally was
 returned and the status was IRC_finished.

*/

struct IRC_Message *IRC_ConsumeParse(struct IRC_ParseState *state);

enum IRC_parseStatus IRC_GetParseStatus(struct IRC_ParseState *state);

/* Clean up the state.
*/
void IRC_DestroyParseState(struct IRC_ParseState *state);

#ifdef __cplusplus
}
#endif
