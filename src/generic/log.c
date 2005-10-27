/*
 * log.c - logging  module of websh 3
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

#include "log.h"
#include "logtochannel.h"
#include "logtocmd.h"
#include "logtofile.h"
#include "logtosyslog.h"
#include "tcl.h"
#include <stdio.h>


/* ----------------------------------------------------------------------------
 * Init --
 *   entry point for module weblog of websh3
 * ------------------------------------------------------------------------- */
int log_Init(Tcl_Interp * interp)
{

    LogData *logData = NULL;
    LogPlugIn *logtofile = NULL;
    LogPlugIn *logtochannel = NULL;
    LogPlugIn *logtocmd = NULL;
    LogPlugIn *logtosyslog = NULL;
    int ires = 0;

    /* --------------------------------------------------------------------------
     * interpreter running ?
     * ----------------------------------------------------------------------- */
    if (interp == NULL)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * init internal data, and register with interp
     * ----------------------------------------------------------------------- */
    logData = createLogData();
    WebAssertData(interp, logData, "log", TCL_ERROR);

    /* --------------------------------------------------------------------------
     * register data with interp
     * ----------------------------------------------------------------------- */
    Tcl_SetAssocData(interp, WEB_LOG_ASSOC_DATA,
		     destroyLogData, (ClientData) logData);

    /* --------------------------------------------------------------------------
     * register commands
     * ----------------------------------------------------------------------- */
    Tcl_CreateObjCommand(interp, "web::log",
			 Web_Log,
			 (ClientData) logData, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateObjCommand(interp, "web::logfilter",
			 Web_LogFilter,
			 (ClientData) logData, (Tcl_CmdDeleteProc *) NULL);

    Tcl_CreateObjCommand(interp, "web::logdest",
			 Web_LogDest,
			 (ClientData) logData, (Tcl_CmdDeleteProc *) NULL);

    /* --------------------------------------------------------------------------
     * register log handler "channel"
     * ----------------------------------------------------------------------- */
    logtochannel = createLogPlugIn();
    WebAssertData(interp, logtochannel, "log_Init/logtochannel plugin",
		  TCL_ERROR)

	logtochannel->constructor = createLogToChannel;
    logtochannel->destructor = destroyLogToChannel;
    logtochannel->handler = logToChannel;

    ires = registerLogPlugIn(interp, "channel", logtochannel);

    /* --------------------------------------------------------------------------
     * register log handler "file"
     * ----------------------------------------------------------------------- */
    logtofile = createLogPlugIn();
    WebAssertData(interp, logtofile, "log_Init/logtofile plugin", TCL_ERROR)

    logtofile->constructor = createLogToFile;
    logtofile->destructor = destroyLogToFile;
    logtofile->handler = logToFile;

    ires = registerLogPlugIn(interp, "file", logtofile);

    /* --------------------------------------------------------------------------
     * register log handler "command"
     * ----------------------------------------------------------------------- */
    logtocmd = createLogPlugIn();
    WebAssertData(interp, logtocmd, "log_Init/logtocmd plugin", TCL_ERROR)

	logtocmd->constructor = createLogToCmd;
    logtocmd->destructor = destroyLogToCmd;
    logtocmd->handler = logToCmd;

    ires = registerLogPlugIn(interp, "command", logtocmd);

    /* --------------------------------------------------------------------------
     * register log handler "syslog"
     * ----------------------------------------------------------------------- */
#ifndef WIN32
    logtosyslog = createLogPlugIn();
    WebAssertData(interp, logtocmd, "log_Init/logtosyslog plugin", TCL_ERROR)

	logtosyslog->constructor = createLogToSyslog;
    logtosyslog->destructor = destroyLogToSyslog;
    logtosyslog->handler = logToSyslog;

    ires = registerLogPlugIn(interp, "syslog", logtosyslog);
#endif
    /* --------------------------------------------------------------------------
     * done
     * ----------------------------------------------------------------------- */
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * createLogData --
 * ------------------------------------------------------------------------- */
LogData *createLogData()
{

    LogData *logData = NULL;

    logData = WebAllocInternalData(LogData);

    if (logData != NULL) {

	/* initialize list of destination and levels */
	logData->listOfFilters = NULL;
	logData->filterSize = 0;
	logData->listOfDests = NULL;
	logData->destSize = 0;
	insertIntoFilterList(logData, (LogLevel *) NULL);
	insertIntoDestList(logData, (LogDest *) NULL);

	HashUtlAllocInit(logData->listOfPlugIns, TCL_STRING_KEYS);
	logData->logSubst = 0;
    }

    return logData;
}

/* ----------------------------------------------------------------------------
 * destroyLogData --
 * ------------------------------------------------------------------------- */
void destroyLogData(ClientData clientData, Tcl_Interp * interp)
{

    LogData *logData = NULL;

    if (clientData != NULL) {

	logData = (LogData *) clientData;

	/* ..................................................................... */
	if (logData->listOfFilters != NULL) {

	  int i;
	  LogLevel ** logLevels = logData->listOfFilters;
	  for (i = 0; i < logData->filterSize; i++) {
	    if (logLevels[i] != NULL) {
	      destroyLogLevel(logLevels[i], interp);
	    }
	  }
	  WebFreeIfNotNull(logData->listOfFilters);
	  logData->filterSize = 0;
	}

	/* ................................................................ */
	if (logData->listOfDests != NULL) {

	  int i;
	  LogDest ** logDests = logData->listOfDests;
	  for (i = 0; i < logData->destSize; i++) {
	    if (logDests[i] != NULL) {
	      destroyLogDest(logDests[i], interp);
	    }
	  }
	  WebFreeIfNotNull(logData->listOfDests);
	  logData->destSize = 0;
	}


	/* ................................................................ */
	if (logData->listOfPlugIns != NULL) {

	    resetHashTableWithContent(logData->listOfPlugIns, TCL_STRING_KEYS,
				      destroyLogPlugIn, interp);

	    HashUtlDelFree(logData->listOfPlugIns);
	    logData->listOfPlugIns = NULL;
	}

	WebFreeIfNotNull(logData);
    }
}


/* ----------------------------------------------------------------------------
 * createLogLevel -- allocate memory for a new LogLevel, and set content
 *   facility -> like "websh3"
 *   min      -> alert | error | warning | info | debug
 *   max      -> as min
 *   cnt      -> if 0 <= cnt <= 2^15, use cnt for name of filter
 *               otherwise: don't set name (as for web::log)
 * ------------------------------------------------------------------------- */
LogLevel *createLogLevel()
{

    LogLevel *logLevel = NULL;

    logLevel = WebAllocInternalData(LogLevel);
    if (logLevel != NULL) {

	logLevel->facility = NULL;
	logLevel->minSeverity = -1;
	logLevel->maxSeverity = -1;
    }

    return logLevel;
}

/* ----------------------------------------------------------------------------
 * destroyLogLevel -- free memory for LogLevel
 * ------------------------------------------------------------------------- */
int destroyLogLevel(void *level, void *dum)
{

    LogLevel *logLevel = NULL;

    if (level != NULL) {

	logLevel = (LogLevel *) level;

	WebFreeIfNotNull(logLevel->facility);
	WebFreeIfNotNull(logLevel);
    }
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * createLogName --
 * ------------------------------------------------------------------------- */
char *createLogName(char *prefix, int cnt)
{

    char str[64];		/* more than enough space for logDest%d */
    char *name;

    if (prefix == NULL)
	return NULL;
    if (strlen(prefix) > 10)
	return NULL;

    if ((cnt >= 0) && (cnt < (1 << 15))) {
	sprintf(str, "%s%d", prefix, cnt);
	name = allocAndSet(str);
    } else {
	name = NULL;
    }

    return name;
}

/* ----------------------------------------------------------------------------
 * getIndextFromLogName --
 * ------------------------------------------------------------------------- */
int getIndexFromLogName(char *format, char *string) {
  int i = -1;
  int index = -1;
  if (sscanf(string, format, &index) == 1) {
    i = index;
  }
  return i;
}

/* ----------------------------------------------------------------------------
 * insertIntoDestList --
 * ------------------------------------------------------------------------- */
char * insertIntoDestList(LogData *logData, LogDest *logDest) {
  int i;
  LogDest ** logDests = logData->listOfDests;
  for (i = 0; i < logData->destSize; i++) {
    if (logDests[i] == NULL) {
      logDests[i] = logDest;
      return createLogName(LOG_DEST_PREFIX, i);
    }
  }
  {
    /* list too small: increase size */
    LogDest ** newLogDests = (LogDest **) Tcl_Alloc((LOG_LIST_INITIAL_SIZE + logData->destSize) * sizeof(LogDest *));
    if (newLogDests == NULL) {return NULL;}
    /* copy old list to new list */
    memcpy(newLogDests, logDests, logData->destSize * sizeof(LogDest *));
    /* init new entries to NULL */
    for (i = 0; i < LOG_LIST_INITIAL_SIZE; i++) {
      newLogDests[i + logData->destSize] = NULL;
    }
    logData->listOfDests = newLogDests;
    logData->destSize += LOG_LIST_INITIAL_SIZE;
    WebFreeIfNotNull(logDests);
    return insertIntoDestList(logData, logDest);
  }
}

/* ----------------------------------------------------------------------------
 * insertIntoFilterList --
 * ------------------------------------------------------------------------- */
char * insertIntoFilterList(LogData *logData, LogLevel *logLevel) {
  int i;
  LogLevel ** logLevels = logData->listOfFilters;
  for (i = 0; i < logData->filterSize; i++) {
    if (logLevels[i] == NULL) {
      logLevels[i] = logLevel;
      return createLogName(LOG_FILTER_PREFIX, i);
    }
  }
  {
    /* list too small: increase size */
    LogLevel ** newLogLevels = (LogLevel **) Tcl_Alloc((LOG_LIST_INITIAL_SIZE + logData->filterSize) * sizeof(LogLevel *));
    if (newLogLevels == NULL) {return NULL;}
    /* copy old list to new list */
    memcpy(newLogLevels, logLevels, logData->filterSize * sizeof(LogLevel *));
    /* init new entries to NULL */
    for (i = 0; i < LOG_LIST_INITIAL_SIZE; i++) {
      newLogLevels[i + logData->filterSize] = NULL;
    }
    logData->listOfFilters = newLogLevels;
    logData->filterSize += LOG_LIST_INITIAL_SIZE;
    WebFreeIfNotNull(logLevels);
    return insertIntoFilterList(logData, logLevel);
  }
}

/* ----------------------------------------------------------------------------
 * createLogDest --
 * ------------------------------------------------------------------------- */
LogDest *createLogDest(void)
{

    LogDest *logDest = NULL;

    logDest = WebAllocInternalData(LogDest);

    logDest->filter = NULL;
    logDest->format = NULL;
    logDest->maxCharInMsg = -1;	/* Tcl_Append... --> -1 is all chars */
    logDest->plugIn = NULL;
    logDest->plugInData = NULL;

    return logDest;
}

/* ----------------------------------------------------------------------------
 * destroyLogDest --
 * ------------------------------------------------------------------------- */
int destroyLogDest(void *dest, void *env)
{

    Tcl_Interp *interp = NULL;
    LogDest *logDest = NULL;

    if ((dest == NULL) || (env == NULL))
	return TCL_ERROR;

    logDest = (LogDest *) dest;
    interp = (Tcl_Interp *) env;

    /* ------------------------------------------------------------------------
     * get rid of any data
     * --------------------------------------------------------------------- */
    if ((logDest->plugIn != NULL) && (logDest->plugInData != NULL)) {

	/* --------------------------------------------------------------------
	 * call destructor
	 * ----------------------------------------------------------------- */
	logDest->plugIn->destructor(interp, logDest->plugInData);
    }

    /* ------------------------------------------------------------------------
     * unlink plugIn from logDest. Keep plugIn alive, though.
     * --------------------------------------------------------------------- */
    logDest->plugIn = NULL;

    if (logDest->filter != NULL)
	destroyLogLevel(logDest->filter, NULL);

    /* ------------------------------------------------------------------------
     * free rest items in struct, if needed
     * --------------------------------------------------------------------- */
    WebFreeIfNotNull(logDest->format);

    WebFreeIfNotNull(logDest);

    return TCL_OK;
}



/* ----------------------------------------------------------------------------
 * Web_Log -- distribute a log message
 * ------------------------------------------------------------------------- */
int Web_Log(ClientData clientData,
	    Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    LogData *logData = NULL;
    int iTmp = 0;
    int iCur = 0;

    /* ------------------------------------------------------------------------
     * check for internal data
     * --------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_Log", TCL_ERROR)
	logData = (LogData *) clientData;

    /* ------------------------------------------------------------------------
     * try to be fast
     * --------------------------------------------------------------------- */
    if (objc == 3) {
	return logImpl(interp, logData, Tcl_GetString(objv[1]), objv[2]);
    }

    /* ------------------------------------------------------------------------
     * wrong args
     * --------------------------------------------------------------------- */
    Tcl_WrongNumArgs(interp, 1, objv, "level message");
    return TCL_ERROR;
}


/* ----------------------------------------------------------------------------
 * Web_LogDest -- manage list of dests
 * ------------------------------------------------------------------------- */
int Web_LogDest(ClientData clientData,
		Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    LogData *logData = NULL;
    int idx;
    int iCurArg;

    static TCLCONST char *params[] = { "-maxchar",
	"-format",
	NULL
    };
    enum params
    { MAXCHAR, FORMAT };
    static TCLCONST char *subCommands[] = {
      WEB_LOG_SUBCMD_ADD,
      WEB_LOG_SUBCMD_DELETE,
      WEB_LOG_SUBCMD_NAMES,
      WEB_LOG_SUBCMD_LEVELS,
      NULL
    };
    enum subCommands
    { ADD, DELETE, NAMES, LEVELS };

    /* --------------------------------------------------------------------------
     * check for internal data
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_LogDest", TCL_ERROR)
	logData = (LogData *) clientData;

    /* --------------------------------------------------------------------------
     * check arguments
     * ----------------------------------------------------------------------- */
    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "option arg ?arg ...?");
	return TCL_ERROR;
    }

    /* --------------------------------------------------------------------------
     * scan for options
     * ----------------------------------------------------------------------- */
    if (Tcl_GetIndexFromObj(interp, objv[1], subCommands, "option", 0, &idx)
	!= TCL_OK)
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * switch on subcommand
     * ----------------------------------------------------------------------- */

    switch ((enum subCommands) idx) {

    case ADD:{

	    /* ------------------------------------------------------------------------
	     * ok, add sytnax is as follows:
	     * web::logdest add [-format bla -maxchar 100] level {type specific stuff}
	     * 0            1   2                          j     j+1
	     * --------------------------------------------------------------------- */

	    char *name = NULL;
	    char *format = NULL;
	    LogLevel *logLevel = NULL;
	    LogPlugIn *logPlugIn = NULL;
	    ClientData logPlugInData = NULL;
	    LogDest *logDest = NULL;
	    Tcl_Obj *tcloTmp;
	    long maxCharInMsg = -1;
	    int iUnknown = 0;

	    /* argdbg(objc,objv,stdout); */

	    iCurArg =
		argIndexOfFirstArg(objc - 1, &(objv[1]), params, NULL) + 1;

	    /* check for known options */
	    iUnknown =
		argHasOnlyAccepted(objc - 1, &(objv[1]), params, iCurArg - 1);
	    if (iUnknown > 0) {
		/* let Tcl format the error message */
		Tcl_GetIndexFromObj(interp, objv[iUnknown + 1], params,
				    "option", 0, &idx);
		return TCL_ERROR;
	    }

	    /* check for -format */
	    if ((tcloTmp = argValueOfKey(objc, objv, (char *)params[FORMAT])) != NULL) {
		format = allocAndSet(Tcl_GetString(tcloTmp));
	    }
	    else {
		format = allocAndSet(WEB_LOG_DEFAULTFORMAT);
	    }

	    /* check for -maxchar */
	    if ((tcloTmp =
		 argValueOfKey(objc, objv, (char *)params[MAXCHAR])) != NULL) {
		if (Tcl_GetLongFromObj(interp, tcloTmp, &maxCharInMsg) ==
		    TCL_ERROR) {
		    LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__,
			    __LINE__, "web::logdest", WEBLOG_INFO,
			    "cannot read long from -maxchar \"",
			    Tcl_GetString(tcloTmp), "\"", NULL);
		    return TCL_ERROR;
		}
	    }

	    /* ------------------------------------------------------------------------
	     * now iCurArg is level, iCurArg+1 is type, (+2 is type specific)
	     * --------------------------------------------------------------------- */
	    if ((iCurArg + 1) >= objc) {
		Tcl_WrongNumArgs(interp, 1, objv, WEB_LOG_USAGE_LOGDEST_ADD);
		WebFreeIfNotNull(format);
		return TCL_ERROR;
	    }

	    /* ------------------------------------------------------------------------
	     * get handler for type
	     * --------------------------------------------------------------------- */
	    logPlugIn = (LogPlugIn *) getFromHashTable(logData->listOfPlugIns,
						       Tcl_GetString(objv
								     [iCurArg
								      + 1]));
	    if (logPlugIn == NULL) {
		Tcl_SetResult(interp, "no log handler of type \"", NULL);
		Tcl_AppendResult(interp, Tcl_GetString(objv[iCurArg + 1]),
				 "\" registered", NULL);
		WebFreeIfNotNull(format);
		return TCL_ERROR;
	    }

	    /* ------------------------------------------------------------------------
	     * parse level
	     * --------------------------------------------------------------------- */
	    logLevel =
		parseLogLevel(interp, Tcl_GetString(objv[iCurArg]), "user",
			      -1);
	    if (logLevel == NULL) {
		WebFreeIfNotNull(format);
		return TCL_ERROR;
	    }

	    /* ------------------------------------------------------------------------
	     * call constructor
	     * --------------------------------------------------------------------- */
	    if ((logPlugInData =
		 logPlugIn->constructor(interp,
					clientData, objc - (iCurArg + 1),
					&(objv[iCurArg + 1]))) == NULL) {
		destroyLogLevel(logLevel, NULL);
		WebFreeIfNotNull(name);
		WebFreeIfNotNull(format);
		return TCL_ERROR;
	    }

	    /* ------------------------------------------------------------------------
	     * ok, make the logDest
	     * --------------------------------------------------------------------- */
	    logDest = createLogDest();
	    if (logDest == NULL) {
		Tcl_SetResult(interp, "cannot create log destination", NULL);
		destroyLogLevel(logLevel, NULL);
		WebFreeIfNotNull(name);
		WebFreeIfNotNull(format);
		return TCL_ERROR;
	    }
	    logDest->filter = logLevel;
	    logDest->format = format;
	    logDest->plugIn = logPlugIn;
	    logDest->plugInData = logPlugInData;
	    logDest->maxCharInMsg = maxCharInMsg;

	    /* ----------------------------------------------------------
	     * and add to list
	     * ---------------------------------------------------------- */
	    
	    name = insertIntoDestList(logData, logDest);
	    if (name == NULL) {
		Tcl_SetResult(interp, "cannot append new log destination to list", NULL);
		destroyLogDest(logDest, interp);
		destroyLogLevel(logLevel, NULL);
		WebFreeIfNotNull(format);
		return TCL_ERROR;
	    }
	    Tcl_SetResult(interp, name, Tcl_Free);
	    return TCL_OK;
	}
    case NAMES:{

	    Tcl_ResetResult(interp);

	    if (logData->listOfDests != NULL) {
	      int i;
	      LogDest ** logDests = logData->listOfDests;
	      for (i = 0; i < logData->destSize; i++) {
		if (logDests[i] != NULL) {
		  Tcl_AppendElement(interp, createLogName(LOG_DEST_PREFIX, i));
		}
	      }
	    }
	    return TCL_OK;
	}
    case LEVELS:{

	    int namesIsFirst = TCL_OK;
	    LogDest *logDest = NULL;

	    Tcl_SetResult(interp, "", NULL);

	    if (logData->listOfDests != NULL) {

	      int i;
	      LogDest ** logDests = logData->listOfDests;
	      for (i = 0; i < logData->destSize; i++) {
		if (logDests[i] != NULL) {

		    if (namesIsFirst == TCL_ERROR)
			Tcl_AppendResult(interp, "\n", NULL);
		    else
			namesIsFirst = TCL_ERROR;
		    logDest = logDests[i];
		    Tcl_AppendResult(interp,
				     createLogName(LOG_DEST_PREFIX, i), " ",
				     logDest->filter->facility, ".",
				     getSeverityName(logDest->filter->
						     minSeverity), "-",
				     getSeverityName(logDest->filter->
						     maxSeverity), NULL);
		}
	      }
	    }
	    return TCL_OK;
	}
    case DELETE:{

	    /* HashTableIterator iterator; */
	    LogDest *logDest = NULL;

	    /*      0               1      2 */
	    /*      web::loglogDest delete logDest1 */

	    switch (objc) {
	    case 3: {
	      int inx = getIndexFromLogName(LOG_DEST_PREFIX"%d", Tcl_GetString(objv[2]));
	      LogDest ** logDests = logData->listOfDests;
	      if (inx < 0 
		  || inx >= logData->destSize
		  || logDests[inx] == NULL) {
		Tcl_SetResult(interp, "no such log destination \"", NULL);
		Tcl_AppendResult(interp, Tcl_GetString(objv[2]), "\"",
				     NULL);
		return TCL_ERROR;
	      }
	      destroyLogDest(logDests[inx], interp);
	      logDests[inx] = NULL;
	      return TCL_OK;
	      break;
	    }
	    case 2:
		/* --------------------------------------------------------
		 * no argument --> resets the list
		 * -------------------------------------------------------- */
		if (logData->listOfDests != NULL) {
		  int i;
		  LogDest ** logDests = logData->listOfDests;
		  for (i = 0; i < logData->destSize; i++) {
		    if (logDests[i] != NULL) {
		      destroyLogDest(logDests[i], interp);
		      logDests[i] = NULL;
		    }
		  }
		}
		return TCL_OK;
		break;
	    default:
		Tcl_WrongNumArgs(interp, 1, objv, "delete ?destname?");
		return TCL_ERROR;
	    }
	    break;
	}
    default:
	return TCL_OK;
    }
}


/* ----------------------------------------------------------------------------
 * Web_LogFilter -- manage list of filters
 * ------------------------------------------------------------------------- */
int Web_LogFilter(ClientData clientData,
		  Tcl_Interp * interp, int objc, Tcl_Obj * CONST objv[])
{

    LogData *logData;
    int idx;

    static TCLCONST char *subCommands[] = {
	WEB_LOG_SUBCMD_ADD,
	WEB_LOG_SUBCMD_DELETE,
	WEB_LOG_SUBCMD_NAMES,
	WEB_LOG_SUBCMD_LEVELS,
	NULL
    };
    enum subCommands
    { ADD, DELETE, NAMES, LEVELS };

    /* --------------------------------------------------------------------------
     * check for internal data
     * ----------------------------------------------------------------------- */
    WebAssertData(interp, clientData, "Web_LogFilter", TCL_ERROR)
	logData = (LogData *) clientData;

    /* --------------------------------------------------------------------------
     * enough arguments ?
     * ----------------------------------------------------------------------- */
    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "option ?arg?");
	return TCL_ERROR;
    }

    /* ------------------------------------------------------------------------
     * check subcommand
     * --------------------------------------------------------------------- */
    if (Tcl_GetIndexFromObj(interp, objv[1], subCommands, "option", 0, &idx)
	!= TCL_OK) {
	return TCL_ERROR;
    }

    switch ((enum subCommands) idx) {
    case ADD:{

	    /* ------------------------------------------------------------------------
	     * web::logfilter add sytnax is as follows:
	     * web::logfilter add level
	     * 0              1   2
	     * --------------------------------------------------------------------- */
	    LogLevel *logLevel = NULL;
	    char *name = NULL;

	    /* ------------------------------------------------------------------------
	     * enough arguments ?
	     * --------------------------------------------------------------------- */
	    if (objc != 3) {
	        Tcl_WrongNumArgs(interp, 2, objv, "level");
		return TCL_ERROR;
	    }

	    /* ------------------------------------------------------------------------
	     * parse level
	     * --------------------------------------------------------------------- */
	    logLevel =
		parseLogLevel(interp, Tcl_GetString(objv[2]), "user", -1);
	    if (logLevel == NULL) {
		WebFreeIfNotNull(name);
		return TCL_ERROR;
	    }

	    /* ------------------------------------------------------------------------
	     * and add to list
	     * --------------------------------------------------------------------- */
	    name = insertIntoFilterList(logData, logLevel);
	    if (name == NULL) {
		Tcl_SetResult(interp, "cannot append new log filter to list", NULL);
		destroyLogLevel(logLevel, NULL);
		return TCL_ERROR;
	    }
	    Tcl_SetResult(interp, name, Tcl_Free);
	    return TCL_OK;
	    break;
	}
    case NAMES:{

	    Tcl_ResetResult(interp);

	    if (logData->listOfFilters != NULL) {
	      int i;
	      LogLevel **logLevels = logData->listOfFilters;
	      for (i = 0; i < logData->filterSize; i++) {
		if (logLevels[i] != NULL) {
		  Tcl_AppendElement(interp, createLogName(LOG_FILTER_PREFIX, i));
		}
	      }
	    }
	    return TCL_OK;
	    break;
	}

    case LEVELS:{

	    HashTableIterator iterator;
	    LogLevel *logLevel = NULL;
	    int namesIsFirst = TCL_OK;

	    Tcl_SetResult(interp, "", NULL);

	    if (logData->listOfFilters != NULL) {
	      int i;
	      LogLevel ** logLevels = logData->listOfFilters;
	      for (i = 0; i < logData->filterSize; i++) {
		if (logLevels[i] != NULL) {

		    if (namesIsFirst == TCL_ERROR)
			Tcl_AppendResult(interp, "\n", NULL);
		    else
			namesIsFirst = TCL_ERROR;
		    logLevel = logLevels[i];
		    Tcl_AppendResult(interp,
				     createLogName(LOG_FILTER_PREFIX, i), " ",
				     logLevel->facility, ".",
				     getSeverityName(logLevel->minSeverity),
				     "-",
				     getSeverityName(logLevel->maxSeverity),
				     NULL);
		}
	      }
	    }
	    return TCL_OK;
	    break;
	}
    case DELETE:{

	    /* HashTableIterator iterator; */
	    LogLevel *logLevel = NULL;

	    /*      0              1      2 */
	    /*      web::logfilter delete logLevel1 */

	    switch (objc) {
	    case 3: {
	      int inx = getIndexFromLogName(LOG_FILTER_PREFIX"%d", Tcl_GetString(objv[2]));
	      LogLevel ** logLevels = logData->listOfFilters;
	      if (inx < 0 
		  || inx >= logData->filterSize
		  || logLevels[inx] == NULL) {
		Tcl_SetResult(interp, "no such log filter \"", NULL);
		Tcl_AppendResult(interp, Tcl_GetString(objv[2]), "\"",
				     NULL);
		return TCL_ERROR;
	      }
	      destroyLogLevel(logLevels[inx], interp);
	      logLevels[inx] = NULL;
	      return TCL_OK;
	      break;
	    }
	    case 2:
		/* -----------------------------------------------------
		 * no argument --> resets the list
		 * ----------------------------------------------------- */
		if (logData->listOfFilters != NULL) {
		  int i;
		  LogLevel ** logLevels = logData->listOfFilters;
		  for (i = 0; i < logData->filterSize; i++) {
		    if (logLevels[i] != NULL) {
		      destroyLogLevel(logLevels[i], interp);
		      logLevels[i] = NULL;
		    }
		  }
		}
		return TCL_OK;
		break;
	    default:
		Tcl_WrongNumArgs(interp, 1, objv, "delete ?filtername?");
		return TCL_ERROR;
	    }
	    break;
        }
    default:
	return TCL_OK;
    }
}

/* ----------------------------------------------------------------------------
 * createLogPlugIn --
 * ------------------------------------------------------------------------- */
LogPlugIn *createLogPlugIn()
{

    LogPlugIn *logPlugIn = NULL;

    logPlugIn = WebAllocInternalData(LogPlugIn);
    if (logPlugIn != NULL) {
	logPlugIn->constructor = NULL;
	logPlugIn->destructor = NULL;
	logPlugIn->handler = NULL;
    }
    return logPlugIn;
}

/* ----------------------------------------------------------------------------
 * destroyLogPlugIn --
 * ------------------------------------------------------------------------- */
int destroyLogPlugIn(void *plugIn, void *dum)
{

    LogPlugIn *logPlugIn = (LogPlugIn *) plugIn;

    if (logPlugIn != NULL) {
	WebFreeIfNotNull(logPlugIn);
    } else {
	fprintf(stderr, "destroyLogPlugin: plugin doesn't exist.\n");
    }
    return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * registerLogPlugIn --
 * ------------------------------------------------------------------------- */
int registerLogPlugIn(Tcl_Interp * interp, char *type, LogPlugIn * logPlugIn)
{

    LogData *logData;

    if ((interp == NULL) || (logPlugIn == NULL) || (type == NULL))
	return TCL_ERROR;

    /* --------------------------------------------------------------------------
     * try to get the logData data
     * ----------------------------------------------------------------------- */
    logData = (LogData *) Tcl_GetAssocData(interp, WEB_LOG_ASSOC_DATA, NULL);
    WebAssertData(interp, logData, "log", TCL_ERROR)

	/* --------------------------------------------------------------------------
	 * append
	 * ----------------------------------------------------------------------- */
	if (logData->listOfPlugIns == NULL)
	return TCL_ERROR;
    return appendToHashTable(logData->listOfPlugIns, type,
			     (ClientData) logPlugIn);
}

/* ----------------------------------------------------------------------------
 * LOG_MSG -- write log message to webshell log facility
 * <interp>   interp (needed to access log module data)
 * <flag>     use WRITE_LOG , SET_RESULT, INTERP_ERRORINFO
 * <filename> of caller "test.c"
 * <linenr>   of caller   "123"
 * <cmd>      caller         "testCommand"
 * <level>    webshell log level
 * <msg>      message parts (NULL-terminated list)
 * ------------------------------------------------------------------------- */
void LOG_MSG(Tcl_Interp * interp, int flag, char *filename, int linenr,
	     char *cmd, char *level, char *msg, ...)
{

    Tcl_Obj *errobj = NULL;
    Tcl_Obj *resobj = NULL;
    char *msgpart = NULL;

    va_list ap;

    errobj = Tcl_NewObj();
    Tcl_IncrRefCount(errobj);

    resobj = Tcl_NewObj();
    Tcl_IncrRefCount(resobj);

    if (interp == NULL)
	Tcl_AppendToObj(errobj, "interp = null", -1);

#ifdef DEBUG
    Tcl_AppendStringsToObj(errobj, "File:", filename,
			   ", Line:", Tcl_GetString(Tcl_NewIntObj(linenr)),
			   ", ", (char *) NULL);
#endif

    Tcl_AppendStringsToObj(errobj, cmd, ": ", msg, (char *) NULL);

    if (flag & SET_RESULT)
	Tcl_AppendStringsToObj(resobj, msg, NULL);

    for (va_start(ap, msg); (msgpart = va_arg(ap, char *)) != NULL;) {
	Tcl_AppendStringsToObj(errobj, msgpart, NULL);
	if (flag & SET_RESULT)
	    Tcl_AppendStringsToObj(resobj, msgpart, NULL);
    }

    va_end(ap);

    if (interp != NULL && (flag & WRITE_LOG)) {
	webLog(interp, level, Tcl_GetString(errobj));
    }

#ifdef WRITE_TO_STDOUT
    printf("%s\n", Tcl_GetString(errobj));
    fflush(stdout);
#endif

    if ((flag & INTERP_ERRORINFO) && (interp != NULL)) {
	char *errorInfo = (char *) Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY);
	if (errorInfo != NULL)
	    webLog(interp, WEBLOG_DEBUG, errorInfo);
	else
	    webLog(interp, WEBLOG_DEBUG, "panic: errorInfo not set");
    }

    if (flag & SET_RESULT)
	Tcl_SetObjResult(interp, resobj);

    Tcl_DecrRefCount(errobj);
    Tcl_DecrRefCount(resobj);	/* ok, ref count was incremented before */
}
