/* ----------------------------------------------------------------------------
 * cfg.c -- websh configuration
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
 * ------------------------------------------------------------------------- */

#include <tcl.h>
#include "cfg.h"
#include "log.h"
#include "request.h"
#include "crypt.h"

void dCfgData(ClientData clientData);

void dCfgData(ClientData clientData) {

  destroyCfgData(clientData,NULL);
}


/* ----------------------------------------------------------------------------
 * Init --
 * ------------------------------------------------------------------------- */
int cfg_Init(Tcl_Interp *interp) {

  CfgData *cfgData;

  /* --------------------------------------------------------------------------
   * interpreter running ?
   * ----------------------------------------------------------------------- */
  if (interp == NULL) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * new data
   * ----------------------------------------------------------------------- */
  cfgData = createCfgData(interp);
  WebAssertData(interp,cfgData,"cfg",TCL_ERROR);

  /* --------------------------------------------------------------------------
   * register commands
   * ----------------------------------------------------------------------- */
   Tcl_CreateObjCommand(interp, "web::config",
			Web_Cfg,
			(ClientData)cfgData,
			NULL);

   /* -------------------------------------------------------------------------
    * associate data with Interpreter
    * ---------------------------------------------------------------------- */
   Tcl_SetAssocData(interp,WEB_CFG_ASSOC_DATA,
                    destroyCfgData,
                    (ClientData)cfgData);

   /* -------------------------------------------------------------------------
    * need an exit handler, too (if this is the main interp)
    * ---------------------------------------------------------------------- */
   if( Tcl_GetMaster(interp) == NULL )
     Tcl_CreateThreadExitHandler(dCfgData, (ClientData)cfgData);

   /* -------------------------------------------------------------------------
    * done
    * ---------------------------------------------------------------------- */
   return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * create
 * ------------------------------------------------------------------------- */
CfgData *createCfgData(Tcl_Interp *interp) {

  CfgData *cfgData = NULL;

  cfgData = WebAllocInternalData(CfgData);

  if( cfgData != NULL ) {

   cfgData->requestData = \
     (RequestData *)Tcl_GetAssocData(interp,WEB_REQ_ASSOC_DATA,NULL);

   cfgData->cryptData = \
     (CryptData *)Tcl_GetAssocData(interp,WEB_CRYPT_ASSOC_DATA,NULL);

   cfgData->outData = \
     (OutData *)Tcl_GetAssocData(interp,WEB_OUT_ASSOC_DATA,NULL);

   cfgData->logData = \
     (LogData *)Tcl_GetAssocData(interp,WEB_LOG_ASSOC_DATA,NULL);

  }

  return cfgData;
}

/* ----------------------------------------------------------------------------
 * destroy
 * ------------------------------------------------------------------------- */
void destroyCfgData(ClientData clientData, Tcl_Interp *interp) {

  WebFreeIfNotNull(clientData);
}

/* ----------------------------------------------------------------------------
 * Web_Cfg -- access internal data from cfg
 * ------------------------------------------------------------------------- */
int Web_Cfg(ClientData clientData, Tcl_Interp *interp, 
	    int objc, Tcl_Obj *CONST objv[]) {

  /* keep consistent with enum PutxMarkup in request.h */
  static char *putxMarkups[] = {"brace",
			    "tag",
			    NULL};

  static char *subCmd1[] = {"uploadfilesize",
			    "encryptchain",
			    "decryptchain",
			    "cmdparam",
			    "timeparam",
			    "putxmarkup",
			    "logsubst",
			    "version",
			    "copyright",
			    NULL};

  enum subCmd1 {UPLOADFILESIZE,
		ENCRYPTCHAIN,
		DECRYPTCHAIN,
		CMDTAG,
		TIMETAG,
		PUTXMARKUP,
		LOGSUBST,
		WEBSHVERSION,
		WEBSHCOPYRIGHT};


  int     idx1;
  CfgData *cfgData = NULL;

  WebAssertData(interp,clientData,"Web_Cfg",TCL_ERROR);
  cfgData = (CfgData *)clientData;

  /* --------------------------------------------------------------------------
   * parse arguments
   * ----------------------------------------------------------------------- */
  if (objc < 2) {

    Tcl_GetIndexFromObj(interp, objv[0],subCmd1, "usage", 0, &idx1);
    return TCL_ERROR;
  }

  /* ------------------------------------------------------------------------
   * determine first sub-command
   * --------------------------------------------------------------------- */
  if (Tcl_GetIndexFromObj(interp, objv[1],subCmd1, "subcommand", 0, &idx1)
      != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum subCmd1)idx1) {
  case UPLOADFILESIZE: {

    long tmpLong = -1;

    WebAssertData(interp,cfgData->requestData,
			"web::config uploadfilesize",TCL_ERROR);

    WebAssertData(interp,cfgData->requestData->upLoadFileSize,
			"web::config uploadfilesize",TCL_ERROR);

    Tcl_SetObjResult(interp,
		     Tcl_DuplicateObj(cfgData->requestData->upLoadFileSize));

    switch (objc) {
    case 2:
      return TCL_OK;
      break;
    case 3:
      /* --------------------------------------------------------------------
       * only accept integers
       * ----------------------------------------------------------------- */
      if( Tcl_GetLongFromObj(interp,objv[2],&tmpLong) == TCL_ERROR ) {
	LOG_MSG(interp,WRITE_LOG | SET_RESULT,
		__FILE__,__LINE__,
		"web::config uploadfilesize",WEBLOG_ERROR,
		"web::config uploadfilesize only accepts integers but ",
		"got \"", Tcl_GetString(objv[2]),"\"",NULL);
	return TCL_ERROR;
      }
      WebDecrOldIncrNew(cfgData->requestData->upLoadFileSize,
			Tcl_DuplicateObj(objv[2]));
      return TCL_OK;
      break;
    default:
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::config uploadfilesize",WEBLOG_INFO,
	      "usage: web::config uploadfilesize ?size?.",NULL);
      return TCL_ERROR;
      break;
    }
    break;
  }
  case CMDTAG: {
    if( cfgData->requestData != NULL) {
      switch (objc) {
      case 2:
	if( cfgData->requestData->cmdTag != NULL) {
	  Tcl_SetObjResult(interp,cfgData->requestData->cmdTag);
	  return TCL_OK;
	}
	break;
      case 3:
	WebDecrOldIncrNew(cfgData->requestData->cmdTag,
			  Tcl_DuplicateObj(objv[2]));
	return TCL_OK;
	break;
      default:
	LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
		"web::config cmdtag",WEBLOG_INFO,
		"usage: web::config cmdtag ?tag?.",NULL);
	return TCL_ERROR;
	break;
      }
    } else {
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	      "Web_Cfg",WEBLOG_INFO,
	      "cannot access data.",NULL);
      return TCL_ERROR;
    }
    break;
  }
  case TIMETAG: {
    if( cfgData->requestData != NULL) {
      switch (objc) {
      case 2:
	if( cfgData->requestData->timeTag != NULL) {
	  Tcl_SetObjResult(interp,cfgData->requestData->timeTag);
	  return TCL_OK;
	}
	break;
      case 3:
	WebDecrOldIncrNew(cfgData->requestData->timeTag,
			  Tcl_DuplicateObj(objv[2]));
	return TCL_OK;
	break;
      default:
	LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
		"web::config timetag",WEBLOG_INFO,
		"usage: web::config timetag ?tag?.",NULL);
	return TCL_ERROR;
	break;
      }
    } else {
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	      "Web_Cfg",WEBLOG_INFO,
	      "cannot access data.",NULL);
      return TCL_ERROR;
    }
    break;
  }
  case ENCRYPTCHAIN: {

    CryptData *cryptData = NULL;
    Tcl_Obj   *tmpObj = NULL;

    cryptData = cfgData->cryptData;

    if( cryptData != NULL ) {

      if( objc == 3 ) tmpObj = objv[2];
      else            tmpObj = NULL;

      return handleConfig(interp, &cryptData->encryptChain, tmpObj, 0);
    } else {
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::config encryptchain",WEBLOG_ERROR,
	      "error accessing internal data",NULL);
      return TCL_ERROR;
    }
  }
  break;
  case DECRYPTCHAIN: {

    CryptData *cryptData = NULL;
    Tcl_Obj   *tmpObj = NULL;

    cryptData = cfgData->cryptData;

    if( cryptData != NULL ) {

      if( objc == 3 ) tmpObj = objv[2];
      else            tmpObj = NULL;

      return handleConfig(interp, &cryptData->decryptChain, tmpObj, 0);
    } else {
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::config decryptchain",WEBLOG_ERROR,
	      "error accessing internal data",NULL);
      return TCL_ERROR;
    }
  }
  break;
  case PUTXMARKUP: {
    Tcl_Obj *value = Tcl_NewStringObj(putxMarkups[cfgData->outData->putxMarkup], -1);
    switch (objc) {
    case 2:
      break;
    case 3: {
      int type;
      if (Tcl_GetIndexFromObj(interp, objv[2], putxMarkups, "type", 0, &type)
	  != TCL_OK) {
	Tcl_DecrRefCount(value);
	return TCL_ERROR;
      }
      cfgData->outData->putxMarkup = type;
      break;
    }
    default: 
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::config putxmarkup",WEBLOG_ERROR,
	      "usage: web::config putxmarkup ?brace|tag?",NULL);
      Tcl_DecrRefCount(value);
      return TCL_ERROR;
    }
    Tcl_SetObjResult(interp, value);
    return TCL_OK;
  }
  case LOGSUBST:
    Tcl_SetObjResult(interp,Tcl_NewBooleanObj(cfgData->logData->logSubst));
    switch (objc) {
    case 2:
      break;
    case 3:
      /* --------------------------------------------------------------------
       * only accept integers
       * ----------------------------------------------------------------- */
      if( Tcl_GetBooleanFromObj(interp,objv[2],\
				&(cfgData->logData->logSubst)) == TCL_ERROR )
	return TCL_ERROR;
      break;
    default:
      LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::config logsubst",WEBLOG_INFO,
	      "usage: web::config logsubst ?boolean?",NULL);
      return TCL_ERROR;
    }
    return TCL_OK;
  case WEBSHVERSION: {
    WebAssertObjc(objc != 2, 2, NULL);

    Tcl_SetResult(interp,WEBSH,NULL);
    Tcl_AppendResult(interp,VERSION,NULL);
    Tcl_AppendResult(interp," (built: ",
		     __DATE__," ",__TIME__,")",NULL);
    
    return TCL_OK;
  }
  case WEBSHCOPYRIGHT: {
    WebAssertObjc(objc != 2, 2, NULL);

    Tcl_ResetResult(interp);
    Tcl_AppendResult(interp,"Copyright (c) 1996-2000 ",NETCETERA,
		     "\nFor license and information details, "
		     "see http://websh.com\n",
		     "Send comments and requests for help to info@websh.com",
		   NULL);
    return TCL_OK;
  }
  default:
    break;
  }
  return TCL_ERROR;
}
