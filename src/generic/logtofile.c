/*
 * logtofile.c -- plugin for log module of websh3
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
#include <tcl.h>
#include <stdio.h>
#include <string.h>
#include "macros.h"
#include "logtofile.h"
#include "webutl.h" /* args */

/* ----------------------------------------------------------------------------
 * createLogToFileData --
 * ------------------------------------------------------------------------- */
LogToFileData *createLogToFileData() {

  LogToFileData *logToFileData;

  logToFileData = WebAllocInternalData(LogToFileData);
  logToFileData->channel    = NULL;
  logToFileData->fileName   = NULL;
  logToFileData->isBuffered = TCL_OK;

  return logToFileData;
}

/* ----------------------------------------------------------------------------
 * destroyLogToFileData --
 * ------------------------------------------------------------------------- */
int destroyLogToFileData(Tcl_Interp *interp, LogToFileData *logToFileData) {

  if( (interp == NULL) || (logToFileData == NULL) ) return TCL_ERROR;


  WebFreeIfNotNull(logToFileData->fileName);

  if(Tcl_Close(interp,logToFileData->channel) != TCL_OK) {

    Tcl_AppendResult(interp,
		     "destroyLogToFileData -- error closing channel for \"", 
		     logToFileData->fileName,"\"",NULL);
    return TCL_ERROR;
  }

  WebFreeIfNotNull(logToFileData);

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * constructor
 * ------------------------------------------------------------------------- */
ClientData createLogToFile(Tcl_Interp *interp, ClientData clientData,
                           int objc, Tcl_Obj *CONST objv[]) {

  LogToFileData *logToFileData = NULL;
  int           iCurArg;
  char          *fileName = NULL;
  Tcl_Channel   channel;

  /* --------------------------------------------------------------------------
   * syntax is: file ?-unbuffered? fileName
   *            0    1             2
   * ----------------------------------------------------------------------- */
  if( (objc < 2) || (objc > 4)) {
    Tcl_WrongNumArgs(interp, 1, objv, WEB_LOGTOFILE_USAGE);
    return NULL;
  }

  if( strcmp(Tcl_GetString(objv[0]),"file") != 0 ) {
    Tcl_SetResult(interp,WEB_LOGTOFILE_USAGE,NULL);
    return NULL;
  }

  /* --------------------------------------------------------------------------
   * do we have the filename ?
   * ----------------------------------------------------------------------- */
  iCurArg = argIndexOfFirstArg(objc,objv,NULL,NULL);
  if( iCurArg >= objc ) {
    Tcl_SetResult(interp,WEB_LOGTOFILE_USAGE,NULL);
    return NULL;
  }
  fileName = Tcl_GetString(objv[iCurArg]);

  /* --------------------------------------------------------------------------
   * try to open channel
   * ----------------------------------------------------------------------- */
  channel = Tcl_OpenFileChannel(interp,fileName,"a",0644);
  if( channel == NULL ) return NULL;

  /* --------------------------------------------------------------------------
   * everything ok so far. Get memory
   * ----------------------------------------------------------------------- */
  logToFileData = createLogToFileData();
  if( logToFileData == NULL ) {
    Tcl_SetResult(interp,"cannot alloc memory for internal data.",NULL);
    WebFreeIfNotNull(fileName);
    return NULL;
  }

  /* --------------------------------------------------------------------------
   * and set values
   * ----------------------------------------------------------------------- */
  logToFileData->channel = channel;
  logToFileData->fileName = allocAndSet(fileName);
  if( argKeyExists(objc,objv,WEB_LOGTOFILE_SWITCH_UNBUFFERED) == TCL_OK )
    logToFileData->isBuffered = TCL_ERROR;
  else
    logToFileData->isBuffered = TCL_OK;
  
  return (ClientData)logToFileData;
}


/* ----------------------------------------------------------------------------
 * destructor
 * ------------------------------------------------------------------------- */
int destroyLogToFile(Tcl_Interp *interp, ClientData clientData) {
  return destroyLogToFileData(interp,(LogToFileData *)clientData);
}

/* ----------------------------------------------------------------------------
 * logToFile
 * ------------------------------------------------------------------------- */
int logToFile(Tcl_Interp *interp,ClientData clientData, char *msg) {

  LogToFileData *logToFileData;
  int res;

  if( (interp == NULL) || (clientData == NULL) || (msg == NULL) ) 
    return TCL_ERROR;

  logToFileData = (LogToFileData *)clientData;

  Tcl_Seek(logToFileData->channel, 0, SEEK_END);

  res = Tcl_WriteChars( logToFileData->channel, msg, -1);
  if( res < 0 ) return TCL_ERROR;
  if( logToFileData->isBuffered == TCL_ERROR )
    Tcl_Flush( logToFileData->channel );

  return TCL_OK;
}
