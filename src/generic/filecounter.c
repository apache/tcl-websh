/* ----------------------------------------------------------------------------
 * filecounter.c ---
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

#include "filecounter.h"

/* --------------------------------------------------------------------------
 * Init
 * --------------------------------------------------------------------------*/
int filecounter_Init(Tcl_Interp * interp)
{
    RequestData * requestData = NULL;

    if (interp == NULL)
	return TCL_ERROR;

    /* ----------------------------------------------------------------------
     * ClientData
     * --------------------------------------------------------------------*/

    /* we need the request data, so that we have access 
       to default file permissions */

    requestData = (RequestData *) Tcl_GetAssocData(interp, WEB_REQ_ASSOC_DATA, NULL);

    /* ----------------------------------------------------------------------
     * register commands
     * ------------------------------------------------------------------- */
    Tcl_CreateObjCommand(interp, "web::filecounter",
			 (Tcl_ObjCmdProc *) filecounter,
			 (ClientData) requestData, (Tcl_CmdDeleteProc *) NULL);

    /* ----------------------------------------------------------------------
     * register private data with interp
     * ------------------------------------------------------------------- */
    /* no data for module, but for each handle */
    return TCL_OK;
}

/* --------------------------------------------------------------------------
 * This function handles requests given a handle
 * --------------------------------------------------------------------------*/
int Web_Filecounter(ClientData clientData,
		    Tcl_Interp * interp, int objc, Tcl_Obj * objv[])
{

    SeqNoGenerator *seqnogen = (SeqNoGenerator *) clientData;
    static TCLCONST char *subCommands[] = { "curval", "nextval", "getval", "config", NULL };
    enum subCommands
    { CURVAL, NEXTVAL, GETVAL, CONFIG };
    char **ptr = (char **) subCommands;

    int idx;
    int seqno;
    Tcl_Obj *result = NULL;

    /* ----------------------------------------------------------------------
     * deal with an existing filecounter
     * ------------------------------------------------------------------- */

    if (objc != 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "curval|nextval|getval|config");
	return TCL_ERROR;
    }

    if (seqnogen == NULL)
	return TCL_ERROR;

    /* ------------------------------------------------------------------------
     * scan for options
     * --------------------------------------------------------------------- */
    if (Tcl_GetIndexFromObj(interp, objv[1], subCommands, "option", 0, &idx)
	!= TCL_OK) {
	return TCL_ERROR;
    }

    switch ((enum subCommands) idx) {
    case NEXTVAL:{
	    if (nextSeqNo(interp, seqnogen, &seqno, 1) != TCL_OK) {
		/* error reporting done in subfunction */
		return TCL_ERROR;
	    }
	    result = Tcl_NewIntObj(seqno);
	    Tcl_SetObjResult(interp, result);
	    return TCL_OK;
	    break;
	}
    case GETVAL:{
	    if (nextSeqNo(interp, seqnogen, &seqno, 0) != TCL_OK) {
		/* error reporting done in subfunction */
		return TCL_ERROR;
	    }
	    result = Tcl_NewIntObj(seqno);
	    Tcl_SetObjResult(interp, result);
	    return TCL_OK;
	    break;
	}
    case CURVAL:{
	    if (seqnogen->hasCurrent) {
		result = Tcl_NewIntObj(seqnogen->currValue);
		Tcl_SetObjResult(interp, result);
		return TCL_OK;
		break;
	    }
	    else {
		Tcl_SetResult(interp,
			      "web::filecounter: no current value available",
			      TCL_STATIC);
		return TCL_ERROR;
	    }
	}
    case CONFIG:{

	    Tcl_Obj *kv[18];
	    int i;
	    char buf[10];

	    for (i = 0; i < 18; i++)
		kv[i] = Tcl_NewObj();

	    Tcl_SetStringObj(kv[0], "file", -1);
	    Tcl_SetStringObj(kv[1], seqnogen->fileName, -1);
	    Tcl_SetStringObj(kv[2], "handle", -1);
	    Tcl_SetStringObj(kv[3], seqnogen->handleName, -1);
	    Tcl_SetStringObj(kv[4], "seed", -1);
	    Tcl_SetIntObj(kv[5], seqnogen->seed);
	    Tcl_SetStringObj(kv[6], "min", -1);
	    Tcl_SetIntObj(kv[7], seqnogen->minValue);
	    Tcl_SetStringObj(kv[8], "max", -1);
	    Tcl_SetIntObj(kv[9], seqnogen->maxValue);
	    Tcl_SetStringObj(kv[10], "incr", -1);
	    Tcl_SetIntObj(kv[11], seqnogen->incrValue);
	    Tcl_SetStringObj(kv[12], "perms", -1);
	    sprintf(buf, "%04o", seqnogen->mask);
	    Tcl_SetStringObj(kv[13], buf, -1);
	    Tcl_SetStringObj(kv[14], "wrap", -1);
	    if (seqnogen->doWrap)
		Tcl_SetStringObj(kv[15], "true", -1);
	    else
		Tcl_SetStringObj(kv[15], "false", -1);
	    Tcl_SetStringObj(kv[16], "curr", -1);
	    if (seqnogen->hasCurrent)
		Tcl_SetIntObj(kv[17], seqnogen->currValue);
	    else
		Tcl_SetStringObj(kv[17], "not valid", -1);

	    result = Tcl_NewListObj(18, kv);
	    Tcl_SetObjResult(interp, result);
	    return TCL_OK;
	    break;
	}
    }
    Tcl_SetResult(interp, "error during web::filecounter", NULL);
    return TCL_ERROR;
}


/* --------------------------------------------------------------------------
 * Creates a new filecounter
 * --------------------------------------------------------------------------*/
int filecounter(ClientData clientData, Tcl_Interp * interp,
		int objc, Tcl_Obj * CONST objv[])
{

    RequestData * requestData = (RequestData *) clientData;

    Tcl_Obj *hnameobj, *fnameobj, *seedobj, *maxobj,
	*minobj, *incrobj, *maskobj, *wrapobj;
    SeqNoGenerator *seqnogen = NULL;
    Tcl_Obj *result = NULL;
    Tcl_CmdInfo cmdInfo;
    static TCLCONST char *params[] = { "-filename", "-seed", "-min", "-max",
			      "-incr", "-perms", "-wrap", NULL
    };
    enum params
	{ FILENAME, SEED, MIN, MAX, INCR, MASK, WRAP };
    int idx;

    /* ----------------------------------------------------------------------
     * check for unknown params
     * ------------------------------------------------------------------- */
    WebAssertArgs(interp, objc, objv, params, idx, -1);

    /* ----------------------------------------------------------------------
     * minimum requirement is: handle -filename <filename>
     * ------------------------------------------------------------------- */
    if (objc < 4 ||
	argIndexOfFirstArg(objc, objv, NULL, NULL) != 1 ||
	(fnameobj = argValueOfKey(objc, objv, (char *) params[FILENAME])) == NULL) {
	Tcl_WrongNumArgs(interp, 1, objv,
			 "handle -filename filename ?options?");
	return TCL_ERROR;
    }

    /* ----------------------------------------------------------------------
     * ok - retrieve params
     * ------------------------------------------------------------------- */
    hnameobj = objv[1];
    /* fnameobj already done */
    seedobj = argValueOfKey(objc, objv, (char *)params[SEED]);
    maxobj = argValueOfKey(objc, objv, (char *)params[MAX]);
    minobj = argValueOfKey(objc, objv, (char *)params[MIN]);
    incrobj = argValueOfKey(objc, objv, (char *)params[INCR]);
    maskobj = argValueOfKey(objc, objv, (char *)params[MASK]);
    wrapobj = argValueOfKey(objc, objv, (char *)params[WRAP]);

    /* ----------------------------------------------------------------------
     * check if handle already exists
     * ------------------------------------------------------------------- */
    if (Tcl_GetCommandInfo(interp, Tcl_GetString(hnameobj), &cmdInfo) != 0) {
	Tcl_SetResult(interp, "web::filecounter: handle already exists",
		      NULL);
	return TCL_ERROR;
    }

    /* ----------------------------------------------------------------------
     * create SeqNoGenerator
     * ------------------------------------------------------------------- */
    seqnogen = createSeqNoGenerator(requestData,
				    hnameobj, fnameobj, seedobj, minobj,
				    maxobj, incrobj, maskobj, wrapobj);

    if (seqnogen == NULL) {
	Tcl_SetResult(interp,
		      "web::filecounter: invalid or inconsistent arguments",
		      NULL);
	return TCL_ERROR;
    }

    result = Tcl_NewStringObj(seqnogen->handleName, -1);
    Tcl_CreateObjCommand(interp, seqnogen->handleName,
			 (Tcl_ObjCmdProc *) Web_Filecounter,
			 (ClientData) seqnogen, (Tcl_CmdDeleteProc *) NULL);

    /* ----------------------------------------------------------------------
     * register private data with interp under the name of the handle
     * ------------------------------------------------------------------- */
    Tcl_SetAssocData(interp, seqnogen->handleName,
		     (Tcl_InterpDeleteProc *) destroySeqNoGenerator,
		     (ClientData) seqnogen);

    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

/* ------------------------------------------------------------------------
 * Member functions of SeqNoGenerator
 * --------------------------------------------------------------------- */

SeqNoGenerator *createSeqNoGenerator(RequestData * requestData,
				     Tcl_Obj * hn, Tcl_Obj * fn,
				     Tcl_Obj * seed, Tcl_Obj * min,
				     Tcl_Obj * max, Tcl_Obj * incr, 
				     Tcl_Obj * mask, Tcl_Obj * wrap)
{

    SeqNoGenerator *seqnogen = NULL;
    int err = 0;

    if (hn == NULL || fn == NULL)
	return NULL;

    seqnogen = (SeqNoGenerator *) Tcl_Alloc(sizeof(SeqNoGenerator));

    seqnogen->fileName = allocAndSet(Tcl_GetString(fn));
    seqnogen->handleName = allocAndSet(Tcl_GetString(hn));
    if (seed == NULL)
	seqnogen->seed = WEB_FILECOUNTER_SEED;
    else if (Tcl_GetIntFromObj(NULL, seed, &(seqnogen->seed)) == TCL_ERROR)
	err++;
    if (min == NULL)
	seqnogen->minValue = WEB_FILECOUNTER_MINVAL;
    else if (Tcl_GetIntFromObj(NULL, min, &(seqnogen->minValue)) == TCL_ERROR)
	err++;
    if (max == NULL)
	seqnogen->maxValue = WEB_FILECOUNTER_MAXVAL;
    else if (Tcl_GetIntFromObj(NULL, max, &(seqnogen->maxValue)) == TCL_ERROR)
	err++;
    if (incr == NULL)
	seqnogen->incrValue = WEB_FILECOUNTER_INCR;
    else if (Tcl_GetIntFromObj(NULL, incr, &(seqnogen->incrValue)) == TCL_ERROR)
	err++;
    if (mask == NULL)
       seqnogen->mask = requestData->filePermissions;
    else if (Tcl_GetIntFromObj(NULL, mask, &(seqnogen->mask)) == TCL_ERROR)
        err++;
    if (wrap == NULL)
       seqnogen->doWrap = WEB_FILECOUNTER_WRAP;
    else if (Tcl_GetBooleanFromObj(NULL, wrap, &(seqnogen->doWrap)) == TCL_ERROR)
        err++;

    if (err ||
	seqnogen->minValue > seqnogen->maxValue ||
	seqnogen->seed < seqnogen->minValue ||
	seqnogen->seed > seqnogen->maxValue) {
	deleteSeqNoGenerator(seqnogen);
	return NULL;
    }
    seqnogen->hasCurrent = 0;
    return seqnogen;
}

int deleteSeqNoGenerator(SeqNoGenerator * seqnogen)
{
    if (seqnogen == NULL)
	return TCL_ERROR;
    Tcl_Free(seqnogen->fileName);
    Tcl_Free(seqnogen->handleName);
    Tcl_Free((char *) seqnogen);
    return TCL_OK;
}

int destroySeqNoGenerator(ClientData clientData, Tcl_Interp * interp)
{
    return deleteSeqNoGenerator((SeqNoGenerator *) clientData);
}

/* ------------------------------------------------------------------------
 * nextSeqNo
 * --------------------------------------------------------------------- */
int nextSeqNo(Tcl_Interp * interp, SeqNoGenerator * seqnogen, int *seqno, int next)
{
  /* if next == 1: incr filecounter, if next == 0: just get current value of filecounter */ 
    int currentSeqNo = -1;
    Tcl_Channel channel;
    Tcl_Obj *lineObj = NULL;
    int bytesRead = -1;
    int newfile = 0;

    if (seqnogen == NULL)
	return TCL_ERROR;

    Tcl_SetResult(interp, "", TCL_STATIC);

    /* ----------------------------------------------------------------------
     * Try to create file
     * ------------------------------------------------------------------- */
    if ((channel = Tcl_OpenFileChannel(interp,
				       seqnogen->fileName,
				       "CREAT RDWR", seqnogen->mask)) == NULL) {

	LOG_MSG(interp, WRITE_LOG,
		__FILE__, __LINE__,
		"web::filecounter", WEBLOG_ERROR,
		(char *) Tcl_GetStringResult(interp), NULL);

	return TCL_ERROR;
    }

    /* --------------------------------------------------------------------
     * Try to lock file
     * ----------------------------------------------------------------- */
    if (lock_TclChannel(interp, channel) == TCL_ERROR) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::filecounter", WEBLOG_ERROR, "error getting lock", NULL);
	return TCL_ERROR;
    }

    /* --------------------------------------------------------------------
     * Try to read file
     * ----------------------------------------------------------------- */
    lineObj = Tcl_NewObj();

    if ((bytesRead = Tcl_GetsObj(channel, lineObj)) < 0) {

	if (!Tcl_Eof(channel)) {

	    /* failed -> unlock and close */
	    unlock_TclChannel(interp, channel);
	    Tcl_Close(interp, channel);

	    LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		    __FILE__, __LINE__,
		    "web::filecounter", WEBLOG_ERROR,
		    "error reading file: ", Tcl_ErrnoMsg(Tcl_GetErrno()),
		    NULL);

	    Tcl_DecrRefCount(lineObj);
	    return TCL_ERROR;
	}
	else {
	    bytesRead = 0;
	}
    }

    /* --------------------------------------------------------------------
     * new file
     * ----------------------------------------------------------------- */
    if (bytesRead == 0) {

	LOG_MSG(interp, WRITE_LOG,
		__FILE__, __LINE__,
		"web::filecounter", WEBLOG_INFO, "new file", NULL);

	currentSeqNo = seqnogen->seed;
	newfile = 1;
    }
    else {

	/* --------------------------------------------------------------------
	 * have read
	 * ----------------------------------------------------------------- */
	if (Tcl_GetIntFromObj(interp, lineObj, &currentSeqNo) != TCL_OK) {

	    /* ----------------------------------------------------------------
	     * ... but cannot understand what I read
	     * ------------------------------------------------------------- */
	    unlock_TclChannel(interp, channel);
	    Tcl_Close(interp, channel);

	    Tcl_DecrRefCount(lineObj);

	    LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		    __FILE__, __LINE__,
		    "web::filecounter", WEBLOG_ERROR,
		    "file \"", seqnogen->fileName,
		    "\" contains invalid data: ",
		    Tcl_GetStringResult(interp), NULL);

	    return TCL_ERROR;
	}

	/* --------------------------------------------------------------------
	 * get value (and wrap)
	 * ----------------------------------------------------------------- */
	if (next == 1) {
	  currentSeqNo += seqnogen->incrValue;

	  if (currentSeqNo > seqnogen->maxValue) {

	    if (seqnogen->doWrap) {

		currentSeqNo = seqnogen->minValue;

	    }
	    else {

		unlock_TclChannel(interp, channel);
		Tcl_Close(interp, channel);

		Tcl_DecrRefCount(lineObj);

		LOG_MSG(interp, WRITE_LOG | SET_RESULT,
			__FILE__, __LINE__,
			"web::filecounter", WEBLOG_ERROR,
			"counter overflow", NULL);

		return TCL_ERROR;
	    }
	  }
	}
    }

    /* ------------------------------------------------------------------------
     * set result to return
     * --------------------------------------------------------------------- */
    *seqno = currentSeqNo;

    /* ------------------------------------------------------------------------
     * write new value, but only if we need to:
     * either next == 1 (i.e. we have a new value)
     * or newfile == 1 (we have a new file to write)
     * --------------------------------------------------------------------- */
    if (next == 1 || newfile == 1) {
      Tcl_SetIntObj(lineObj, *seqno);

      if (Tcl_Seek(channel, 0, SEEK_SET) < 0) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::filecounter", WEBLOG_ERROR,
		"error rewinding channel", NULL);

	unlock_TclChannel(interp, channel);
	Tcl_Close(interp, channel);

	Tcl_DecrRefCount(lineObj);

	return TCL_ERROR;
      }

      Tcl_AppendToObj(lineObj, "\n", 1);

      {

	int written = 0;
	int expected = 0;

	written = Tcl_WriteObj(channel, lineObj);
	expected = Tcl_GetCharLength(lineObj);

	/* printf("DBG written: %d, expected: %d\n",written,expected); fflush(stdout); */

	/*   if ( (written = Tcl_WriteObj(channel,lineObj)) != Tcl_GetCharLength(lineObj))  */

	if (written < expected) {

	    /*we try to close the file */
	    unlock_TclChannel(interp, channel);
	    Tcl_Close(interp, channel);

	    LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		    __FILE__, __LINE__,
		    "web::filecounter", WEBLOG_ERROR,
		    "error writing to file \"",
		    seqnogen->fileName, "\": ", Tcl_GetStringResult(interp),
		    NULL);

	    Tcl_DecrRefCount(lineObj);

	    return TCL_ERROR;
	}
      }

      /* ------------------------------------------------------------------------
       * that's it
       * --------------------------------------------------------------------- */
      Tcl_Flush(channel);
    }

    unlock_TclChannel(interp, channel);
    Tcl_Close(interp, channel);

    Tcl_DecrRefCount(lineObj);

    seqnogen->currValue = *seqno;
    seqnogen->hasCurrent = 1;
    return TCL_OK;
}
