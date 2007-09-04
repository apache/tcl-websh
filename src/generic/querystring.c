/*
 * querystring.c -- CGI QUERY_STRING parsing
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
#include <string.h>
#include "crypt.h"
#include "paramlist.h"
#include "request.h"
#include "conv.h"
#include "url.h"

/* ----------------------------------------------------------------------------
 * parseQueryString --
 *   parse "k1=v1&k2=v2" kind of data, and store in web::params structure
 * ------------------------------------------------------------------------- */
int parseQueryString(RequestData * requestData, Tcl_Interp * interp,
		     Tcl_Obj * query_string)
{
    Tcl_Obj *tclo = NULL;
    int tRes = 0;
    int listLen = 0;

    tRes = TCL_OK;

    if ((requestData == NULL) || (query_string == NULL))
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * decrypt
     * ----------------------------------------------------------------------- */

    Tcl_IncrRefCount(query_string);
    if (dodecrypt(interp, query_string, 1) == TCL_OK) {

	tclo = Tcl_DuplicateObj(Tcl_GetObjResult(interp));
	Tcl_IncrRefCount(tclo);
	Tcl_ResetResult(interp);
	Tcl_DecrRefCount(query_string);

	/* ------------------------------------------------------------------------
	 * only add if list length > 0
	 * --------------------------------------------------------------------- */
	if ((listLen = tclGetListLength(interp, tclo)) == -1) {
	    Tcl_DecrRefCount(tclo);
	    return TCL_ERROR;
	}

	if (listLen > 0) {

	    /* ----------------------------------------------------------------------
	     * add list to requestData
	     * ------------------------------------------------------------------- */
	    tRes = listObjAsParamList(tclo, requestData->paramList);
	    Tcl_DecrRefCount(tclo);
	    return tRes;
	}

	/* ------------------------------------------------------------------------
	 * done
	 * --------------------------------------------------------------------- */
	Tcl_DecrRefCount(tclo);
	return TCL_OK;
    }
    else {

	/* just write a note */
	LOG_MSG(interp, WRITE_LOG,
		__FILE__, __LINE__,
		"web::dispatch", WEBLOG_DEBUG,
		"error decrypting \"", Tcl_GetString(query_string), "\"",
		NULL);
    }

    Tcl_DecrRefCount(query_string);
    return TCL_OK;

}
