/****************************************************************************************************************************
 * NOTE: look in "es.h" for function documentation. The ez_XXX() macros here only wrap the ES_XXX() functions
 * for the purpose of boilerplate error handling.
 */

#ifndef EZ_ES_H
#define EZ_ES_H

#include "es.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ez_ES_registerFd(fd, events, callback_f, ctxt) \
   _ez_ES_registerFd(__FILE__, __LINE__, __FUNCTION__, fd, events, callback_f, ctxt)
int _ez_ES_registerFd (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd,
      short events,
      int (*callback_f)(void *ctxt, int fd, short events),
      void *ctxt
      );



#define ez_ES_registerSignal(signum, callback_f, ctxt) \
   _ez_ES_registerSignal(__FILE__, __LINE__, __FUNCTION__, signum, callback_f, ctxt)
int _ez_ES_registerSignal (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      );

#define ez_ES_registerVSignal(signum, callback_f, ctxt) \
   _ez_ES_registerVSignal(__FILE__, __LINE__, __FUNCTION__, signum, callback_f, ctxt)
int _ez_ES_registerVSignal (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      );

#define ez_ES_VSignal(tid, signum) \
   _ez_ES_VSignal(__FILE__, __LINE__, __FUNCTION__, tid, signum)
int _ez_ES_VSignal (
      const char *fileName,
      int lineNo,
      const char *funcName,
      pthread_t tid,
      int signum
      );

#define ez_ES_registerTimer(pause_ms, interval_ms, callback_f, ctxt) \
   _ez_ES_registerTimer(__FILE__, __LINE__, __FUNCTION__, pause_ms, interval_ms, callback_f, ctxt)
int _ez_ES_registerTimer (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int64_t pause_ms,
      int64_t interval_ms,
      int (*callback_f)(void *ctxt),
      void *ctxt
      );


#define ez_ES_unregister(key) \
   {_ez_ES_unregister(__FILE__, __LINE__, __FUNCTION__, key); (key)= 0;}
int _ez_ES_unregister (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int key
      );

#define ez_ES_run() \
   _ez_ES_run(__FILE__, __LINE__, __FUNCTION__)
int _ez_ES_run (
      const char *fileName,
      int lineNo,
      const char *funcName
      );

#ifdef __cplusplus
}
#endif

#endif
