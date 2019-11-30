#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <time.h>
#include <assert.h>

#include "util.h"
#include "map.h"
#include "msgqueue.h"
#include "es.h"

/* Types of registered callbacks */
enum ES_type {
   ES_FD_TYPE,
   ES_SIG_TYPE,
   ES_VSIG_TYPE,
   ES_TIMER_TYPE
};

#define NUMSIGS 30

/* NOTE: if this queue becomes full, it
 * can wreak havok on your mutlithreading
 * logic!
 */
#define VSIG_QUEUE_MAX 1000

/****************************************************
 * We get one of these anonymous structs per-process.
 ****************************************************/
static struct {
   pthread_mutex_t mtx;       /* global initialization mutex */

   volatile int keySrc;

   volatile enum {
      GLOBAL_INIT_FLG=1<<0
   } flags;

   /* Default sigaction stuff */
   struct {
      sigset_t set;
      struct sigaction arr[NUMSIGS];
   } dflt_sa;

   struct { /* Stuff for ES_spawn_thread_sched() */
      pthread_cond_t cond;       /* Condition used for thread synchronization */
      pthread_mutex_t cond_mtx;  /* condition mutex */
      pthread_mutex_t mtx;       /* mutex for ES spawn operation */
      int release_parent;        /* Value to test for pthread_cond_wait() */
   } spawn;

   struct {
      pthread_mutex_t mtx;       /* mutex for virtual signal operations */
      MAP thrd_ts_map;           /* Map associating thread identifier to TS object */
   } vsig;

} S= {
   .mtx= PTHREAD_MUTEX_INITIALIZER,

   .spawn.cond= PTHREAD_COND_INITIALIZER,
   .spawn.cond_mtx= PTHREAD_MUTEX_INITIALIZER,
   .spawn.mtx= PTHREAD_MUTEX_INITIALIZER,

   .vsig.mtx= PTHREAD_MUTEX_INITIALIZER
};

/****************************************************
 * We get one of these anonymous structs per-thread.
 ****************************************************/
static _Thread_local struct _TS {

   /* Current pthread identifier. If it doesn't match
    * pthread_self(), then we are not yet initialized
    * in the current thread.
    */
   pthread_t tid;

   /* Vectors of Cb by type, for fast processing */
   PTRVEC fd_vec,
          timer_vec;
   PTRVEC sig_vec_arr[NUMSIGS]; // One vector for each Unix signal

   /* Hash table to quickly find Cb's */
   MAP key_map;

   /* Simple bit field to know if a signal has been
    * raised at least once.
    */
   sigset_t sigsRaised;

   struct {
      /* virtual signal message queue */
      MSGQUEUE mq;
      MAP map;
   } vsig;

} TS;

static void
UnixSignalHandler (int signo)
/****************************************************
 * Unix signal handler
 */
{
   /* Simply note that a signal has been raised */
   sigaddset(&TS.sigsRaised, signo);
}

/******************************************************************/
/** Class for callback objects ************************************/
/******************************************************************/
typedef struct {

   int64_t lastActivity_ms;

   /* Process-wide unique integer */
   int key;

   /* Which type of callback object */
   enum ES_type type;

   /* Registrant's supplied context pointer, passed back
    * into callback function
    */
   void *ctxt;

   union { /* Union to accommodate the different callback types */

      /* Unix file descriptors */
      struct {
         int fd;
         short events;
         int (*callback_f)(void *ctxt, int fd, short events);
      } fd;

      /* Unix & virtual signals */
      struct {
         int signum;
         int (*callback_f)(void *ctxt, int signo);
      } sig;

      /* Interval timers */
      struct {
         int64_t register_ms,
                 pause_ms,
                 interval_ms,
                 remaining_ms,
                 count;
         int (*callback_f)(void *ctxt);
      } timer;

   } un;

} Cb;

static int64_t
msec2timeout(const Cb *cb, int64_t time_ms)
/*********************************************************************
 * Compute the number of milliseconds remaining for an interval timer.
 * May be negative if timeout should have already happened.
 */
{
   assert(ES_TIMER_TYPE == cb->type);

   int64_t when_ms= cb->un.timer.register_ms +
                    cb->un.timer.pause_ms +
                    cb->un.timer.count * cb->un.timer.interval_ms;

   return when_ms - time_ms;
}

#define Cb_FdCreate(self, fd, events, callback_f, ctxt)\
   (Cb_FdConstructor((self)=malloc(sizeof(Cb)), fd, events, callback_f, ctxt) ? (self) : ( self ?  realloc(Cb_destructor(self),0): 0))
static Cb*
Cb_FdConstructor(
      Cb *self,
      int fd,
      short events,
      int (*callback_f)(void *ctxt, int fd, short events),
      void *ctxt
      )
/*********************************************************************
 * Initialize for Unix fd.
 */
{
   assert(self);
   self->key= ++S.keySrc;
   self->type= ES_FD_TYPE;
   self->un.fd.fd= fd;
   self->un.fd.callback_f= callback_f;
   self->un.fd.events= events;
   self->ctxt= ctxt;
   return self;
}



#define Cb_SignalCreate(self, signum, callback_f, ctxt)\
   (Cb_SignalConstructor((self)=malloc(sizeof(Cb)), signum, callback_f, ctxt) ? (self) : ( self ?  realloc(Cb_destructor(self),0): 0))
static Cb*
Cb_SignalConstructor(
      Cb *self,
      int signum,
      int (*callback_f)(void *ctxt, int signum),
      void *ctxt
      )
/*********************************************************************
 * Initialize for Unix signal.
 */
{
   assert(self);
   self->key= ++S.keySrc;
   self->type= ES_SIG_TYPE;
   self->un.sig.signum= signum;
   self->un.sig.callback_f= callback_f;
   self->ctxt= ctxt;
   return self;
}

#define Cb_VSignalCreate(self, signum, callback_f, ctxt)\
   (Cb_VSignalConstructor((self)=malloc(sizeof(Cb)), signum, callback_f, ctxt) ? (self) : ( self ?  realloc(Cb_destructor(self),0): 0))
static Cb*
Cb_VSignalConstructor(
      Cb *self,
      int signum,
      int (*callback_f)(void *ctxt, int signum),
      void *ctxt
      )
/*********************************************************************
 * Initialize for Unix signal.
 */
{
   assert(self);
   self->key= ++S.keySrc;
   self->type= ES_VSIG_TYPE;
   self->un.sig.signum= signum;
   self->un.sig.callback_f= callback_f;
   self->ctxt= ctxt;
   return self;
}

#define Cb_TimerCreate(self, pause_secs, interval_secs, callback_f, ctxt)\
   (Cb_TimerConstructor((self)=malloc(sizeof(Cb)), pause_secs, interval_secs, callback_f, ctxt) ? (self) : ( self ?  realloc(Cb_destructor(self),0): 0))
static Cb*
Cb_TimerConstructor(
      Cb *self,
      int64_t pause_ms,
      int64_t interval_ms,
      int (*callback_f)(void *ctxt),
      void *ctxt
      )
/*********************************************************************
 * Initialize for an interval timer.
 */
{
   assert(self);
   self->key= ++S.keySrc;
   self->type= ES_TIMER_TYPE;
#if ! (defined (_WIN32) || defined (__CYGWIN__))
   self->un.timer.register_ms= clock_gettime_ms(CLOCK_MONOTONIC_COARSE);
#else
   self->un.timer.register_ms= clock_gettime_ms(CLOCK_MONOTONIC);
#endif
   self->un.timer.pause_ms= pause_ms;
   self->un.timer.interval_ms= interval_ms;
   self->un.timer.callback_f= callback_f;
   self->un.timer.count= 0;
   self->ctxt= ctxt;
   return self;
}

#define Cb_destroy(s)\
  {if(Cb_destructor(s)) {free(s); (s)=NULL;}}
static void*
Cb_destructor(Cb *self)
/************************************************
 * Free resources associated with object.
 */
{
   return self;
}

/******************************************************************/
/***************** ES *********************************************/
/******************************************************************/

static int
sigusr2_h(void *ctxt, int unused)
/**********************************************************************
 * Handle any vsignals.
 */
{
   int vsigno;
   Cb *cb_arr[VSIG_QUEUE_MAX];

   while(EOF != MSGQUEUE_extractMsg(&TS.vsig.mq, &vsigno)) {

      int rc= MAP_findItems(&TS.vsig.map, (void**)cb_arr, VSIG_QUEUE_MAX, &vsigno, sizeof(int));
      assert(-1 != rc);
      if(!rc) continue;

      for(int i= 0; i < rc; ++i) {
         Cb *cb= cb_arr[i];
         int error= (* cb->un.sig.callback_f)(cb->ctxt, vsigno);
         if(error) return error;
      }
   }
   return 0;
}

static void
initialize()
/**********************************************************************
 * Initialization for current thread, and once for the whole process.
 */
{
  /* Get the global mutex */
  if(pthread_mutex_lock(&S.mtx)) assert (0);

   /* Processwide static data */
   if(!(S.flags & GLOBAL_INIT_FLG)) {

      S.flags |= GLOBAL_INIT_FLG;
      if(-1 == sigemptyset(&S.dflt_sa.set)) assert(0);
      MAP_constructor(&S.vsig.thrd_ts_map, 10, 10);
   }
   /* Release the global mutex */
   if(pthread_mutex_unlock(&S.mtx)) assert (0);

   /* Per-thread static data */
   PTRVEC_constructor(&TS.fd_vec, 10);
   PTRVEC_constructor(&TS.timer_vec, 10);
   for(int i= 0; i < NUMSIGS; ++i) {
      PTRVEC_constructor(&TS.sig_vec_arr[i], 10);
   }
   if(-1 == sigemptyset(&TS.sigsRaised)) assert(0);

   MAP_constructor(&TS.key_map, 10, 10);

   /* Remember so we don't call ourselves more than once in the same thread */
   TS.tid= pthread_self();

   /* Add ourself to the vsig thread to TS map */
   if(pthread_mutex_lock(&S.vsig.mtx)) assert (0);
   MAP_addTypedKey(&S.vsig.thrd_ts_map, TS.tid, &TS);
   if(pthread_mutex_unlock(&S.vsig.mtx)) assert (0);

   /*--- virtual signal infrastructure ---*/
   MSGQUEUE_constructor(&TS.vsig.mq, sizeof(int), VSIG_QUEUE_MAX);
   MAP_constructor(&TS.vsig.map, 10, 10);
   /* Register a signal handler for SIGUSR2 so we can have virtual signals. */
   if(-1 == ES_registerSignal(SIGUSR2, sigusr2_h, NULL)) assert(0);

}

inline static unsigned
signum2dflt_sa_ndx(int signum)
/**********************************************************************
 * Convert signum to an index for S.dflt_sa.XX
 */
{
   assert(SIGKILL != signum && SIGSTOP != signum);
   if(signum < SIGKILL) return signum - 1;
   if(signum < SIGSTOP) return signum - 2;
   return signum - 3;
}

int
ES_registerSignal (
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      )
/**********************************************************************
 * Register a function to be called when a particular Unix signal is
 * raised.
 */ 
{
   if(TS.tid != pthread_self()) initialize();

   Cb *cb;
   unsigned ndx= signum2dflt_sa_ndx(signum);

   /* Only install a new Unix signal handler if we do not already handle this signal */
   if(!PTRVEC_numItems(&TS.sig_vec_arr[ndx])) {

      struct sigaction act;

      act.sa_handler = UnixSignalHandler;
      sigemptyset (&act.sa_mask);
      act.sa_flags = SA_RESTART|SA_NODEFER;

      /* We only store the default action once per process */
      if(!sigismember(&S.dflt_sa.set, signum)) {

         sigaddset(&S.dflt_sa.set, signum);

         if (sigaction (signum, &act, &S.dflt_sa.arr[ndx])) assert (0);

      } else {

         if (sigaction (signum, &act, NULL)) assert (0);

      }
   }

   if(!Cb_SignalCreate(cb, signum, callback_f, ctxt)) assert(0);

   /* All callbacks are put in the key table */
   MAP_addTypedKey(&TS.key_map, cb->key, cb);


   /* Add to the signal vector */
   PTRVEC_addTail(&TS.sig_vec_arr[signum2dflt_sa_ndx(signum)], cb);

   return cb->key;
}

int
ES_registerVSignal (
      int signum,
      int (*callback_f)(void *ctxt, int signo),
      void *ctxt
      )
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
{
   if(TS.tid != pthread_self()) initialize();

   Cb *cb;

   if(!Cb_VSignalCreate(cb, signum, callback_f, ctxt)) assert(0);

   /* Place in the virtual signal map indexed on signum */
   MAP_addTypedKey(&TS.vsig.map, cb->un.sig.signum, cb);

   /* All callbacks are put in the key table */
   MAP_addTypedKey(&TS.key_map, cb->key, cb);

   return cb->key;
}

int
ES_registerFd (
      int fd,
      short events,
      int (*callback_f)(void *ctxt, int fd, short events),
      void *ctxt
      )
/**********************************************************************
 * Register a function to be called when there is activity on the
 * file descriptor (which may be a file, socket, pipe, etc. under Unix).
 */ 
{
   if(TS.tid != pthread_self()) initialize();

   Cb *cb;
   if(!Cb_FdCreate(cb, fd, events, callback_f, ctxt)) assert(0);

   /* Index to vector for quick processing */
   PTRVEC_addTail(&TS.fd_vec, cb);

   /* All callbacks are put in the key table */
   MAP_addTypedKey(&TS.key_map, cb->key, cb);
   return cb->key;
}

int
ES_registerTimer (
      int64_t pause_ms,
      int64_t interval_ms,
      int (*callback_f)(void *ctxt),
      void *ctxt
      )
/**********************************************************************
 * Register a function to be called when a timer times out.
 */ 
{
   if(TS.tid != pthread_self()) initialize();

   Cb *cb;
   if(!Cb_TimerCreate(cb, pause_ms, interval_ms, callback_f, ctxt)) assert(0);

   /* Add to the timer vector */
   PTRVEC_addTail(&TS.timer_vec, cb);

   /* All callbacks are put in the key table */
   MAP_addTypedKey(&TS.key_map, cb->key, cb);

   return cb->key;
}

int
ES_unregister (int key)
/**********************************************************************
 * Unegister a previously registered callback. 
 */ 
{
   if(TS.tid != pthread_self()) initialize();

//   unsigned i;
   Cb *cb = MAP_findTypedItem(&TS.key_map, key);
   if(!cb) return -1;

   /* Remove from key table */
   if(!MAP_removeTypedItem(&TS.key_map, key)) assert(0);;

   /* Different operations needed based on type */
   switch(cb->type) {

      case ES_FD_TYPE:
         /* Remove from file descriptor vector */
         if(!PTRVEC_remove(&TS.fd_vec, cb)) assert(0);
         break;

      case ES_SIG_TYPE:
         {
            unsigned ndx= signum2dflt_sa_ndx(cb->un.sig.signum);

            /* Remove from appropriate signals vector */
            if(!PTRVEC_remove(&TS.sig_vec_arr[ndx], cb)) assert(0);

            /* If there are no more signals in this vector */
            if(!PTRVEC_numItems(&TS.sig_vec_arr[ndx])) {

               assert(sigismember(&S.dflt_sa.set, cb->un.sig.signum));

               /* Restore default signal handling */
               if (sigaction (cb->un.sig.signum, &S.dflt_sa.arr[ndx], NULL)) assert (0);
            }
         } break;

      case ES_VSIG_TYPE:
         if(!MAP_removeSpecificTypedItem(&TS.vsig.map, cb->un.sig.signum, cb)) assert(0);
         break;

      case ES_TIMER_TYPE:
         /* Remove from timer vector */
         if(!PTRVEC_remove(&TS.timer_vec, cb)) assert(0);
         break;

      default:
         assert(0);
   };

   /* Free the callback object */
   Cb_destroy(cb);

   return 0;
}

static int
cmp_remaining_ms (const void *const*p1, const void *const*p2)
/**********************************************************************
 * Compare the time remaining for PTRVEC_sort().
 */
{
   const Cb *cb1= (const Cb*)(*p1),
            *cb2= (const Cb*)(*p2);

   assert(ES_TIMER_TYPE == cb1->type &&
          ES_TIMER_TYPE == cb2->type);

   if(cb1->un.timer.remaining_ms < cb2->un.timer.remaining_ms) return -1;
   if(cb1->un.timer.remaining_ms == cb2->un.timer.remaining_ms) return 0;
   return 1;
}

static int
lastActivity_cmp(const void *const* pp1, const void *const* pp2)
{
   const Cb *cb1= *(const Cb *const*)pp1,
            *cb2= *(const Cb *const*)pp2;

   /* Put oldest at the top of the vector */
   if(cb1->lastActivity_ms < cb2->lastActivity_ms) return -1;
   return 1;
}

int
ES_run (void)
/**********************************************************************
 * For this thread, use poll() to monitor socket activity until one
 * of the registered callback_f() returns non-zero.
 *
 * RETURNS:
 * Whatever nonzero value one of the callbacks() returned.
 */ 
{

   if(TS.tid != pthread_self())
      initialize();

   /* Loop forever */
   for(;;) {

      int numFds= PTRVEC_numItems(&TS.fd_vec);
      struct pollfd pollItemArr[numFds];
      Cb *cbArr[numFds];


      /* This sort provides fair queuing */
      PTRVEC_sort(&TS.fd_vec, lastActivity_cmp);

      /****** Load up the ZeroMQ pollItemArr *****/
      unsigned i;
      Cb *cb;
      PTRVEC_loopFwd(&TS.fd_vec, i, cb) {

         struct pollfd *item= pollItemArr+i;

         switch(cb->type) {

            case ES_FD_TYPE:
               item->fd= cb->un.fd.fd;
               item->events= cb->un.fd.events;
               break;

            default:
               assert(0);
         }

         /* Clear the return event field for good measure */
         item->revents= 0;
         /* Remember the Cb object */
         cbArr[i]= cb;
      }

      /* There may not be any timers */
      int64_t poll_ms= -1; 

      /***** If there are any timers to consider ****/
      if(PTRVEC_numItems(&TS.timer_vec)) {

#if ! (defined (_WIN32) || defined (__CYGWIN__))
         int64_t time_ms= clock_gettime_ms(CLOCK_MONOTONIC_COARSE);
#else
         int64_t time_ms= clock_gettime_ms(CLOCK_MONOTONIC);
#endif

         /* Prepare timers to be sorted */
         unsigned i;
         PTRVEC_loopFwd(&TS.timer_vec, i, cb) {
            assert(ES_TIMER_TYPE == cb->type);
            cb->un.timer.remaining_ms= msec2timeout(cb, time_ms);
         }

         /* Sort them so the most urgent timer is at the top */
         PTRVEC_sort(&TS.timer_vec, cmp_remaining_ms);

         /* Get the top item */
         cb= PTRVEC_first(&TS.timer_vec);
         assert(cb);

         /* This is how long we need to wait */
         poll_ms= MAX(cb->un.timer.remaining_ms, 0);

      }

      /*******************************************************/
      /******** Wait for something to happen *****************/
      /*******************************************************/

      int poll_rc= poll(pollItemArr, numFds, poll_ms);


      /********* Check return code *****/
      if(-1 == poll_rc) {
         switch(errno) {

            case EFAULT:
               eprintf("\tpoll() failed");
               return -1;

            case EINTR:
               /* Signal caused poll() to return, which is OK */
               break;

            default:
               assert(0);
         }
      }

      /********* Respond to signals *****/
      int signum;
      for(signum= 1; signum < 32; ++signum) {
         /* Can't do anything with these signals */
         if(signum == SIGKILL || signum == SIGSTOP) continue;

         /* See if signum was raised */
         while(sigismember(&TS.sigsRaised, signum)) {

            /* Clear signum from the set of raised signals */
            if(-1 == sigdelset(&TS.sigsRaised, signum)) assert(0);

            unsigned ndx= signum2dflt_sa_ndx(signum);

            /* See if any of our callbacks are for signum */
            PTRVEC_loopFwd(&TS.sig_vec_arr[ndx], i, cb) {

               assert(ES_SIG_TYPE == cb->type);

               /* Call the callback function */
               int error= (* cb->un.sig.callback_f)(cb->ctxt, signum);

               if(error) return error;
            }
         }
      }

      /********* Service timers ********/
      if(PTRVEC_numItems(&TS.timer_vec)) {

         int64_t remaining_ms, 
                 time_ms;

#if ! (defined (_WIN32) || defined (__CYGWIN__))
         time_ms= clock_gettime_ms(CLOCK_MONOTONIC_COARSE);
#else
         time_ms= clock_gettime_ms(CLOCK_MONOTONIC);
#endif

         PTRVEC_loopFwd(&TS.timer_vec, i, cb) {

            /* See how much time remains for this callback */
            remaining_ms= msec2timeout(cb, time_ms);

            /* close enough */
            if(remaining_ms < 2) {

               /* Keep track of how many times this timer has fired */
               ++cb->un.timer.count;

               /* Call the callback function */
               int error= (* cb->un.timer.callback_f)(cb->ctxt);

               /* If this is a single-shot timer, get rid of it now */
               if(!cb->un.timer.interval_ms) {
                  ES_unregister(cb->key);
                  /* Do this so next vector entry doesn't get skipped */
                  --i;
               }

               /* If the callback returned non-zero, bail out */
               if(error) return error;

            } else break;  /* time remaining will increase from here on out */
         }
      }


      /********** Service file descriptors *******/
      for(int i= 0; i < numFds; ++i) {

         struct pollfd *item= pollItemArr+i;

         if(!item->revents) continue;

         Cb *cb= cbArr[i];

#if ! (defined (_WIN32) || defined (__CYGWIN__))
         cb->lastActivity_ms= clock_gettime_ms(CLOCK_MONOTONIC_COARSE);
#else
         cb->lastActivity_ms= clock_gettime_ms(CLOCK_MONOTONIC);
#endif

         int error;
         switch(cb->type) {

            case ES_FD_TYPE:
               /* Call the callback function */
               error= (* cb->un.fd.callback_f)(cb->ctxt, item->fd, item->revents);
               break;

            default:
               assert(0);
         }

         /* If the callback returned non-zero, bail out */
         if(error) return error;
      }
   }

   /* Shouldn't ever get to here */
   assert(0);
}

pthread_t
ES_spawn_thread_sched(
    void *(*user_main) (void *),
    void *arg,
    int sched_policy,  /* SCHED_NORMAL || SCHED_FIFO || SCHED_RR || SCHED_BATCH */
    int priority
    )
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
{
   pthread_t tid;
   pthread_attr_t attr;
   int rtn;

   pthread_attr_init(&attr);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

   if(sched_policy == -1) {
      pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
   } else {
      struct sched_param sp;
      pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
      if(priority < sched_get_priority_min(sched_policy) ||
         priority > sched_get_priority_max(sched_policy)) {
         eprintf("ERROR: priority= %d must be between %d and %d inclusive.", priority, sched_get_priority_min(sched_policy), sched_get_priority_max(sched_policy));
         return 1;
      }

      if(pthread_attr_setschedpolicy(&attr, sched_policy)) assert(0);
      sp.sched_priority= priority;
      if(pthread_attr_setschedparam(&attr, &sp)) assert(0);
  }

  /* Get the global mutex */
  if(pthread_mutex_lock(&S.spawn.mtx)) assert (0);

  /* Get the condition ready for use */
  pthread_cond_init(&S.spawn.cond, NULL);

  /* This is the flag we will test to know when the child is ready to recieve signals */
  S.spawn.release_parent= 0;

  /* Get the condition mutex */
  if(pthread_mutex_lock(&S.spawn.cond_mtx)) assert (0);

  /* Spawn the new thread */
  memset(&tid, 0, sizeof(tid));
  /* JDR Sat 30 Nov 2019 10:39:04 AM EST
   * it appears that this fails at 300 threads.
   */
  rtn = pthread_create (&tid, &attr, user_main, arg);
  if(rtn) {
     sys_eprintf("ERROR: pthread_create()");
     abort();
  }

  /* Now we, the parent, wait on the child */
  while(!S.spawn.release_parent) {
    if(pthread_cond_wait(&S.spawn.cond, &S.spawn.cond_mtx)) assert(0);
  }

  /* Release the condition mutex */
  if(pthread_mutex_unlock(&S.spawn.cond_mtx)) assert (0);

  /* Release the global lock */
  if(pthread_mutex_unlock(&S.spawn.mtx)) assert (0);

  return tid;
}

void
ES_release_parent(void)
/**********************************************************************
 * Called by a new thread created with ES_spawn_thread_sched(), so
 * that the parent can continue execution.
 */
{
  /* Condition manipulation must be protected by a mutex */
  if (pthread_mutex_lock (&S.spawn.cond_mtx)) assert (0);

  /* Note that parent may be released */
  S.spawn.release_parent= 1;

  /* Signal the parent */
  if(pthread_cond_signal(&S.spawn.cond)) assert(0);

  /* Free up the condition mutex */
  if (pthread_mutex_unlock (&S.spawn.cond_mtx)) assert (0);

}

void
ES_cleanup(void)
/**********************************************************************
 * Called by a thread when it exits, to clean up resources.
 */
{
   assert(TS.tid == pthread_self());

   /* Remove ourself from the vsig thread to TS map */
   if(pthread_mutex_lock(&S.vsig.mtx)) assert (0);
   MAP_removeTypedItem(&S.vsig.thrd_ts_map, TS.tid);
   if(pthread_mutex_unlock(&S.vsig.mtx)) assert (0);

   { /* Destroy key map */
      unsigned len= MAP_numItems(&TS.key_map);
      Cb *cbArr[len];
      MAP_fetchAllItems(&TS.key_map, (void**)cbArr);
      for(unsigned i= 0; i < len; ++i) {
         Cb_destroy(cbArr[i]);
      }

      MAP_destructor(&TS.key_map);
   }

   { /* Destroy vsignal infrastructure */
      unsigned len= MAP_numItems(&TS.vsig.map);
      Cb *cbArr[len];
      MAP_fetchAllItems(&TS.vsig.map, (void**)cbArr);
      for(unsigned i= 0; i < len; ++i) {
         Cb_destroy(cbArr[i]);
      }
      MAP_destructor(&TS.vsig.map);

      /* Tear down the message queue */
      MSGQUEUE_destructor(&TS.vsig.mq);
   }

   PTRVEC_destructor(&TS.fd_vec);
   PTRVEC_destructor(&TS.timer_vec);

   for(unsigned i= 0; i < NUMSIGS; ++i) {
      PTRVEC_destructor(TS.sig_vec_arr+i);
   }
}

int
ES_VSignal (pthread_t tid, int signum)
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
{
   int rtn= EOF-1;
   /* find the correct TS by thread identifier */
   if(pthread_mutex_lock(&S.vsig.mtx)) assert (0);
   struct _TS *ts= MAP_findTypedItem(&S.vsig.thrd_ts_map, tid);
   if(pthread_mutex_unlock(&S.vsig.mtx)) assert (0);

   if(!ts) {
      eprintf("ERROR: tid= %s not found!", pthread_t_str(tid));
      goto abort;
   }

   assert(tid == ts->tid);

   /* Place virtual signal in message queue */
   ez_MSGQUEUE_submitMsg(&ts->vsig.mq, &signum);

   /* And finally tell the target thread to check it's message queue */
   if(pthread_kill(tid, SIGUSR2)) {
      sys_eprintf("ERROR: kill(%s, SIGUSR2)", pthread_t_str(tid));
      goto abort;
   }

   rtn= 0;
abort:
   return rtn;
}
