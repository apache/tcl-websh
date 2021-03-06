/*
 * modwebsh_ap.c -- web::initializer, web::finalizer
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

#include <sys/stat.h>
#include "tcl.h"
#include "mod_websh.h"
#include "modwebsh.h"
#include "interpool.h"

/* -------------------------------------------------------------------------
 * Web_Initializer -- if request counter is 0, eval the code
 * ------------------------------------------------------------------------- */
int Web_Initializer_AP(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    WebInterp *webInterp = NULL;
    int res = 0;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "code");
	return TCL_ERROR;
    }
    if (clientData == NULL) {
	LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
		__FILE__, __LINE__,
		"web::initializer", WEBLOG_ERROR,
		"panic - cannot access web interp", NULL);
	return TCL_ERROR;
    }

    webInterp = (WebInterp *) clientData;

    if (webInterp->numrequests == 0) {

	/* keep track of log stuff */
	LogData * logData = (LogData *) Tcl_GetAssocData(interp, WEB_LOG_ASSOC_DATA, NULL);
	if (logData != NULL)
	  logData->keep = 1;

	Tcl_IncrRefCount(objv[1]);
	res = Tcl_EvalObjEx(interp, objv[1], 0);
	Tcl_DecrRefCount(objv[1]);

	/* reset log flag */
	if (logData != NULL)
	  logData->keep = 0;

	if (res != TCL_OK) {

	    LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
		    __FILE__, __LINE__,
		    "web::initializer", WEBLOG_ERROR,
		    "error evaluating \"", Tcl_GetString(objv[1]), "\"",
		    NULL);
	}

	Tcl_ResetResult(interp);

	return res;
    }
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_Finalizer -- register the destructor (to be called when WebInterp dies)
 * ------------------------------------------------------------------------- */
int Web_Finalizer_AP(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    WebInterp *webInterp = (WebInterp *) clientData;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "code");
	return TCL_ERROR;
    }
    if (clientData == NULL) {
	LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
		__FILE__, __LINE__,
		"web::finalizer", WEBLOG_ERROR,
		"panic - cannot access web interp", NULL);
	return TCL_ERROR;
    }

    if (webInterp->numrequests == 0) {

	if (webInterp->dtor == NULL) {

	    /* first time: make Tcl_Obj */
	    webInterp->dtor = Tcl_NewListObj(1, &objv[1]);
	    Tcl_IncrRefCount(webInterp->dtor);
	}
	else {

	    int length = -1;
	    int result = 0;

	    /* add code to list */
	    result = Tcl_ListObjLength(interp, webInterp->dtor, &length);
	    if (result == TCL_OK) {
		result = Tcl_ListObjReplace(interp,
					    webInterp->dtor, length, 0, 1,
					    &objv[1]);
		/* checkme: check result */
	    }
	}
/*     printf("DBG Web_Finalizer -- now: %s\n",Tcl_GetString(webInterp->dtor)); fflush(stdout); */
    }

    return TCL_OK;
}

/* -------------------------------------------------------------------------
 * Web_Finalize -- call registered finalizers, in turn
 * ------------------------------------------------------------------------- */
int Web_Finalize_AP(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    WebInterp *webInterp = (WebInterp *) clientData;

    int res = TCL_OK;
    int len = -1;
    int i = 0;
    Tcl_Obj *tobj = NULL;

    if (webInterp == NULL)
	return TCL_ERROR;
    if (webInterp->interp == NULL)
	return TCL_ERROR;

    if (webInterp->dtor == NULL)
	return TCL_OK;

    res = Tcl_ListObjLength(webInterp->interp, webInterp->dtor, &len);
    if (res == TCL_OK) {

	for (i = (len - 1); i >= 0; i--) {

	    /* call finalizers in turn */
	    res =
		Tcl_ListObjIndex(webInterp->interp, webInterp->dtor, i,
				 &tobj);
	    if ((res == TCL_OK) && (tobj != NULL)) {

/*         printf("DBG Finalize: calling \"%s\"\n",Tcl_GetString(tobj)); fflush(stdout); */

		Tcl_IncrRefCount(tobj);
		res = Tcl_EvalObjEx(webInterp->interp, tobj, 0);
		Tcl_DecrRefCount(tobj);

		/* Tcl_ResetResult(webInterp->interp); */

		if (res != TCL_OK) {

/*           fprintf(stderr,"DBG Finalize -- PROBLEM: %s\n",Tcl_GetStringResult(webInterp->interp)); fflush(stderr); */

		    LOG_MSG(webInterp->interp, WRITE_LOG | INTERP_ERRORINFO,
			    __FILE__, __LINE__,
			    "web::finalize", WEBLOG_ERROR,
			    "error evaluating \"", Tcl_GetString(tobj), "\"",
			    NULL);
		    continue;
		}
	    }
	}
/*     printf("DBG Finalize -- done\n"); fflush(stdout); */

    }
    return res;
}


/* -------------------------------------------------------------------------
 * Web_InterpCfg -- set WebInterp properties
 * ------------------------------------------------------------------------- */
int Web_InterpCfg_AP(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int index;

    static TCLCONST char *interpParams[] = {
	"numrequests",
	"starttime",
	"lastusedtime",
	"retire",
	NULL
    };

    enum params
    {
      INTERP_REQUESTS,
      INTERP_START,
      INTERP_LASTUSED,
      INTERP_RETIRE
    };

    WebInterp *webInterp = (WebInterp *) clientData;

    WebAssertObjc(objc > 3, 1, "?key ?value??");


    if (objc == 1) {
	/* return interpreter class name */
	Tcl_SetResult(interp, webInterp->interpClass->filename, TCL_VOLATILE);
	return TCL_OK;
    }

    if (Tcl_GetIndexFromObj(interp, objv[1], interpParams, "parameter", 0,
			    &index) != TCL_OK) {
	return TCL_ERROR;
    }

    switch ((enum params) index) {
    case INTERP_REQUESTS:{
	    long numrequests = webInterp->numrequests;
	    if (objc == 3)
		if (Tcl_GetLongFromObj
		    (interp, objv[2], &(webInterp->numrequests)) != TCL_OK) {
		    return TCL_ERROR;
		}
	    Tcl_SetObjResult(interp, Tcl_NewLongObj(numrequests));
	    break;
	}
    case INTERP_START:{
	    long starttime = webInterp->starttime;
	    if (objc == 3)
		if (Tcl_GetLongFromObj
		    (interp, objv[2], &(webInterp->starttime)) != TCL_OK) {
		    return TCL_ERROR;
		}
	    Tcl_SetObjResult(interp, Tcl_NewLongObj(starttime));
	    break;
	}
    case INTERP_LASTUSED:{
	    long lastusedtime = webInterp->lastusedtime;
	    if (objc == 3)
		if (Tcl_GetLongFromObj
		    (interp, objv[2], &(webInterp->lastusedtime)) != TCL_OK) {
		    return TCL_ERROR;
		}
	    Tcl_SetObjResult(interp, Tcl_NewLongObj(lastusedtime));
	    break;
	}
    case INTERP_RETIRE:{
	    int expire = (webInterp->state == WIP_EXPIREDINUSE);
	    if (objc == 3) {
		int retire = 0;
		if (Tcl_GetBooleanFromObj(interp, objv[2], &retire) != TCL_OK) {
		    return TCL_ERROR;
		}
		if (retire)
		    webInterp->state = WIP_EXPIREDINUSE;
		else
		    webInterp->state = WIP_INUSE;
	    }
	    Tcl_SetObjResult(interp, Tcl_NewIntObj(expire));
	    break;
	}
    }
    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * Web_InterpClassCfg -- set WebInterpClass properties
 * ------------------------------------------------------------------------- */
int Web_InterpClassCfg_AP(ClientData clientData,
		       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    Tcl_HashEntry *entry;
    char *id;
    WebInterpClass *webInterpClass = NULL;
    int index;

    static TCLCONST char *classParams[] = {
      "maxttl", 
      "maxidletime",
      "maxrequests",
      NULL
    };
    enum params
    {
      CLASS_TTL,
      CLASS_IDLETIME,
      CLASS_REQUESTS
    };

    websh_server_conf *conf = (websh_server_conf *) clientData;

    WebAssertObjc(objc < 3 || objc > 4, 1, "id parameter ?value?");

    id = Tcl_GetString(objv[1]);

    Tcl_MutexLock(&(conf->webshPoolLock));

    /* see if we have that id */
    entry = Tcl_FindHashEntry(conf->webshPool, id);
    if (entry != NULL) {
	webInterpClass = (WebInterpClass *) Tcl_GetHashValue(entry);
    }

    if (webInterpClass == NULL) {
	struct stat statPtr;
	int isnew = 0;

	Tcl_Stat(id, &statPtr);
	webInterpClass = createWebInterpClass(conf, id, statPtr.st_mtime);
	entry = Tcl_CreateHashEntry(conf->webshPool, id, &isnew);
	Tcl_SetHashValue(entry, (ClientData) webInterpClass);
    }

    if (Tcl_GetIndexFromObj(interp, objv[2], classParams, "parameter", 0,
			    &index) != TCL_OK) {
	Tcl_MutexUnlock(&(conf->webshPoolLock));
	return TCL_ERROR;
    }

    switch ((enum params) index) {
    case CLASS_TTL:{
	    long maxttl = webInterpClass->maxttl;
	    if (objc == 4)
		if (Tcl_GetLongFromObj
		    (interp, objv[3], &(webInterpClass->maxttl)) != TCL_OK) {
		    Tcl_MutexUnlock(&(conf->webshPoolLock));
		    return TCL_ERROR;
		}
	    Tcl_SetObjResult(interp, Tcl_NewLongObj(maxttl));
	    break;
	}
    case CLASS_IDLETIME:{
	    long maxidletime = webInterpClass->maxidletime;
	    if (objc == 4)
		if (Tcl_GetLongFromObj
		    (interp, objv[3],
		     &(webInterpClass->maxidletime)) != TCL_OK) {
		    Tcl_MutexUnlock(&(conf->webshPoolLock));
		    return TCL_ERROR;
		}
	    Tcl_SetObjResult(interp, Tcl_NewLongObj(maxidletime));
	    break;
	}
    case CLASS_REQUESTS:{
	    long maxrequests = webInterpClass->maxrequests;
	    if (objc == 4)
		if (Tcl_GetLongFromObj
		    (interp, objv[3],
		     &(webInterpClass->maxrequests)) != TCL_OK) {
		    Tcl_MutexUnlock(&(conf->webshPoolLock));
		    return TCL_ERROR;
		}
	    Tcl_SetObjResult(interp, Tcl_NewLongObj(maxrequests));
	    break;
	}
    }

    Tcl_MutexUnlock(&(conf->webshPoolLock));
    return TCL_OK;

}


/* ----------------------------------------------------------------------------
 * Web_MainEval -- eval in main interp
 * ------------------------------------------------------------------------- */
int Web_MainEval_AP(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int res = 0;
    websh_server_conf *conf;

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "code");
	return TCL_ERROR;
    }

    if (clientData == NULL) {
	LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
		__FILE__, __LINE__,
		"web::maineval", WEBLOG_ERROR,
		"panic - cannot access main interp", NULL);
	return TCL_ERROR;
    }

    conf = ((WebInterp *) clientData)->interpClass->conf;

    if (conf->mainInterp == NULL) {
	LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
		__FILE__, __LINE__,
		"web::maineval", WEBLOG_ERROR,
		"panic - cannot access main interp", NULL);
	return TCL_ERROR;
    }

    Tcl_MutexLock(&(conf->mainInterpLock));

    Tcl_IncrRefCount(objv[1]);
    res = Tcl_EvalObjEx(conf->mainInterp, objv[1], 0);
    Tcl_DecrRefCount(objv[1]);

    if (res != TCL_OK) {

	LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
		__FILE__, __LINE__,
		"web::maineval", WEBLOG_ERROR,
		"error evaluating \"", Tcl_GetString(objv[1]), "\"", NULL);
    }

    /* code from TclTransferResult tclResult.c */

    /* since Interp struct is not visible there is no access to flags
       -> just do a simple transfer of what errorInfo, errorCode an Result */

    {
	Tcl_Obj *objPtr;

	if (res == TCL_ERROR) {

	    Tcl_ResetResult(interp);

	    objPtr = Tcl_GetVar2Ex(conf->mainInterp, "errorInfo", NULL,
				   TCL_GLOBAL_ONLY);
	    Tcl_SetVar2Ex(interp, "errorInfo", NULL, objPtr, TCL_GLOBAL_ONLY);

	    objPtr = Tcl_GetVar2Ex(conf->mainInterp, "errorCode", NULL,
				   TCL_GLOBAL_ONLY);
	    Tcl_SetVar2Ex(interp, "errorCode", NULL, objPtr, TCL_GLOBAL_ONLY);

	}

	Tcl_SetObjResult(interp, Tcl_GetObjResult(conf->mainInterp));
	Tcl_ResetResult(conf->mainInterp);
    }

    Tcl_MutexUnlock(&(conf->mainInterpLock));

    return res;
}


/* ----------------------------------------------------------------------------
 * Web_ConfigPath -- (sub)command in pool interp (called from Web_Cfg)
 * ------------------------------------------------------------------------- */

int Web_ConfigPath_AP(Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {

  /* these options should be in sync with the options in Web_Cfg
   * not the order or anything, but the actual text strings */
  static TCLCONST char *subCmd[] = {
    "script",
    "server_root",
    "document_root",
    "interpclass",
    NULL
  };
  
  enum subCmd
  {
    SCRIPT,
    SERVER_ROOT,
    DOCUMENT_ROOT,
    INTERPCLASS
  };
  
  int index;
  
  if (Tcl_GetIndexFromObj(interp, objv[1], subCmd, "subcommand", 0, &index)
      != TCL_OK) {
    /* let the caller handle the web::config command */
    Tcl_ResetResult(interp);
    return TCL_CONTINUE;
  }
  
  WebAssertObjc(objc != 2, 2, NULL);
  
  switch ((enum subCmd) index) {
    
  case SCRIPT: {
    request_rec *r;
    r = (request_rec *)Tcl_GetAssocData(interp, WEB_AP_ASSOC_DATA, NULL);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(r->filename, -1));
    break;
  }
  case SERVER_ROOT: {
    Tcl_SetObjResult(interp, Tcl_NewStringObj(ap_server_root, -1));
    break;
  }
  case DOCUMENT_ROOT: {
    request_rec *r;
    r = (request_rec *)Tcl_GetAssocData(interp, WEB_AP_ASSOC_DATA, NULL);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(ap_document_root(r), -1));
    break;
  }
  case INTERPCLASS: {
    WebInterp *webInterp;
    webInterp = (WebInterp *)Tcl_GetAssocData(interp, WEB_INTERP_ASSOC_DATA, NULL);
    Tcl_SetObjResult(interp, Tcl_NewStringObj(webInterp->interpClass->filename, -1));
    break;
  }
  }
  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * ModWebsh_Init_AP -- init call backs to mod_websh (InitStubs)
 * ------------------------------------------------------------------------- */

int ModWebsh_Init_AP(Tcl_Interp * interp) {

  if (Tcl_InitStubs(interp, "8.2", 0) != NULL)
    return TCL_OK;
  return TCL_ERROR;
  
}
