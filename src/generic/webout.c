/*
 * webout.c --- Tcl interface to webout, the output handler of websh 3
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
#include <stdio.h>
#include "webout.h"		/* is member of output module of websh */
#include "args.h"		/* arg processing */
#include "webutl.h"
#include "hashutl.h"
#include "paramlist.h"		/* destroyParamList */


/* ----------------------------------------------------------------------------
 * init -- start up output handler module of websh3
 * ------------------------------------------------------------------------- */
int webout_Init(Tcl_Interp * interp)
{

    OutData *outData;

    /* --------------------------------------------------------------------------
     * interpreter running ?
     * ----------------------------------------------------------------------- */
    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * init internal data
     * ----------------------------------------------------------------------- */
    outData = createOutData(interp);
    WebAssertData(interp, outData, "webout_Init", TCL_ERROR);

    /* --------------------------------------------------------------------------
     * register new commands
     * ----------------------------------------------------------------------- */
    Tcl_CreateObjCommand(interp, "web::putx",
			 Web_Eval, (ClientData) outData, NULL);

    Tcl_CreateObjCommand(interp, "web::put",
			 Web_Puts, (ClientData) outData, NULL);

    Tcl_CreateObjCommand(interp, "web::response",
			 Web_Response, (ClientData) outData, NULL);

/*   Tcl_CreateObjCommand(interp, "web::varopen",  */
/* 		       Web_VarOpen,  */
/* 		       (ClientData)outData, */
/* 		       NULL); */

    /* --------------------------------------------------------------------------
     * register private data with interp
     * ----------------------------------------------------------------------- */
    Tcl_SetAssocData(interp, WEB_OUT_ASSOC_DATA,
		     (Tcl_InterpDeleteProc *) destroyOutData,
		     (ClientData) outData);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_Eval -- the web::putx command
 * ------------------------------------------------------------------------- */
int Web_Eval(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    ResponseObj *responseObj = NULL;
    OutData *outData = NULL;
    Tcl_Obj *code = NULL;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "web::putx", TCL_ERROR);
    outData = (OutData *) clientData;

    /* --------------------------------------------------------------------------
     * web::putx myVar test
     * 0         1     2
     * ----------------------------------------------------------------------- */
    WebAssertObjc((objc < 2)
		  || (objc > 3), 1, "?channel|#globalvar? extendedstring");

    if (objc == 2) {

	responseObj = outData->defaultResponseObj;
	code = objv[1];

    }
    else {

	responseObj = getResponseObj(interp, outData, Tcl_GetString(objv[1]));
	code = objv[2];
    }

    if (responseObj == NULL) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::putx", WEBLOG_ERROR,
		"error accessing response object", NULL);
	return TCL_ERROR;
    }

    /* --------------------------------------------------------------------------
     * call eval
     * ----------------------------------------------------------------------- */

    switch (outData->putxMarkup) {
    case brace:
	return webout_eval_brace(interp, responseObj, code);
    case tag:
	return webout_eval_tag(interp, responseObj, code);
    default:
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::putx", WEBLOG_ERROR, "unknown putxmarkup type", NULL);
	return TCL_ERROR;
    }

}


/* ----------------------------------------------------------------------------
 * Web_Puts -- the web::puts command
 * ------------------------------------------------------------------------- */
int Web_Puts(ClientData clientData,
	     Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    ResponseObj *responseObj = NULL;
    OutData *outData = NULL;
    Tcl_Obj *code = NULL;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "web::put", TCL_ERROR);
    outData = (OutData *) clientData;

    /* --------------------------------------------------------------------------
     * web::put myVar test
     * 0        1     2
     * ----------------------------------------------------------------------- */
    WebAssertObjc((objc < 2) || (objc > 3), 1, "?channel|#globalvar? string");

    if (objc == 2) {

	responseObj = outData->defaultResponseObj;
	code = objv[1];

    }
    else {

	responseObj = getResponseObj(interp, outData, Tcl_GetString(objv[1]));
	code = objv[2];
    }

    if (responseObj == NULL) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::put", WEBLOG_ERROR,
		"error accessing response object", NULL);
	return TCL_ERROR;
    }

    return putsCmdImpl(interp, responseObj, code);
}


/* ----------------------------------------------------------------------------
 * Web_Response -- the web::output command (config of web::put and web::putx)
 * ------------------------------------------------------------------------- */
int Web_Response(ClientData clientData, Tcl_Interp * interp,
		 int objc, Tcl_Obj * CONST objv[])
{


    ResponseObj *responseObj = NULL;
    OutData *outData = NULL;

    /*
       char            *channelName = NULL;
       Tcl_Obj         *result = NULL;
       Tcl_Channel     channel = NULL;
       int             mode = 0;
       int lastIndex = objc -1;
       int idx = -1;
       int iCurArg = -1;
     */
    int res;

    static char *params[] = { "-sendheader",
	"-select",
	"-bytessent",
	"-httpresponse",
	"-reset",
	"-resetall",
	NULL
    };
    enum params
    { SENDHEADER, SELECT, BYTESSENT, HTTPRESPONSE, RESET, RESETALL };

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "web::response", TCL_ERROR)
	outData = (OutData *) clientData;
    WebAssertData(interp, outData->responseObjHash, "web::response",
		  TCL_ERROR);

    /* get the current response object */
    responseObj = outData->defaultResponseObj;
    if (responseObj == NULL) {
	Tcl_SetResult(interp, "no current response object", TCL_STATIC);
	return TCL_ERROR;
    }

    /* handle first paramList things */
    /* fixme: here the keys might not be strictly case sensitive */
    res = paramGet((ParamList *) responseObj->headers, interp, objc, objv, 1);

    if (res == TCL_CONTINUE) {

	if (objc == 1) {
	    /* ----------------------------------------------------------------------
	     * return name of default channel
	     * ------------------------------------------------------------------- */
	    Tcl_ResetResult(interp);	/* empty string */
	    if (responseObj->name != NULL) {
		Tcl_SetObjResult(interp, responseObj->name);
		return TCL_OK;
	    }
	    Tcl_SetResult(interp, "current response has no name", TCL_STATIC);
	    return TCL_ERROR;
	}
	else {
	    int opt;
	    if (paramGetIndexFromObj
		(interp, objv[1], params, "subcommand", 0, &opt) == TCL_ERROR)
		return TCL_ERROR;

	    switch ((enum params) opt) {
	    case RESETALL:
		WebAssertObjc(objc != 2, 2, NULL);
		return resetOutData(interp, outData);

	    case RESET:{
		    Tcl_Obj *tmp = NULL;
		    int err = 0;
		    char *tname = NULL;

		    WebAssertObjc(objc != 2, 2, NULL);

		    /* --------------------------------------------------------------------
		     * just reset this one
		     * ----------------------------------------------------------------- */

		    removeFromHashTable(outData->responseObjHash,
					Tcl_GetString(responseObj->name));

		    tmp = Tcl_DuplicateObj(responseObj->name);
		    tname = Tcl_GetString(tmp);

		    destroyResponseObj((ClientData) responseObj, interp);
		    if (responseObj == outData->defaultResponseObj)
			outData->defaultResponseObj = NULL;
		    responseObj = NULL;

		    /* if we reset the default response object, we have to recreate it 
		     * with our special createDefaultResponseObj function ...
		     */

		    if (isDefaultResponseObj(tname))
			responseObj = createDefaultResponseObj(interp);
		    else
			responseObj = getResponseObj(interp, outData, tname);

		    if (responseObj == NULL) {
			Tcl_SetResult(interp,
				      "could not reset response object",
				      TCL_STATIC);
			return TCL_ERROR;
		    }

		    Tcl_DecrRefCount(tmp);
		    if (outData->defaultResponseObj == NULL)
			outData->defaultResponseObj = responseObj;

		    return TCL_OK;
		    break;
		}
	    case SENDHEADER:{
		    int res;
		    WebAssertObjc(objc > 3, 2, NULL);
		    res = responseObj->sendHeader;
		    if (objc == 3)
			/* set new value */
			if (Tcl_GetBooleanFromObj(interp, objv[2],
						  &(responseObj->
						    sendHeader)) ==
			    TCL_ERROR) {

			    return TCL_ERROR;
			}
		    Tcl_SetObjResult(interp, Tcl_NewBooleanObj(res));
		    return TCL_OK;
		    break;
		}
	    case SELECT:{
		    ResponseObj *old = NULL;

/*         fprintf(stderr,"DBG -select called\n"); fflush(stderr); */

		    WebAssertObjc(objc != 3, 2, "channelName");
		    old = responseObj;
		    /* we have to find the new channel */
		    responseObj =
			getResponseObj(interp, outData,
				       Tcl_GetString(objv[2]));
		    if (responseObj == NULL) {
			Tcl_ResetResult(interp);
			Tcl_AppendResult(interp, "invalid response object \"",
					 Tcl_GetString(objv[2]), "\"",
					 (char *) NULL);
			return TCL_ERROR;
		    }
		    outData->defaultResponseObj = responseObj;
		    Tcl_ResetResult(interp);	/* empty string */
		    if (old->name != NULL)
			Tcl_SetObjResult(interp, old->name);
		    return TCL_OK;
		    /* fixme: same error msg as above if there's no name, but preferably at one place */
		    break;
		}

	    case BYTESSENT:
		WebAssertObjc(objc != 2, 2, NULL);
		Tcl_SetObjResult(interp,
				 Tcl_NewLongObj(responseObj->bytesSent));
		return TCL_OK;

	    case HTTPRESPONSE:{
		    Tcl_Obj *current;
		    WebAssertObjc(objc > 3, 2, NULL);
		    current = responseObj->httpresponse;
		    if (current)
			Tcl_SetObjResult(interp, current);
		    if (objc == 3) {
			/* if length = 0 we reset
			 * if equal to "default", take from HTTP_RESPONSE 
			 * otherwise take value */
			int len;
			char *response = Tcl_GetStringFromObj(objv[2], &len);
			if (len == 0)
			    responseObj->httpresponse = NULL;
			else {
			    if (!strcmp("default", response)) {
				responseObj->httpresponse =
				    Tcl_NewStringObj(HTTP_RESPONSE, -1);
			    }
			    else {
				responseObj->httpresponse =
				    Tcl_DuplicateObj(objv[2]);
			    }
			    Tcl_IncrRefCount(responseObj->httpresponse);
			}
			/* forget old value */
			if (current)
			    Tcl_DecrRefCount(current);
		    }
		    return TCL_OK;
		    break;
		}
	    default:
		break;

	    }
	}
	WebAssertObjc(1, 1, "(unknown syntax)");
    }
    return res;

}
