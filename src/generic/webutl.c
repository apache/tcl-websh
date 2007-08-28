/*
 * webutl.c --- common utils used in more than one module
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
#include <string.h>		/* strlen */
#include "webutl.h"

/* ----------------------------------------------------------------------------
 * allocAndSet --
 *   wrapper to Tcl_Alloc for zero-terminated char * (or 8-bit) strings
 * ------------------------------------------------------------------------- */
char __declspec(dllexport) *allocAndSet(const char *aString)
{

    char *cPtr = NULL;

    if (aString != NULL) {

	cPtr = Tcl_Alloc(strlen(aString) + 1);
	if (cPtr != NULL) {
	    strcpy(cPtr, aString);
	}
    }

    return cPtr;
}

/* ----------------------------------------------------------------------------
 * allocAndSetN --
 *   as allocAndSet, but only N bytes. Result is NULL-terminated.
 * ------------------------------------------------------------------------- */
char *allocAndSetN(const char *aString, int N)
{

    char *cPtr = NULL;

    if ((aString != NULL) && (N > 0)) {

	cPtr = Tcl_Alloc(N + 1);
	if (cPtr != NULL) {
	    strncpy(cPtr, aString, N);
	    cPtr[N] = 0;
	}
    }

    return cPtr;
}

/* ----------------------------------------------------------------------------
 * strchrchr -- strchr for c0 first, or c1 if c0 fails. tag reports which trig.
 * returns earlier match!
 * ------------------------------------------------------------------------- */
char *strchrchr(const char *cs, const char c0, const char c1, char *tag)
{

    char *res = NULL;

    *tag = -1;

    if (cs != NULL) {

      char *res0 = NULL;
      char *res1 = NULL;
    
      res0 = strchr(cs, c0);
      res1 = strchr(cs, c1);

      if (res0 != NULL) {
	if (res1 != NULL) {
	  if (res1 > res0) {
	    res = res0;
	    *tag = 0;
	  } else {
	    res = res1;
	    *tag = 1;
	  }
	} else {
	  res = res0;
	  *tag = 0;
	}
      } else {
	if (res1 != NULL) {
	  res = res1;
	  *tag = 1;
	}
      }	
    }
    return res;
}

/* ----------------------------------------------------------------------------
 * myUtfStrStr -- first try strstr, then try to lowercase and try again
 * ------------------------------------------------------------------------- */
char *myUtfStrStr(const char *s1, const char *s2)
{

    char *test = NULL;
    char *internalS1 = NULL;
    char *internalS2 = NULL;

    test = strstr(s1, s2);

    if (test == NULL) {

	internalS1 = allocAndSet(s1);
	internalS2 = allocAndSet(s2);

	Tcl_UtfToLower(internalS1);
	Tcl_UtfToLower(internalS2);

	test = strstr(internalS1, internalS2);

	if (test != NULL)
	    test = (char *) s1 + (test - internalS1);

	WebFreeIfNotNull(internalS1);
	WebFreeIfNotNull(internalS2);
    }
    return test;
}


/* ----------------------------------------------------------------------------
 * strWithoutLinebreak -- string without "\n" in there
 * ------------------------------------------------------------------------- */
char *strWithoutLinebreak(char *cs)
{

    int i = 0;
    int delta = 0;
    int len = 0;

    if (cs == NULL)
	return NULL;

    len = strlen(cs);

    for (i = 0; (i + delta) < len; i++) {
	if ((cs[i + delta] == '\r') || (cs[i + delta] == '\n')) {
	    delta++;
	}
	cs[i] = cs[i + delta];
    }

    for (; i < len; i++)
	cs[i] = 0;

    return cs;
}

/* ----------------------------------------------------------------------------
 * webEat -- remove specified character from beginning
 * ------------------------------------------------------------------------- */
char *webEat(char eat, char *cs)
{

    int i = 0;
    int len = 0;

    if (cs == NULL)
	return NULL;

    len = strlen(cs);

    for (i = 0; i < len; i++) {
	if (cs[i] != eat)
	    break;
    }

    return &(cs[i]);
}


/* ----------------------------------------------------------------------------
 * handleConfig
 * ------------------------------------------------------------------------- */
int handleConfig(Tcl_Interp * interp, Tcl_Obj ** tclo, Tcl_Obj * newValue,
		 int deleteIfEmpty)
{

    if (*tclo == NULL) {
	*tclo = Tcl_NewObj();
    }
    Tcl_SetObjResult(interp, Tcl_DuplicateObj(*tclo));

    /* --------------------------------------------------------------------------
     * get
     * ----------------------------------------------------------------------- */
    if (newValue == NULL)
	return TCL_OK;

    /* --------------------------------------------------------------------------
     * set
     * ----------------------------------------------------------------------- */
    Tcl_DecrRefCount(*tclo);
    if (deleteIfEmpty && !strcmp(Tcl_GetString(newValue), ""))
	*tclo = NULL;
    else {
	*tclo = Tcl_DuplicateObj(newValue);
	Tcl_IncrRefCount(*tclo);
    }

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * tclGetListLength -- return list length of list, or -1 in case of error
 * ------------------------------------------------------------------------- */
int tclGetListLength(Tcl_Interp * interp, Tcl_Obj * list)
{

    int tmp = -1;

    if (Tcl_ListObjLength(interp, list, &tmp) == TCL_ERROR) {
	LOG_MSG(interp, WRITE_LOG | SET_RESULT, __FILE__, __LINE__,
		WEBSH, WEBLOG_INFO,
		"error accessing length of \"", Tcl_GetString(list),
		"\"", NULL);
	return -1;
    }
    return tmp;
}

/* ----------------------------------------------------------------------------
 * tclSetEnv -- set/get environment variables via Tcl
 * ------------------------------------------------------------------------- */
Tcl_Obj *tclSetEnv(Tcl_Interp * interp, char *key, Tcl_Obj * val)
{

    Tcl_Obj *arrayName = NULL;
    Tcl_Obj *res = NULL;
    Tcl_Obj *tmp = NULL;
    Tcl_Obj *keyObj = NULL;

    if ((interp == NULL) || (key == NULL))
	return NULL;

    arrayName = Tcl_NewStringObj("::env", 5);

    keyObj = Tcl_NewStringObj(key, -1);

    if (val == NULL) {

	/* get */
	tmp = Tcl_ObjGetVar2(interp, arrayName, keyObj, TCL_LEAVE_ERR_MSG);
	if (tmp != NULL)
	    res = Tcl_DuplicateObj(tmp);

    }
    else {

	/* set */
	res =
	    Tcl_ObjSetVar2(interp, arrayName, keyObj, val, TCL_LEAVE_ERR_MSG);
    }

    Tcl_DecrRefCount(arrayName);
    Tcl_DecrRefCount(keyObj);

    return res;
}

/* ----------------------------------------------------------------------------
 * deleteTclObj_fnc -- helper for hashutl (wrapper for Tcl_DecrRefCount)
 * ------------------------------------------------------------------------- */
int deleteTclObj_fnc(void *tclo, void *dum)
{

    if (tclo == NULL)
	return TCL_ERROR;

    Tcl_DecrRefCount((Tcl_Obj *) tclo);
    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * Web_GetOrCreateGlobalVar - get global var from tcl
 * ------------------------------------------------------------------------- */
Tcl_Obj *Web_GetOrCreateGlobalVar(Tcl_Interp * interp, Tcl_Obj * name,
				  int *isNew)
{

    Tcl_Obj *var = NULL;

    if (interp == NULL)
	return NULL;

    if (Tcl_InterpDeleted(interp))
	return NULL;

/*   fprintf(stdout,"DBG Web_GetOrCreateGlobalVar - trying to get global '%s'\n",Tcl_GetString(name)); fflush(stdout); */

    /* --------------------------------------------------------------------------
     * exists ? 
     * ----------------------------------------------------------------------- */
    var = Tcl_ObjGetVar2(interp, name, NULL, TCL_GLOBAL_ONLY);

    /* --------------------------------------------------------------------------
     * ... no, make
     * ----------------------------------------------------------------------- */
    if (var == NULL) {

/*     fprintf(stderr,"DBG Web_GetVarChannel - gonna create global '%s'\n",Tcl_GetString(name)); fflush(stderr); */
	var = Tcl_ObjSetVar2(interp, name, NULL,
			     Tcl_NewObj(),
			     TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG);
	*isNew = 1;
    }

    return var;
}
