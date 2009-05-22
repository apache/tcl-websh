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
#include "web.h"
#include "mod_websh.h"
#include "request.h"
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#else
/* define access mode constants if they are not defined yet 
   if we're under Windows (they are defined in unistd.h, which doesn't
   exist under Windows) currently only R_OK is used.
 */
#ifndef R_OK
#define R_OK 4
#endif
#endif

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

	webInterpClass->nextid = 0;

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
			   long mtime, request_rec *r)
{

    int result = 0;
    LogPlugIn *logtoap = NULL;
    Tcl_Obj *code = NULL;
    ApFuncs *apFuncs = NULL;

    WebInterp *webInterp = (WebInterp *) Tcl_Alloc(sizeof(WebInterp));

    webInterp->interp = Tcl_CreateInterp();

    if (webInterp->interp == NULL) {
	Tcl_Free((char *) webInterp);
#ifndef APACHE2
	ap_log_printf(conf->server, "createWebInterp: Could not create interpreter (id %ld, class %s)", webInterpClass->nextid, filename);
#else /* APACHE2 */
	ap_log_error(APLOG_MARK, APLOG_ERR, 0, conf->server,
		     "createWebInterp: Could not create interpreter (id %ld, class %s)", webInterpClass->nextid, filename);
#endif /* APACHE2 */
	return NULL;
    }

    /* just to be sure the memory command is imported if 
       the corresponding Tcl features it */
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(webInterp->interp);
#endif

    /* now register here all websh modules */
    result = Tcl_Init(webInterp->interp);
    /* checkme: test result */

    apFuncs = Tcl_GetAssocData(conf->mainInterp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs == NULL)
	return NULL;
    Tcl_SetAssocData(webInterp->interp, WEB_APFUNCS_ASSOC_DATA, NULL, (ClientData *) apFuncs);

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
    logtoap->handler = (int (*)(Tcl_Interp *, ClientData, char *)) logToAp;
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
	("rename exit exit.apache; proc exit {} {if {[info tclversion] >= 8.5} {return -level [expr {[info level] + 1}]} else {return -code error \"cannot exit script in mod_websh because process would terminate (use Tcl 8.5 or later if you want to use exit)\"}}", -1);

    Tcl_IncrRefCount(code);
    Tcl_EvalObjEx(webInterp->interp, code, 0);
    Tcl_DecrRefCount(code);

    Tcl_ResetResult(webInterp->interp);

    webInterp->dtor = NULL;
    webInterp->state = WIP_FREE;
    webInterp->numrequests = 0;
    webInterp->starttime = (long) r->request_time;
    webInterp->lastusedtime = (long) r->request_time;
    webInterp->interpClass = webInterpClass;
    webInterp->id = webInterpClass->nextid++;
    
    /* add to beginning of list of webInterpClass */
    webInterp->next = webInterpClass->first;
    if (webInterp->next != NULL)
      webInterp->next->prev = webInterp;
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
	    webInterp->code = NULL;
#ifndef APACHE2
	    ap_log_printf(r->server, 
			  "Could not readWebInterpCode (id %ld, class %s): %s",
			  webInterp->id, filename, Tcl_GetStringResult(webInterp->interp));
#else /* APACHE2 */
	    ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
			 "Could not readWebInterpCode (id %ld, class %s): %s",
			 webInterp->id, filename, Tcl_GetStringResult(webInterp->interp));
#endif /* APACHE2 */
	}
    }

    return webInterp;
}

/* ----------------------------------------------------------------------------
 * destroyWebInterp
 * ------------------------------------------------------------------------- */
void destroyWebInterp(WebInterp * webInterp)
{
    request_rec *r;

    if (webInterp->dtor != NULL) {

	int result;

	result = Tcl_Eval(webInterp->interp, "web::finalize");

	if (result != TCL_OK) {
	    r = (request_rec *) Tcl_GetAssocData(webInterp->interp, WEB_AP_ASSOC_DATA, NULL);
#ifndef APACHE2
	    ap_log_printf(r->server,
			 "web::finalize error: %s",
			 Tcl_GetStringResult(webInterp->interp));
#else /* APACHE2 */
	    ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
			 "web::finalize error: %s",
			 Tcl_GetStringResult(webInterp->interp));
#endif /* APACHE2 */
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
    char *id = NULL;
    Tcl_Obj *idObj = NULL;
    Tcl_Obj *mapCmd = NULL;
    Tcl_Obj *cmdList[2];
    int res;

    /* get interpreter id for filename */

    Tcl_MutexLock(&(conf->mainInterpLock));

    cmdList[0] = Tcl_NewStringObj("web::interpmap", -1);
    cmdList[1] = Tcl_NewStringObj(filename, -1);
    Tcl_IncrRefCount(cmdList[0]);
    Tcl_IncrRefCount(cmdList[1]);
    mapCmd = Tcl_NewListObj(2, cmdList);
    Tcl_IncrRefCount(mapCmd);
    res = Tcl_EvalObjEx(conf->mainInterp, mapCmd, 0);
    Tcl_DecrRefCount(mapCmd);
    Tcl_DecrRefCount(cmdList[0]);
    Tcl_DecrRefCount(cmdList[1]);

    if (res != TCL_OK) {
	/* no valid id for filename */
	Tcl_MutexUnlock(&(conf->mainInterpLock));
#ifndef APACHE2
	ap_log_printf(r->server,
		      "web::interpmap: %s",
		      Tcl_GetStringResult(conf->mainInterp));
#else /* APACHE2 */
	ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
		      "web::interpmap: %s",
		      Tcl_GetStringResult(conf->mainInterp));
#endif /* APACHE2 */
	return NULL;
    }

    idObj = Tcl_DuplicateObj(Tcl_GetObjResult(conf->mainInterp));
    Tcl_IncrRefCount(idObj);
    Tcl_ResetResult(conf->mainInterp);

    /* get absolute filename (if already absolute, same as
       Tcl_GetString(idObj) -> no DecrRefCount yet) */
    id = (char *) ap_server_root_relative(r->pool, Tcl_GetString(idObj));

    /* get last modified time for id */
    if (strcmp(id, filename)) {
	struct stat statPtr;
	if (Tcl_Access(id, R_OK) != 0 ||
 	    Tcl_Stat(id, &statPtr) != TCL_OK)
	{
	    Tcl_MutexUnlock(&(conf->mainInterpLock));
#ifndef APACHE2
	    ap_log_printf(r->server,
			  "cannot access or stat webInterpClass file '%s'", id);
#else /* APACHE2 */
	    ap_log_error(APLOG_MARK, APLOG_ERR, 0, r->server,
			 "cannot access or stat webInterpClass file '%s'", id);
#endif /* APACHE2 */
	    Tcl_DecrRefCount(idObj);
	    return NULL;
	}
	mtime = statPtr.st_mtime;
    }

    Tcl_MutexUnlock(&(conf->mainInterpLock));

    Tcl_MutexLock(&(conf->webshPoolLock));

    /* see if we have that id */
    entry = Tcl_FindHashEntry(conf->webshPool, id);

    if (entry != NULL) {

	/* yes, we have such a class */
	WebInterp *webInterp = NULL;

	webInterpClass = (WebInterpClass *) Tcl_GetHashValue(entry);

	/* check if mtime is ok */

	if (mtime > webInterpClass->mtime) {
	    /* invalidate all interpreters, code must be loaded from scratch */
	    webInterp = webInterpClass->first;
	    while (webInterp != NULL) {

		logToAp(webInterp->interp, NULL,
			"interpreter expired: source changed (id %ld, class %s)", webInterp->id, webInterp->interpClass->filename);
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
	webInterp = webInterpClass->first;

	while (webInterp != NULL) {

	    if ((webInterp->state) == WIP_FREE) {
		if (webInterpClass->maxidletime
		    && (r->request_time - webInterp->lastusedtime) >
		    webInterpClass->maxidletime) {
		    logToAp(webInterp->interp, NULL,
			    "interpreter expired: idle time reached (id %ld, claa %s)", webInterp->id, webInterp->interpClass->filename);
		    webInterp->state = WIP_EXPIRED;

		}
		else {
		    if (webInterpClass->maxttl
			&& (r->request_time - webInterp->starttime) >
			webInterpClass->maxttl) {
			logToAp(webInterp->interp, NULL,
				"interpreter expired: time to live reached (id %ld, class %s)", webInterp->id, webInterp->interpClass->filename);
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
#ifndef APACHE2
	    ap_log_printf(conf->server, 
			  "cannot create webInterpClass '%s'", id);
#else /* APACHE2 */
	    ap_log_error(APLOG_MARK, APLOG_NOERRNO|APLOG_ERR, 0, conf->server,
			 "cannot create webInterpClass '%s'", id);
#endif /* APACHE2 */
	    Tcl_DecrRefCount(idObj);
	    return NULL;
	}
    }

    if (found == NULL) {
	/* we have to create one */
	found = createWebInterp(conf, webInterpClass, id, mtime, r);
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

	    if (webInterpClass->maxrequests && (webInterp->numrequests >= webInterpClass->maxrequests)) {
		logToAp(webInterp->interp, NULL,
			"interpreter expired: request count reached (id %ld, class %s)", webInterp->id, webInterp->interpClass->filename);
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
    Tcl_FindExecutable(NULL);

    if (conf->mainInterp != NULL || conf->webshPool != NULL) {
	/* we have to cleanup */
#ifndef APACHE2
 	ap_log_printf(conf->server, "initPool: mainInterp or webshPool not NULL");
#else /* APACHE2 */
	ap_log_error(APLOG_MARK, APLOG_NOERRNO | APLOG_ERR, 0, conf->server,
		     "initPool: mainInterp or webshPool not NULL");
#endif /* APACHE2 */
	return 0;
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
    ApFuncs *apFuncs;

    if (mainInterp == NULL)
	return NULL;

    /* just to be sure the memory command is imported if 
       the corresponding Tcl features it */
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(mainInterp);
#endif

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(mainInterp,"8.2.0",0) == NULL) {
	Tcl_DeleteInterp(mainInterp);
	return NULL;
    }
#endif

    apFuncs = createApFuncs();
    if (apFuncs == NULL) {
        Tcl_DeleteInterp(mainInterp);
	return NULL;
    }
    Tcl_SetAssocData(mainInterp, WEB_APFUNCS_ASSOC_DATA, destroyApFuncs, (ClientData *) apFuncs);

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
    logtoap->handler = (int (*)(Tcl_Interp *, ClientData, char *)) logToAp;
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

		    /* check for expiry */
		    if (webInterpClass->maxidletime
			&& (t - webInterp->lastusedtime) >
			webInterpClass->maxidletime) {
			logToAp(webInterp->interp, NULL,
				"interpreter expired: idle time reached (id %ld, class %s)", webInterp->id, webInterp->interpClass->filename);
			webInterp->state = WIP_EXPIRED;
		    }
		    else {
			if (webInterpClass->maxttl
			    && (t - webInterp->starttime) >
			    webInterpClass->maxttl) {
			    logToAp(webInterp->interp, NULL,
				    "interpreter expired: time to live reached (id %ld, class %s)", webInterp->id, webInterp->interpClass->filename);
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
    Tcl_IncrRefCount(objPtr);
    chan = Tcl_OpenFileChannel(interp, filename, "r", 0644);
    if (chan == (Tcl_Channel) NULL) {
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "couldn't read file \"", filename,
			 "\": ", Tcl_ErrnoMsg(Tcl_GetErrno()), (char *) NULL);
    }
    else {
	if (Tcl_ReadChars(chan, objPtr, -1, 0) < 0) {
	    Tcl_Close(interp, chan);
	    Tcl_AppendResult(interp, "couldn't read file \"", filename,
			     "\": ", Tcl_ErrnoMsg(Tcl_GetErrno()), (char *) NULL);
	}
	else {
	    if (Tcl_Close(interp, chan) == TCL_OK) {
		/* finally success ... */
		webInterp->code = objPtr;
		return TCL_OK;
	    }
	}
    }
    Tcl_DecrRefCount(objPtr);
    return TCL_ERROR;
}

ApFuncs *createApFuncs() {
  ApFuncs *apFuncs = (ApFuncs *) Tcl_Alloc(sizeof(ApFuncs));
  if (apFuncs == NULL)
    return NULL;
  apFuncs->createDefaultResponseObj = createDefaultResponseObj_AP;
  apFuncs->isDefaultResponseObj = isDefaultResponseObj_AP;
  apFuncs->requestGetDefaultChannelName = requestGetDefaultChannelName_AP;
  apFuncs->requestGetDefaultOutChannelName = requestGetDefaultOutChannelName_AP;
  apFuncs->requestFillRequestValues = requestFillRequestValues_AP;
  apFuncs->Web_Initializer = Web_Initializer_AP;
  apFuncs->Web_Finalizer = Web_Finalizer_AP;
  apFuncs->Web_Finalize = Web_Finalize_AP;
  apFuncs->Web_InterpCfg = Web_InterpCfg_AP;
  apFuncs->Web_InterpClassCfg = Web_InterpClassCfg_AP;
  apFuncs->Web_MainEval = Web_MainEval_AP;
  apFuncs->Web_ConfigPath = Web_ConfigPath_AP;
  return apFuncs;
}

void destroyApFuncs(ClientData apFuncs, Tcl_Interp * interp) {
  if (apFuncs != NULL)
    Tcl_Free((char *) apFuncs);
}
