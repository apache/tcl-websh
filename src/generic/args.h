/*
 * args.h --- argument and option processing
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

#ifndef WEBARGS_H
#define WEBARGS_H

#define OPTION_TYPE_NONE 0
#define OPTION_TYPE_PARAM 1
#define OPTION_TYPE_DASHDASH 2
#define OPTION_TYPE_NUMBER 3

int argIndexOfKey(int objc, Tcl_Obj * CONST objv[], char *key);
int argIndexOfFirstOpt(int objc, Tcl_Obj * CONST objv[]);
int argIndexOfNextKey(int objc, Tcl_Obj * CONST objv[], int previous);
int argKeyExists(int objc, Tcl_Obj * CONST objv[], char *key);
Tcl_Obj *argValueOfKey(int objc, Tcl_Obj * CONST objv[], char *key);
int argPosParam(char **params, char *key);
int argIndexOfFirstOpt(int objc, Tcl_Obj * CONST objv[]);
int argIndexOfFirstArg(int objc, Tcl_Obj * CONST objv[],
		       char **params, int *Nparams);
int argHasOnlyAccepted(int objc, Tcl_Obj * CONST objv[], char *params[],
		       int scanc);
int argOptionType(Tcl_Obj * in);

#endif
