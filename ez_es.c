#include <stdlib.h>

#include "util.h"
#include "ez_es.h"

/***************************************************/
int _ez_ES_registerFd (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int fd,
      short events,
      int (*callback_f)(void *ctxt, int fd, short events),
      void *ctxt
      )
{
   int rtn= ES_registerFd(fd, events, callback_f, ctxt);
   if(-1 == rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
            "ES_registerFd() failed.");
      abort();
   }
   return rtn;
}

/***************************************************/

int _ez_ES_registerSignal (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      )
{
   int rtn= ES_registerSignal(signum, callback_f, ctxt);
   if(-1 == rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
            "ES_registerSignal() failed.");
      abort();
   }
   return rtn;
}

/***************************************************/

int _ez_ES_registerVSignal (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      )
{
   int rtn= ES_registerVSignal(signum, callback_f, ctxt);
   if(-1 == rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
             "ES_registerVSignal() failed.");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_ES_VSignal (
      const char *fileName,
      int lineNo,
      const char *funcName,
      pthread_t tid,
      int signum
      )
{
   int rtn= ES_VSignal(tid, signum);
   if(rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
            "ES_VSignal() returned %d.", rtn);
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_ES_registerTimer (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int64_t pause_ms,
      int64_t interval_ms,
      int (*callback_f)(void *ctxt),
      void *ctxt
      )
{
   int rtn= ES_registerTimer(pause_ms, interval_ms, callback_f, ctxt);
   if(-1 == rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
            "ES_registerTimer() failed.");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_ES_unregister (
      const char *fileName,
      int lineNo,
      const char *funcName,
      int key
      )
{
   int rtn= ES_unregister(key);
   if(-1 == rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
            "ES_unregister() failed.");
      abort();
   }
   return rtn;
}

/***************************************************/
int _ez_ES_run (
      const char *fileName,
      int lineNo,
      const char *funcName
      )
{
   int rtn= ES_run();
   if(rtn) {
      _eprintf(
#ifdef DEBUG
            fileName, lineNo, funcName,
#endif
            "ES_run() returned %d", rtn);
      abort();
   }
   return rtn;
}

