/*
 * response_cgi.c -- get request data from env
 * nca-073-9
 * 
 * Copyright (c) 1996-2000 by Netcetera AG.
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
#include "webout.h"
#include "request.h"

/* ----------------------------------------------------------------------------
 * createDefaultResponseObj
 * ------------------------------------------------------------------------- */
ResponseObj *createDefaultResponseObj(Tcl_Interp *interp) {
  return createResponseObj(interp, CGICHANNEL, &objectHeaderHandler);
}

/* ----------------------------------------------------------------------------
 * isDefaultResponseObj
 * ------------------------------------------------------------------------- */
int isDefaultResponseObj(char *name) {
  return !strcmp(name, CGICHANNEL);
}
