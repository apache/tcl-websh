/*
 * webshpool.h - pool of Tcl interpreters for mod_websh
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

#ifndef WEBSHPOOL_H
#define WEBSHPOOL_H

#include "tcl.h"		/* tcl is not necesseraly a system-wide include */
#include "macros.h"		/* for WebAssert and friends. can also reside in .c */
#include "hashutl.h"		/* hash table utitilies */
#include <time.h>

#include "modwebsh.h"
#include "mod_websh.h"

/* ----------------------------------------------------------------------------
 * the interp-pool is kept in a hash table where
 * - key is the ID (calculated from URL)
 * - entry is wInterpList
 * the pool is locked
 * ------------------------------------------------------------------------- */

typedef enum WebInterpState
{
    WIP_INUSE, WIP_FREE, WIP_EXPIRED, WIP_EXPIREDINUSE
}
WebInterpState;

struct WebInterpClass;


typedef struct WebInterp
{

    Tcl_Interp *interp;		/* the interp */
    WebInterpState state;	/* its state */

    struct WebInterpClass *interpClass;	/* infos about this type of interp */

    /* code blocks */
    Tcl_Obj *code;		/* per-request code (=file content) */
    Tcl_Obj *dtor;		/* destructor */

    /* dynamic members */

    long numrequests;		/* number of current request */
    long starttime;		/* start time (Tcl does not handle time_t */
    long lastusedtime;		/* last used time */

    /* we double-link this list so it's easier to remove elements */
    struct WebInterp *next;
    struct WebInterp *prev;

}
WebInterp;



typedef struct WebInterpClass
{

    char *filename;

    /* death reasons */
    long maxrequests;
    long maxttl;
    long maxidletime;
    long mtime;

    Tcl_Obj *code;		/* per-request code (=file content) */

    WebInterp *first;
    WebInterp *last;

    /* configuration of our main Interpreter */
    websh_server_conf *conf;

}
WebInterpClass;

WebInterp *createWebInterp(websh_server_conf * conf,
			   WebInterpClass * wic, char *filename, long mtime);

void destroyWebInterp(WebInterp * webInterp);

WebInterpClass *createWebInterpClass(websh_server_conf * conf, char *filename,
				     long mtime);



/* ----------------------------------------------------------------------------
 * External API
 * ------------------------------------------------------------------------- */

/*
 * creates a new interp or gets it from the pool based on the id
 * if it is new, then the file is loaded and placed into the preq
 * member.
 * also, it initalizes the req/resp structures given the passed
 * apache structures (which need to be type-adapted of course)

 *  this function does the following
    - aquire lock
    - search for id in hash table
    - if not found, set needtocreate = true;
      else: get HashEntry (which is a pooledInterp)
      walk through list (pooledInterp->next) until there is a free one
      if is free, check filemtime, if filemtime< mtime of the file
        - call "unsafeReleaseInterp" of the found interp
      else use this
      if at end of list and none found  -> set neetocreate = true;
    if needtocreate
      create a new Interp
      create a new pooledInterp
      insert it into list (if we have only next, then in the beginning
      of the list)
      if it is a new has entry, create hash entry
    endif
    - set current paramters (numrequests, req and resp structs)
    - release lock
 */

Tcl_Interp *createMainInterp(websh_server_conf * conf);

WebInterp *poolGetWebInterp(websh_server_conf * conf, char *filename,
			    long mtime, request_rec * r);
int initPool(websh_server_conf * conf);
void destroyPool(websh_server_conf * conf);
void cleanupPool(websh_server_conf * conf);
void poolReleaseWebInterp(WebInterp * webInterp);
void deleteInterpClass(WebInterpClass * webInterpClass);
int readWebInterpCode(WebInterp * wi, char *filename);

#endif
