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
#include "webout.h"

#ifndef MODWEBSH_H
#define MODWEBSH_H

/* declarations for command registration */

int __declspec(dllexport) Web_Initializer(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int __declspec(dllexport) Web_Finalizer(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int __declspec(dllexport) Web_Finalize(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int __declspec(dllexport) Web_MainEval(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int __declspec(dllexport) Web_InterpCfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int __declspec(dllexport) Web_InterpClassCfg(ClientData clientData,
		       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

/* declarations for local (mod_websh) implementation */

int Web_Initializer_AP(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Finalizer_AP(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_Finalize_AP(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_MainEval_AP(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_InterpCfg_AP(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

int Web_InterpClassCfg_AP(ClientData clientData,
		       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]);

ResponseObj* createDefaultResponseObj_AP(Tcl_Interp * interp);

int isDefaultResponseObj_AP(Tcl_Interp * interp, char *name);

Tcl_Obj* requestGetDefaultChannelName_AP(Tcl_Interp * interp);

char* requestGetDefaultOutChannelName_AP(Tcl_Interp * interp);

int requestFillRequestValues_AP(Tcl_Interp *interp, RequestData *requestData);

int Web_ConfigPath_AP(Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);


#endif
