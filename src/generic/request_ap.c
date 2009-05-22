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
#include "paramlist.h"

#include "mod_websh.h"

#ifdef APACHE2
#include "apr_base64.h"
#endif /* APACHE2 */

/* ----------------------------------------------------------------------------
 * web::request -channel: where input for request obj comes from
 * ------------------------------------------------------------------------- */
Tcl_Obj *requestGetDefaultChannelName_AP(Tcl_Interp * interp)
{
    return Tcl_NewStringObj(APCHANNEL, -1);
}

/* default output channel */

char *requestGetDefaultOutChannelName_AP(Tcl_Interp * interp)
{
    return APCHANNEL;
}

int requestFillRequestValues_AP(Tcl_Interp * interp, RequestData * requestData)
{

    request_rec *r = NULL;
#ifndef APACHE2
    array_header *hdrs_arr = NULL;
    table_entry *hdrs = NULL;
#else /* APACHE2 */
    apr_array_header_t *hdrs_arr = NULL;
    apr_table_entry_t *hdrs = NULL;
#endif /* APACHE2 */
    int i = 0;
    int remote_user = 0;

    Tcl_Obj *valo = NULL;

    if (interp == NULL)
	return TCL_ERROR;

    /* fetch request object */
    r = (request_rec *) Tcl_GetAssocData(interp, WEB_AP_ASSOC_DATA, NULL);
    if (r == NULL) {
	Tcl_SetResult(interp, "error accessing httpd request object", NULL);
	return TCL_ERROR;
    }

#ifndef APACHE2
    hdrs_arr = ap_table_elts(r->subprocess_env);
    hdrs = (table_entry *) hdrs_arr->elts;
#else /* APACHE2 */
    hdrs_arr = (apr_array_header_t *) apr_table_elts(r->subprocess_env);
    hdrs = (apr_table_entry_t *) hdrs_arr->elts;
#endif /* APACHE2 */
    for (i = 0; i < hdrs_arr->nelts; ++i) {

	if (!hdrs[i].key)
	    continue;

	if (!hdrs[i].val)
	    valo = Tcl_NewObj();
	else
	    valo = Tcl_NewStringObj(hdrs[i].val, -1);

	if (paramListAdd(requestData->request, hdrs[i].key, valo) != TCL_OK)
	    /* fatal case */
	    return TCL_ERROR;

	if (!remote_user && !strcmp(hdrs[i].key, "REMOTE_USER")) {
	  remote_user = 1;
	}
    }

    paramListSetAsWhole(requestData->request, "GATEWAY_INTERFACE",
			Tcl_NewStringObj("CGI-websh/1.1", -1));

    /* create AUTH_USER and AUTH_PW if not set (i.e. if not handeled by Apache),
       otherwise don't set them for security reasons */
    if (!remote_user) {

      int ret = 0;
      const char *pw = NULL;
      const char *user = NULL;
      const char *auth_line;

      /* Check to see if a Authorization header is there */
#ifndef APACHE2
      auth_line = (char *)ap_table_get(r->headers_in, "Authorization");
#else /* APACHE2 */
      auth_line = (char *)apr_table_get(r->headers_in, "Authorization");
#endif
      if (auth_line) {

	char *decoded_line;
	int length;

	/* check if we know how to handle the Auth string */
	if (!strcasecmp((char *)ap_getword(r->pool, &auth_line, ' '), "Basic")) {

	  /* Skip leading spaces. */
	  while (isspace((int)*auth_line)) {
	    auth_line++;
	  }
#ifndef APACHE2
	  decoded_line = (char *) ap_palloc(r->pool, ap_base64decode_len(auth_line) + 1);
	  length = ap_base64decode(decoded_line, auth_line);
#else /* APACHE2 */
	  decoded_line = (char *) apr_palloc(r->pool, apr_base64_decode_len(auth_line) + 1);
	  length = apr_base64_decode(decoded_line, auth_line);
#endif
	  /* Null-terminate the string. */
	  decoded_line[length] = '\0';

	  user = ap_getword_nulls(r->pool, (const char**)&decoded_line, ':');
	  pw = decoded_line;

	  if (paramListAdd(requestData->request, "AUTH_USER", Tcl_NewStringObj(user, -1)) != TCL_OK)
	    /* fatal case */
	    return TCL_ERROR;

	  if (paramListAdd(requestData->request, "AUTH_PW", Tcl_NewStringObj(pw, -1)) != TCL_OK)
	    /* fatal case */
	    return TCL_ERROR;

	}

      }

    }

    return TCL_OK;
}
