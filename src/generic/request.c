/*
 * request.c -- request data management
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
#include <time.h>
#include "request.h"
#include "paramlist.h"
#include "crypt.h"
#include <stdio.h>
#include "log.h"

#ifdef WIN32
#include <errno.h>
#else
#include <sys/errno.h>
#endif

void destroyRequestDataHook(ClientData clientData)
{
    destroyRequestData(clientData, NULL);
}

/* ----------------------------------------------------------------------------
 * Init --
 * ------------------------------------------------------------------------- */
int request_Init(Tcl_Interp * interp)
{

    RequestData *requestData;

    /* --------------------------------------------------------------------------
     * interpreter running ?
     * ----------------------------------------------------------------------- */
    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * new data
     * ----------------------------------------------------------------------- */
    requestData = createRequestData(interp);
    WebAssertData(interp, requestData, "request", TCL_ERROR);

    /* --------------------------------------------------------------------------
     * register commands
     * ----------------------------------------------------------------------- */

    Tcl_CreateObjCommand(interp, "web::request",
			 Web_Request, (ClientData) requestData, NULL);

    Tcl_CreateObjCommand(interp, "web::param",
			 Web_Param, (ClientData) requestData, NULL);

    Tcl_CreateObjCommand(interp, "web::formvar",
			 Web_FormVar, (ClientData) requestData, NULL);

    Tcl_CreateObjCommand(interp, "web::tempfile",
			 Web_TempFile, (ClientData) requestData, NULL);

    Tcl_CreateObjCommand(interp, "web::command",
			 Web_Command, (ClientData) requestData, NULL);

    Tcl_CreateObjCommand(interp, "web::getcommand",
			 Web_GetCommand, (ClientData) requestData, NULL);

    Tcl_CreateObjCommand(interp, "web::dispatch",
			 Web_Dispatch, (ClientData) requestData, NULL);

    /* -------------------------------------------------------------------------
     * associate data with Interpreter
     * ---------------------------------------------------------------------- */
    Tcl_SetAssocData(interp, WEB_REQ_ASSOC_DATA,
		     destroyRequestData, (ClientData) requestData);

    /* -------------------------------------------------------------------------
     * we need an exit handler (if this is the main interp)
     * because we need to delete temp files on regular exit too
     * ---------------------------------------------------------------------- */
    if (Tcl_GetMaster(interp) == NULL) {
      Tcl_CreateExitHandler(destroyRequestDataHook, (ClientData) requestData);
    }

    /* -------------------------------------------------------------------------
     * done
     * ---------------------------------------------------------------------- */
    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * create
 * ------------------------------------------------------------------------- */
RequestData *createRequestData(Tcl_Interp * interp)
{

    RequestData *requestData = NULL;

    requestData = WebAllocInternalData(RequestData);

    if (requestData != NULL) {

	WebNewStringObjFromStringIncr(requestData->cmdTag, CMDTAGDEFAULT);
	WebNewStringObjFromStringIncr(requestData->timeTag, TIMETAGDEFAULT);
	requestData->cmdUrlTimestamp = Tcl_NewBooleanObj(1);
	HashUtlAllocInit(requestData->request, TCL_STRING_KEYS);

	requestData->upLoadFileSize = Tcl_NewLongObj(UPLOADFILESIZEDEFAULT);
	requestData->filePermissions = DEFAULT_FILEPERMISSIONS;

	HashUtlAllocInit(requestData->paramList, TCL_STRING_KEYS);
	HashUtlAllocInit(requestData->formVarList, TCL_STRING_KEYS);
	HashUtlAllocInit(requestData->cmdList, TCL_STRING_KEYS);
	HashUtlAllocInit(requestData->tmpFnList, TCL_STRING_KEYS);
	HashUtlAllocInit(requestData->staticList, TCL_STRING_KEYS);
	requestData->requestIsInitialized = 0;
    }

    return requestData;
}

/* ----------------------------------------------------------------------------
 * reset
 * ------------------------------------------------------------------------- */
int resetRequestData(Tcl_Interp * interp, RequestData * requestData)
{

    if ((interp == NULL) || (requestData == NULL))
	return TCL_ERROR;

    if (removeTempFiles(interp, requestData) != TCL_OK)
	return TCL_ERROR;

    if (resetHashTableWithContent(requestData->staticList,
				  TCL_STRING_KEYS,
				  deleteTclObj_fnc, NULL) != TCL_OK)
	return TCL_ERROR;

    /* do not touch cmdList */

    if (resetHashTableWithContent(requestData->formVarList,
				  TCL_STRING_KEYS,
				  deleteTclObj_fnc, NULL) != TCL_OK)
	return TCL_ERROR;

    if (resetHashTableWithContent(requestData->paramList, TCL_STRING_KEYS,
				  deleteTclObj_fnc, NULL) != TCL_OK)
	return TCL_ERROR;

    if (resetHashTableWithContent(requestData->request, TCL_STRING_KEYS,
				  deleteTclObj_fnc, NULL) != TCL_OK)
	return TCL_ERROR;

#if 0
    WebDecrRefCountIfNotNullAndSetNull(requestData->upLoadFileSize);
    requestData->upLoadFileSize = Tcl_NewLongObj(0);

    requestData->filePermissions = DEFAULT_FILEPERMISSIONS;

    WebDecrRefCountIfNotNullAndSetNull(requestData->timeTag);
    WebNewStringObjFromStringIncr(requestData->timeTag, "t");

    WebDecrRefCountIfNotNullAndSetNull(requestData->cmdTag);
    WebNewStringObjFromStringIncr(requestData->cmdTag, "cmd");

    Tcl_SetBooleanObj(requestData->cmdUrlTimestamp, 1);
#endif

    requestData->requestIsInitialized = 0;
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * removeTempFiles -- remove all temporary files accumulated so far,
 *   and reset hashtable tmpFnList
 * ------------------------------------------------------------------------- */
int removeTempFiles(Tcl_Interp * interp, RequestData * requestData)
{

    HashTableIterator iterator;
    Tcl_Obj *tclo = NULL;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if (requestData == NULL)
	return TCL_ERROR;
    if (requestData->tmpFnList == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * loop
     * ----------------------------------------------------------------------- */
    assignIteratorToHashTable(requestData->tmpFnList, &iterator);

    while (nextFromHashIterator(&iterator) != TCL_ERROR) {
	tclo = (Tcl_Obj *) valueOfCurrent(&iterator);
	if (tclo != NULL) {

	    if (remove(Tcl_GetString(tclo)) < 0) {
	      /* not successful: usually because there is no such file */
	      if (Tcl_GetErrno() != ENOENT) {
		/* a different reason: create error log */
		LOG_MSG(interp, WRITE_LOG, __FILE__, __LINE__,
			"removeTempFiles", WEBLOG_ERROR,
			"Error: ", Tcl_ErrnoMsg(Tcl_GetErrno()),
			NULL);
	      }
	    } else {
	      /* log if successfully removed */
	      LOG_MSG(interp, WRITE_LOG, __FILE__, __LINE__,
		      "removeTempFiles", WEBLOG_DEBUG,
		      "removing temporary file ", Tcl_GetString(tclo), ".",
		      NULL);
	    }

	    Tcl_DecrRefCount(tclo);
	}
    }
    return resetHashTable(requestData->tmpFnList, TCL_STRING_KEYS);
}


/* ----------------------------------------------------------------------------
 * destroy
 * ------------------------------------------------------------------------- */
void destroyRequestData(ClientData clientData, Tcl_Interp * interp)
{

    RequestData *requestData = NULL;

    if (clientData != NULL) {

	requestData = (RequestData *) clientData;

	/* delete exit handler to prevent memory leak */
	Tcl_DeleteExitHandler(destroyRequestDataHook, (ClientData) requestData);

	WebDecrRefCountIfNotNull(requestData->cmdTag);
	WebDecrRefCountIfNotNull(requestData->timeTag);
	WebDecrRefCountIfNotNull(requestData->cmdUrlTimestamp);
	destroyParamList(requestData->request);

	WebDecrRefCountIfNotNull(requestData->upLoadFileSize);

	destroyParamList(requestData->paramList);
	destroyParamList(requestData->formVarList);

	destroyParamList(requestData->cmdList);

	/* ------------------------------------------------------------------------
	 * now delete temporary files
	 * --------------------------------------------------------------------- */
	if (requestData->tmpFnList != NULL) {

	    removeTempFiles(interp, requestData);
	    /* this time delete the hash */
	    HashUtlDelFree(requestData->tmpFnList);
	}

	destroyParamList(requestData->staticList);

	WebFreeIfNotNull(requestData);
    }
}

/* ----------------------------------------------------------------------------
 * Web_Param
 * ------------------------------------------------------------------------- */
int Web_Param(ClientData clientData,
	      Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    RequestData *requestData = NULL;

    /* --------------------------------------------------------------------------
     * check for internal data
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_Param", TCL_ERROR)
	requestData = (RequestData *) clientData;

    return paramGet((ParamList *) requestData->paramList, interp, objc, objv,
		    0);
}

/* ----------------------------------------------------------------------------
 * Web_FormVar
 * ------------------------------------------------------------------------- */
int Web_FormVar(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    RequestData *requestData = NULL;

    /* --------------------------------------------------------------------------
     * check for internal data
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_FormVar", TCL_ERROR)
	requestData = (RequestData *) clientData;

    return paramGet((ParamList *) requestData->formVarList, interp, objc,
		    objv, 0);
}



/* ----------------------------------------------------------------------------
 * Web_TempFile -- return a temporary filename
 * note: websh3 keeps a list of all filenames that have been generated with
 * note: this command, and will attempt to delete these files at the end
 * note: or if you call web::tempfile -remove
 * ------------------------------------------------------------------------- */
int Web_TempFile(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    Tcl_Obj *tclo = NULL;
    RequestData *requestData = NULL;
    static TCLCONST char *params[] = { "-path", "-prefix", "-remove", NULL };
    enum params
    { PATH, PREFIX, REMOVE };
    int idx = -1;

    WebAssertData(interp, clientData, "Web_TempFile", TCL_ERROR)
	requestData = (RequestData *) clientData;

    WebAssertArgs(interp, objc, objv, params, idx, -1);

    /* do we see "-remove" ? */
    if (argKeyExists(objc, objv, (char *)params[REMOVE]) == TCL_OK) {

	/* do remove */
	return removeTempFiles(interp, requestData);
    }

    /* lazy arg-check - we do not check values of params.
     * this works beause argValueOfKey may return NULL and
     * tempFileName takes NULL as "default" */
    tclo = tempFileName(interp, requestData,
			argValueOfKey(objc, objv, (char *)params[PATH]),
			argValueOfKey(objc, objv, (char *)params[PREFIX]));

    if (tclo == NULL)
	return TCL_ERROR;

    Tcl_SetObjResult(interp, tclo);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * tempFileName -- return a temporary filename
 * note: websh3 keeps a list of all filenames that have been generated with
 * note: this command, and will attempt to delete these files at the end !
 * note: If you would like to keep the file, you will have to copy it
 * note: to a safe place before the end of the websh3 app.
 * ------------------------------------------------------------------------- */
Tcl_Obj *tempFileName(Tcl_Interp * interp, RequestData * requestData,
		      Tcl_Obj * path, Tcl_Obj * prefix)
{

    Tcl_Obj *tclo = NULL;
    char *pathstring = NULL;
    char *prefixstring = NULL;
    char *tmpn = NULL;
    int trycnt = 0;
    Tcl_Obj *mytime = NULL;

    if (requestData == NULL)
	return NULL;

    if (path != NULL)
	pathstring = Tcl_GetString(path);
    if (prefix != NULL)
	prefixstring = Tcl_GetString(prefix);

#ifdef WIN32
    tmpn = _tempnam(pathstring, prefixstring);
#else
    tmpn = tempnam(pathstring, prefixstring);
#endif

    if (tmpn == NULL) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::tempfile", WEBLOG_ERROR,
		"error requesting unique filename", NULL);
	return NULL;
    }

    tclo = Tcl_NewStringObj(tmpn, -1);
    Tcl_IncrRefCount(tclo);

#ifndef WIN32
    free(tmpn);
#endif

    /* now try to add to hash list */

    while ((appendToHashTable(requestData->tmpFnList,
			      Tcl_GetString(tclo),
			      (void *) tclo) == TCL_ERROR)
	   && (trycnt++ < 100)) {

	mytime = Tcl_NewLongObj(((unsigned long) clock()) % 1000);
	Tcl_IncrRefCount(mytime);
	Tcl_AppendObjToObj(tclo, mytime);
	Tcl_DecrRefCount(mytime);
    }

    if (trycnt >= 100) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::tempfile", WEBLOG_ERROR,
		"error adding \"", Tcl_GetString(tclo),
		"\" to internal list of files", NULL);

	removeFromHashTable(requestData->tmpFnList, Tcl_GetString(tclo));
	Tcl_DecrRefCount(tclo);
	return NULL;
    }

    /* fixme-later: should I check for for TMP_MAX filenames per app ?
    */
    return tclo;
}

/* --------------------------------------------------------------------------
 * accessor to request object
 * ------------------------------------------------------------------------*/
int Web_Request(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    static char *params[] = { "-reset", "-channel", NULL };
    enum params
    { REQUESTRESET, DEFAULTCHANNELNAME };

    int res;
    RequestData *requestData;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "web::request", TCL_ERROR);
    requestData = (RequestData *) clientData;
    WebAssertData(interp, requestData->request, "web::request", TCL_ERROR);

    /* make sure we have values */
    if (requestFillRequestValues(interp, requestData) == TCL_ERROR)
	return TCL_ERROR;

    res = paramGet((ParamList *) requestData->request, interp, objc, objv, 1);

    if (res == TCL_CONTINUE) {
	int opt;
	WebAssertObjc(objc != 2, 1, "args ....");
	if (paramGetIndexFromObj
	    (interp, objv[1], params, "subcommand", 0, &opt) == TCL_ERROR)
	    return TCL_ERROR;

	switch ((enum params) opt) {

	case DEFAULTCHANNELNAME:
	    Tcl_SetObjResult(interp, requestGetDefaultChannelName());
	    return TCL_OK;
	    break;
	case REQUESTRESET:
	    return resetRequestData(interp, requestData);
	    break;
	default:
	    break;
	}
    }
    return TCL_OK;
}
