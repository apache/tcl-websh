/*
 * request_ap.c -- get request data from apaches request object
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
#include "hashutl.h"
#include "webutl.h"

#include "mod_websh.h"
#include "webout.h"

#include "request.h"

/* ----------------------------------------------------------------------------
 * apHeaderHandler -- set headers in mod_websh case
 * ------------------------------------------------------------------------- */
int apHeaderHandler(Tcl_Interp * interp, ResponseObj * responseObj,
		    Tcl_Obj * out)
{

    Tcl_Obj *httpResponse = NULL;
    request_rec *r = NULL;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if ((interp == NULL) || (responseObj == NULL))
	return TCL_ERROR;
    if (responseObj->sendHeader == 1) {

	HashTableIterator iterator;
	char *key;
	Tcl_Obj *headerList;

	/* fixme: might get that differently */
	r = (request_rec *) Tcl_GetAssocData(interp, WEB_AP_ASSOC_DATA, NULL);
	if (r == NULL) {
	    Tcl_SetResult(interp, "error accessing httpd request object",
			  NULL);
	    return TCL_ERROR;
	}

	httpResponse = responseObj->httpresponse;
	if (httpResponse != NULL) {
	    /* fixme: is this the proper way of dealing with the problem that
	     * apache ALWAYS adds the protocol by itself? */
	    char *response = strchr(Tcl_GetString(httpResponse), (int) ' ');
	    if (response)
#ifndef APACHE2
		r->status_line = ap_pstrdup(r->pool, ++response);
#else /* APACHE2 */
		r->status_line = (char *) apr_pstrdup(r->pool, ++response);
#endif /* APACHE2 */
	}
	assignIteratorToHashTable(responseObj->headers, &iterator);
	while (nextFromHashIterator(&iterator) != TCL_ERROR) {
	    key = keyOfCurrent(&iterator);
	    if (key != NULL) {
		headerList = (Tcl_Obj *) valueOfCurrent(&iterator);
		if (headerList != NULL) {
		    int lobjc = -1;
		    Tcl_Obj **lobjv = NULL;
		    int i;
		    if (Tcl_ListObjGetElements(interp, headerList,
					       &lobjc, &lobjv) == TCL_ERROR) {
			LOG_MSG(interp, WRITE_LOG,
				__FILE__, __LINE__,
				"web::put", WEBLOG_ERROR,
				Tcl_GetStringResult(interp), NULL);
			return TCL_ERROR;
		    }
		    /* add all occurrences of this header */
		    /* fixme: ap_table_setn overwrites -> only last header is set !!! */
		    for (i = 0; i < lobjc; i++) {
			/* fixme: check if case insenitive case compare */
			if (strcmp(key, "Content-Type") == 0) {
#ifndef APACHE2
			    r->content_type =
				ap_pstrdup(r->pool, Tcl_GetString(lobjv[i]));
			}
			else {
			    ap_table_setn(r->headers_out,
					  ap_pstrdup(r->pool, key),
					  ap_pstrdup(r->pool,
						     Tcl_GetString(lobjv
								   [i])));
#else /* APACHE2 */
			    r->content_type =
				(char *) apr_pstrdup(r->pool,
						     Tcl_GetString(lobjv[i]));
			}
			else {
			    apr_table_setn(r->headers_out,
					   (char *) apr_pstrdup(r->pool, key),
					   (char *) apr_pstrdup(r->pool,
								Tcl_GetString
								(lobjv[i])));
#endif /* APACHE2 */
			}
		    }
		}
	    }
	}
#ifndef APACHE2
	ap_send_http_header(r);
#else /* APACHE2 */
	/* fixme: how to force headers to be sent? */
	/*ap_send_http_header(r); */
#endif /* APACHE2 */
	responseObj->sendHeader = 0;
    }
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * createDefaultResponseObj
 * ------------------------------------------------------------------------- */
ResponseObj *createDefaultResponseObj(Tcl_Interp * interp)
{
    return createResponseObj(interp, APCHANNEL, &apHeaderHandler);
}

/* ----------------------------------------------------------------------------
 * isDefaultResponseObj
 * ------------------------------------------------------------------------- */
int isDefaultResponseObj(char *name)
{
    return !strcmp(name, APCHANNEL);
}
