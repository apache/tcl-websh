/*
 * paramlist.h -- key-value pairs, where value is a Tcl list Obj
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
#include "webutl.h"

#ifndef PARAMLIST_H
#define PARAMLIST_H

typedef Tcl_HashTable ParamList;

int paramListGetValueToResult(Tcl_Interp *interp, ParamList *paramList, Tcl_Obj *key, Tcl_Obj *defaultObj);

int paramListSet(ParamList *hash, char *key, Tcl_Obj *value);
int paramListSetAsWhole(ParamList *hash, char *key, Tcl_Obj *value);
int paramListAdd(ParamList *hash, char *key, Tcl_Obj *value);
int paramListDel(ParamList *hash, char *key);
Tcl_Obj *paramListAsListObj(ParamList *hash);
int     listObjAsParamList(Tcl_Obj *list, ParamList *hash);
Tcl_Obj *paramListNamesAll(ParamList *hash);
void    destroyParamList(ParamList *hash);
int     paramGet(ParamList *paramList, 
                 Tcl_Interp *interp, 
                 int objc, Tcl_Obj *CONST objv[],
		 int hasPrivate);

int paramGetIndexFromObj(Tcl_Interp *interp, 
			 Tcl_Obj *obj,
			 char **tablePtr,
			 char *msg,
			 int flags,
			 int *indexPtr);

Tcl_Obj *
paramListGetObject(Tcl_Interp *interp,
		   ParamList *paramList,
		   Tcl_Obj *key);

Tcl_Obj *
paramListGetObjectByString(Tcl_Interp *interp,
		   ParamList *paramList,
		   char *key);

int
paramListGetValuetoResult(Tcl_Interp *interp,
			  ParamList *paramList,
			  Tcl_Obj *key,
			  Tcl_Obj *defaultObj);


int
paramListCountValue(Tcl_Interp *interp,
		    ParamList *paramList,
		    Tcl_Obj *key);

void
emptyParamList(ParamList *paramList);
#endif
