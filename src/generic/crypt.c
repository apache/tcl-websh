/*
 * cryp.c --- The encryption facility of websh
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

#include <tcl.h>
#include "crypt.h"
#include "log.h"
#include "macros.h"
#include "nca_d.h"

/* ----------------------------------------------------------------------------
 * init
 * ------------------------------------------------------------------------- */
int crypt_Init(Tcl_Interp *interp) {

  CryptData      *cryptData = NULL;
  int            ires = 0;
  Tcl_Obj        *tmp = NULL;

  /* --------------------------------------------------------------------------
   * interpreter running ?
   * ----------------------------------------------------------------------- */
  if (interp == NULL) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * init internal data, and register with interp
   * ----------------------------------------------------------------------- */
  cryptData = createCryptData();
  WebAssertData(interp,cryptData,"web::crypt",TCL_ERROR);

  /* --------------------------------------------------------------------------
   * register data with interp
   * ----------------------------------------------------------------------- */
  Tcl_SetAssocData(interp,WEB_CRYPT_ASSOC_DATA,
                   destroyCryptData,
                   (ClientData)cryptData);

  /* --------------------------------------------------------------------------
   * register commands
   * ----------------------------------------------------------------------- */
  Tcl_CreateObjCommand(interp, "web::encrypt",
		       Web_Encrypt,
  		       NULL,
  		       NULL);

  Tcl_CreateObjCommand(interp, "web::decrypt", 
		       Web_Decrypt,
 		       NULL,
 		       NULL);

  /* --------------------------------------------------------------------------
   * default encrypt and decrypt chains
   * ----------------------------------------------------------------------- */
  tmp = Tcl_NewStringObj("web::encryptd",-1);
  cryptData->encryptChain = Tcl_NewListObj(1,&tmp);

  tmp = Tcl_NewStringObj("web::decryptd",-1);
  cryptData->decryptChain = Tcl_NewListObj(1,&tmp);

  /* --------------------------------------------------------------------------
   * done
   * ----------------------------------------------------------------------- */
  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * createCryptData --
 * ------------------------------------------------------------------------- */
CryptData *createCryptData() {

  CryptData *cryptData = NULL;

  cryptData = WebAllocInternalData(CryptData);

  if( cryptData != NULL ) {

    cryptData->encryptChain = NULL;
    cryptData->decryptChain = NULL;
  }

  return cryptData;
}

/* ----------------------------------------------------------------------------
 * destroyCryptData --
 * ------------------------------------------------------------------------- */
void destroyCryptData(ClientData clientData, Tcl_Interp *interp) {

  CryptData *cryptData = NULL;

  if( clientData != NULL ) {

    cryptData = (CryptData *)clientData;

    WebDecrRefCountIfNotNull(cryptData->encryptChain);
    WebDecrRefCountIfNotNull(cryptData->decryptChain);

    WebFreeIfNotNull(cryptData);
  }
}


/* --------------------------------------------------------------------------
 * Web_Encrypt -- in: list, out: str
 * ------------------------------------------------------------------------*/ 
int Web_Encrypt(ClientData clientData, 
                Tcl_Interp *interp, 
                int objc, Tcl_Obj *CONST objv[]) {

  WebAssertObjc(objc != 2, 1, "string");

  return doencrypt(interp, objv[1], 0);
}



/* --------------------------------------------------------------------------
 * Web_Decrypt
 * ------------------------------------------------------------------------*/ 
int Web_Decrypt(ClientData clientData, 
                Tcl_Interp *interp, 
                int objc, Tcl_Obj *CONST objv[]) {

  /* ------------------------------------------------------------------------
   * arg check: web::decrypt msg
   *            0            1
   * --------------------------------------------------------------------- */
  WebAssertObjc(objc != 2, 1, "string");

  return dodecrypt(interp, objv[1], 0);
}


/* --------------------------------------------------------------------------
 * C API -- sets interp result
 * ------------------------------------------------------------------------*/ 
int doencrypt(Tcl_Interp *interp, Tcl_Obj *in, int internal) {

  CryptData *cryptData = NULL;
  int       lobjc = -1;
  Tcl_Obj   **lobjv = NULL;
  int       i = -1;

  if( (interp == NULL) || (in == NULL) ) return TCL_ERROR;

  cryptData = (CryptData *)Tcl_GetAssocData(interp,WEB_CRYPT_ASSOC_DATA,NULL);
  WebAssertData(interp,cryptData,"doencrypt",TCL_ERROR);

  WebAssertData(interp,cryptData->encryptChain,"doencrypt",TCL_ERROR);

  /* ------------------------------------------------------------------------
   * get elements from encryptchain
   * --------------------------------------------------------------------- */
  if( Tcl_ListObjGetElements(interp,cryptData->encryptChain,
			     &lobjc,&lobjv) == TCL_ERROR ) {

    LOG_MSG(interp,WRITE_LOG | SET_RESULT,
            __FILE__,__LINE__,
            "web::encrypt",WEBLOG_ERROR,
            "error accessing encryptchain",NULL);
    return TCL_ERROR;
  }

  /* --------------------------------------------------------------------------
   * ... and loop
   * ----------------------------------------------------------------------- */
  for (i = 0; i <= lobjc; i++) {

    int     res = 0;
    Tcl_Obj *cmd = NULL;

    if (i < lobjc) {
      if(lobjv[i] != NULL)
	cmd = Tcl_DuplicateObj(lobjv[i]);
      else
	cmd = NULL;
    } else {
      if (!internal) {
	Tcl_SetObjResult(interp, in);
	return TCL_OK;
      }
      cmd = Tcl_NewListObj(0, NULL);
      Tcl_ListObjAppendElement(interp, cmd, Tcl_NewStringObj("web::list2uri", -1));
    }

    if (cmd != NULL) {

      if( Tcl_ListObjAppendElement(interp,cmd,in) != TCL_OK ) {
	Tcl_DecrRefCount(cmd);
	return TCL_ERROR;
      }

      Tcl_IncrRefCount(cmd);
      res = Tcl_EvalObjEx(interp,cmd,TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(cmd);

      switch( res ) {
      case TCL_OK:
	return TCL_OK;
	break;
      case TCL_CONTINUE:
	break;
      default:
	if (i < lobjc) {
	  LOG_MSG(interp,WRITE_LOG | SET_RESULT,
		  __FILE__,__LINE__,
		  "web::encrypt",WEBLOG_ERROR,
		  "encrypt method \"",Tcl_GetString(lobjv[i]),"\": ",
		  Tcl_GetStringResult(interp),NULL);
	}
	return TCL_ERROR;
	break;
      }     
    }
  }

  LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	  __FILE__,__LINE__,
	  "web::encrypt",WEBLOG_ERROR,
	  "no matching encryption method found",
	  NULL);

  return TCL_ERROR;
}

/* --------------------------------------------------------------------------
 * C API
 * ------------------------------------------------------------------------*/ 
int dodecrypt(Tcl_Interp *interp, Tcl_Obj *in, int internal) {

  CryptData *cryptData = NULL;
  int       lobjc = -1;
  Tcl_Obj   **lobjv = NULL;
  int       i = -1;

  if( (interp == NULL) || (in == NULL) ) return TCL_ERROR;

  cryptData = (CryptData *)Tcl_GetAssocData(interp,WEB_CRYPT_ASSOC_DATA,NULL);
  WebAssertData(interp,cryptData,"web::decrypt",TCL_ERROR);

  WebAssertData(interp,cryptData->decryptChain,"web::decrypt",TCL_ERROR);

  /* ------------------------------------------------------------------------
   * loop over encryptchain
   * --------------------------------------------------------------------- */
  if( Tcl_ListObjGetElements(interp,cryptData->decryptChain,
			     &lobjc,&lobjv) == TCL_ERROR ) {
    LOG_MSG(interp,WRITE_LOG | SET_RESULT,
            __FILE__,__LINE__,
            "web::decrypt",WEBLOG_ERROR,
            "error accessing decryptchain",NULL);
    return TCL_ERROR;
  }

  /* --------------------------------------------------------------------------
   * now see what we got
   * ----------------------------------------------------------------------- */
  for (i = 0; i <= lobjc; i++) {

    int     res = 0;
    Tcl_Obj *cmd = NULL;

    if (i < lobjc) {
      if(lobjv[i] != NULL)
	cmd = Tcl_DuplicateObj(lobjv[i]);
      else 
	cmd = NULL;
    } else {
      if (!internal) {
	Tcl_SetObjResult(interp, in);
	return TCL_OK;
      }
      cmd = Tcl_NewListObj(0, NULL);
      Tcl_ListObjAppendElement(interp, cmd, Tcl_NewStringObj("web::uri2list", -1));
    }

    if (cmd != NULL) {

      if (Tcl_ListObjAppendElement(interp,cmd,in) != TCL_OK) {
	Tcl_DecrRefCount(cmd);
	return TCL_ERROR;
      }

      Tcl_IncrRefCount(cmd);
      res = Tcl_EvalObjEx(interp,cmd,TCL_EVAL_DIRECT);
      Tcl_DecrRefCount(cmd);

      switch (res) {
      case TCL_OK:
	return TCL_OK;
	break;
      case TCL_CONTINUE:
	break;
      default:
	if (i < lobjc) {
	  LOG_MSG(interp,WRITE_LOG | SET_RESULT,
		  __FILE__,__LINE__,
		  "web::decrypt",WEBLOG_ERROR,
		  "decrypt method \"",Tcl_GetString(lobjv[i]),"\": ",
		  Tcl_GetStringResult(interp),NULL);
	}
	return TCL_ERROR;
	break;
      }
    }
  }

  LOG_MSG(interp,WRITE_LOG | SET_RESULT,
	  __FILE__,__LINE__,
	  "web::decrypt",WEBLOG_ERROR,
	  "no matching decryption method found",
	  NULL);


  return TCL_ERROR;
}
