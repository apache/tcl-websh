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
#include "request.h"

#include "mod_websh.h"


/* ----------------------------------------------------------------------------
 * web::request -channel: where input for request obj comes from
 * ------------------------------------------------------------------------- */
Tcl_Obj *requestGetDefaultChannelName()
{
    return Tcl_NewStringObj(APCHANNEL, -1);
}


int requestFillRequestValues(Tcl_Interp * interp, RequestData * requestData)
{

    int res = 0;
    Tcl_Obj *reso = NULL;
    request_rec *r = NULL;
#ifndef APACHE2
    array_header *hdrs_arr = NULL;
    table_entry *hdrs = NULL;
#else /* APACHE2 */
    apr_array_header_t *hdrs_arr = NULL;
    apr_table_entry_t *hdrs = NULL;
#endif /* APACHE2 */
    int i = 0;

    if (requestData->requestIsInitialized)
	return TCL_OK;
    requestData->requestIsInitialized = 1;

    if (interp == NULL)
	return TCL_ERROR;

    /* fetch request object */
    r = (request_rec *) Tcl_GetAssocData(interp, WEB_AP_ASSOC_DATA, NULL);
    /* fixme: make the following possible */
    /* r = (request_rec *)(requestData->handleToSpecificReqData); */
    if (r == NULL) {
	Tcl_SetResult(interp, "error accessing httpd request object", NULL);
	return TCL_ERROR;
    }

    reso = Tcl_NewObj();

#ifndef APACHE2
    hdrs_arr = ap_table_elts(r->subprocess_env);
    hdrs = (table_entry *) hdrs_arr->elts;
#else /* APACHE2 */
    hdrs_arr = apr_table_elts(r->subprocess_env);
    hdrs = (apr_table_entry_t *) hdrs_arr->elts;
#endif /* APACHE2 */
    for (i = 0; i < hdrs_arr->nelts; ++i) {

	Tcl_Obj *valo = NULL;
	if (!hdrs[i].key)
	    continue;

	if (!hdrs[i].val)
	    valo = Tcl_NewObj();
	else
	    valo = Tcl_NewStringObj(hdrs[i].val, -1);

	if (paramListAdd(requestData->request, hdrs[i].key, valo) != TCL_OK)
	    /* fata case */
	    return TCL_ERROR;
    }

    /* fixme: better name */
    paramListSetAsWhole(requestData->request, "GATEWAY_INTERFACE",
			Tcl_NewStringObj("websh", -1));
    return TCL_OK;
}
