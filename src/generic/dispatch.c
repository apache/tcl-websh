/* ----------------------------------------------------------------------------
 * dispatch.c -- implement web::dispatch for websh3
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
 * ------------------------------------------------------------------------- */

#include "args.h"
#include "log.h"
#include "paramlist.h"
#include "request.h"
#include "tcl.h"
#include "webutl.h"

#define FORM_URLENCODED    "application/x-www-form-urlencoded"
#define FORM_MULTIPART     "multipart/form-data"
#define FORM_MULTIPART_LEN 19
#define FORM_DEFAULT_TYPE  FORM_URLENCODED

Tcl_Obj *requestGetDefaultChannelName();

int parsePostData(Tcl_Interp * interp, Tcl_Obj * name,
		  Tcl_Obj * type, Tcl_Obj * len, RequestData * requestData);


/* ----------------------------------------------------------------------------
 * Web_Dispatch
 * ------------------------------------------------------------------------- */
int Web_Dispatch(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    RequestData *requestData = NULL;
    TCLCONST char *params[] = { "-track",
	"-querystring",
	"-postdata",
	"-cmd",
	"-hook",
	NULL
    };
    enum params
    { TRACK,
	QUERYSTRING,
	POSTDATA,
	CMD,
	HOOK
    };

    int idx = 0;
    Tcl_Obj *post_data = NULL;
    Tcl_Obj *query_string = NULL;
    Tcl_Obj *formDataChannel = NULL;
    Tcl_Obj *formDataBoundary = NULL;
    Tcl_Obj *urlEncString = NULL;

    /* --------------------------------------------------------------------------
     * check for private data
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_Dispatch", TCL_ERROR)
	requestData = (RequestData *) clientData;

    /* --------------------------------------------------------------------------
     * check for not-accepted options
     * ----------------------------------------------------------------------- */
    WebAssertArgs(interp, objc, objv, params, idx, -1);

    /* make sure we have values */
    if (requestFillRequestValues(interp, requestData) == TCL_ERROR)
	return TCL_ERROR;


    /* ==========================================================================
     * query_string
     * ======================================================================= */
    query_string = argValueOfKey(objc, objv, (char *)params[QUERYSTRING]);

    if (query_string == NULL) {
	/* ------------------------------------------------------------------------
	 * not on command line. try to get from response object
	 * --------------------------------------------------------------------- */
	query_string =
	    paramListGetObjectByString(interp, requestData->request,
				       "QUERY_STRING");
    }

    /* --------------------------------------------------------------------------
     * did we find any query_string ?
     * ----------------------------------------------------------------------- */
    if (query_string != NULL) {

	/* ------------------------------------------------------------------------
	 * empty string means: skip
	 * --------------------------------------------------------------------- */
	if (Tcl_GetCharLength(query_string) > 0) {

	    if (parseQueryString(requestData, interp, query_string) ==
		TCL_ERROR)
		return TCL_ERROR;
	}
    }

    /* ==========================================================================
     * post_data
     * ======================================================================= */
    post_data = argValueOfKey(objc, objv, (char *)params[POSTDATA]);

    if (post_data != NULL) {

	/* ------------------------------------------------------------------------
	 * on command line; empty string means: skip
	 * --------------------------------------------------------------------- */
	if (Tcl_GetCharLength(post_data) > 0) {

	    /* ----------------------------------------------------------------------
	     * parse -postdata input
	     * ------------------------------------------------------------------- */
	    int idx1 = 0;
	    int idx2 = 0;

	    if ((idx1 = argIndexOfKey(objc, objv, (char *)params[POSTDATA])) > 0) {

		idx2 = argIndexOfNextKey(objc, objv, idx1);

/*         for( i = idx1; i < objc; i++) { */
/*           fprintf(stdout,"DBG %d (%d) %s\n",i,(i == idx2),Tcl_GetString(objv[i])); fflush(stdout); */
/*         } */

		switch ((idx2 - idx1)) {

		case 2:
		    /* ------------------------------------------------------------------
		     * -postdata channel
		     * --------------------------------------------------------------- */
		    if (parsePostData(interp, objv[idx1 + 1], NULL, NULL,
				      requestData) == TCL_ERROR) {
			return TCL_ERROR;
		    }

		    break;
		case 3:
		    /* ------------------------------------------------------------------
		     * -postdata channel length
		     * --------------------------------------------------------------- */
		    /* log is handled by parsePostData */
		    if (parsePostData(interp, objv[idx1 + 1], objv[idx1 + 2],
				      NULL, requestData) == TCL_ERROR) {
			return TCL_ERROR;
		    }
		    break;
		case 4:
		    /* ------------------------------------------------------------------
		     * -postdata channel length type
		     * --------------------------------------------------------------- */
		    /* log is handled by parsePostData */
		    if (parsePostData(interp, objv[idx1 + 1], objv[idx1 + 2],
				      objv[idx1 + 3], requestData) == TCL_ERROR) {
			return TCL_ERROR;
		    }
		    break;
		default:
		    Tcl_WrongNumArgs(interp, 1, objv,
				     "-postdata ?#?channel content_length ?content_type?");
		    return TCL_ERROR;
		    break;
		}
	    }
	}
    }
    else {

	/* ------------------------------------------------------------------------
	 * get postdata from default input
	 * --------------------------------------------------------------------- */

	Tcl_Obj *content_type = NULL;
	Tcl_Obj *content_length = NULL;

	content_type =
	    paramListGetObjectByString(interp, requestData->request,
				       "CONTENT_TYPE");
	content_length =
	    paramListGetObjectByString(interp, requestData->request,
				       "CONTENT_LENGTH");

	if ((content_type != NULL) && (content_length != NULL)) {

	    Tcl_Obj *tmp = NULL;

	    tmp = requestGetDefaultChannelName();

	    parsePostData(interp, tmp, content_length, content_type,
			  requestData);

	    Tcl_DecrRefCount(tmp);
	}
    }

    /* ==========================================================================
     * track
     * ======================================================================= */
    {
	Tcl_Obj *trackList = NULL;
	int listLen = -1;
	int i = -1;
	Tcl_Obj *key = NULL;
	Tcl_Obj *val = NULL;

	trackList = argValueOfKey(objc, objv, (char *)params[TRACK]);

	if ((trackList != NULL) &&
	    (listLen = tclGetListLength(interp, trackList)) != -1) {

	    /* ----------------------------------------------------------------------
	     * loop over list and add these parameters to "static"
	     * ------------------------------------------------------------------- */

	    for (i = 0; i < listLen; i++) {

		key = NULL;
		val = NULL;

		Tcl_ListObjIndex(interp, trackList, i, &key);

		/* Tcl_ListObjIndex: The reference count for the list element
		 * is not incremented; the caller must do that if it needs to
		 * retain a pointer to the element. */

		if (key != NULL) {

		    /* ------------------------------------------------------------------
		     * get value from params
		     * --------------------------------------------------------------- */
		    val = (Tcl_Obj *) getFromHashTable(requestData->paramList,
						       Tcl_GetString(key));

		    if (val != NULL) {

			/* this obj now belongs to two lists */
			val = Tcl_DuplicateObj(val);

			if (paramListSetAsWhole(requestData->staticList,
						Tcl_GetString(key),
						val) == TCL_ERROR) {
			    LOG_MSG(interp, WRITE_LOG, __FILE__, __LINE__,
				    "web::dispatch -track", WEBLOG_INFO,
				    "error adding \"", Tcl_GetString(key),
				    ", ", Tcl_GetString(val),
				    "\" to static params", NULL);
			}
		    }
		}
	    }
	}
    }

    /* ==========================================================================
     * dispatch
     * ======================================================================= */
    {

	Tcl_Obj *cmdName = NULL;
	Tcl_Obj *cmdCode = NULL;
	char *cmdNameStr = NULL;
	int res;

	/* ------------------------------------------------------------------------
	 * reset interp result
	 * --------------------------------------------------------------------- */
	Tcl_ResetResult(interp);

	cmdName = argValueOfKey(objc, objv, (char *)params[CMD]);

	if (cmdName != NULL) {

	    /* ----------------------------------------------------------------------
	     * call command (from command-line); empty string means: no dispatch
	     * ------------------------------------------------------------------- */
	    if (Tcl_GetCharLength(cmdName) < 1)
		return TCL_OK;

	}
	else {

	    /* ----------------------------------------------------------------------
	     * get command name from default source
	     * ------------------------------------------------------------------- */
	    cmdName = (Tcl_Obj *) getFromHashTable(requestData->paramList,
						   Tcl_GetString(requestData->
								 cmdTag));
	}

	/* ------------------------------------------------------------------------
	 * cmd: "",NULL --> default
	 * --------------------------------------------------------------------- */
	if (cmdName == NULL) {
	    cmdName = Tcl_NewStringObj(WEB_REQ_DEFAULT, -1);
	}
	else if (Tcl_GetCharLength(cmdName) == 0) {
	    cmdName = Tcl_NewStringObj(WEB_REQ_DEFAULT, -1);
	}
	else {
	    cmdName = Tcl_DuplicateObj(cmdName);
	}
	cmdNameStr = Tcl_GetString(cmdName);

	LOG_MSG(interp, WRITE_LOG,
		__FILE__, __LINE__,
		"web::dispatch", WEBLOG_INFO,
		"Handling command \"", cmdNameStr, "\"", NULL);

	/* ------------------------------------------------------------------------
	 * get command code
	 * --------------------------------------------------------------------- */
	cmdCode =
	    (Tcl_Obj *) getFromHashTable(requestData->cmdList, cmdNameStr);

	/* ------------------------------------------------------------------------
	 * command found ?
	 * --------------------------------------------------------------------- */
	if (cmdCode == NULL) {

	    LOG_MSG(interp, WRITE_LOG,
		    __FILE__, __LINE__,
		    "web::dispatch", WEBLOG_INFO,
		    "command \"", cmdNameStr, "\" not found.",
		    " Switching to command \"default\"", NULL);

	    cmdNameStr = WEB_REQ_DEFAULT;
	    cmdCode =
		(Tcl_Obj *) getFromHashTable(requestData->cmdList,
					     cmdNameStr);
	}

	/* ------------------------------------------------------------------------
	 * eval command, if any
	 * --------------------------------------------------------------------- */
	if (cmdCode == NULL) {

	    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		    "web::dispatch", WEBLOG_ERROR,
		    "command \"", cmdNameStr, "\" not found", NULL);

	    WebDecrRefCountIfNotNull(cmdName);
	    return TCL_ERROR;

	}
	else {

	    Tcl_Obj *hook = NULL;

	    /* first eval hook, if any */
	    hook = argValueOfKey(objc, objv, (char *)params[HOOK]);

	    if (hook != NULL) {

		Tcl_IncrRefCount(hook);
		res = Tcl_EvalObjEx(interp, hook, TCL_EVAL_DIRECT);
		Tcl_DecrRefCount(hook);

		if (res == TCL_ERROR) {
		    LOG_MSG(interp, WRITE_LOG | INTERP_ERRORINFO,
			    __FILE__, __LINE__,
			    "web::dispatch", WEBLOG_ERROR,
			    "error evaluating hook \"", Tcl_GetString(hook),
			    "\"", NULL);
		    return TCL_ERROR;
		}
	    }

	    /* reuse var hook */
	    Tcl_ListObjIndex(interp, cmdCode, 0, &hook);

	    Tcl_IncrRefCount(hook);
	    res = Tcl_EvalObjEx(interp, hook, TCL_EVAL_DIRECT);
	    Tcl_DecrRefCount(hook);

	    if (res == TCL_ERROR) {

		LOG_MSG(interp, WRITE_LOG | SET_RESULT | INTERP_ERRORINFO,
			__FILE__, __LINE__,
			"web::dispatch", WEBLOG_ERROR,
			"error evaluating command \"", cmdNameStr,
			"\"", NULL);
		WebDecrRefCountIfNotNull(cmdName);
		return TCL_ERROR;

	    }
	}
	WebDecrRefCountIfNotNull(cmdName);
    }

    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * parsePostData -- parse postdata from a channel
 * ------------------------------------------------------------------------- */
int parsePostData(Tcl_Interp * interp, Tcl_Obj * name,
		  Tcl_Obj * len, Tcl_Obj * type, RequestData * requestData)
{

    char *content_type = NULL;
    int iTmp = 0;

/*   printf("DBG parsePostData - starting\n"); fflush(stdout); */

    /* --------------------------------------------------------------------------
     * input checking
     * ----------------------------------------------------------------------- */
    if (name == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"we;::dispatch -postdata", WEBLOG_ERROR,
		"cannot access channelName", NULL);
	return TCL_ERROR;
    }

    if (requestData == NULL) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"we;::dispatch -postdata", WEBLOG_ERROR,
		"cannot access internal data", NULL);
	return TCL_ERROR;
    }

    if (type == NULL)
	content_type = FORM_DEFAULT_TYPE;
    else
	content_type = Tcl_GetString(type);

/*   printf("DBG parsePostData - content_type: %s\n",content_type); fflush(stdout); */

    /* --------------------------------------------------------------------------
     * application/x-www-form-urlencoded
     * ----------------------------------------------------------------------- */
    if (strcmp(content_type, FORM_URLENCODED) == 0) {

	return parseUrlEncodedFormData(requestData, interp,
				       Tcl_GetString(name), len);
    }
    /* ------------------------------------------------------------------------
     * multipart/form-data
     * --------------------------------------------------------------------- */
    if (strncmp(content_type, FORM_MULTIPART, FORM_MULTIPART_LEN) == 0) {

	return parseMultipartFormData(requestData, interp,
				      Tcl_GetString(name), content_type);
    }

    LOG_MSG(interp, WRITE_LOG, __FILE__, __LINE__,
	    "web::dispatch -postdata", WEBLOG_WARNING,
	    "unknown content-type \"", content_type, "\"", NULL);
    return TCL_ERROR;
}
