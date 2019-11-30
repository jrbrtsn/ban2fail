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

/*
 * JDR Sat 30 Nov 2019 08:27:23 AM EST
 * Performs DNS reverse lookups in parallel. Having to use a bunch of threads
 * to overcome serialization inherent in a blocking call is a travesty, but I
 * couldn't find a non-blocking version of getnameinfo().
 */

#include <assert.h>
#include <limits.h>
#include <signal.h>

#include "ez_es.h"
#include "ez_libc.h"
#include "msgqueue.h"
#include "pdns.h"
#include "util.h"

enum vsignals {
   /* All vsigs before this are used to indicate child ready to join */
   EXIT_VSIG=           PDNS_MAX_THREADS,
   CHECK_INBOX_VSIG
};


/*============================================================*/
/*=========== Forward declarations ===========================*/
/*============================================================*/
static int check_inbox_f(void *data, int signo);
static int child_check_inbox_f(void *vp_ndx, int signo);
static void* child_main (void *data);
static int child_exit_f(void *data, int signo);
static int join_f(void *data, int signo);
static void stop_remaining_children(void);
static int timeout_f(void *data);
static int shutdown_f(void *data);
static unsigned nThreads_joined(void);

/*============================================================*/
/*=========== Static data ====================================*/
/*============================================================*/
static struct {

   enum {
      EXIT_FLG= 1<<0
   } flags;

   int64_t start_ms;

   int timeoutKey,
       shutdownKey,
       inboxKey,
       joinKeyArr[PDNS_MAX_THREADS];

   pthread_t tid;
   MSGQUEUE inbox;
   LOGENTRY **lePtrArr;
   unsigned ndx,
            nThreads,
            nItems;

   unsigned nCompleted;

   /* One of these for each child thread */
   struct child {

      int is_joined;

      pthread_t tid;
      MSGQUEUE inbox;

   } childArr[PDNS_MAX_THREADS];
   
} S;

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

   /* Check for nothing-to-do case */
   if(!nItems)
      return 0;

   /* Note when we start */
   S.start_ms= clock_gettime_ms(CLOCK_REALTIME);

   /* Publish our thread ID */
   S.tid= pthread_self();

   /* Prepare our inbox */
   MSGQUEUE_constructor(&S.inbox, sizeof(unsigned), PDNS_INBOX_SZ);

   /* Stash this where it's easy to get to */
   S.nItems= nItems;
   S.nThreads= MIN(nItems, PDNS_MAX_THREADS);
   S.lePtrArr= lePtrArr;

   /* Register a countdown timer to know when to stop */
   S.timeoutKey= ez_ES_registerTimer(timeout_ms, 0, timeout_f, NULL);
   /* Check inbox on CHECK_INBOX_VSIG */
   S.inboxKey= ez_ES_registerVSignal(CHECK_INBOX_VSIG, check_inbox_f, NULL);

   /* Start child threads */
   for(unsigned i= 0; i < S.nThreads; ++i) {

      struct child *ch= S.childArr + i;

      /* Register the join handler on vsig= array index */
      S.joinKeyArr[i]= ez_ES_registerVSignal(i, join_f, NULL);

      /* Pass the child's array index in to child_main() */
      ch->tid= ES_spawn_thread(child_main, (void*)(long unsigned)i);
   }

   /* Give child threads something to do */
   for(; S.ndx < S.nThreads; ++S.ndx) {

      struct child *ch= S.childArr + S.ndx;

      /* Give the child something to do */
      int rc= MSGQUEUE_submitTypedMsg(&ch->inbox, S.lePtrArr[S.ndx]);
      assert(0 == rc);
      /* Prompt child to check inbox */
      ES_VSignal(ch->tid, CHECK_INBOX_VSIG);

   }

   /* Wait for something to happen */
   ES_run();
//eprintf("------------ ALL THREADS TERMINATED ------------");

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

   rtn= S.nCompleted;
abort:
   return rtn;
}

static int
check_inbox_f(void *data, int signo)
/*********************************************************
 * Parent was prompted to check the inbox to see which
 * child is ready for another task.
 */
{
   int rtn= -1;
   unsigned child_ndx;

   while(EOF != MSGQUEUE_extractTypedMsg(&S.inbox, child_ndx)) {

      /* Get pointer to child */
      struct child *ch= S.childArr + child_ndx;

      /* Noting left to do here */
      if(S.ndx == S.nItems) {
//eprintf("Killing thread %u early", child_ndx);
         pthread_kill(ch->tid, SIGTERM);
         continue;
      }

      /* Tell child to work on next task */
      int rc= MSGQUEUE_submitTypedMsg(&ch->inbox, S.lePtrArr[S.ndx]);
      assert(0 == rc);
      ES_VSignal(ch->tid, CHECK_INBOX_VSIG);

      /* Move the "todo" ndx forward */
      ++S.ndx;
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

      struct child *ch= S.childArr + i;
      if(!ch->is_joined) continue;
      ++rtn;
   }

   return rtn;
}

static int
join_f(void *data, int signo)
/*********************************************************
 * Child prompted us to join
 */
{
   struct child *ch= S.childArr + signo;
   void *pRtn;

//eprintf("joining thread %d", signo);

   pthread_join(ch->tid, &pRtn);

   ch->is_joined= 1;

   /* This will naturally terminate when we are done.*/
   return S.nThreads == nThreads_joined() ? -1 : 0;
}

static void
stop_remaining_children(void)
/*********************************************************
 * Signal any remaining children to stop.
 */
{
   /* Tell all remaining child threads to exit now */
   unsigned i;
   for(i= 0; i < S.nThreads; ++i) {

      struct child *ch= S.childArr + i;

      /* If it has already joined, skip it */
      if(ch->is_joined) continue;

      /* Prompt child to shut down now */
      pthread_kill(ch->tid, SIGTERM);
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

eprintf("Timed out with %u threads remaining", S.nThreads - nThreads_joined());

//eprintf("EXTERMINATE!!!!");

   stop_remaining_children();

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
eprintf("WTF? %u threads _still_ remain", S.nThreads - nThreads_joined());
   return -1;
}

/*============================================================*/
/*================= Child threads ============================*/
/*============================================================*/

static void*
child_main (void *vp_ndx)
/*********************************************************
 * Children begin execution here.
 */
{
   unsigned ndx= (long unsigned)vp_ndx;
   struct child *self= S.childArr + ndx;

   /* Prepare child's static data */
   MSGQUEUE_constructor(&self->inbox, sizeof(LOGENTRY*), PDNS_CHILD_INBOX_SZ);

   /* Register to exit when prompted */
   ez_ES_registerSignal(SIGTERM, child_exit_f, NULL);

   /* Register to check inbox when prompted */
   ez_ES_registerVSignal(CHECK_INBOX_VSIG, child_check_inbox_f, vp_ndx);

   /* Parent has been blocked waiting for this call */
   ES_release_parent();

   /* Respond to directives from parent */
   ES_run();
   int64_t ms= clock_gettime_ms(CLOCK_REALTIME) - S.start_ms;

//eprintf("thread %u exiting at %f seconds", ndx, (double)ms/1000.);

   /* Let the main thread know we are ready to join */
   ES_VSignal(S.tid, ndx);

   return NULL;
}

static int
child_check_inbox_f(void *vp_ndx, int signo)
/*********************************************************
 * Child was prompted to check the inbox for tasks.
 */
{
   int rtn= -1;
   unsigned ndx= (long unsigned)vp_ndx;
   struct child *self= S.childArr + ndx;
   char hostBuf[PATH_MAX];
   LOGENTRY *e;
   const static struct addrinfo hints= {
      .ai_family = AF_UNSPEC,    /* Allow IPv4 or IPv6 */
      .ai_flags = AI_NUMERICHOST /* doing reverse lookups */
   };

   while(!(S.flags & EXIT_FLG) &&
         EOF != MSGQUEUE_extractTypedMsg(&self->inbox, e))
   {
      assert(e);
      int64_t ms= clock_gettime_ms(CLOCK_REALTIME) - S.start_ms;
//eprintf("thread %u doing lookup at %f seconds", ndx, (double)ms/1000.);
      /* Get a populated addrinfo object */
      struct addrinfo *res= NULL;
      int rc= ez_getaddrinfo(e->addr, NULL, &hints, &res);
      assert(0 == rc);
      assert(res && res->ai_addr && res->ai_addrlen);
      /* Now do blocking reverse lookup */
      rc= ez_getnameinfo(res->ai_addr, res->ai_addrlen, hostBuf, sizeof(hostBuf)-1, NULL, 0, NI_NAMEREQD);
      switch(rc) {
         case 0:
            e->dnsName= strdup(hostBuf);
            break;

         case EAI_NONAME:
            e->dnsName= "[3(NXDOMAIN)]";
            break;

         case EAI_AGAIN:
            e->dnsName= "[2(SERVFAIL)]";
            break;

         default:
            eprintf("FATAL: getnameinfo() returned %d", rc);
            abort();
      }
   }
   ++S.nCompleted;

   if(S.flags & EXIT_FLG) return -1;

   /* Submit the child's array ndx to main parent
    * thread's inbox to indicate we are ready for
    * more.
    */
   MSGQUEUE_submitTypedMsg(&S.inbox, ndx);
   ES_VSignal(S.tid, CHECK_INBOX_VSIG);

   rtn= 0;
abort:
   return rtn;
}

static int
child_exit_f(void *vp_ndx, int signo)
/*********************************************************
 * Child was prompted to exit now, so return -1 so child_main()
 * will return from ES_run().
 */
{
   return -1;
}
