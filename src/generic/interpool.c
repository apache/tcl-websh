/*
 * interpool.c -- Interpreter Pool Manager for Apache Module
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

#include "hashutl.h"
#include "interpool.h"
#include "log.h"
#include "log.h"
#include "logtoap.h"
#include "macros.h"
#include "modwebsh.h"
#include "tcl.h"
#include "webutl.h"
#include "mod_websh.h"
#include "request.h"
#include <time.h>

/* init script for main interpreter */

#define MAININTERP_INITCODE "proc web::interpmap {filename} {return $filename}"

/* ----------------------------------------------------------------------------
 * createWebInterpClass
 * ------------------------------------------------------------------------- */
WebInterpClass *createWebInterpClass(websh_server_conf * conf, char *filename,
				     long mtime)
{

    WebInterpClass *webInterpClass = WebAllocInternalData(WebInterpClass);

    if (webInterpClass != NULL) {

	webInterpClass->conf = conf;
	webInterpClass->filename = allocAndSet(filename);

	webInterpClass->maxrequests = 1L;
	webInterpClass->maxttl = 0L;
	webInterpClass->maxidletime = 0L;

	webInterpClass->mtime = mtime;

	webInterpClass->first = NULL;
	webInterpClass->last = NULL;

	webInterpClass->code = NULL;	/* will be loaded on demand by first interp */

    }

    return webInterpClass;
}

/* ----------------------------------------------------------------------------
 * destroyWebInterpClass
 * ------------------------------------------------------------------------- */
int destroyWebInterpClass(WebInterpClass * webInterpClass)
{

    if (webInterpClass == NULL)
	return TCL_ERROR;

    while ((webInterpClass->first) != NULL) {
	destroyWebInterp(webInterpClass->first);
    }

    if (webInterpClass->code != NULL) {
	Tcl_DecrRefCount(webInterpClass->code);
    }

    Tcl_Free(webInterpClass->filename);
    Tcl_Free((char *) webInterpClass);

    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * createWebInterp
 * ------------------------------------------------------------------------- */
WebInterp *createWebInterp(websh_server_conf * conf,
			   WebInterpClass * webInterpClass, char *filename,
			   long mtime)
{

    int result = 0;
    time_t t = 0;
    LogPlugIn *logtoap = NULL;
    Tcl_Obj *code = NULL;

    WebInterp *webInterp = (WebInterp *) Tcl_Alloc(sizeof(WebInterp));

    webInterp->interp = Tcl_CreateInterp();

    if (webInterp->interp == NULL) {
	Tcl_Free((char *) webInterp);
	/* fixme: some logging needed? */
	return NULL;
    }


    /* now register here all websh modules */
    result = Tcl_Init(webInterp->interp);
    /* checkme: test result */
    result = Websh_Init(webInterp->interp);

    /* also register the destrcutor, etc. functions, passing webInterp as
       client data */

    /* ------------------------------------------------------------------------
     * register log handler "apachelog"
     * --------------------------------------------------------------------- */
    logtoap = createLogPlugIn();
    if (logtoap == NULL)
	return NULL;

    logtoap->constructor = createLogToAp;
    logtoap->destructor = destroyLogToAp;
    logtoap->handler = logToAp;
    registerLogPlugIn(webInterp->interp, APCHANNEL, logtoap);

    /* ------------------------------------------------------------------------
     * create commands
     * --------------------------------------------------------------------- */
    Tcl_CreateObjCommand(webInterp->interp, "web::initializer",
			 Web_Initializer, (ClientData) webInterp, NULL);

    Tcl_CreateObjCommand(webInterp->interp, "web::finalizer",
			 Web_Finalizer, (ClientData) webInterp, NULL);

    Tcl_CreateObjCommand(webInterp->interp, "web::finalize",
			 Web_Finalize, (ClientData) webInterp, NULL);

    Tcl_CreateObjCommand(webInterp->interp, "web::maineval",
			 Web_MainEval, (ClientData) webInterp, NULL);

    Tcl_CreateObjCommand(webInterp->interp, "web::interpcfg",
			 Web_InterpCfg, (ClientData) webInterp, NULL);

    Tcl_CreateObjCommand(webInterp->interp, "web::interpclasscfg",
			 Web_InterpClassCfg, (ClientData) conf, NULL);

    /* ------------------------------------------------------------------------
     * rename exit !
     * --------------------------------------------------------------------- */
    code =
	Tcl_NewStringObj
	("rename exit exit.apache; proc exit {} {uplevel #0 return}", -1);

    Tcl_IncrRefCount(code);
    Tcl_EvalObjEx(webInterp->interp, code, 0);
    Tcl_DecrRefCount(code);

    Tcl_ResetResult(webInterp->interp);

    webInterp->dtor = NULL;
    webInterp->state = WIP_FREE;
    webInterp->numrequests = 0;

    /* checkme: set correct starttime */
    time(&t);
    webInterp->starttime = (long) t;
    webInterp->lastusedtime = (long) t;
    webInterp->interpClass = webInterpClass;

    /* add to beginning of list of webInterpClass */
    webInterp->next = webInterpClass->first;
    webInterpClass->first = webInterp;
    webInterp->prev = NULL;

    if (webInterpClass->last == NULL)
	webInterpClass->last = webInterp;

    if (webInterpClass->code != NULL) {
	/* copy code from class */
	webInterp->code = Tcl_DuplicateObj(webInterpClass->code);
	Tcl_IncrRefCount(webInterp->code);
    }
    else {
	/* load the code into the object */
	if (readWebInterpCode(webInterp, filename) == TCL_OK) {
	    /* copy code to class */
	    webInterpClass->code = Tcl_DuplicateObj(webInterp->code);
	    Tcl_IncrRefCount(webInterpClass->code);
	    webInterpClass->mtime = mtime;
	}
	else {
	    /* fixme: BIG EXCEPTION, what to do? */
	    webInterp->code = NULL;
	}
    }
    return webInterp;
}

/* ----------------------------------------------------------------------------
 * destroyWebInterp
 * ------------------------------------------------------------------------- */
void destroyWebInterp(WebInterp * webInterp)
{

    if (webInterp->dtor != NULL) {

	int result;

	result = Tcl_Eval(webInterp->interp, "web::finalize");

	if (result != TCL_OK) {
	    /* fixme: log the message into our main interp */
	}

	Tcl_ResetResult(webInterp->interp);
    }

    /* --------------------------------------------------------------------------
     * cleanup code cache
     * ----------------------------------------------------------------------- */
    if (webInterp->dtor) {
	Tcl_DecrRefCount(webInterp->dtor);
	/* webInterp->dtor = NULL; */
    }
    if (webInterp->code) {
	Tcl_DecrRefCount(webInterp->code);
	webInterp->code = NULL;
    }

    Tcl_DeleteInterp(webInterp->interp);

    /* --------------------------------------------------------------------------
     * fixup list linkage
     * ----------------------------------------------------------------------- */
    if (webInterp->prev != NULL)
	/* we are not the first */
	webInterp->prev->next = webInterp->next;
    else
	webInterp->interpClass->first = webInterp->next;

    if (webInterp->next != NULL)
	/* we are not the last */
	webInterp->next->prev = webInterp->prev;
    else
	webInterp->interpClass->last = webInterp->prev;

    Tcl_Free((char *) webInterp);
}


/* ----------------------------------------------------------------------------
 * poolGetWebInterp - request new interp from pool
 * ------------------------------------------------------------------------- */
WebInterp *poolGetWebInterp(websh_server_conf * conf, char *filename,
			    long mtime, request_rec * r)
{

    Tcl_HashEntry *entry = NULL;
    WebInterp *found = NULL;
    WebInterpClass *webInterpClass = NULL;
    LogToApData *logToApData = NULL;
    char *id = NULL;
    Tcl_Obj *idObj = NULL;
    Tcl_Obj *mapCmd = NULL;
    int res;

    /* get interpreter id for filename */

    mapCmd = Tcl_NewStringObj("web::interpmap ", -1);
    Tcl_AppendToObj(mapCmd, filename, -1);
    Tcl_IncrRefCount(mapCmd);
    Tcl_MutexLock(&(conf->mainInterpLock));

    res = Tcl_EvalObjEx(conf->mainInterp, mapCmd, 0);

    idObj = Tcl_DuplicateObj(Tcl_GetObjResult(conf->mainInterp));
    Tcl_ResetResult(conf->mainInterp);

    if (res != TCL_OK) {
	/* no valid id for filename */
	Tcl_MutexUnlock(&(conf->mainInterpLock));
	Tcl_DecrRefCount(mapCmd);
	return NULL;
    }

    /* get absolute filename (if already absolute, same as 
       Tcl_GetString(idObj) -> no DecrRefCount yet) */
    id = (char *) ap_server_root_relative(r->pool, Tcl_GetString(idObj));

    /* get last modified time for id */
    if (strcmp(id, filename)) {
	/* take mtime from system */
	Tcl_Obj *fileCmd = Tcl_NewStringObj("file mtime ", -1);
	Tcl_AppendToObj(fileCmd, id, -1);
	Tcl_IncrRefCount(fileCmd);

	if (Tcl_EvalObjEx(conf->mainInterp, fileCmd, 0) == TCL_OK) {
	    /* we don't really care if it works or not */
	    Tcl_GetLongFromObj(conf->mainInterp,
			       Tcl_GetObjResult(conf->mainInterp), &mtime);
	}

	Tcl_ResetResult(conf->mainInterp);
	Tcl_DecrRefCount(fileCmd);
    }

    Tcl_MutexUnlock(&(conf->mainInterpLock));
    Tcl_DecrRefCount(mapCmd);

    Tcl_MutexLock(&(conf->webshPoolLock));

    /* see if we have that id */
    entry = Tcl_FindHashEntry(conf->webshPool, id);

    if (entry != NULL) {

	/* yes, we have such a class */
	WebInterp *webInterp = NULL;

	time_t t;

	webInterpClass = (WebInterpClass *) Tcl_GetHashValue(entry);

	/* check if mtime is ok */
	if (mtime > webInterpClass->mtime) {
	    /* invalidate all interpreters, code must be loaded from scratch */
	    webInterp = webInterpClass->first;
	    while (webInterp != NULL) {
		logToAp(webInterp->interp, NULL,
			"interpreter expired (source changed)");
		if (webInterp->state == WIP_INUSE)
		    webInterp->state = WIP_EXPIREDINUSE;
		else
		    webInterp->state = WIP_EXPIRED;
		webInterp = webInterp->next;
	    }
	    /* free code (will be loaded on demand) */
	    if (webInterpClass->code) {
		Tcl_DecrRefCount(webInterpClass->code);
		webInterpClass->code = NULL;
	    }
	}

	/* search a free interp */
	time(&t);
	webInterp = webInterpClass->first;

	while (webInterp != NULL) {

	    if ((webInterp->state) == WIP_FREE) {

		/* fixme: put id in log (for easier debugging) */

		/* check for expiry */
		if (webInterpClass->maxidletime
		    && (t - webInterp->lastusedtime) >
		    webInterpClass->maxidletime) {
		    logToAp(webInterp->interp, NULL,
			    "interpreter expired (idle time reached)");
		    webInterp->state = WIP_EXPIRED;

		}
		else {
		    if (webInterpClass->maxttl
			&& (t - webInterp->starttime) >
			webInterpClass->maxttl) {
			logToAp(webInterp->interp, NULL,
				"interpreter expired (time to live reached)");
			webInterp->state = WIP_EXPIRED;
		    }
		    else {
			found = webInterp;
			break;
		    }
		}
	    }
	    webInterp = webInterp->next;

	}
    }
    else {

	/* no, we have to create this new interpreter class */

	webInterpClass = createWebInterpClass(conf, id, mtime);

	if (webInterpClass != NULL) {

	    int isnew = 0;
	    entry = Tcl_CreateHashEntry(conf->webshPool, id, &isnew);
	    /* isnew must be 1, since we searched already */
	    Tcl_SetHashValue(entry, (ClientData) webInterpClass);

	}
	else {

	    Tcl_MutexUnlock(&(conf->webshPoolLock));
	    Tcl_DecrRefCount(idObj);
	    /* fixme: do something useful */
	    /* logToAp(webInterp->interp,NULL,"panic - cannot create webInterpClass '%s'", id); */

	    return NULL;
	}
    }

    if (found == NULL) {
	/* we have to create one */
	found = createWebInterp(conf, webInterpClass, id, mtime);
    }

    if (found != NULL) {
	/* mark the found one as INUSE */
	found->state = WIP_INUSE;
    }

    Tcl_MutexUnlock(&(conf->webshPoolLock));
    Tcl_DecrRefCount(idObj);
    return found;
}

/* ----------------------------------------------------------------------------
 * poolReleaseWebInterp - bring intrp back into pool
 * ------------------------------------------------------------------------- */
void poolReleaseWebInterp(WebInterp * webInterp)
{

    if (webInterp != NULL) {

	WebInterpClass *webInterpClass = webInterp->interpClass;

	Tcl_MutexLock(&(webInterpClass->conf->webshPoolLock));

	webInterp->lastusedtime = (long) time(NULL);

	webInterp->numrequests++;

	if (webInterp->state == WIP_EXPIREDINUSE)
	    webInterp->state = WIP_EXPIRED;
	else {
	    webInterp->state = WIP_FREE;

	    /* check for num requests */

	    /* fixme: add interp id to log */
	    if (webInterp->numrequests >= webInterpClass->maxrequests) {
		logToAp(webInterp->interp, NULL,
			"interpreter expired (request count reached)");
		webInterp->state = WIP_EXPIRED;
	    }
	}

	/* cleanup all EXPIRED interps */
	cleanupPool(webInterpClass->conf);

	Tcl_MutexUnlock(&(webInterpClass->conf->webshPoolLock));
    }
}


/* ----------------------------------------------------------------------------
 * init
 * ------------------------------------------------------------------------- */
int initPool(websh_server_conf * conf)
{

    /* fixme: is this really called once, by a single thread, and we don't do
       any locking in here ?? */

    Tcl_FindExecutable(NULL);

    if (conf->mainInterp != NULL || conf->webshPool != NULL) {
	/* we have to cleanup */
	/* fixme: in threaded mode, this is a bit too rude maybe */
	destroyPool(conf);
    }

    /* create a single main interpreter */
    conf->mainInterp = createMainInterp(conf);

    if (conf->mainInterp == NULL) {
	errno = 0;
#ifndef APACHE2
	ap_log_printf(conf->server, "could'nt create main interp");
#else /* APACHE2 */
	ap_log_error(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, conf->server,
		     "could'nt create main interp");
#endif /* APACHE2 */
	return 0;
    }

    /* create our table of interp classes */
    HashUtlAllocInit(conf->webshPool, TCL_STRING_KEYS);


    /* see if we have a config file to evaluate */
    if (conf->scriptName != NULL) {
	if (Tcl_EvalFile(conf->mainInterp, (char *) conf->scriptName) ==
	    TCL_ERROR) {
	    errno = 0;
#ifndef APACHE2
	    ap_log_printf(conf->server,
			  Tcl_GetStringResult(conf->mainInterp));
#else /* APACHE2 */
	    ap_log_error(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0,
			 conf->server, Tcl_GetStringResult(conf->mainInterp));
#endif /* APACHE2 */
	    return 0;
	}
	Tcl_ResetResult(conf->mainInterp);
    }

    /* if we're in threaded mode, spawn a watcher thread
       that runs a possibly defined code and does cleanup, something like:

       if (weAreAThreadedOne) {
       CreateThread(interp);
       }
     */
    /* that's it */

    return 1;
}


/* ----------------------------------------------------------------------------
 * create main interpreter (including init stuff)
 * ------------------------------------------------------------------------- */
Tcl_Interp *createMainInterp(websh_server_conf * conf)
{

    LogPlugIn *logtoap = NULL;
    Tcl_Interp *mainInterp = Tcl_CreateInterp();

    if (mainInterp == NULL)
	return NULL;

    /* init with tcl >= 8.2 is allowed */
    Tcl_InitStubs(mainInterp, "8.2", 0);

    /* standard Init */
    if (Tcl_Init(mainInterp) == TCL_ERROR) {
	Tcl_DeleteInterp(mainInterp);
	return NULL;
    }

    /* register Log Module in here */
    if (log_Init(mainInterp) == TCL_ERROR) {
	Tcl_DeleteInterp(mainInterp);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * register log handler "apachelog"
     * ----------------------------------------------------------------------- */
    logtoap = createLogPlugIn();
    if (logtoap == NULL) {
	Tcl_DeleteInterp(mainInterp);
	return NULL;
    }
    logtoap->constructor = createLogToAp;
    logtoap->destructor = destroyLogToAp;
    logtoap->handler = logToAp;
    registerLogPlugIn(mainInterp, APCHANNEL, logtoap);

    /* eval init code */
    if (Tcl_Eval(mainInterp, MAININTERP_INITCODE) == TCL_ERROR) {
	Tcl_DeleteInterp(mainInterp);
	return NULL;
    }

    Tcl_CreateObjCommand(mainInterp, "web::interpclasscfg",
			 Web_InterpClassCfg, (ClientData) conf, NULL);

    return mainInterp;
}


/* ----------------------------------------------------------------------------
 * destroy
 * ------------------------------------------------------------------------- */
void destroyPool(websh_server_conf * conf)
{

    if (conf == NULL)
	return;

    if (conf->webshPool != NULL) {
	Tcl_HashEntry *entry;
	Tcl_HashSearch search;

	Tcl_MutexLock(&(conf->webshPoolLock));
	while ((entry = Tcl_FirstHashEntry(conf->webshPool, &search)) != NULL) {
	    /* loop through entries */
	    destroyWebInterpClass((WebInterpClass *) Tcl_GetHashValue(entry));
	    Tcl_DeleteHashEntry(entry);
	}
	Tcl_DeleteHashTable(conf->webshPool);
	Tcl_MutexUnlock(&(conf->webshPoolLock));
	conf->webshPool = NULL;
    }

    if (conf->mainInterp != NULL) {
	/* now delete the interp */
	Tcl_DeleteInterp(conf->mainInterp);
	conf->mainInterp = NULL;
    }

}


/* -------------------------------------------------------------------------
 * cleanupPool NOTE: pool must be locked by caller
 * ------------------------------------------------------------------------- */
void cleanupPool(websh_server_conf * conf)
{

    if (conf->webshPool != NULL) {
	Tcl_HashEntry *entry;
	Tcl_HashSearch search;
	WebInterpClass *webInterpClass;
	WebInterp *webInterp, *expiredInterp;
	time_t t;

	time(&t);

	entry = Tcl_FirstHashEntry(conf->webshPool, &search);
	while (entry != NULL) {
	    /* loop through entries */
	    webInterpClass = (WebInterpClass *) Tcl_GetHashValue(entry);

	    webInterp = webInterpClass->first;
	    while (webInterp != NULL) {
		/* NOTE: check on max requests is done by poolReleaseWebInterp */
		if ((webInterp->state) == WIP_FREE) {

		    /* fixme: put id in log (for easier debugging) */

		    /* check for expiry */
		    if (webInterpClass->maxidletime
			&& (t - webInterp->lastusedtime) >
			webInterpClass->maxidletime) {
			logToAp(webInterp->interp, NULL,
				"interpreter expired (idle time reached)");
			webInterp->state = WIP_EXPIRED;

		    }
		    else {
			if (webInterpClass->maxttl
			    && (t - webInterp->starttime) >
			    webInterpClass->maxttl) {
			    logToAp(webInterp->interp, NULL,
				    "interpreter expired (time to live reached)");
			    webInterp->state = WIP_EXPIRED;
			}
		    }
		}
		expiredInterp = webInterp;
		webInterp = webInterp->next;

		if (expiredInterp->state == WIP_EXPIRED)
		    destroyWebInterp(expiredInterp);
	    }
	    entry = Tcl_NextHashEntry(&search);
	}
    }

    return;
}


/* ----------------------------------------------------------------------------
 * readWebInterpCode
 * ------------------------------------------------------------------------- */
int readWebInterpCode(WebInterp * webInterp, char *filename)
{

    Tcl_Channel chan;
    Tcl_Obj *objPtr;
    Tcl_Interp *interp = webInterp->interp;

    objPtr = Tcl_NewObj();
    chan = Tcl_OpenFileChannel(interp, filename, "r", 0644);
    if (chan == (Tcl_Channel) NULL) {
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "couldn't read file \"", filename,
			 "\": ", Tcl_PosixError(interp), (char *) NULL);
    }
    else {
	if (Tcl_ReadChars(chan, objPtr, -1, 0) < 0) {
	    Tcl_Close(interp, chan);
	    Tcl_AppendResult(interp, "couldn't read file \"", filename,
			     "\": ", Tcl_PosixError(interp), (char *) NULL);
	}
	else {
	    if (Tcl_Close(interp, chan) == TCL_OK) {
		/* finally success ... */
		webInterp->code = objPtr;
		Tcl_IncrRefCount(objPtr);
		return TCL_OK;
	    }
	}
    }
    Tcl_DecrRefCount(objPtr);
    return TCL_ERROR;
}
