/* ----------------------------------------------------------------------------
 * args.c --- Argument parsing utilities
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
#include "args.h"
#include <ctype.h>
#include <stdio.h>		/* argdbg */
#include <string.h>		/* strcmp */


/* -- doc ---------------------------------------------------------------------
 * argIndexOfKey -- given an array of arguments, return index of arg matching key
 * 1: array of pointers to Tcl Objects serving as arguments
 * 2: number of elements in \1
 * 3: key to search for
 * R: 0..objc-1 or -1 in case of error
 * ------------------------------------------------------------------------- */
int argIndexOfKey(int objc, Tcl_Obj * CONST objv[], char *key)
{

    int pos = 1;

    if ((objv != NULL) && (key != NULL)) {

	while (pos < objc) {

	    if (objv[pos] != NULL) {
		switch (argOptionType(objv[pos])) {
		case OPTION_TYPE_DASHDASH:
		    return -1;
		    break;
		case OPTION_TYPE_PARAM:
		    if (strcmp(Tcl_GetString(objv[pos]), key) == 0)
			return pos;
		    break;
		default:
		    break;
		}
	    }
	    pos++;
	}
    }
    return -1;
}

/* -- doc ---------------------------------------------------------------------
 * optionType -- determine option type (NONE,PARAM,DASHDASH,NUMBER)
 * ------------------------------------------------------------------------- */
int argOptionType(Tcl_Obj * in)
{

    char *tmp = NULL;
    int tmpLen = -1;

    if (in == NULL)
	return 0;

    tmp = Tcl_GetStringFromObj(in, &tmpLen);

    if (tmp[0] == '-') {

	if (tmpLen > 1) {
	    if (tmp[1] != '-') {
		if (isdigit(tmp[1]))
		    return OPTION_TYPE_NUMBER;
		return OPTION_TYPE_PARAM;
	    }
	    else {
		if ((tmp[1] == '-') && (tmpLen == 2))
		    return OPTION_TYPE_DASHDASH;
	    }
	}
    }
    return OPTION_TYPE_NONE;
}


/* -- doc ---------------------------------------------------------------------
 * argIndexOfNextKey -- search for next argument with "-"
 * returns index to next Key in objv, or objc
 * ------------------------------------------------------------------------- */
int argIndexOfNextKey(int objc, Tcl_Obj * CONST objv[], int previous)
{

    int pos = 0;
    char *tmp = NULL;

    for (pos = (previous + 1); pos < objc; pos++) {

	if (objv[pos] != NULL) {

	    switch (argOptionType(objv[pos])) {
	    case OPTION_TYPE_PARAM:
		return pos;
		break;
	    case OPTION_TYPE_DASHDASH:
		return objc;
		break;
	    default:
		break;
	    }
	}
    }
    return objc;
}

/* -- doc ---------------------------------------------------------------------
 * argPosParam -- scan params to find key
 * returns index to **params, if key is found, or -1
 * ------------------------------------------------------------------------- */
int argPosParam(char **params, char *key)
{

    char **intParams;
    int pos = 0;

    intParams = params;

    if ((key == NULL) || (params == NULL))
	return -1;

    while (*intParams != NULL) {

	if (strcmp(*intParams, key) == 0)
	    return pos;

	pos++;
	intParams++;
    }

    return -1;
}


/* -- doc ---------------------------------------------------------------------
 * indexOfFirstOpt -- 
 * ------------------------------------------------------------------------- */
int argIndexOfFirstOpt(int objc, Tcl_Obj * CONST objv[])
{

    int pos = 1;

    if (objc < 2)
	return -1;
    if (objv == NULL)
	return -1;

    while (pos < objc) {

	if (objv[pos] != NULL) {

	    switch (argOptionType(objv[pos])) {
	    case OPTION_TYPE_PARAM:
		return pos;
		break;
	    case OPTION_TYPE_DASHDASH:
		/* found "--" before any other option
		 * --> first arg is i + 1, and no options to be expected
		 */
		return -1;
		break;
	    default:
		/* continue search */
		break;
	    }
	}
	pos++;
    }
    return -1;
}

/* -- doc ---------------------------------------------------------------------
 * indexOfFirstArg -- for array of args, return index of first arg,
 *   skipping params
 * 1: number of elements in \2
 * 2: array of pointers to Tcl Objects serving as arguments
 * R: 1..objc-1 or objc if only switches
 * note: assumes that first arg is command name
 * ------------------------------------------------------------------------- */
int argIndexOfFirstArg(int objc, Tcl_Obj * CONST objv[],
		       char **params, int *Nparams)
{

    int pos = -1;
    int first = -1;
    int pidx = 0;

    if (objc < 2)
	return objc;
    if (objv == NULL)
	return objc;

    if (argOptionType(objv[1]) == OPTION_TYPE_NONE)
	return 1;

    first = argIndexOfFirstOpt(objc, objv);

    /* --------------------------------------------------------------------------
     * no switches
     * ----------------------------------------------------------------------- */
    if (first == -1)
	first = 1;

    /* --------------------------------------------------------------------------
     * scan to the last param
     * ----------------------------------------------------------------------- */
    pos = first;
    while (pos < objc) {

	if (objv[pos] != NULL) {

	    switch (argOptionType(objv[pos])) {
	    case OPTION_TYPE_NONE:
		/* we found the fisrt argument */
		return pos;
	    case OPTION_TYPE_DASHDASH:
		/* first arg is i + 1, and no more options to be expected */
		pos++;
		return pos;
		break;
	    case OPTION_TYPE_PARAM:
		/* switch:       i ==> -unbuffered     --> first arg is i + 1 + 0
		 * simple param: i ==> -format "$m"    --> first arg is i + 1 + 1
		 * param:        i ==> -postdata a b c --> first arg is i + 1 + 3
		 */
		if ((pidx =
		     argPosParam(params, Tcl_GetString(objv[pos]))) != -1) {
		    if (Nparams != NULL)
			pos += Nparams[pidx];
		    else
			pos++;
		}
		pos++;
		break;
	    default:
		pos++;
		break;
	    }
	}
    }

    /* ----------------------------------------------------------------------
     * no args (switches only)
     * ------------------------------------------------------------------- */
    return objc;
}

/* ----------------------------------------------------------------------------
 * argKeyExists -- check if one element of array of arguments matches key
 * 1: array of pointers to Tcl Objects serving as arguments
 * 2: number of elements in \1
 * 3: key to search for
 * R: TCL_OK if found, TCL_ERROR else
 * ------------------------------------------------------------------------- */
int argKeyExists(int objc, Tcl_Obj * CONST objv[], char *key)
{

    if (argIndexOfKey(objc, objv, key) == -1) {

	return TCL_ERROR;
    }

    return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * argValueOfKey -- returns next arg after key as value
 * 2: array of pointers to Tcl Objects serving as arguments
 * 1: number of elements in \2
 * 3: key to search for
 * R: Tcl_Obj* if found, else NULL
 * ------------------------------------------------------------------------- */
Tcl_Obj *argValueOfKey(int objc, Tcl_Obj * CONST objv[], char *key)
{

    int pos = 0;

    if ((objv != NULL) && (key != NULL)) {

	pos = argIndexOfKey(objc, objv, key);

	if ((pos != -1) && (pos < (objc - 1))) {

	    return objv[pos + 1];
	}
    }
    return NULL;
}

/* ----------------------------------------------------------------------------
 * argHasOnlyAccepted -- checks if all options are known
 * (from 1 to min(objc,scanc)). If scanc == -1, scan up to objc.
 * returns 0 if ok, index of argument which is unknown, if found
 * ------------------------------------------------------------------------- */
int argHasOnlyAccepted(int objc, Tcl_Obj * CONST objv[], char *params[],
		       int scanc)
{

    int i;
    char *tmp = NULL;
    int tmpLen = -1;

    if (scanc < 0)
	scanc = objc;
    if (scanc > objc)
	scanc = objc;

    for (i = 1; i < scanc; i++) {

	if (objv[i] != NULL) {

	    /* fprintf(stdout,"DBG argHasOnlyAccepted: '%s'\n",Tcl_GetString(objv[i])); fflush(stdout); */

	    switch (argOptionType(objv[i])) {
	    case OPTION_TYPE_PARAM:
		tmp = Tcl_GetString(objv[i]);
		if (argPosParam(params, tmp) == -1)
		    return i;
		break;
	    case OPTION_TYPE_DASHDASH:
		return 0;
		break;
	    default:
		break;
	    }
	}
    }
    return 0;
}

/* -- doc ---------------------------------------------------------------------
 * argdbg - write args to file handle for debugging
 * ------------------------------------------------------------------------- */
void argdbg(int objc, Tcl_Obj * CONST objv[], FILE * fh)
{

    int i = 0;
    for (i = 0; i < objc; i++) {
	if (objv[i] == NULL) {
	    fprintf(fh, "DBG arg %d -> 'null'\n", i);
	}
	else {
	    fprintf(fh, "DBG arg %d -> '%s'\n", i, Tcl_GetString(objv[i]));
	}
    }
    fflush(fh);
}
