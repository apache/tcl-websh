/*
 * modwebsh.h -- header file for modwebsh_ap.c and modwebsh_cgi.c
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

#ifndef MODWEBSH_H
#define MODWEBSH_H

int Web_Initializer(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);
int Web_Finalizer(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Finalize(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_MainEval(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_InterpCfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_InterpClassCfg(ClientData clientData,
		       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_ConfigPath(Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int modwebsh_createcmd(Tcl_Interp * interp);


#endif
