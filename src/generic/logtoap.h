/*
 * logtoap.h --
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

#ifndef WEB_LOGTOAP_H
#define WEB_LOGTOAP_H

/* ----------------------------------------------------------------------------
 * plugin logger: toAp
 * ------------------------------------------------------------------------- */
typedef struct LogToApData
{
    int dum;
}
LogToApData;
LogToApData *createLogToApData();
int destroyLogToApData(Tcl_Interp * interp, LogToApData * logToApData);

ClientData createLogToAp(Tcl_Interp * interp, ClientData clientData,
			 int objc, Tcl_Obj * CONST objv[]);
int destroyLogToAp(Tcl_Interp * interp, ClientData clientData);
int logToAp(Tcl_Interp * interp, ClientData clientData, char *msg, ...);

#endif
