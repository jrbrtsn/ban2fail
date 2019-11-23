/***************************************************************************
                          map.h  -  description

Generic hashed map class.
                             -------------------
    begin                : Thu Jul 12 2001
    copyright            : (C) 2001 by John D. Robertson
    email                : john@rrci.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef MAP_H
#define MAP_H

#include "ptrvec.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* Structure req'd for each map */
typedef struct {
   PTRVEC* bucketArr;
   unsigned int numBuckets;
} MAP;

/***************** Function prototypes and macros *******************/
#define MAP_is_init(s) \
   ((s)->bucketArr)

MAP*
MAP_constructor(MAP *self,
                    unsigned int numBuckets,
                    unsigned int slotsPerBucket);
/****************************************************************
 * Construct a map with numBuckets buckets. slotsPerBucket provides
 * an initial sizing of the buckets, but they can grow dynamically.
 */

#define MAP_create(self, numBuckets, slotsPerBucket)\
   (MAP_constructor((self)=malloc(sizeof(MAP)), numBuckets, slotsPerBucket) ? (self) : ( self ?  realloc(MAP_destructor(self),0): 0))
/***********************************************************
 * Same as constructor with the addition of the object being
 * malloc()'ed for you.
 */

int
MAP_sinit(MAP *self,
              unsigned int numBuckets,
              unsigned int slotsPerBucket);
/***********************************************************
 * Initialize or clear() for a static instance.
 */

void*
MAP_destructor(MAP *self);
/************************************************
 * Free resources associated with object.
 */

#define MAP_destroy(s)\
  {if(MAP_destructor(s)) {free(s); (s)=NULL;}}
/***********************************************************
 * Same as destructor with the addition of freeing the object.
 */

int
MAP_clearAndDestroy(MAP *self, void *(* destructor)(void *item_ptr));
/****************************************************************
 * Call destructors on all item_ptr's and free() them, and
 * free() all key records.
 */

#define MAP_clear(self) \
  MAP_clearAndDestroy(self, NULL)
/****************************************************************
 * free() all key records.
 */

int
MAP_addKey(MAP *self,
               const void *key_ptr,
               unsigned int keyLen,
               void *item_ptr);
/*********************************************************************************
 * Add a key to map, no checking for duplicates.
 */

#define MAP_addTypedKey(self, key, item_ptr) \
  MAP_addKey(self, &(key), sizeof(key), item_ptr)

#define MAP_addStrKey(self, keystr, item_ptr) \
  MAP_addKey(self, keystr, strlen(keystr), item_ptr)

void*
MAP_findItem(MAP *self,
                 const void *key_ptr,
                 unsigned int keyLen);
/******************************************************************************
 * Find the the first matching key and return it's item_ptr.
 * Returns:
 * NULL           Not found
 * item_ptr       first one found
 */
#define MAP_findTypedItem(self, key) \
  MAP_findItem(self, &(key), sizeof(key))

#define MAP_findStrItem(self, keystr) \
  MAP_findItem(self, keystr, strlen(keystr))

void*
MAP_removeItem(MAP *self,
                   const void *key_ptr,
                   unsigned int keyLen);
/******************************************************************************
 * Find the the first matching key and remove it from the map.
 * Returns:
 * NULL           Not found
 * item_ptr       first one found
 */
#define MAP_removeTypedItem(self, key) \
  MAP_removeItem(self, &(key), sizeof(key))

#define MAP_removeStrItem(self, keystr) \
  MAP_removeItem(self, keystr, strlen(keystr))

void*
MAP_removeSpecificItem(MAP *self,
                           const void *key_ptr,
                           unsigned int keyLen,
                           void *pItem);
/******************************************************************************
 * Find the first matching key and pointer, and remove it from the map.
 * pItem is the address of the specific item to be removed.
 * Returns:
 * NULL           Not found
 * item_ptr       first one found
 */
#define MAP_removeSpecificTypedItem(self, key, pItem) \
  MAP_removeSpecificItem(self, &(key), sizeof(key), pItem)

#define MAP_removeSpecificStrItem(self, keystr, pItem) \
  MAP_removeSpecificItem(self, keystr, strlen(keystr), pItem)

int
MAP_findItems(MAP *self,
                  void* rtnArr[],
                  unsigned int rtnArrSize,
                  const void *key_ptr,
                  unsigned int keyLen);
/******************************************************************************
 * Find all matching key(s) in the map, and put the accompanying item_ptr's
 * into the rtnArr. Returns:
 * -1             Insufficient space in rtnArr
 * 0 .. INT_MAX   Number of item_ptr's returned in rtnArr
 */

#define MAP_findTypedItems(self, rtnArr, rtnArrSize, key) \
  MAP_findItems(self, rtnArr, rtnArrSize, &(key), sizeof(key))

#define MAP_findStrItems(self, rtnArr, rtnArrSize, keystr) \
  MAP_findItems(self, rtnArr, rtnArrSize, keystr, strlen(keystr))

int
MAP_visitAllEntries(MAP *self, int (* func)(void *item_ptr, void *data), void *data);
/******************************************************************************
 * Visit all entries in the map. if (*func) returns nonzero, then
 * the process stops and MAP_visitAllEntries will return non-zero.
 */

void
MAP_fetchAllItems(MAP *self, void **rtn_arr);
/******************************************************************************
 * Place the item pointers into the supplied array.
 */

unsigned
MAP_numItems(MAP *self);
/******************************************************************************
 * Return a count of the items indexed in the map.
 */

#ifdef __cplusplus
}
#endif

#endif
