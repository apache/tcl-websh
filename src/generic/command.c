/* ----------------------------------------------------------------------------
 * command.c -- implement web::command for websh3
 * nca-073-9
 * 
 * Copyright (c) 1996-2000 by Netcetera AG.
 * All rights reserved.
 *
 * See the file "license.terms" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) $Id$
 * ------------------------------------------------------------------------- */

#include <tcl.h>
#include "request.h"
#include "paramlist.h"

/* ----------------------------------------------------------------------------
 * Web_Command
 * ------------------------------------------------------------------------- */
int Web_Command(ClientData clientData, 
		Tcl_Interp *interp, 
		int objc, Tcl_Obj *CONST objv[]) {

  RequestData *requestData = NULL;
  Tcl_Obj     *res = NULL;

  /* --------------------------------------------------------------------------
   * check for private data
   * ----------------------------------------------------------------------- */
  WebAssertData(interp,clientData,"Web_Command",TCL_ERROR)
  requestData = (RequestData *)clientData;

  /* --------------------------------------------------------------------------
   * check for arguments
   * ----------------------------------------------------------------------- */
  WebAssertObjc(objc > 3, 1, "??command? code?");

  /* --------------------------------------------------------------------------
   * no argument --> return command name from querystring, if any
   * ----------------------------------------------------------------------- */
  if (objc == 1) {

    res = (Tcl_Obj *)getFromHashTable(requestData->paramList,
                                     Tcl_GetString(requestData->cmdTag));
    if( res != NULL ) Tcl_SetObjResult(interp,res);
    else Tcl_SetResult(interp,WEB_REQ_DEFAULT,NULL);

    return TCL_OK;
  }

  /* --------------------------------------------------------------------------
   * one argument --> use name WEB_REQ_DEFAULT, append to list
   * ----------------------------------------------------------------------- */
  if (objc == 2) 
    return paramListSet(requestData->cmdList,WEB_REQ_DEFAULT,objv[1]);

  /* --------------------------------------------------------------------------
   * two arguments --> append "name" "command" to list
   * ----------------------------------------------------------------------- */
  if (objc == 3) {

    int res = 0;

    res = paramListSet(requestData->cmdList,
		       Tcl_GetString(objv[1]),
		       objv[2]);
    return res;
  }

  return TCL_ERROR;
}


/* ----------------------------------------------------------------------------
 * Web_GetCommand
 * ------------------------------------------------------------------------- */
int Web_GetCommand(ClientData clientData, 
		   Tcl_Interp *interp, 
		   int objc, Tcl_Obj *CONST objv[]) {

  RequestData *requestData = NULL;
  Tcl_Obj     *res = NULL;

  /* --------------------------------------------------------------------------
   * check for private data
   * ----------------------------------------------------------------------- */
  WebAssertData(interp,clientData,"Web_GetCommand",TCL_ERROR)
  requestData = (RequestData *)clientData;

  /* --------------------------------------------------------------------------
   * check for arguments
   * ----------------------------------------------------------------------- */
  WebAssertObjc(objc < 1 || objc > 2, 1, "?command?");

  /* --------------------------------------------------------------------------
   * no argument --> return code for command WEB_REQ_DEFAULT
   * ----------------------------------------------------------------------- */
  if (objc == 1) {

    res = (Tcl_Obj *)getFromHashTable(requestData->cmdList,WEB_REQ_DEFAULT);
    if( res != NULL ) {

      Tcl_Obj *tmp = NULL;

      Tcl_ListObjIndex(interp,res,0,&tmp);

      Tcl_SetObjResult(interp,tmp);
      return TCL_OK;
    } else {
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	      "web::getcommand",WEBLOG_INFO,
	      "command \"",WEB_REQ_DEFAULT,"\" not found",NULL);
      return TCL_ERROR;
    }
  }

  /* --------------------------------------------------------------------------
   * one argument --> return code for command "name"
   * ----------------------------------------------------------------------- */
  if (objc == 2) {
    res = (Tcl_Obj *)getFromHashTable(requestData->cmdList,
				     Tcl_GetString(objv[1]));
    if( res != NULL ) {

      Tcl_Obj *tmp = NULL;

      Tcl_ListObjIndex(interp,res,0,&tmp);

      Tcl_SetObjResult(interp,tmp);
      return TCL_OK;
    } else {
      Tcl_SetResult(interp,"no such command: \"",NULL);
      Tcl_AppendResult(interp,Tcl_GetString(objv[1]),"\".",NULL);
      return TCL_ERROR;
    }
  }
  return TCL_ERROR;
}
