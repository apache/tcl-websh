/*
 * webutl.h --- common utils used in more than one module
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
#include <stdlib.h>
#include <string.h>
#include "macros.h"

#ifndef WEBUTL_H
#define WEBUTL_H

char __declspec(dllexport) *allocAndSet(const char *aString);
char *allocAndSetN(const char *aString, int N);
char *strchrchr(const char *cs, const char c0, const char c1, char *tag);
char *myUtfStrStr(const char *s1, const char *s2);
char *webEat(char eat, char *cs);
char *strWithoutLinebreak(char *cs);

int handleConfig(Tcl_Interp * interp, Tcl_Obj ** tclo, Tcl_Obj * newValue,
		 int deleteIfEmpty);
int tclGetListLength(Tcl_Interp * interp, Tcl_Obj * list);

Tcl_Obj *tclSetEnv(Tcl_Interp * interp, char *key, Tcl_Obj * val);

int deleteTclObj_fnc(void *tclo, void *dum);

Tcl_Obj *Web_GetOrCreateGlobalVar(Tcl_Interp * interp, Tcl_Obj * name,
				  int *isNew);

#endif
