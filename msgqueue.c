/***************************************************************************
 *   Copyright (C) 2008 by John D. Robertson                               *
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
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ez_libpthread.h"
#include "msgqueue.h"
#include "util.h"


MSGQUEUE *
MSGQUEUE_constructor (MSGQUEUE * self, size_t msgSize, unsigned int queueLen)
{
  if (!self)
    return NULL;

  pthread_mutex_init (&self->mtx, NULL);

  /* Allocate the memory in which to store messages */
  if (!(self->buff_ptr = (char *) malloc (msgSize * queueLen)))
    return NULL;


  /* Initialize the header segment */
  self->numItems = 0;
  self->head = 0;
  self->tail = 0;
  self->msgSize = msgSize;
  self->maxItems = queueLen;

  return self;
}

void*
MSGQUEUE_destructor (MSGQUEUE * self)
{
  pthread_mutex_destroy (&self->mtx);
  free (self->buff_ptr);
  return self;
}

int
MSGQUEUE_submitMsg (MSGQUEUE * self, const void *msgBuf)
/*******************************************************************
 * Submit a message to the message queue.
 */
{
  int rtn = 0;

  ez_pthread_mutex_lock (&self->mtx);

  if (self->numItems == self->maxItems)
    {
#ifdef DEBUG
      eprintf("WARNING: %p queue full.", self);
#endif
      rtn = EOF;
      goto abort;
    }

  /* If this is not the first item to be added */
  if (self->numItems)
    self->tail = (self->tail + 1) % self->maxItems;

  memcpy (self->buff_ptr + self->tail * self->msgSize, msgBuf, self->msgSize);

  ++self->numItems;

abort:
  ez_pthread_mutex_unlock (&self->mtx);
  return rtn;
}

int
MSGQUEUE_extractMsg (MSGQUEUE * self, void *msgBuf)
/*****************************************************************************
 * Extract a message from the message queue. Returns EOF when queue is empty.
 */
{
  int rtn = 0;

  ez_pthread_mutex_lock (&self->mtx);
  if (!self->numItems)
    {
      rtn = EOF;
      goto abort;
    }

  self->numItems--;
  memcpy (msgBuf, self->buff_ptr + self->head * self->msgSize, self->msgSize);

  if (self->numItems)
    self->head = (self->head + 1) % self->maxItems;

abort:
  ez_pthread_mutex_unlock (&self->mtx);
  return rtn;
}


int
MSGQUEUE_checkQueue (MSGQUEUE * self,
		     int (*check) (void *data_ptr, void *arg), void *arg)
/*****************************************************************************
 * Runs through the message queuue calling check() until it returns non-zero,
 * or the queue is fully traversed.
 * Return: 0 -> check() always returned zero, or queue was empty.
 *         otherwise, return value of check().
 */
{
  int rtn = 0;
  unsigned int i;
  void *ptr;

  ez_pthread_mutex_lock (&self->mtx);

  if (!self->numItems)
    goto abort;

  for (i = 0, ptr = self->buff_ptr + self->head * self->msgSize;
       i < self->numItems;
       ++i, ptr =
       self->buff_ptr + ((self->head + i) % self->maxItems) * self->msgSize)
    {
      if ((rtn = (*check) (ptr, arg)))
	break;
    }

abort:
  ez_pthread_mutex_unlock (&self->mtx);
  return rtn;
}

/************************************************************/
int _ez_MSGQUEUE_submitMsg(
   const char *fileName,
   int lineNo,
   const char *funcName,
      MSGQUEUE *self,
      const void *msgBuf
      )
{
   int rtn= MSGQUEUE_submitMsg (self, msgBuf);

   if(!rtn) return 0;

   _eprintf(fileName, lineNo, funcName, "MSGQUEUE_submitMsg() failed");
   abort();
}
