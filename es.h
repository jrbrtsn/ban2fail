#ifndef ES_H
#define ES_H

/****************************************************************************************
* ES is short for "Event Server". This is a per-thread event server
* for multiplexing sockets, regular file descriptors, Unix signals, and interval
* timers.
****************************************************************************************/

#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int
ES_registerFd (
      int fd,
      short events,
      int (*callback_f)(void *ctxt, int fd, short events),
      void *ctxt
      );
/**********************************************************************
 * Register a function to be called when there is activity on the
 * file descriptor (which may be a file, socket, pipe, etc. under Unix).
 *
 * fd: the file descriptor to be registered.
 * events: event bits to monitor (see: man 2 poll).
 * callback_f:  callback function for when activity is detected.
 * ctxt: Pointer which will be passed as the last argument to callback_f().
 *
 * RETURNS:
 * If successful, a positive integer which can be used to unregister the callback.
 * On failure, -1 is returned.
 */ 

int
ES_registerSignal (
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      );
/**********************************************************************
 * Register a function to be called when a particular Unix signal is
 * raised. Note: callback_f() is not called from a Unix signal handler,
 * so it is safe to modify data within callback_f().
 *
 * signum: Unix signal number of interest (type "trap -l" on command line to see a list of signals)
 * callback_f:  callback function for when activity is detected.
 * ctxt: Pointer which will be passed as the last argument to callback_f().
 *
 * RETURNS:
 * If successful, a positive integer which can be used to unregister the callback.
 * On failure, -1 is returned.
 */ 

int
ES_registerVSignal (
      int signum,
      int (*callback_f)(void *ctxt,int signo),
      void *ctxt
      );
/**********************************************************************
 * Register a function to be called when a particular virtual signal is
 * raised. Virtual signals are implemented on top of the Unix signal, SIGUSR2.
 *
 * signum: Any integer number which is meaningful to your application.
 * callback_f:  callback function for when activity is detected.
 * ctxt: Pointer which will be passed as the last argument to callback_f().
 *
 * RETURNS:
 * If successful, a positive integer which can be used to unregister the callback.
 * On failure, -1 is returned.
 */ 

int
ES_VSignal (pthread_t tid, int signum);
/**********************************************************************
 * Send a virtual signal to tid, which is multiplexed on SIGUSR2.
 *
 * tid: Target thread identifier.
 * signum: Any integer number which is meaningful to your application.
 *
 * RETURNS:
 * 0:   successful
 * -1:  failures.
 */ 

int
ES_registerTimer (
      int64_t pause_ms,
      int64_t interval_ms,
      int (*callback_f)(void *ctxt),
      void *ctxt
      );
/**********************************************************************
 * Register a function to be called when an interval or single-shot
 * timer expires. If interval_ms == 0, the timer is single shot, and
 * can only be ES_unregister()'d before it expires.
 *
 * pause_ms: How many milliseconds to wait before initially firing.
 * interval_ms: How many milliseconds to wait between successive firings.
 *              if this is 0, the timer is single shot.
 * callback_f:  callback function for when timer expires.
 * ctxt: Pointer which will be passed as the last argument to callback_f().
 *
 * RETURNS:
 * If successful, a positive integer which can be used to unregister the callback.
 * On failure, -1 is returned.
 */ 



int
ES_unregister (int key);
/**********************************************************************
 * Unegister a previously registered callback. 
 *
 * key: value obtained from ES_registerXXX().
 *
 * RETURNS:
 * 0 for success.
 * -1 for failure (key not found)
 */ 


int
ES_run (void);
/**********************************************************************
 * For this thread, use poll() to process socket activity until one
 * of the registered callback_f() returns non-zero.
 *
 * RETURNS:
 * Whatever nonzero value callback_f() returned.
 */ 

pthread_t
ES_spawn_thread_sched(
    void *(*user_main) (void *),
    void *arg,
    int sched_policy,  /* SCHED_NORMAL || SCHED_FIFO || SCHED_RR || SCHED_BATCH */
    int priority
    );
/**********************************************************************
 * Spawn a thread which will begin executing user_main(arg).
 * NOTE: the calling thread will be blocked until ES_release_parent()
 * is called from user_main()!
 *
 * user_main: function pointer where thread will execute.
 * arg:           address passed to user_main(arg).
 * sched_policy:  Which pthreads scheduling policy to use.
 * priority:      pthreads priority to use.
 *
 * RETURNS:
 * 0 for success, nonzero for error
 */ 

#define ES_spawn_thread(user_main, arg) \
   ES_spawn_thread_sched(user_main, arg, -1, 0)
/**********************************************************************
 * This is a convenience macro.
 */

void
ES_release_parent(void);
/**********************************************************************
 * Called by a new thread created with ES_spawn_thread_sched(), so
 * that the parent can continue execution.
 */

void
ES_cleanup(void);
/**********************************************************************
 * Called by a thread when it exits, to clean up resources.
 */


#ifdef __cplusplus
}
#endif


#endif
