/*
 * modwebsh_cgi.h -- additional header file for modwebsh
 * nca-073-9
 * 
 * Copyright (c) 1996-2000 by Netcetera AG.
 * Copyright (c) 2009 by Apache Software Foundation.
 * All rights reserved.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id: modwebsh.h 322352 2002-09-11 09:14:06Z ronnie $
 *
 */

#include "webout.h"

#ifndef MODWEBSH_CGI_H
#define MODWEBSH_CGI_H

typedef struct
{
  ResponseObj * (*createDefaultResponseObj) (Tcl_Interp * interp);
  int (*isDefaultResponseObj) (Tcl_Interp * interp, char *name);
  Tcl_Obj* (*requestGetDefaultChannelName) (Tcl_Interp * interp);
  char* (*requestGetDefaultOutChannelName) (Tcl_Interp * interp);
  int (*requestFillRequestValues) (Tcl_Interp *interp, RequestData *requestData);
  int (*Web_Initializer) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*Web_Finalizer) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*Web_Finalize) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*Web_InterpCfg) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*Web_InterpClassCfg) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*Web_MainEval) (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*Web_ConfigPath) (Tcl_Interp *interp, int objc, Tcl_Obj *CONST objv[]);
  int (*ModWebsh_Init) (Tcl_Interp *interp);
}
ApFuncs;

#define WEB_APFUNCS_ASSOC_DATA "web::apfuncs"

#endif
