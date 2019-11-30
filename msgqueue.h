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
#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <pthread.h>

typedef struct _MSGQUEUE
/*******************************
 * Necessary info for circular message
 * ring.
 */
{
  pthread_mutex_t mtx;
  unsigned int numItems, head, tail;
  unsigned int msgSize, maxItems;
  char *buff_ptr;
} MSGQUEUE;

#ifdef __cplusplus
extern "C"
{
#endif

MSGQUEUE*
MSGQUEUE_constructor (
      MSGQUEUE *self,
      size_t msgSize,
      unsigned int queueLen
      );
/*****************************************************************************
 * Prepare the MSGQUEUE structure for service.
 *
 * self - Address of the MSGQUEUE structure on which to operate.
 * msgSize - The size of messages this queue will handle.
 * queueLen - How many messages can be stored in this queue.
 *
 * Returns: NULL for failure, 'self' otherwise.
 */


#define MSGQUEUE_create(self, msgSize, queueLen)\
   (MSGQUEUE_constructor((self)=malloc(sizeof(MSGQUEUE)), msgSize, queueLen) ? (self) : ( self ?  realloc(MSGQUEUE_destructor(self),0): 0))
/*****************************************************************************
 * Allocate and prepare the MSGQUEUE structure for service.
 *
 * self - Pointer that will be set to the address of the MSGQUEUE structure.
 * msgSize - The size of messages this queue will handle.
 * queueLen - How many messages can be stored in this queue.
 *
 * Returns: NULL for failure, 'self' otherwise.
 */

void*
MSGQUEUE_destructor (MSGQUEUE *self);
/*****************************************************************************
 * Free resources associated with a MSGQUEUE. Note that 'self' is not free()'d.
 *
 * self - Address of the MSGQUEUE structure on which to operate.
 *
 * Returns: NULL for failure, 'self' otherwise.
 */

#define MSGQUEUE_destroy(s) \
  {if(MSGQUEUE_destructor(s)) {free(s);}}
/*****************************************************************************
 * Free resources associated with a MSGQUEUE, and free the structure.
 *
 * self - Address of the MSGQUEUE structure on which to operate.
 */

int
MSGQUEUE_submitMsg (
      MSGQUEUE *self,
      const void *msgBuf
      );
/*******************************************************************
 * Submit a message to the message queue.
 *
 * self - Address of the MSGQUEUE structure on which to operate.
 * msgBuf - buffer containing the message to be copied into the queue.
 *
 * Returns: 0 for success, non-zero otherwise.
 */
#define ez_MSGQUEUE_submitMsg(self, msgBuf) \
   _ez_MSGQUEUE_submitMsg(__FILE__, __LINE__, __FUNCTION__, self, msgBuf)
int _ez_MSGQUEUE_submitMsg(
   const char *fileName,
   int lineNo,
   const char *funcName,
      MSGQUEUE *self,
      const void *msgBuf
      );

#define MSGQUEUE_submitTypedMsg(self, msg) \
   MSGQUEUE_submitMsg(self, &(msg))

#define ez_MSGQUEUE_submitTypedMsg(self, msg) \
   ez_MSGQUEUE_submitMsg(self, &(msg))


int
MSGQUEUE_extractMsg (
      MSGQUEUE *self,
      void *msgBuf
      );
/*****************************************************************************
 * Extract a message from the message queue if possible.
 *
 * self - Address of the MSGQUEUE structure on which to operate.
 * msgBuf - buffer to which the message will be copied.
 *
 * Returns: 0 for success, EOF if the queue is empty.
 */
#define MSGQUEUE_extractTypedMsg(self, msg) \
   MSGQUEUE_extractMsg(self, &(msg))


int
MSGQUEUE_checkQueue (
      MSGQUEUE *self,
      int (*check) (void *pMsg, void *pData),
      void *pData
      );
/*****************************************************************************
 * Runs through the message queuue calling check() until it returns non-zero,
 * or the queue is fully traversed.
 *
 * self - Address of the MSGQUEUE structure on which to operate.
 * check - Function to be called for each message in the queue.
 * pData - A pointer that will be passed into check() when it is called.
 *
 * Return: 0 -> check() always returned zero, or queue was empty.
 *         otherwise, return value of check().
 */
#ifdef __cplusplus
}
#endif

#endif
