/*
 * modwebsh_cgi.c -- web::initializer, web::finalizer, web::maineval. CGI case
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

/* ----------------------------------------------------------------------------
 * Web_Initializer -- just eval the code
 * ------------------------------------------------------------------------- */
int Web_Initializer(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int res = 0;

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 1, objv, "code");
      return TCL_ERROR;
    }

    Tcl_IncrRefCount(objv[1]);
    res = Tcl_EvalObjEx(interp, objv[1], 0);
    Tcl_DecrRefCount(objv[1]);

    return res;
}

/* ----------------------------------------------------------------------------
 * Web_Finalizer -- just eval the code
 * ------------------------------------------------------------------------- */
int Web_Finalizer(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int res = 0;

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 1, objv, "code");
      return TCL_ERROR;
    }

    Tcl_IncrRefCount(objv[1]);
    res = Tcl_EvalObjEx(interp, objv[1], 0);
    Tcl_DecrRefCount(objv[1]);

    return res;
}

/* ----------------------------------------------------------------------------
 * Web_Finalizer -- just return
 * ------------------------------------------------------------------------- */
int Web_Finalize(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_InterpClassCfg -- just return
 * ------------------------------------------------------------------------- */
int Web_InterpClassCfg(ClientData clientData,
		       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_InterpCfg -- just return
 * ------------------------------------------------------------------------- */
int Web_InterpCfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_MainEval -- just return
 * ------------------------------------------------------------------------- */
int Web_MainEval(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    return TCL_OK;
}

/* -------------------------------------------------------------------------
 * init --
 * ------------------------------------------------------------------------- */
int modwebsh_createcmd(Tcl_Interp * interp)
{

    Tcl_CreateObjCommand(interp, "web::initializer",
			 Web_Initializer, NULL, NULL);

    Tcl_CreateObjCommand(interp, "web::finalizer", Web_Finalizer, NULL, NULL);

    Tcl_CreateObjCommand(interp, "web::finalize", Web_Finalize, NULL, NULL);

    Tcl_CreateObjCommand(interp, "web::maineval", Web_MainEval, NULL, NULL);

    Tcl_CreateObjCommand(interp, "web::interpcfg", Web_InterpCfg, NULL, NULL);

    Tcl_CreateObjCommand(interp, "web::interpclasscfg",
			 Web_InterpClassCfg, NULL, NULL);

    return TCL_OK;
}
