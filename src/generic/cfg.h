/*
 * cfg.h --- websh configuration
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
#include "webutl.h"
#include "request.h"
#include "crypt.h"
#include "webout.h"
#include "log.h"


#ifndef CFG_H
#define CFG_H

#define WEB_CFG_ASSOC_DATA "web::cfgData"


/* ----------------------------------------------------------------------------
 * CfgData
 * ------------------------------------------------------------------------- */
typedef struct CfgData {
  RequestData *requestData;
  CryptData   *cryptData;
  OutData     *outData;
  LogData     *logData;
} CfgData;


CfgData *createCfgData(Tcl_Interp *interp);
void    destroyCfgData(ClientData clientData, Tcl_Interp *interp);

int cfg_Init(Tcl_Interp *interp);

int Web_Cfg(ClientData clientData, 
	    Tcl_Interp *interp, 
	    int objc, Tcl_Obj *CONST objv[]);
#endif
