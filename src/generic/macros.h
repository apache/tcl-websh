/*
 * macros.h --- macros for websh3
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

#ifndef WEB_MACROS_H
#define WEB_MACROS_H

#include <tcl.h>

#if TCL_MAJOR_VERSION > 8 || (TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION >= 4)
#  ifndef NO_CONST
#    define TCLCONST const
#  else
#    define TCLCONST
#  endif
#else
#  define TCLCONST
#endif

#ifndef WIN32
#define __declspec(dllexport)
#endif

#include "log.h"

#define WEBSH     "websh"


/* --------------------------------------------------------------------------
 * Varia concerning NULL 
 * ------------------------------------------------------------------------*/
#define WebAssertData(interp,data,cmd,ret) \
  if( data == NULL ) { \
    LOG_MSG(interp,WRITE_LOG | SET_RESULT, \
      __FILE__,__LINE__, \
      cmd,WEBLOG_ERROR, \
      "error accessing internal data",NULL); \
    return ret; \
  }

#define WebAllocInternalData(type) \
  (type *)Tcl_Alloc(sizeof(type));

#define WebFreeIfNotNull(data) \
    if( data != NULL ) { \
      Tcl_Free((char *)data); \
      data = NULL; \
    }

#define WebNewStringObjFromStringIncr(obj,string) \
    obj = Tcl_NewStringObj(string,-1); \
    Tcl_IncrRefCount(obj);

#define WebDecrRefCountIfNotNull(data) \
    if( data != NULL ) Tcl_DecrRefCount(data);

#define WebIncrRefCountIfNotNull(data) \
    if( data != NULL ) Tcl_IncrRefCount(data);

#define WebDecrOldIncrNew(var,val) \
    WebDecrRefCountIfNotNull(var); \
    var = val; \
    WebIncrRefCountIfNotNull(var);

#define WebDecrRefCountIfNotNullAndSetNull(data) \
    WebDecrRefCountIfNotNull(data); \
    data = NULL;

#define WebHandleGetSetOfTclObj(interp,idx,item,value) \
    switch (idx) { \
    case GET: \
      if( item == NULL ) { \
        Tcl_SetResult(interp,"has not yet been set",NULL); \
        return TCL_ERROR; \
      } \
      Tcl_SetObjResult(interp,item); \
      return TCL_OK; \
      break; \
    case SET: \
      if( item != NULL ) Tcl_DecrRefCount(item); \
      item = Tcl_DuplicateObj(value); \
      Tcl_IncrRefCount(item); \
      return TCL_OK; \
      break; \
    case DEL: \
      if( item != NULL ) Tcl_DecrRefCount(item); \
      item = NULL; \
      return TCL_OK; \
      break; \
    default: \
      Tcl_SetResult(interp,"must be set, get, or unset",NULL); \
      return TCL_ERROR; \
    }

#define IfNullLogRetNull(interp,data,text) \
    if( data == NULL ) { \
      webLog(interp,WEBLOG_DEBUG,text); \
      return NULL; \
    }

#define WebGetLong(interp,obj,val,flag) \
  if( (flag = Tcl_GetLongFromObj(interp,obj,&val)) == TCL_ERROR ) { \
    LOG_MSG(interp,WRITE_LOG | SET_RESULT, \
      __FILE__,__LINE__, \
      WEBSH, WEBLOG_ERROR, \
      "error getting long from \"",Tcl_GetString(obj),"\"",NULL); \
    val = -1; \
  }

#define WebGetInt(interp,obj,val,flag) \
  if( (flag = Tcl_GetIntFromObj(interp,obj,&val)) == TCL_ERROR ) { \
    LOG_MSG(interp,WRITE_LOG | SET_RESULT, \
      __FILE__,__LINE__, \
      WEBSH, WEBLOG_ERROR, \
      "error getting int from \"",Tcl_GetString(obj),"\"",NULL); \
    val = -1; \
  }

/* --------------------------------------------------------------------------
 * Argument parsing 
 * ------------------------------------------------------------------------*/
#define WebAssertArgs(interp,objc,objv,params,idx,scanc)  \
  if( (idx = argHasOnlyAccepted(objc,objv,params,scanc)) != 0 ) { \
    Tcl_GetIndexFromObj(interp,objv[idx],params,"option",0,&idx);  \
    return TCL_ERROR;  \
  }

#define WebAssertObjc(expr, count, msg) \
  if (expr) { \
    Tcl_WrongNumArgs(interp, count, objv, msg); \
    return TCL_ERROR; \
  }

#endif
