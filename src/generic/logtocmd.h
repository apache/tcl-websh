/*
 * logtocmd.h -- plugin for log module of websh3
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

#ifndef WEB_LOGTOCMD_H
#define WEB_LOGTOCMD_H

/* --------------------------------------------------------------------------
 * Commands
 * ------------------------------------------------------------------------*/ 

/* ----------------------------------------------------------------------------
 * SubCommands
 * ------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------
 * Switches (like "string -binary")
 * ------------------------------------------------------------------------- */
#define WEB_LOGTOCMD_SWITCH_UNBUFFERED "-unbuffered"

/* ----------------------------------------------------------------------------
 * Parameters (like "web::cmdurl -cmd aCommand", where there is an argument
 * ------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------
 * Registered Data
 * ------------------------------------------------------------------------*/ 

/* --------------------------------------------------------------------------
 * messages
 * ------------------------------------------------------------------------*/ 
#define WEB_LOGTOCMD_USAGE "cmdName"


/* ----------------------------------------------------------------------------
 * plugin logger: tocmd
 * ------------------------------------------------------------------------- */
typedef char *LogToCmdData;
int          destroyLogToCmdData(Tcl_Interp *interp,
				 LogToCmdData *logToCmdData);

ClientData createLogToCmd(Tcl_Interp *interp, ClientData clientData,
			  int objc, Tcl_Obj *CONST objv[]);
int destroyLogToCmd(Tcl_Interp *interp, ClientData clientData);
int logToCmd(Tcl_Interp *interp,ClientData clientData, char *msg);

#endif
