/*
 * hashutl.h --
 * nca-073-9
 * 
 * Copyright (c) 1996-2000 by Netcetera AG.
 * Copyright (c) 2001 by Apache Software Foundation.
 * All rights reserved.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id$
 *
 */

#include "tcl.h"
#include "macros.h"

#ifndef HASHUTL_H
#define HASHUTL_H

#define HashUtlAllocInit(hash,type) \
  hash = WebAllocInternalData(Tcl_HashTable); \
  if( hash != NULL ) Tcl_InitHashTable(hash,type);


#define HashUtlDelFree(hash) \
  Tcl_DeleteHashTable(hash); \
  Tcl_Free((char *)hash);

typedef struct HashTableIterator
{
    Tcl_HashSearch hashSearch;
    Tcl_HashTable *htable;
    Tcl_HashEntry *e;
    Tcl_HashEntry *current;
}
HashTableIterator;

int resetHashTable(Tcl_HashTable * hash, int keyType);
int resetHashTableWithContent(Tcl_HashTable * hash, int keyType,
			      int (*delete_fnc) (void *, void *), void *env);
int appendToHashTable(Tcl_HashTable * hash, char *key, ClientData data);

ClientData removeFromHashTable(Tcl_HashTable * hash, char *key);
ClientData getFromHashTable(Tcl_HashTable * hash, char *key);

int assignIteratorToHashTable(Tcl_HashTable * hash,
			      HashTableIterator * iterator);
int nextFromHashIterator(HashTableIterator * iterator);

char *keyOfCurrent(HashTableIterator * iterator);
ClientData valueOfCurrent(HashTableIterator * iterator);

int resetHashIterator(HashTableIterator * iterator);

#endif
