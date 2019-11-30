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
   EXIT_VSIG= PDNS_N_THREADS,
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

/*============================================================*/
/*=========== Static data ====================================*/
/*============================================================*/
static struct {

   enum {
      EXIT_FLG= 1<<0,
      DONE_FLG= 1<<1
   } flags;

   int timeoutKey,
       inboxKey,
       joinKeyArr[PDNS_N_THREADS];

   pthread_t tid;
   MSGQUEUE inbox;
   LOGENTRY **leArr;
   unsigned ndx,
            nItems,
            nSucc,
            nFail;

   /* One of these for each child thread */
   struct child {

      int is_joined;

      pthread_t tid;
      MSGQUEUE inbox;

   } childArr[PDNS_N_THREADS];
   
} S;

/*============================================================*/
/*=========== PDNS ===========================================*/
/*============================================================*/
int
PDNS_lookup(LOGENTRY *leArr[], unsigned nItems, unsigned timeout_ms)
/**************************************************************
 * Perform parallel DNS reverse lookups on all LOGENTRY objects
 * referenced in leArr.
 */
{
   int rtn= -1;

   /* Publish our thread ID */
   S.tid= pthread_self();

   /* Prepare our inbox */
   MSGQUEUE_constructor(&S.inbox, sizeof(unsigned), PDNS_INBOX_SZ);

   /* Stash this where it's easy to get to */
   S.nItems= nItems;
   S.leArr= leArr;

   /* Register a countdown timer to know when to stop */
   S.timeoutKey= ez_ES_registerTimer(timeout_ms, 0, timeout_f, NULL);
   /* Check inbox on CHECK_INBOX_VSIG */
   S.inboxKey= ez_ES_registerVSignal(CHECK_INBOX_VSIG, check_inbox_f, NULL);

   assert(S.timeoutKey && S.inboxKey);

   /* Start child threads */
   for(; S.ndx < PDNS_N_THREADS; ++S.ndx) {

      /* Register the join handler on vsig= array index */
      S.joinKeyArr[S.ndx]= ez_ES_registerVSignal(S.ndx, join_f, NULL);

      struct child *ch= S.childArr + S.ndx;
      /* Pass the child's array index in to child_main() */
      ch->tid= ES_spawn_thread(child_main, (void*)(long unsigned)S.ndx);

      /* Give the child something to do */
      int rc= MSGQUEUE_submitTypedMsg(&ch->inbox, S.leArr[S.ndx]);
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

   ez_ES_unregister(S.inboxKey);

   /* Release all the join registrations */
   for(unsigned i= 0; i < PDNS_N_THREADS; ++i) {
      ez_ES_unregister(S.joinKeyArr[i]);
   }

//eprintf("INFO: nItems= %u, nSucc= %u, nFail= %u", nItems, S.nSucc, S.nFail);

   rtn= 0;
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
         pthread_kill(ch->tid, SIGTERM);
         continue;
      }

      /* Tell child to work on next task */
      int rc= MSGQUEUE_submitTypedMsg(&ch->inbox, S.leArr[S.ndx]);
      assert(0 == rc);
      ES_VSignal(ch->tid, CHECK_INBOX_VSIG);

      /* Move the "todo" ndx forward */
      ++S.ndx;

      if(S.ndx == S.nItems)
         S.flags |= DONE_FLG;
   }

   rtn= 0;
abort:
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

   pthread_join(ch->tid, &pRtn);

   ch->is_joined= 1;

   unsigned i;
   for(i= 0; i < PDNS_N_THREADS; ++i) {

      struct child *ch= S.childArr + i;
      if(!ch->is_joined) break;
   }

   /* This will naturally terminate when we are done.*/
   return PDNS_N_THREADS == i ? -1 : 0;
}

static void
stop_remaining_children(void)
/*********************************************************
 * Signal any remaining children to stop.
 */
{
   /* Tell all remaining child threads to exit now */
   unsigned i;
   for(i= 0; i < PDNS_N_THREADS; ++i) {

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

//eprintf("============ TERMINATE NOW ============");

   stop_remaining_children();

   return 0;
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
            ++S.nSucc;
            break;

         case EAI_NONAME:
            e->dnsName= "not found: 3(NXDOMAIN)";
            ++S.nFail;
            break;

         case EAI_AGAIN:
            e->dnsName= "not found: 2(SERVFAIL)";
            ++S.nFail;
            break;

         default:
            abort();
      }
   }

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
