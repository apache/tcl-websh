/*
 * response_cgi.c -- get request data from env
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
#include "request.h"
#include "modwebsh_cgi.h"

/* ----------------------------------------------------------------------------
 * createDefaultResponseObj
 * ------------------------------------------------------------------------- */
ResponseObj *createDefaultResponseObj(Tcl_Interp * interp)
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->createDefaultResponseObj(interp);

    return createResponseObj(interp, CGICHANNEL, &objectHeaderHandler);
}

/* ----------------------------------------------------------------------------
 * isDefaultResponseObj
 * ------------------------------------------------------------------------- */
int isDefaultResponseObj(Tcl_Interp * interp, char *name)
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->isDefaultResponseObj(interp, name);

    return !strcmp(name, CGICHANNEL);
}
