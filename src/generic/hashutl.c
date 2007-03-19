/*
 * hashutl.c --- utilities for Tcl_HashTable
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

#include <tcl.h>
#include "hashutl.h"

/* ----------------------------------------------------------------------------
 * resetHashTable -- destroy existing, init new HashTable
 * ------------------------------------------------------------------------- */
int resetHashTable(Tcl_HashTable * hash, int keyType)
{

    if (hash == NULL)
	return TCL_ERROR;

    Tcl_DeleteHashTable(hash);	/* we still have the struct htable */
    Tcl_InitHashTable(hash, keyType);	/* InitHT returns void */

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * resetHashTableWithContent -- destroy existing HashTable and all entries,
 * init new
 * ------------------------------------------------------------------------- */
int resetHashTableWithContent(Tcl_HashTable * hash, int keyType,
			      int (*delete_fnc) (void *, void *), void *env)
{

    HashTableIterator iterator;
    ClientData cur;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if (hash == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * loop: destroy entries
     * ----------------------------------------------------------------------- */
    assignIteratorToHashTable(hash, &iterator);

    while (nextFromHashIterator(&iterator) != TCL_ERROR) {

	cur = valueOfCurrent(&iterator);
	if (cur != NULL)
	    if (delete_fnc(cur, env) != TCL_OK) {
		return TCL_ERROR;
	    }
    }

    return resetHashTable(hash, keyType);
}

/* ----------------------------------------------------------------------------
 * appendToHashTable -- add new key-value to HashTable
 * ------------------------------------------------------------------------- */
int appendToHashTable(Tcl_HashTable * hash, char *key, ClientData data)
{

    int isNew = 0;
    Tcl_HashEntry *e = NULL;

    if (hash == NULL || key == NULL || data == NULL)
	return TCL_ERROR;
    e = Tcl_CreateHashEntry(hash, key, &isNew);
    if (isNew == 1) {
	Tcl_SetHashValue(e, data);
	return TCL_OK;
    }
    return TCL_ERROR;
}

/* ----------------------------------------------------------------------------
 * removeFromHashTable -- delete entry, return value
 * ------------------------------------------------------------------------- */
ClientData removeFromHashTable(Tcl_HashTable * hash, char *key)
{

    Tcl_HashEntry *e = NULL;
    ClientData data = NULL;

    if (hash == NULL || key == NULL)
	return NULL;

    e = Tcl_FindHashEntry(hash, key);
    if (e != NULL) {
	data = Tcl_GetHashValue(e);
	Tcl_DeleteHashEntry(e);
    }
    return data;
}

/* ----------------------------------------------------------------------------
 * getFromHashTable -- return value
 * ------------------------------------------------------------------------- */
ClientData getFromHashTable(Tcl_HashTable * hash, char *key)
{

    Tcl_HashEntry *e = NULL;
    if (hash == NULL || key == NULL)
	return NULL;

    e = Tcl_FindHashEntry(hash, key);
    if (e == NULL)
	return NULL;
    return Tcl_GetHashValue(e);
}

/* ----------------------------------------------------------------------------
 * assignIteratorToHashTable -- init iterator
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) assignIteratorToHashTable(Tcl_HashTable * hash,
			      HashTableIterator * iterator)
{

    if (hash == NULL || iterator == NULL)
	return TCL_ERROR;

    iterator->e = Tcl_FirstHashEntry(hash, &(iterator->hashSearch));
    iterator->current = NULL;
    iterator->htable = hash;

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * nextFromIterator -- get value from iterator
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) nextFromHashIterator(HashTableIterator * iterator)
{

    if (iterator == NULL)
	return TCL_ERROR;

    iterator->current = iterator->e;
    iterator->e = Tcl_NextHashEntry(&(iterator->hashSearch));
    if (iterator->current == NULL)
	return TCL_ERROR;

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * keyOfCurrent -- get key for current value of iterator
 * ------------------------------------------------------------------------- */
char __declspec(dllexport) *keyOfCurrent(HashTableIterator * iterator)
{

    if (iterator == NULL)
	return NULL;

    if (iterator->current != NULL)
	return Tcl_GetHashKey(iterator->htable, iterator->current);

    return NULL;
}

/* ----------------------------------------------------------------------------
 * valueOfCurrent -- get current value from iterator
 * ------------------------------------------------------------------------- */
ClientData __declspec(dllexport) valueOfCurrent(HashTableIterator * iterator)
{

    if (iterator == NULL)
	return NULL;

    if (iterator->current != NULL)
	return Tcl_GetHashValue(iterator->current);

    return NULL;
}
