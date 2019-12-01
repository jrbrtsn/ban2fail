/***************************************************************************
 *   Copyright (C) 2019 by John D. Robertson                               *
 *   john@rrci.com                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <assert.h>
#include <limits.h>
#include <signal.h>

#include "ez_es.h"
#include "ez_libc.h"
#include "msgqueue.h"
#include "pdns.h"
#include "util.h"

enum vsignals {
   /* All vsigs before this are used to indicate worker ready to join */
   EXIT_VSIG=           PDNS_MAX_THREADS,
   CHECK_INBOX_VSIG
};

enum lookupType {
   FWD_LOOKUP,
   REV_LOOKUP
};

/* Messages in the mgr inbox look like this */
struct mgrMsg {
   LOGENTRY *e;
   unsigned worker_ndx;
};

/* Messages in the worker inbox look like this */
struct workerMsg {
   LOGENTRY *e;
};


/*============================================================*/
/*=========== Forward declarations ===========================*/
/*============================================================*/
static int mgr_check_inbox_f(void *data, int signo);
static int join_f(void *data, int signo);
static unsigned nThreads_joined(void);
static int shutdown_f(void *data);
static void stop_remaining_workers(void);
static int timeout_f(void *data);
static int worker_check_inbox_f(void *vp_ndx, int signo);
static void* worker_main (void *data);
static int worker_exit_f(void *data, int signo);

/*============================================================*/
/*=========== Static data ====================================*/
/*============================================================*/
static struct {

   volatile enum {
      EXIT_FLG= 1<<0,
      ORPHAN_FLG= 1<<1
   } flags;

   int64_t start_ms;

   int timeoutKey,
       shutdownKey,
       inboxKey,
       joinKeyArr[PDNS_MAX_THREADS];

   pthread_t tid;
   MSGQUEUE inbox;
   LOGENTRY **lePtrArr;
   unsigned processedNdx,
            nThreads,
            nItems;

   /* One of these for each worker thread */
   struct worker {

      volatile int is_joined;

      pthread_t tid;
      MSGQUEUE inbox;

   } workerArr[PDNS_MAX_THREADS];
#ifdef DEBUG
   pthread_mutex_t prt_mtx;
#endif
   
} S= {
#ifdef DEBUG
   .prt_mtx= PTHREAD_MUTEX_INITIALIZER
#endif
};

/*============================================================*/
/*=========== PDNS ===========================================*/
/*============================================================*/
int
PDNS_lookup(LOGENTRY *lePtrArr[], unsigned nItems, unsigned timeout_ms)
/**************************************************************
 * Perform parallel DNS reverse lookups on all LOGENTRY objects
 * referenced in lePtrArr.
 */
{
   int rtn= -1;

   /* Check for trivial case */
   if(!nItems)
      return 0;

   /* Note when we start */
   S.start_ms= clock_gettime_ms(CLOCK_REALTIME);

   /* Publish our thread ID */
   S.tid= pthread_self();

   /* Prepare our inbox */
   MSGQUEUE_constructor(&S.inbox, sizeof(struct mgrMsg), PDNS_MGR_INBOX_SZ);

   /* Stash this where it's easy to get to */
   S.nItems= nItems;
   S.nThreads= MIN(nItems, PDNS_MAX_THREADS);
   S.lePtrArr= lePtrArr;

   /* Register a countdown timer to know when to stop */
   S.timeoutKey= ez_ES_registerTimer(timeout_ms, 0, timeout_f, NULL);
   /* Check inbox on CHECK_INBOX_VSIG */
   S.inboxKey= ez_ES_registerVSignal(CHECK_INBOX_VSIG, mgr_check_inbox_f, NULL);

   /* Start worker threads */
   for(unsigned i= 0; i < S.nThreads; ++i) {

      struct worker *wrk= S.workerArr + i;

      /* Register the join handler on vsig= array index */
      S.joinKeyArr[i]= ez_ES_registerVSignal(i, join_f, NULL);

      /* Pass the worker's array index in to worker_main() */
      wrk->tid= ES_spawn_thread(worker_main, (void*)(long unsigned)i);
   }

   /* Give worker threads something to do */
   for(; S.processedNdx < S.nThreads; ++S.processedNdx) {

      struct worker *wrk= S.workerArr + S.processedNdx;
      struct workerMsg worker_msg= {.e= S.lePtrArr[S.processedNdx]};

      /* Give the worker something to do */
      ez_MSGQUEUE_submitTypedMsg(&wrk->inbox, worker_msg);
      /* Prompt worker to check inbox */
      ES_VSignal(wrk->tid, CHECK_INBOX_VSIG);

   }

   /* Wait for something to happen */
   ES_run();

   /* Unregister signal handlers for this thread */
   if(S.timeoutKey)
      ez_ES_unregister(S.timeoutKey);

   if(S.shutdownKey)
      ez_ES_unregister(S.shutdownKey);

   ez_ES_unregister(S.inboxKey);

   /* Release all the join registrations */
   for(unsigned i= 0; i < S.nThreads; ++i) {
      ez_ES_unregister(S.joinKeyArr[i]);
   }

   rtn= S.processedNdx;
abort:
   return rtn;
}

static int
mgr_check_inbox_f(void *data, int signo)
/**************************************************************************
 * Manager was prompted by a worker to check the inbox and see which worker is
 * ready for another task.
 */
{
   int rtn= -1;
   struct mgrMsg msg;

   while(EOF != MSGQUEUE_extractTypedMsg(&S.inbox, msg)) {

      /* Get pointer to worker */
      struct worker *wrk= S.workerArr + msg.worker_ndx;
      struct workerMsg worker_msg= {.e= NULL};

      if(msg.e->dns.flags & PDNS_DONE_MASK) {

         /* If we've finished up, start pruning worker threads */
         if(S.processedNdx == S.nItems) {
            pthread_kill(wrk->tid, SIGTERM);
            continue;
         }

         worker_msg.e= S.lePtrArr[S.processedNdx];
         ++S.processedNdx;

      } else {

          /* Perform forward lookup next */
          worker_msg.e= msg.e;

      }

      /* Give worker another task */
      ez_MSGQUEUE_submitTypedMsg(&wrk->inbox, worker_msg);
      ES_VSignal(wrk->tid, CHECK_INBOX_VSIG);

   }

   rtn= 0;
abort:
   return rtn;
}

static unsigned
nThreads_joined(void)
/*********************************************************
 * Return the number of threads which have already joined.
 */
{
   unsigned rtn= 0;
   for(unsigned i= 0; i < S.nThreads; ++i) {

      struct worker *wrk= S.workerArr + i;
      if(!wrk->is_joined) continue;
      ++rtn;
   }

   return rtn;
}

static int
join_f(void *data, int signo)
/*********************************************************
 * Worker prompted us to join
 */
{
   struct worker *wrk= S.workerArr + signo;
   void *pRtn;

   pthread_join(wrk->tid, &pRtn);

   wrk->is_joined= 1;

   /* This will naturally terminate when we are done.*/
   return S.nThreads == nThreads_joined() ? -1 : 0;
}

static void
stop_remaining_workers(void)
/*********************************************************
 * Signal any remaining workers to stop.
 */
{
   /* Tell all remaining worker threads to exit now */
   unsigned i;
   for(i= 0; i < S.nThreads; ++i) {

      struct worker *wrk= S.workerArr + i;

      /* If it has already joined, skip it */
      if(wrk->is_joined) continue;

      /* Prompt worker to shut down now */
      pthread_kill(wrk->tid, SIGTERM);
   }
}

static int
timeout_f(void *data)
/*********************************************************
 * Countdown timer has expired.
 */
{
   /* Note that the countdown timer fired */
   S.timeoutKey= 0;

   /* Post notice that it is time to shut down */
   S.flags |= EXIT_FLG;

#ifdef DEBUG
   eprintf("Timed out with %u threads remaining", S.nThreads - nThreads_joined());
#endif

   stop_remaining_workers();

   /* Register a countdown timer to know when to forcefully
    * stop remaining threads.
    */
   S.shutdownKey= ez_ES_registerTimer(PDNS_SHUTDOWN_PAUSE_MS, 0, shutdown_f, NULL);

   return 0;
}

static int
shutdown_f(void *data)
/*********************************************************
 * Terminate any remaining threads.
 */
{
   S.shutdownKey= 0;
#ifdef DEBUG
   eprintf("WTF: %u threads *still* remain!", S.nThreads - nThreads_joined());
#endif
   /* Let workerren know not to signal for a join */
   S.flags |= ORPHAN_FLG;
   return -1;
}

/*============================================================*/
/*================= Worker threads ===========================*/
/*============================================================*/

static void*
worker_main (void *vp_ndx)
/*********************************************************
 * Workers begin execution here.
 */
{
   unsigned ndx= (long unsigned)vp_ndx;
   struct worker *self= S.workerArr + ndx;

   /* Prepare worker's static data */
   MSGQUEUE_constructor(&self->inbox, sizeof(struct workerMsg), PDNS_WORKER_INBOX_SZ);

   /* Register to exit when prompted */
   ez_ES_registerSignal(SIGTERM, worker_exit_f, NULL);

   /* Register to check inbox when prompted */
   ez_ES_registerVSignal(CHECK_INBOX_VSIG, worker_check_inbox_f, vp_ndx);

   /* Parent has been blocked waiting for this call */
   ES_release_parent();

   /* Respond to directives from mgr */
   ES_run();
#ifdef qqDEBUG
int64_t ms= clock_gettime_ms(CLOCK_REALTIME) - S.start_ms;
eprintf("thread %u exiting at %f seconds", ndx, (double)ms/1000.);
#endif

   /* Parent thread may have moved on. In that case, don't join. */
   if(!(S.flags & ORPHAN_FLG))
      /* Let the main thread know we are ready to join */
      ES_VSignal(S.tid, ndx);

   return NULL;
}

static int
worker_check_inbox_f(void *vp_ndx, int signo)
/*********************************************************
 * Worker was prompted to check the inbox for tasks.
 */
{
   int rtn= -1;
   /* Our S.workerArr index was passed in as (void*) */
   unsigned ndx= (long unsigned)vp_ndx;
   struct worker *self= S.workerArr + ndx;
   struct workerMsg msg;

   while(!(S.flags & EXIT_FLG) &&
         EOF != MSGQUEUE_extractTypedMsg(&self->inbox, msg))
   {
      assert(msg.e);
      int64_t ms= clock_gettime_ms(CLOCK_REALTIME) - S.start_ms;

      /* Check to see if we've finished the reverse DNS lookup */
      if(msg.e->dns.flags & PDNS_REV_DNS_FLG) {

         const static struct addrinfo hints= {
            .ai_family= AF_UNSPEC,    /* Allow IPv4 or IPv6 */
            .ai_socktype= SOCK_DGRAM,
            .ai_protocol= IPPROTO_UDP
         };

         /* Get a populated addrinfo object */
         struct addrinfo *res= NULL;
         int rc= ez_getaddrinfo(msg.e->dns.name, NULL, &hints, &res);

#ifdef qqDEBUG
if(!strcmp(msg.e->addr, "50.116.38.131")) {
   pthread_mutex_lock(&S.prt_mtx);
   ez_fprintf(stderr, "%s (%s) ----------------------------------\n", msg.e->addr, msg.e->dns.name);
   addrinfo_print(res, stderr);
   fflush(stderr);
   pthread_mutex_unlock(&S.prt_mtx);
}
#endif
         switch(rc) {
            case 0:
               if(!addrinfo_is_match(res, msg.e->addr))
                  msg.e->dns.flags |= PDNS_FWD_MISMATCH_FLG;
               break;

            case EAI_NONAME:
               msg.e->dns.flags |= PDNS_FWD_NONE_FLG;
               break;

            case EAI_FAIL:
            case EAI_NODATA:
            case EAI_AGAIN:
               msg.e->dns.flags |= PDNS_FWD_FAIL_FLG;
               break;

            default:
               eprintf("rc= %d", rc);
               assert(0);

         }

         /* In any case, we are done */
         msg.e->dns.flags |= PDNS_FWD_DNS_FLG;

         if(res) freeaddrinfo(res);

      } else { /* reverse lookup */

         const static struct addrinfo hints= {
            .ai_flags = AI_NUMERICHOST, /* doing reverse lookups */
            .ai_family = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
            .ai_socktype= SOCK_DGRAM,
            .ai_protocol= IPPROTO_UDP
         };

         /* Place to which getnameinfo can copy result */
         char hostBuf[PATH_MAX];

         /* Get a populated addrinfo object */
         struct addrinfo *res= NULL;
         int rc= ez_getaddrinfo(msg.e->addr, NULL, &hints, &res);
         assert(0 == rc);
         assert(res && res->ai_addr && res->ai_addrlen);
         /* Now do blocking reverse lookup */
         rc= ez_getnameinfo(res->ai_addr, res->ai_addrlen, hostBuf, sizeof(hostBuf)-1, NULL, 0, NI_NAMEREQD);

#ifdef qqDEBUG
if(!strcmp(msg.e->addr, "50.116.38.131")) {
   pthread_mutex_lock(&S.prt_mtx);
   ez_fprintf(stderr, "%s ----------------------------------\n", msg.e->addr);
   addrinfo_print(res, stderr);
   fflush(stderr);
   pthread_mutex_unlock(&S.prt_mtx);
}
#endif
         if(res) freeaddrinfo(res);

         switch(rc) {
            case 0:
               msg.e->dns.name= strdup(hostBuf);
               msg.e->dns.flags |= PDNS_REV_DNS_FLG;
               break;

            case EAI_NONAME:
               msg.e->dns.flags |= PDNS_NXDOMAIN_FLG;
               break;

            case EAI_AGAIN:
               msg.e->dns.flags |= PDNS_SERVFAIL_FLG;
               break;

            default:
               eprintf("FATAL: getnameinfo() returned %d", rc);
               abort();
         }

      }

      /* Catch being bumped out of blocking call by signal */
      if(S.flags & EXIT_FLG) break;
   }

   /* Only do follow up if we are not exiting */
   if(!(S.flags & EXIT_FLG)) {

      struct mgrMsg mgr_msg= {.e= msg.e, .worker_ndx= ndx};
      /* Submit the worker's message to main mgr
       * thread's inbox to indicate we are ready for
       * more.
       */
      ez_MSGQUEUE_submitTypedMsg(&S.inbox, mgr_msg);
      ES_VSignal(S.tid, CHECK_INBOX_VSIG);

      rtn= 0;
   }

abort:
   return rtn;
}

static int
worker_exit_f(void *vp_ndx, int signo)
/**************************************************************************
 * Worker was prompted to exit now, so return -1 causing worker_main() return
 * from ES_run().
 */
{
   return -1;
}


