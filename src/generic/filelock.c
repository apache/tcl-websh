/*
 * filelock.c -- file locking primitives
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
 */

#include <tcl.h>
#include "webutl.h"
#include "filelock.h"


/* ----------------------------------------------------------------------------
 * truncate
 * ------------------------------------------------------------------------- */
int truncate_file(ClientData handle) {

  int res = -1;
  int filedes = -1;

  filedes = (int)handle;

#ifdef SYSV
  /* since lockf takes size as argument,
   * or 0 for "up to EOF", perform a seek first */
  res = ftruncate(filedes,0);
#endif
#ifdef BSDI
  /* same as SYSV */
  res = ftruncate(filedes,0);
#endif
#ifdef FREEBSD
  /* same as SYSV */
  res = ftruncate(filedes,0);
#endif
#ifdef AIX
  /* same as SYSV */
  res = ftruncate(filedes,0);
#endif
#ifdef WIN32
  {
    LONG lres = 0;
    BOOL bres = FALSE;
    res = 1;
    lres = SetFilePointer((HANDLE)handle,0,NULL,FILE_BEGIN);
    if( lres != 0xFFFFFFFF ) {
      bres = SetEndOfFile((HANDLE)handle);
    }
    if( bres ) res = 0;
  }
#endif
  return res;
}

/* ----------------------------------------------------------------------------
 * from websh2: lock
 * ------------------------------------------------------------------------- */
int lock_file(ClientData handle) {

  int res = -1;
  int filedes = -1;

  filedes = (int)handle;

#ifdef SYSV
  /* since lockf takes size as argument,
   * or 0 for "up to EOF", perform a seek first */
  lseek(filedes, 0L ,0);
  res = lockf(filedes, F_LOCK, (long)0);
#endif
#ifdef WIN32
  res = (int)LockFile((HANDLE)handle,0,0,1,0);
  if( res != 0 ) res = 0; else res = 1;
#endif
#ifdef FREEBSD
  res = flock(filedes,LOCK_EX);
#endif
#ifdef BSDI
  res = flock(filedes,LOCK_EX);
#endif
#ifdef AIX
  lseek(filedes, 0L ,0);
  res = lockf(filedes,F_LOCK,0);
#endif

  return res;
}

/* ----------------------------------------------------------------------------
 * from websh2: unlock
 * ------------------------------------------------------------------------- */
int unlock_file(ClientData handle) {

	int res = 0;
	int filedes = -1;

	filedes = (int)handle;

#ifdef SYSV
  /* since lockf takes size as argument,
   * or 0 for "up to EOF", perform a seek first */
  lseek(filedes, 0L ,0);
  res = lockf(filedes, F_ULOCK, (long)0);
#endif
#ifdef WIN32
  res = (int)UnlockFile((HANDLE)handle,0,0,1,0);
  /* return non-zero in case of error */
  if( res != 0 ) res = 0; else res = 1;
#endif
#ifdef BSDI
  res = flock(filedes,LOCK_UN) ;
#endif
#ifdef FREEBSD
  res = flock(filedes,LOCK_UN) ;
#endif
#ifdef AIX
  lseek(filedes, 0L ,0);
  res = lockf(filedes,F_ULOCK,0);
#endif

  return res;
}


/* ----------------------------------------------------------------------------
 * Tcl_Channel locking mechanism
 * ------------------------------------------------------------------------- */
int lock_TclChannel(Tcl_Interp *interp, Tcl_Channel channel) {

  ClientData handle;
  int res = 0;

  if (Tcl_GetChannelHandle(channel,
			   TCL_WRITABLE,
			   &handle) != TCL_OK) {

    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::lockfile",WEBLOG_ERROR,
	    "error getting channelhandle: ",Tcl_GetStringResult(interp),NULL);
    return TCL_ERROR;
  }

  if( lock_file(handle) ) {
    LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	    "web::lockfile",WEBLOG_ERROR,
	    "error getting lock: ",Tcl_ErrnoMsg(Tcl_GetErrno()),NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Tcl_Channel unlocking mechanism
 * ------------------------------------------------------------------------- */
int unlock_TclChannel(Tcl_Interp *interp, Tcl_Channel channel) {

  ClientData handle;
  int res = 0;
  
  if (Tcl_GetChannelHandle(channel,
			   TCL_WRITABLE,
			   &handle) != TCL_OK) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::unlockfile",WEBLOG_ERROR,
	    "error getting handle:",Tcl_GetStringResult(interp),NULL);
    return TCL_ERROR;
  }

  /* flush channel (unlock_file will rewind file) */
  if (Tcl_Flush(channel) != TCL_OK) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::unlockfile",WEBLOG_ERROR,
	    "error flushing channel: ",Tcl_ErrnoMsg(Tcl_GetErrno()),NULL);
    return TCL_ERROR;
  }

  if( unlock_file(handle) ) {
    LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	    "web::unlockfile",WEBLOG_ERROR,
	    "error unlocking: ",Tcl_ErrnoMsg(Tcl_GetErrno()),NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}

/* --------------------------------------------------------------------------
 * Channel locking
 * ------------------------------------------------------------------------*/ 
int Web_LockChannel(ClientData clientData, 
		    Tcl_Interp *interp, 
		    int objc, Tcl_Obj *CONST objv[]) {
 
  Tcl_Channel channel;
  WebAssertObjc(objc != 2, 1, "channel");
  /* get the channel */
  if ((channel = Tcl_GetChannel(interp,Tcl_GetString(objv[1]),NULL)) == NULL) 
     return TCL_ERROR;
  return lock_TclChannel(interp,channel);
}

/* --------------------------------------------------------------------------
 * Channel unlocking
 * ------------------------------------------------------------------------*/ 
int Web_UnLockChannel(ClientData clientData, 
		    Tcl_Interp *interp, 
		    int objc, Tcl_Obj *CONST objv[]) {
  
  Tcl_Channel channel;
  WebAssertObjc(objc != 2, 1, "channel");
  /* get the channel */
  if ((channel = Tcl_GetChannel(interp,Tcl_GetString(objv[1]),NULL)) == NULL) 
     return TCL_ERROR;
  return unlock_TclChannel(interp,channel);
}

/* ----------------------------------------------------------------------------
 * web::filetruncate
 * ------------------------------------------------------------------------- */
int Web_TruncateFile(ClientData clientData, 
		     Tcl_Interp *interp, 
		     int objc, Tcl_Obj *CONST objv[]) {

  Tcl_Channel channel;
  ClientData  handle;
  int res = 0;

  WebAssertObjc(objc != 2, 1, "channel");

  /* get the channel */
  if ((channel = Tcl_GetChannel(interp,Tcl_GetString(objv[1]),NULL)) == NULL) 
     return TCL_ERROR;

  if (Tcl_GetChannelHandle(channel,
			   TCL_WRITABLE,
			   &handle) != TCL_OK) {

    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::truncatefile",WEBLOG_ERROR,
	    "error getting channelhandle: ",Tcl_GetStringResult(interp),NULL);
    return TCL_ERROR;
  }

  if( truncate_file(handle) ) {
    LOG_MSG(interp,WRITE_LOG | SET_RESULT,__FILE__,__LINE__,
	    "web::truncatefile",WEBLOG_ERROR,
	    "error truncating file: ",Tcl_ErrnoMsg(Tcl_GetErrno()),NULL);
    return TCL_ERROR;
  }

  return TCL_OK;
}


