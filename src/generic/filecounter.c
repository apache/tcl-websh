/* ----------------------------------------------------------------------------
 * filecounter.c --- 
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

#include "filecounter.h"
#include "tcl.h"

/* --------------------------------------------------------------------------
 * Init
 * --------------------------------------------------------------------------*/
int filecounter_Init(Tcl_Interp* interp) {

  if (interp == NULL) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * ClientData
   * ------------------------------------------------------------------------*/ 
  
  /* --------------------------------------------------------------------------
   * register commands
   * ----------------------------------------------------------------------- */
  Tcl_CreateObjCommand(interp, "web::filecounter", 
		       (Tcl_ObjCmdProc *) filecounter,
  		       (ClientData) NULL,
  		       (Tcl_CmdDeleteProc *) NULL);

  /* --------------------------------------------------------------------------
   * register private data with interp
   * ----------------------------------------------------------------------- */
  /* no data for module, but for each handle */
  return TCL_OK;
}

/* --------------------------------------------------------------------------
 * This function handles requests given a handle
 * --------------------------------------------------------------------------*/
int Web_Filecounter(ClientData clientData, 
		    Tcl_Interp *interp, 
		    int objc, Tcl_Obj *objv[]) {

  SeqNoGenerator* seqnogen = (SeqNoGenerator*)clientData;
  static char *subCommands[] = {"curval","nextval","config",NULL};
  enum subCommands {CURVAL,NEXTVAL,CONFIG};
  char **ptr = subCommands;

  int idx;
  int seqno;
  Tcl_Obj* result = NULL;

  /* --------------------------------------------------------------------------
   * deal with an existing filecounter
   * ----------------------------------------------------------------------- */

  if (objc != 2) {
    Tcl_WrongNumArgs(interp, 1, objv, NULL);
    while (*ptr != (char*)NULL) {
      Tcl_AppendResult(interp, *ptr, (char*)NULL);
      ptr++;
      if (*ptr != (char*)NULL)
	Tcl_AppendResult(interp,"|",(char*)NULL);
    }
    return TCL_ERROR;
  }

  if (seqnogen == NULL)
    return TCL_ERROR;

  /* ------------------------------------------------------------------------
   * scan for options
   * --------------------------------------------------------------------- */
  if (Tcl_GetIndexFromObj(interp, objv[1],subCommands, "option", 0, &idx)
      != TCL_OK) {
    return TCL_ERROR;
  }

  switch ((enum subCommands) idx) {
  case NEXTVAL: {
    if (nextSeqNo(interp,seqnogen,&seqno) != TCL_OK) {
      /* error reporting done in subfunction */
      return TCL_ERROR;
    }
    result = Tcl_NewIntObj(seqno);
    Tcl_SetObjResult(interp,result);
    return TCL_OK;
    break;
  }
  case CURVAL: {
    if (seqnogen->hasCurrent) {
      result = Tcl_NewIntObj(seqnogen->currValue);
      Tcl_SetObjResult(interp,result);
      return TCL_OK;
      break;
    } else {
      Tcl_SetResult(interp,
		    "web::filecounter: no current value available",TCL_STATIC);
      return TCL_ERROR;
    }
  }
  case CONFIG: {
    
    Tcl_Obj* kv[16]; 
    int i; 

    for (i=0;i<16;i++)
      kv[i] = Tcl_NewObj();
    
    Tcl_SetStringObj(kv[0],"file",-1);
    Tcl_SetStringObj(kv[1],seqnogen->fileName,-1); 
    Tcl_SetStringObj(kv[2],"handle",-1);
    Tcl_SetStringObj(kv[3],seqnogen->handleName,-1);     
    Tcl_SetStringObj(kv[4],"seed",-1);
    Tcl_SetIntObj(kv[5],seqnogen->seed); 
    Tcl_SetStringObj(kv[6],"min",-1); 
    Tcl_SetIntObj(kv[7],seqnogen->minValue); 
    Tcl_SetStringObj(kv[8],"max",-1); 
    Tcl_SetIntObj(kv[9],seqnogen->maxValue);
    Tcl_SetStringObj(kv[10],"incr",-1); 
    Tcl_SetIntObj(kv[11],seqnogen->incrValue); 
    Tcl_SetStringObj(kv[12],"wrap",-1); 
    if (seqnogen->doWrap) 
       Tcl_SetStringObj(kv[13],"true",-1); 
    else 
      Tcl_SetStringObj(kv[13],"false",-1); 
    Tcl_SetStringObj(kv[14],"curr",-1); 
    if (seqnogen->hasCurrent)
      Tcl_SetIntObj(kv[15],seqnogen->currValue); 
    else  
      Tcl_SetStringObj(kv[15],"not valid",-1); 
    
    result = Tcl_NewListObj(16,kv); 
    Tcl_SetObjResult(interp,result);
    return TCL_OK;
    break; 
  }
  }
  Tcl_SetResult(interp,"error during web::filecounter",NULL);
  return TCL_ERROR;
}


/* --------------------------------------------------------------------------
 * Creates a new filecounter
 * --------------------------------------------------------------------------*/
int filecounter(ClientData clientData, Tcl_Interp *interp, 
		int objc, Tcl_Obj *CONST objv[]) {
  
  int wrap;
  Tcl_Obj *hnameobj,*fnameobj,*seedobj,*maxobj,*minobj,*incrobj;
  SeqNoGenerator* seqnogen = NULL;
  Tcl_Obj* result = NULL;
  Tcl_CmdInfo cmdInfo;
  static char *params[] = {"-filename","-seed","-min","-max",
                           "-incr","-wrap",NULL};
  enum params {FILENAME, SEED, MIN, MAX, INCR, WRAP};
  int idx;

  /* --------------------------------------------------------------------------
   * check for unknown params
   * ----------------------------------------------------------------------- */
  WebAssertArgs(interp, objc, objv, params, idx, -1);

  /* --------------------------------------------------------------------------
   * minimum requirement is: handle -filename <filename>
   * ----------------------------------------------------------------------- */
  if (objc < 4 ||
      argIndexOfFirstArg(objc,objv,NULL,NULL) != 1 ||
      (fnameobj = argValueOfKey(objc,objv,params[FILENAME])) == NULL) {
    Tcl_WrongNumArgs(interp,1,objv,"handle -filename filename ?options?");
    return TCL_ERROR;
  }

  /* --------------------------------------------------------------------------
   * ok - retrieve params
   * ----------------------------------------------------------------------- */
  hnameobj = objv[1];
  /* fnameobj already done */
  seedobj = argValueOfKey(objc,objv,params[SEED]);
  maxobj = argValueOfKey(objc,objv,params[MAX]);
  minobj = argValueOfKey(objc,objv,params[MIN]);
  incrobj = argValueOfKey(objc,objv,params[INCR]);
  if (argKeyExists(objc,objv,params[WRAP]) == TCL_OK)
    wrap = 1;
  else
    wrap = 0;

  /* --------------------------------------------------------------------------
   * check if handle already exists
   * ----------------------------------------------------------------------- */
  if (Tcl_GetCommandInfo(interp,Tcl_GetString(hnameobj),&cmdInfo) != 0) {
    Tcl_SetResult(interp,"web::filecounter: handle already exists",NULL);
    return TCL_ERROR;
  }
  
  /* --------------------------------------------------------------------------
   * create SeqNoGenerator
   * ----------------------------------------------------------------------- */
  seqnogen = createSeqNoGenerator(hnameobj, fnameobj, seedobj, minobj,
				  maxobj, incrobj, wrap);
  
  if (seqnogen == NULL) {
    Tcl_SetResult(interp,
		  "web::filecounter: invalid or inconsistent arguments",
		  NULL);
    return TCL_ERROR;
  }

  result = Tcl_NewStringObj(seqnogen->handleName,-1);
  Tcl_CreateObjCommand(interp, seqnogen->handleName, 
		       (Tcl_ObjCmdProc *) Web_Filecounter,
		       (ClientData) seqnogen,
		       (Tcl_CmdDeleteProc *) NULL);

  /* --------------------------------------------------------------------------
   * register private data with interp under the name of the handle
   * ----------------------------------------------------------------------- */
  Tcl_SetAssocData(interp,seqnogen->handleName,
		   (Tcl_InterpDeleteProc *)destroySeqNoGenerator,
		   (ClientData)seqnogen);

  Tcl_SetObjResult(interp,result);
  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Member functions of SeqNoGenerator
 * ------------------------------------------------------------------------- */

SeqNoGenerator* createSeqNoGenerator(Tcl_Obj* hn, Tcl_Obj* fn, Tcl_Obj* seed, 
				     Tcl_Obj* min, Tcl_Obj* max, 
				     Tcl_Obj* incr, int wrap) {

  SeqNoGenerator* seqnogen = NULL; 
  int err = 0;

  if (hn == NULL || fn == NULL)
    return NULL;

  seqnogen = (SeqNoGenerator*)Tcl_Alloc(sizeof(SeqNoGenerator));

  seqnogen->fileName = allocAndSet(Tcl_GetString(fn));
  seqnogen->handleName = allocAndSet(Tcl_GetString(hn));
  if (seed == NULL)
    seqnogen->seed = 0;
  else if (Tcl_GetIntFromObj(NULL,seed,&(seqnogen->seed)) == TCL_ERROR)
    err++;
  if (min == NULL)
    seqnogen->minValue = WEB_FILECOUNTER_MINVAL;
  else if (Tcl_GetIntFromObj(NULL,min,&(seqnogen->minValue)) == TCL_ERROR)
    err++;
  if (max == NULL)
    seqnogen->maxValue = WEB_FILECOUNTER_MAXVAL;
  else if (Tcl_GetIntFromObj(NULL,max,&(seqnogen->maxValue)) == TCL_ERROR)
    err++;
  if (incr == NULL)
    seqnogen->incrValue = 1;
  else if (Tcl_GetIntFromObj(NULL,incr,&(seqnogen->incrValue)) == TCL_ERROR)
    err++;

  if (err || 
      seqnogen->minValue > seqnogen->maxValue || 
      seqnogen->seed < seqnogen->minValue ||
      seqnogen->seed > seqnogen->maxValue) {
    deleteSeqNoGenerator(seqnogen);
    return NULL;
  }
  seqnogen->hasCurrent = 0;
  seqnogen->doWrap = wrap;
  return seqnogen;
}

int deleteSeqNoGenerator(SeqNoGenerator* seqnogen){
  if (seqnogen == NULL)
    return TCL_ERROR;
  Tcl_Free(seqnogen->fileName);
  Tcl_Free(seqnogen->handleName);
  Tcl_Free((char*)seqnogen);
  return TCL_OK;
}

int destroySeqNoGenerator(ClientData clientData, Tcl_Interp *interp) {
  return deleteSeqNoGenerator((SeqNoGenerator*)clientData);
}

/* ----------------------------------------------------------------------------
 * nextSeqNo
 * ------------------------------------------------------------------------- */
int nextSeqNo(Tcl_Interp* interp, SeqNoGenerator* seqnogen, int *seqno){

  int         currentSeqNo = -1;
  Tcl_Channel channel;
  Tcl_Obj*    lineObj = NULL;
  int         bytesRead = -1;

  if (seqnogen == NULL)  return TCL_ERROR;

  Tcl_SetResult(interp,"",TCL_STATIC);

  /* --------------------------------------------------------------------------
   * Try to create file
   * ----------------------------------------------------------------------- */
  /* FIXME: 0644 MUST BE PARAM */
  if ((channel = Tcl_OpenFileChannel(interp,
				     seqnogen->fileName,
				     "CREAT RDWR",0644) ) == NULL) {

    LOG_MSG(interp, WRITE_LOG,
	    __FILE__,__LINE__,
	    "web::filecounter",WEBLOG_ERROR,
	    Tcl_GetStringResult(interp),NULL);
      
    return TCL_ERROR;
  }

  /* ------------------------------------------------------------------------
   * Try to lock file
   * --------------------------------------------------------------------- */
  if (lock_TclChannel(interp,channel) == TCL_ERROR) {

    LOG_MSG(interp, WRITE_LOG | SET_RESULT,
	    __FILE__,__LINE__,
	    "web::filecounter",WEBLOG_ERROR,
	    "error getting lock",NULL);
    return TCL_ERROR;
  }

  /* ------------------------------------------------------------------------
   * Try to read file
   * --------------------------------------------------------------------- */
  lineObj = Tcl_NewObj();

  if ( (bytesRead = Tcl_GetsObj(channel,lineObj)) < 0 ) {

    if( !Tcl_Eof(channel) ) {

      /* failed -> unlock and close */
      unlock_TclChannel(interp,channel);
      Tcl_Close(interp,channel);

      LOG_MSG(interp, WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::filecounter",WEBLOG_ERROR,
	      "error reading file: ",Tcl_ErrnoMsg(Tcl_GetErrno()),NULL);
    
      Tcl_DecrRefCount(lineObj);
      return TCL_ERROR;
    } else {
      bytesRead = 0;
    }
  }

  /* ------------------------------------------------------------------------
   * new file
   * --------------------------------------------------------------------- */
  if( bytesRead == 0 ) {
    
    LOG_MSG(interp, WRITE_LOG,
	    __FILE__,__LINE__,
	    "web::filecounter",WEBLOG_INFO,
	    "new file",NULL);

    currentSeqNo = seqnogen->seed;

  } else {

    /* ------------------------------------------------------------------------
     * have read
     * --------------------------------------------------------------------- */
    if (Tcl_GetIntFromObj(interp,lineObj,&currentSeqNo) != TCL_OK) {

      /* ----------------------------------------------------------------------
       * ... but cannot understand what I read
       * ------------------------------------------------------------------- */
      unlock_TclChannel(interp,channel);
      Tcl_Close(interp,channel);
      
      Tcl_DecrRefCount(lineObj);
      
      LOG_MSG(interp, WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::filecounter",WEBLOG_ERROR,
	      "file \"", seqnogen->fileName, 
	      "\" contains invalid data: ", 
	      Tcl_GetStringResult(interp), NULL);
      
      return TCL_ERROR;
    }
    
    /* ------------------------------------------------------------------------
     * get value (and wrap)
     * --------------------------------------------------------------------- */
    currentSeqNo += seqnogen->incrValue;
    
    if (currentSeqNo > seqnogen->maxValue) {
      
      if (seqnogen->doWrap) {
	
	currentSeqNo = seqnogen->minValue;
	
      } else {
	
	unlock_TclChannel(interp,channel);
	Tcl_Close(interp,channel);
	
	Tcl_DecrRefCount(lineObj);
	
	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__,__LINE__,
		"web::filecounter",WEBLOG_ERROR,
		"counter overflow",NULL);
	
	return TCL_ERROR;
      }
    }
  }

  /* ------------------------------------------------------------------------
   * write new value
   * --------------------------------------------------------------------- */
  *seqno = currentSeqNo;

  Tcl_SetIntObj(lineObj,*seqno);

  if (Tcl_Seek(channel,0,SEEK_SET) < 0) {

    LOG_MSG(interp, WRITE_LOG | SET_RESULT,
	    __FILE__,__LINE__,
	    "web::filecounter",WEBLOG_ERROR,
	    "error rewinding channel",NULL);

    unlock_TclChannel(interp,channel);
    Tcl_Close(interp,channel);

    Tcl_DecrRefCount(lineObj);
    
    return TCL_ERROR;
  }

  /* truncate existing file while holding lock ! */
  /*
  {
    Tcl_Channel channel2 = NULL;

    Tcl_Flush(channel);
    channel2 = Tcl_OpenFileChannel(interp,
				   seqnogen->fileName,
				   "CREAT WRONLY TRUNC",0644);
    Tcl_Close(interp,channel2);
    Tcl_Seek(channel,0,SEEK_SET);
  }
  */

  Tcl_AppendToObj(lineObj,"\n",1);

  {

    int written = 0;
    int expected = 0;

    written = Tcl_WriteObj(channel,lineObj);
    expected = Tcl_GetCharLength(lineObj);

    /* printf("DBG written: %d, expected: %d\n",written,expected); fflush(stdout); */

    /*   if ( (written = Tcl_WriteObj(channel,lineObj)) != Tcl_GetCharLength(lineObj))  */

    if( written < expected ) {
	
      /*we try to close the file*/
      unlock_TclChannel(interp,channel);
      Tcl_Close(interp,channel);
      
      LOG_MSG(interp, WRITE_LOG | SET_RESULT,
	      __FILE__,__LINE__,
	      "web::filecounter",WEBLOG_ERROR,
	      "error writing to file \"",
	      seqnogen->fileName,"\": ",Tcl_GetStringResult(interp),
	      NULL);
    
      Tcl_DecrRefCount(lineObj);

      return TCL_ERROR;
    }
  }

  /* ------------------------------------------------------------------------
   * that's it
   * --------------------------------------------------------------------- */
  Tcl_Flush(channel);
  unlock_TclChannel(interp,channel);
  Tcl_Close(interp,channel);

  Tcl_DecrRefCount(lineObj);

  seqnogen->currValue = *seqno;
  seqnogen->hasCurrent = 1;
  return TCL_OK;
}
