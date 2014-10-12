#include "notification.h"

#include <stdio.h>
#include <string.h>

#define STR_EMPTY_IF_NULL(S) ((S==NULL)?"":S)

static FILE *notificationlog = NULL;

static const char * const logfile_name = "Kashyyyk_Notifications.log";
static const char * const logfile_mode = "a";

enum NotificationSuccess Kashyyyk_InitNotifications(){
  /* If we were already open, reopen the file.
  */
    if(notificationlog!=NULL)
      notificationlog = freopen(logfile_name, logfile_mode, notificationlog);
    else
      notificationlog = fopen(logfile_name, logfile_mode);

    if(notificationlog==NULL)
      return eNoteFailure;

    return eNoteSuccess;
}

enum NotificationSuccess Kashyyyk_CloseNotifications(){
    if(notificationlog==NULL)
      return eNoteNotInitialized;

    fflush(notificationlog);
    fclose(notificationlog);
    return eNoteSuccess;
}

#define NOTE_ERROR "Error writing notification"

static void CheckError(){
    if(ferror(notificationlog)){
        perror(NOTE_ERROR);
        clearerr(notificationlog);
    }
    else if(feof(notificationlog)){
      fprintf(stderr, NOTE_ERROR ". Premature EOF.\n");
      notificationlog = freopen(logfile_name, logfile_mode, notificationlog);
    }
    else
      fprintf(stderr, NOTE_ERROR ".\n");
}


static int CheckError_W(unsigned long err, unsigned long len){
  if((err==0) || (err<len)){
      fprintf(stderr, "String write error.\n");
      CheckError();
      return 1;
  }
  return 0;
}

static int CheckError_C(unsigned long err){
  if(err==EOF){
      fprintf(stderr, "Char write error.\n");
      CheckError();
      return 1;
  }
  return 0;
}

enum NotificationSuccess Kashyyyk_GiveNotification(const char *title, const char *message){

    const char * const title_c = STR_EMPTY_IF_NULL(title);
    const char * const msg_c = STR_EMPTY_IF_NULL(message);

    const unsigned long title_len = strlen(title_c);
    const unsigned long msg_len = strlen(msg_c);

    int err;
    unsigned long count;

    if(notificationlog==NULL)
      return eNoteNotInitialized;

    if(title_len>0){
        count = fwrite(title_c, 1, title_len, notificationlog);
        if(CheckError_W(count, title_len))
          return eNoteFailure;

    }

    if((title_len>0) && (msg_len>0)){
        err = fputc('|', notificationlog);
        if(CheckError_C(err))
          return eNoteFailure;
    }

    if(msg_len>0){
        count = fwrite(msg_c, 1, msg_len, notificationlog);
        if(CheckError_W(count, msg_len))
          return eNoteFailure;
    }

    if((title_len>0) || (msg_len>0)){
        err = fputc('\n', notificationlog);
        if(CheckError_C(err))
          return eNoteFailure;

        fflush(notificationlog);
    }

    return eNoteSuccess;
}
