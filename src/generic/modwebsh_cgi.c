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
#include "macros.h"
#include "modwebsh_cgi.h"

/* ----------------------------------------------------------------------------
 * Web_Initializer -- just eval the code
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) Web_Initializer(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    int res = 0;

    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->Web_Initializer(clientData, interp, objc, objv);

    if (objc != 2) {
      Tcl_WrongNumArgs(interp, 1, objv, "code");
      return TCL_ERROR;

    } else {

      /* keep track of log stuff */
      LogData * logData = (LogData *) Tcl_GetAssocData(interp, WEB_LOG_ASSOC_DATA, NULL);
      if (logData != NULL)
	logData->keep = 1;

      Tcl_IncrRefCount(objv[1]);
      res = Tcl_EvalObjEx(interp, objv[1], 0);
      Tcl_DecrRefCount(objv[1]);

      /* reset log flag */
      if (logData != NULL)
	logData->keep = 0;
    }

    return res;
}

/* ----------------------------------------------------------------------------
 * Web_Finalizer -- just eval the code
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) Web_Finalizer(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    int res = 0;

    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->Web_Finalizer(clientData, interp, objc, objv);

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
int __declspec(dllexport) Web_Finalize(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->Web_Finalize(clientData, interp, objc, objv);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_InterpClassCfg -- just return
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) Web_InterpClassCfg(ClientData clientData,
		       Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->Web_InterpClassCfg(clientData, interp, objc, objv);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_InterpCfg -- just return
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) Web_InterpCfg(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->Web_InterpCfg(clientData, interp, objc, objv);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_MainEval -- just return
 * ------------------------------------------------------------------------- */
int __declspec(dllexport) Web_MainEval(ClientData clientData,
		 Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL)
      return apFuncs->Web_MainEval(clientData, interp, objc, objv);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_ConfigPath -- (sub)command (called from Web_Cfg)
 * ------------------------------------------------------------------------- */

int Web_ConfigPath(Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[]) {

  /* these options should be in sync with the options in Web_Cfg
   * not the order or anything, but the actual text strings */
  static TCLCONST char *subCmd[] = {
    "script",
    "server_root",
    "document_root",
    "interpclass",
    NULL
  };
  
  enum subCmd
  {
    SCRIPT,
    SERVER_ROOT,
    DOCUMENT_ROOT,
    INTERPCLASS
  };
  
  int index;
  Tcl_Obj *res = NULL;
 
  ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
  if (apFuncs != NULL)
    return apFuncs->Web_ConfigPath(interp, objc, objv);

  if (Tcl_GetIndexFromObj(interp, objv[1], subCmd, "subcommand", 0, &index)
      != TCL_OK) {
    /* let the caller handle the web::config command */
    Tcl_ResetResult(interp);
    return TCL_CONTINUE;
  }
  
  WebAssertObjc(objc != 2, 2, NULL);
  
  switch ((enum subCmd) index) {
    
  case SCRIPT: {
    res = tclSetEnv(interp, "SCRIPT_FILENAME", NULL);
    break;
  }
  case SERVER_ROOT: {
    res = tclSetEnv(interp, "SERVER_ROOT", NULL);
    break;
  }
  case DOCUMENT_ROOT: {
    res = tclSetEnv(interp, "DOCUMENT_ROOT", NULL);
    break;
  }
  case INTERPCLASS: {
    res = tclSetEnv(interp, "SCRIPT_FILENAME", NULL);
    break;
  }
  }
  /* reset errors from getting invalid env vars */
  Tcl_ResetResult(interp);
  if (res) {
    Tcl_SetObjResult(interp, res);
    Tcl_DecrRefCount(res);
  }
  return TCL_OK;
}


/* -------------------------------------------------------------------------
 * init --
 * ------------------------------------------------------------------------- */
int modwebsh_createcmd(Tcl_Interp * interp)
{
    ApFuncs *apFuncs = Tcl_GetAssocData(interp, WEB_APFUNCS_ASSOC_DATA, NULL);
    if (apFuncs != NULL) {
      /* no need to create commands, but make sure call backs work */
      return apFuncs->ModWebsh_Init(interp);
    }

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
