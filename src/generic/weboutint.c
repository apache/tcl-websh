/*
 * weboutint.c --- output handler of websh3
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
#include <stdio.h>
#include "webout.h"		/* is member of output module of websh */
#include "args.h"		/* arg processing */
#include "webutl.h"
#include "hashutl.h"
#include "paramlist.h"		/* destroyParamList */
#include "varchannel.h"

/* ----------------------------------------------------------------------------
 * getChannel
 * ------------------------------------------------------------------------- */
Tcl_Channel getChannel(Tcl_Interp * interp, ResponseObj * responseObj)
{

    char *varName = NULL;
    char *channelName = NULL;
    int mode = 0;
    Tcl_Channel channel = NULL;

    if ((interp == NULL) || (responseObj == NULL))
	return NULL;

    channel = Web_GetChannelOrVarChannel(interp,
					 Tcl_GetString(responseObj->name),
					 &mode);

    if (channel == NULL) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"response", WEBLOG_ERROR,
		"error getting channel \"", Tcl_GetString(responseObj->name),
		"\"", NULL);
	return NULL;
    }

    if (!(mode & TCL_WRITABLE)) {

	LOG_MSG(interp, WRITE_LOG, __FILE__, __LINE__,
		"response", WEBLOG_ERROR,
		"channel \"", Tcl_GetString(responseObj->name),
		"\" not open for writing", NULL);
	return NULL;
    }

    return channel;
}

/* ----------------------------------------------------------------------------
 * getResponseObj
 * ------------------------------------------------------------------------- */
ResponseObj *getResponseObj(Tcl_Interp * interp, OutData * outData,
			    char *name)
{

    ResponseObj *responseObj = NULL;

    if ((interp == NULL) || (outData == NULL))
	return NULL;

    /* --------------------------------------------------------------------------
     * default ?
     * ----------------------------------------------------------------------- */
    if (name == NULL) {

	responseObj = outData->defaultResponseObj;

    } else {

	/* ------------------------------------------------------------------------
	 * do we have it ?
	 * --------------------------------------------------------------------- */
	responseObj =
	    (ResponseObj *) getFromHashTable(outData->responseObjHash, name);
	/* ------------------------------------------------------------------------
	 * no such object. implicitely build one
	 * --------------------------------------------------------------------- */
	if (responseObj == NULL) {

	    int err = 0;
	    responseObj =
		createResponseObj(interp, name, &objectHeaderHandler);

	    if (responseObj == NULL)
		err++;
	    else if (appendToHashTable(outData->responseObjHash,
				       Tcl_GetString(responseObj->name),
				       (ClientData) responseObj) != TCL_OK)
		err++;

	    if (err) {

		LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
			"response", WEBLOG_ERROR,
			"error creating response object", NULL);
		return NULL;
	    }
	}
    }

    if (responseObj == NULL) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"web::putx", WEBLOG_ERROR,
		"error accessing response object", NULL);
	return NULL;
    }

    return responseObj;
}


/* ----------------------------------------------------------------------------
 * createResponseObj
 * ------------------------------------------------------------------------- */
ResponseObj *createResponseObj(Tcl_Interp * interp, char *channelName,
			       ResponseHeaderHandler * headerHandler)
{

    Tcl_HashTable *hash = NULL;
    ResponseObj *responseObj = NULL;
    int err = 0;
    int mode = 0;
    char *name = NULL;
    Tcl_Obj *tmp = NULL;
    Tcl_Obj *tmp2 = NULL;
    char *defheaders[] = { HEADER, NULL };
    int i;

    if (channelName == NULL)
	return NULL;
    name = channelName;

/*   fprintf(stderr,"creating '%s'\n",channelName); fflush(stderr); */


    /* --------------------------------------------------------------------------
     * create response object
     * ----------------------------------------------------------------------- */
    responseObj = WebAllocInternalData(ResponseObj);

    if (responseObj == NULL) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		"createResponseObj", WEBLOG_ERROR,
		"error creating internal data", NULL);
	return NULL;
    }

    /* --------------------------------------------------------------------------
     * headers
     * ----------------------------------------------------------------------- */
    HashUtlAllocInit(hash, TCL_STRING_KEYS);

    if (hash == NULL)
	return NULL;

    i = 0;

    while (defheaders[i]) {
	char *key = NULL;
	Tcl_Obj *val = NULL;

	key = defheaders[i++];
	val = Tcl_NewStringObj(defheaders[i++], -1);

	paramListAdd(hash, key, val);
    }

    responseObj->sendHeader = 1;
    responseObj->bytesSent = 0;
    responseObj->headers = hash;
    responseObj->name = Tcl_NewStringObj(name, -1);
    responseObj->httpresponse = NULL;
    responseObj->headerHandler = headerHandler;

    Tcl_IncrRefCount(responseObj->name);	/* it's mine */

    return responseObj;
}

/* ----------------------------------------------------------------------------
 * destroyResponseObj
 * ------------------------------------------------------------------------- */
int destroyResponseObj(ClientData clientData, Tcl_Interp * interp)
{

    ResponseObj *responseObj = NULL;

    if (clientData == NULL)
	return TCL_ERROR;

    responseObj = (ResponseObj *) clientData;

    /* unregister if was a varchannel */
/*   printf("DBG destroyResponseObj '%s'\n",Tcl_GetString(responseObj->name)); fflush(stdout); */
    Web_UnregisterVarChannel(interp, Tcl_GetString(responseObj->name), NULL);

    WebDecrRefCountIfNotNull(responseObj->name);
    WebDecrRefCountIfNotNull(responseObj->httpresponse);

    if (responseObj->headers != NULL) {
	destroyParamList(responseObj->headers);
	responseObj->headers = NULL;
    }

    WebFreeIfNotNull(responseObj);

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * createResponseObjHash
 * ------------------------------------------------------------------------- */
int createResponseObjHash(OutData * outData)
{

    if (outData == NULL)
	return TCL_ERROR;
    if (outData->defaultResponseObj == NULL)
	return TCL_ERROR;

    HashUtlAllocInit(outData->responseObjHash, TCL_STRING_KEYS);

    if (outData->responseObjHash != NULL) {
	if (appendToHashTable(outData->responseObjHash,
			      Tcl_GetString(outData->defaultResponseObj->
					    name),
			      (ClientData) outData->defaultResponseObj)
	    != TCL_OK) {
	    HashUtlDelFree(outData->responseObjHash);
	    outData->responseObjHash = NULL;
	    return TCL_ERROR;
	}
    }

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * destroyResponseObjHash
 * ------------------------------------------------------------------------- */
int destroyResponseObjHash(OutData * outData, Tcl_Interp * interp)
{

    HashTableIterator iterator;
    ResponseObj *responseObj = NULL;

    if (outData == NULL)
	return TCL_ERROR;
    if (outData->responseObjHash == NULL)
	return TCL_ERROR;

    /* delete all response Obj */
    assignIteratorToHashTable(outData->responseObjHash, &iterator);

    while (nextFromHashIterator(&iterator) != TCL_ERROR) {

	responseObj = (ResponseObj *) valueOfCurrent(&iterator);
	if (responseObj != NULL)
	    destroyResponseObj((ClientData) responseObj, interp);
    }

    HashUtlDelFree(outData->responseObjHash);

    outData->responseObjHash = NULL;

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * createOutData
 * ------------------------------------------------------------------------- */
OutData *createOutData(Tcl_Interp * interp)
{

    OutData *outData = NULL;

    outData = WebAllocInternalData(OutData);

    if (outData != NULL) {

	outData->defaultResponseObj = createDefaultResponseObj(interp);
	if (outData->defaultResponseObj != NULL) {

	    if (createResponseObjHash(outData) != TCL_OK) {

		destroyResponseObj((ClientData) outData->defaultResponseObj,
				   interp);
		WebFreeIfNotNull(outData);
		return NULL;
	    }
	}
	else {

	    WebFreeIfNotNull(outData);
	    return NULL;
	}

	outData->putxMarkup = 0;
    }

    return outData;
}

/* ----------------------------------------------------------------------------
 * reset
 * ------------------------------------------------------------------------- */
int resetOutData(Tcl_Interp * interp, OutData * outData)
{

    if ((interp == NULL) || (outData == NULL))
	return TCL_ERROR;

    outData->putxMarkup = 0;

    if (destroyResponseObjHash(outData, interp) == TCL_ERROR)
	return TCL_ERROR;
    outData->responseObjHash = NULL;

    /* create standard channel */
    outData->defaultResponseObj = createDefaultResponseObj(interp);
    if (outData->defaultResponseObj == NULL)
	return TCL_ERROR;

    /* create Hash (and add default channel) */
    if (createResponseObjHash(outData) != TCL_OK)
	return TCL_ERROR;

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * destroy internal data structure
 * ------------------------------------------------------------------------- */
int destroyOutData(ClientData clientData, Tcl_Interp * interp)
{

    OutData *outData = NULL;
    /* HashTableIterator iterator; */
    ResponseObj *responseObj = NULL;

    if (clientData == NULL)
	return TCL_ERROR;

    outData = (OutData *) clientData;

    /* delete all response Obj */
    destroyResponseObjHash(outData, interp);

    WebFreeIfNotNull(outData);

    return TCL_OK;
}

/* --------------------------------------------------------------------------
 * quote_append (quote Tcl syntax characters and append to Tcl_DString)
 * ----------------------------------------------------------------------- */

int quote_append(Tcl_DString *str, char *in, int len)
{
    int i = 0;
    while (i < len) {
	switch (*in)
	{
	case '{':
	    Tcl_DStringAppend(str, "\\{", -1);
	    break;
	case '}':
	    Tcl_DStringAppend(str, "\\}", -1);
	    break;
	case '$':
	    Tcl_DStringAppend(str, "\\$", -1);
	    break;
	case '[':
	    Tcl_DStringAppend(str, "\\[", -1);
	    break;
	case ']':
	    Tcl_DStringAppend(str, "\\]", -1);
	    break;
	case '"':
	    Tcl_DStringAppend(str, "\\\"", -1);
	    break;
/* 	    case '\\':
	    Tcl_DStringAppend(str, "\\\\", -1);
	    break;  */
	default:
	    Tcl_DStringAppend(str, in, 1);
	    break;
	}
	in ++;
	i ++;
    }
    return 0;
}


/* ----------------------------------------------------------------------------
 * webout_eval_tag (code in <? ?>)
 * ------------------------------------------------------------------------- */
int webout_eval_tag(Tcl_Interp * interp, ResponseObj * responseObj,
		    Tcl_Obj * in, const char *strstart, const char *strend)
{
    Tcl_DString dstr;
    Tcl_DString convdstr;
    Tcl_Obj *tclo = NULL;

    int inLen;
    char *cur = NULL;
    char *prev = NULL;
    int cntOpen = 0;
    int res = 0;
    int startmatch = 0;
    int endmatch = 0;

    int begin = 1;
    char *start;

/*     const char *strstart = START_TAG;
    const char *strend = END_TAG;  */
/*     int endseqlen = strlen(END_TAG);
    int startseqlen = strlen(START_TAG);
  */
    int endseqlen = strlen(strstart);
    int startseqlen = strlen(strend);

    if ((responseObj == NULL) || (in == NULL))
	return TCL_ERROR;

    Tcl_DStringInit(&dstr);

    cur = Tcl_GetStringFromObj(in, &inLen);
    prev = cur;
    start = cur;

    if (inLen == 0)
	return TCL_OK;

    while (*cur != 0) {
	if (*cur == strstart[startmatch])
	{
	    if (*prev == '\\') {
		Tcl_DStringAppend(&dstr, cur, 1);
	    } else if ((++startmatch) == startseqlen) {
		/* We have matched the starting sequence. */
		if (cntOpen < 1) {
		    if (!((cur - (startseqlen - 1)) - start)) {
			begin = 0;
		    } else {
			Tcl_DStringAppend(&dstr, "\"\n", 2);
		    }
		} else {
		    Tcl_DStringAppend(&dstr, strstart, -1);
		}
		cntOpen ++;
		startmatch = 0;
	    }
	    prev = cur;
	    cur ++;
	    continue;
	} else if (*cur == strend[endmatch]) {
	    if (*prev == '\\') {
		Tcl_DStringAppend(&dstr, cur, 1);
	    } else if ((++endmatch) == endseqlen)
	    {
		/* We have matched the ending sequence. */
		if (cntOpen == 1) {
		    /* build up the command with the name of the channel. */
		    Tcl_DStringAppend(&dstr, "\n web::put \"", -1);
		} else {
		    Tcl_DStringAppend(&dstr, strend, -1);
		}
		cntOpen --;
		endmatch = 0;
	    }
	    prev = cur;
	    cur ++;
	    continue;
	} else if (startmatch) {
	    if (cntOpen < 1) {
		quote_append(&dstr, (char *)strstart, startmatch);
	    } else {
		Tcl_DStringAppend(&dstr, (char *)strstart, startmatch);
	    }
	    startmatch = 0;
	} else if (endmatch) {
	    if (cntOpen < 1) {
		quote_append(&dstr, (char *)strend, endmatch);
	    } else {
		Tcl_DStringAppend(&dstr, (char *)strend, endmatch);
	    }
	    endmatch = 0;
	}
	/* Put the current character in the output.  If we are in Tcl
	       code, then don't escape Tcl characters. */
	if (cntOpen < 1) {
	    quote_append(&dstr, cur, 1);
	} else {
	    Tcl_DStringAppend(&dstr, cur, 1);
	}
	prev = cur;
	cur ++;
    }

    Tcl_ExternalToUtfDString(NULL,
			     Tcl_DStringValue(&dstr),
			     Tcl_DStringLength(&dstr),
			     &convdstr);

    /* build up the web::put with the name of the channel. */
    if (begin) {
	tclo = Tcl_NewStringObj("web::put \"", -1);
    } else {
	tclo = Tcl_NewStringObj("", -1);
    }

    Tcl_AppendToObj(tclo, Tcl_DStringValue(&convdstr),
		    Tcl_DStringLength(&convdstr));

    if (cntOpen < 1) {
	Tcl_AppendToObj(tclo, "\"\n", 2);
    }

    Tcl_DStringFree(&dstr); 
    Tcl_DStringFree(&convdstr); 
    res = Tcl_EvalObjEx(interp, tclo, TCL_EVAL_DIRECT);
    return res;
}

/* ----------------------------------------------------------------------------
 * putsCmdImpl -- do the work here
 * ------------------------------------------------------------------------- */
int putsCmdImpl(Tcl_Interp * interp, ResponseObj * responseObj, Tcl_Obj * str)
{

    Tcl_Obj *sendString = NULL;
    long bytesSent = 0;
    Tcl_Channel channel;

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if ((responseObj == NULL) || (str == NULL))
	return TCL_ERROR;

/*   printf("DBG putsCmdImpl - got '%s'\n",Tcl_GetString(str)); fflush(stdout); */

    channel = getChannel(interp, responseObj);
    if (channel == NULL)
	return TCL_ERROR;

    sendString = Tcl_NewObj();

    if (responseObj->sendHeader) {
	responseObj->headerHandler(interp, responseObj, sendString);
    }

    Tcl_AppendObjToObj(sendString, str);

    if ((bytesSent = Tcl_WriteObj(channel, sendString)) == -1) {

	LOG_MSG(interp, WRITE_LOG | SET_RESULT,
		__FILE__, __LINE__,
		"web::put", WEBLOG_ERROR,
		"error writing to response object:",
		Tcl_GetStringResult(interp), NULL);
	Tcl_DecrRefCount(sendString);
	return TCL_ERROR;
    }

    responseObj->bytesSent += bytesSent;

    /* flush varchannel */
    if (responseObj->name != NULL) {
	char *channelName = Tcl_GetString(responseObj->name);
	if (channelName != NULL)
	    if (channelName[0] == '#')
		Tcl_Flush(channel);
    }

    Tcl_DecrRefCount(sendString);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * objectHeaderHandler -- send headers into a Tcl_Obj, used for variables and channels
 * ------------------------------------------------------------------------- */
int objectHeaderHandler(Tcl_Interp * interp, ResponseObj * responseObj,
			Tcl_Obj * out)
{

    /* --------------------------------------------------------------------------
     * sanity
     * ----------------------------------------------------------------------- */
    if ((out == NULL) || (responseObj == NULL))
	return TCL_ERROR;

    if (responseObj->sendHeader == 1) {

	HashTableIterator iterator;
	char *key;
	Tcl_Obj *headerList;

	if (responseObj->httpresponse != NULL) {
	    Tcl_AppendObjToObj(out, responseObj->httpresponse);
	    Tcl_AppendToObj(out, "\r\n", 2);
	}

	assignIteratorToHashTable(responseObj->headers, &iterator);

	while (nextFromHashIterator(&iterator) != TCL_ERROR) {

	    key = keyOfCurrent(&iterator);

	    if (key != NULL) {

		headerList = (Tcl_Obj *) valueOfCurrent(&iterator);
		if (headerList != NULL) {

		    int lobjc = -1;
		    Tcl_Obj **lobjv = NULL;
		    int i;
		    if (Tcl_ListObjGetElements(interp, headerList,
					       &lobjc, &lobjv) == TCL_ERROR) {
			LOG_MSG(interp, WRITE_LOG,
				__FILE__, __LINE__,
				"web::put", WEBLOG_ERROR,
				Tcl_GetStringResult(interp), NULL);
			return TCL_ERROR;
		    }
		    /* add all occurrences of this header */
		    for (i = 0; i < lobjc; i++) {

			Tcl_AppendToObj(out, key, -1);
			Tcl_AppendToObj(out, ": ", 2);
			Tcl_AppendObjToObj(out, lobjv[i]);
			Tcl_AppendToObj(out, "\r\n", 2);
		    }
		}
	    }
	}
	Tcl_AppendToObj(out, "\r\n", 2);
	responseObj->sendHeader = 0;
    }
    return TCL_OK;
}
