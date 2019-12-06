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

#ifdef DEBUG
#       define ez_ES_registerFd(fd, events, callback_f, ctxt) \
   _ez_ES_registerFd(__FILE__, __LINE__, __FUNCTION__, fd, events, callback_f, ctxt)
#else
#       define ez_ES_registerFd(fd, events, callback_f, ctxt) \
   _ez_ES_registerFd(fd, events, callback_f, ctxt)
#endif
int _ez_ES_registerFd (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int fd,
      short events,
      int (*callback_f)(void *ctxt, int fd, short events),
      void *ctxt
      );



#ifdef DEBUG
#       define ez_ES_registerSignal(signum, callback_f, ctxt) \
   _ez_ES_registerSignal(__FILE__, __LINE__, __FUNCTION__, signum, callback_f, ctxt)
#else
#       define ez_ES_registerSignal(signum, callback_f, ctxt) \
   _ez_ES_registerSignal(signum, callback_f, ctxt)
#endif
int _ez_ES_registerSignal (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      );

#ifdef DEBUG
#       define ez_ES_registerVSignal(signum, callback_f, ctxt) \
   _ez_ES_registerVSignal(__FILE__, __LINE__, __FUNCTION__, signum, callback_f, ctxt)
#else
#       define ez_ES_registerVSignal(signum, callback_f, ctxt) \
   _ez_ES_registerVSignal(signum, callback_f, ctxt)
#endif
int _ez_ES_registerVSignal (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      );

#ifdef DEBUG
#       define ez_ES_VSignal(tid, signum) \
   _ez_ES_VSignal(__FILE__, __LINE__, __FUNCTION__, tid, signum)
#else
#       define ez_ES_VSignal(tid, signum) \
   _ez_ES_VSignal(tid, signum)
#endif
int _ez_ES_VSignal (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      pthread_t tid,
      int signum
      );

#ifdef DEBUG
#       define ez_ES_registerTimer(pause_ms, interval_ms, callback_f, ctxt) \
   _ez_ES_registerTimer(__FILE__, __LINE__, __FUNCTION__, pause_ms, interval_ms, callback_f, ctxt)
#else
#       define ez_ES_registerTimer(pause_ms, interval_ms, callback_f, ctxt) \
   _ez_ES_registerTimer(pause_ms, interval_ms, callback_f, ctxt)
#endif
int _ez_ES_registerTimer (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int64_t pause_ms,
      int64_t interval_ms,
      int (*callback_f)(void *ctxt),
      void *ctxt
      );


#ifdef DEBUG
#       define ez_ES_unregister(key) \
   {_ez_ES_unregister(__FILE__, __LINE__, __FUNCTION__, key); (key)= 0;}
#else
#       define ez_ES_unregister(key) \
   {_ez_ES_unregister(key); (key)= 0;}
#endif
int _ez_ES_unregister (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName,
#endif
      int key
      );

#ifdef DEBUG
#       define ez_ES_run() \
   _ez_ES_run(__FILE__, __LINE__, __FUNCTION__)
#else
#       define ez_ES_run() \
   _ez_ES_run()
#endif
int _ez_ES_run (
#ifdef DEBUG
      const char *fileName,
      int lineNo,
      const char *funcName
#endif
      );

#ifdef __cplusplus
}
#endif

#endif
