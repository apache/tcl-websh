/*
 * paramlist.c -- key-value pairs, where value is a Tcl list Obj
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
#include "webutl.h"
#include "paramlist.h"
#include "log.h"
#include "hashutl.h"

static char *paramsubcmd[] = {"-count","-unset","-set","-lappend","-names",NULL};
enum paramsubcmd {PARAM_COUNT,PARAM_UNSET,PARAM_SET,PARAM_LAPPEND,PARAM_NAMES};

/* ----------------------------------------------------------------------------
 * paramGetIndexFromObj -- same as Tcl_GetIndexFromObj, but includes
 * the values from static table paramsubcmd.
 * ------------------------------------------------------------------------- */
int paramGetIndexFromObj(Tcl_Interp *interp, Tcl_Obj *obj, char **tablePtr, char *msg, int flags, int *indexPtr) {

  /* fixme-later: fixed allocation */
  char *allopts[20];
  int i = 0, numprivateopts, po;
  Tcl_Obj *objCopy = Tcl_DuplicateObj(obj);

  while ( tablePtr[i] ) {
    allopts[i] = tablePtr[i];
    i++;
  }
  numprivateopts = i;
  po = i;
  i=0;
  while ( paramsubcmd[i] ) {
    allopts[po] = paramsubcmd[i];
    po++; i++;
  }
  allopts[po] = NULL;
  if (Tcl_GetIndexFromObj(interp, objCopy, allopts, msg, flags, indexPtr) == TCL_OK) {
    if (*indexPtr < numprivateopts) {
      Tcl_DecrRefCount(objCopy);
      return TCL_OK;
    }
  }
  Tcl_DecrRefCount(objCopy);
  return TCL_ERROR;
}
  





/* ----------------------------------------------------------------------------
 * paramListSet -- set value to new ListObj with just the given value
 * ------------------------------------------------------------------------- */
int paramListSet(ParamList *hash, char *key, Tcl_Obj *value) {

  Tcl_Obj *existingValue = NULL;
  Tcl_Obj *copy = NULL;
  int     len = 0;

  if( (hash == NULL) || (key == NULL) || (value == NULL) ) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * remove existing
   * ----------------------------------------------------------------------- */
  if( (existingValue = (Tcl_Obj *)removeFromHashTable(hash,key)) != NULL )
    Tcl_DecrRefCount(existingValue);

  /* --------------------------------------------------------------------------
   * force it into a list
   * ----------------------------------------------------------------------- */

  copy = Tcl_NewListObj(1,&value);
  /* fixme: ref count of value correct here? */
  Tcl_IncrRefCount(copy);
  return appendToHashTable(hash, key, (ClientData)copy);
}

/* ----------------------------------------------------------------------------
 * paramListSetAsWhole -- set value to new ListObj with just the given value
 *                        without listifying the element
 * ------------------------------------------------------------------------- */
int paramListSetAsWhole(ParamList *hash, char *key, Tcl_Obj *value) {

  Tcl_Obj *existingValue = NULL;

  if( (hash == NULL) || (key == NULL) || (value == NULL) ) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * remove existing
   * ----------------------------------------------------------------------- */
  if( (existingValue = (Tcl_Obj *)removeFromHashTable(hash,key)) != NULL )
    Tcl_DecrRefCount(existingValue);

  Tcl_IncrRefCount(value);
  return appendToHashTable(hash, key, (ClientData)value);
}

/* ----------------------------------------------------------------------------
 * paramListAdd -- append value to existing ListObj under key
 * ------------------------------------------------------------------------- */
int paramListAdd(ParamList *hash, char *key, Tcl_Obj *value) {

  Tcl_Obj *existing = NULL;
  Tcl_Obj *new = NULL;
  int     iRes = -1;

  if( (hash == NULL) || (key == NULL) || (value == NULL) ) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * does key already exist ?
   * ----------------------------------------------------------------------- */
  if( (existing = (Tcl_Obj *)getFromHashTable(hash,key)) == NULL ) {

    /* ------------------------------------------------------------------------
     * no, create new
     * --------------------------------------------------------------------- */
    return paramListSet(hash,key,value);

  }

  /* --------------------------------------------------------------------------
   * yes, append
   * ----------------------------------------------------------------------- */
  if( Tcl_IsShared(existing) ) {

    if ((existing = (Tcl_Obj*)removeFromHashTable(hash,key)) == NULL)
      return TCL_ERROR;

    new = Tcl_DuplicateObj(existing);
    Tcl_DecrRefCount(existing);

    iRes = Tcl_ListObjAppendElement(NULL,new,value);
    appendToHashTable(hash,key,(ClientData)new);

    return iRes;
  }
  iRes = Tcl_ListObjAppendElement(NULL,existing,value);
  return iRes;
}

/* ----------------------------------------------------------------------------
 * paramListDel -- delete existing
 * ------------------------------------------------------------------------- */
int paramListDel(ParamList *hash, char *key) {

  Tcl_Obj *tclo = NULL;

  if( (hash == NULL) || (key == NULL) ) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * remove existing
   * ----------------------------------------------------------------------- */
  if( (tclo = (Tcl_Obj *)removeFromHashTable(hash,key)) != NULL ) 
    Tcl_DecrRefCount(tclo);

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * paramListAsListObj -- append each entry of list to ListObj
 * results in list with even number of elements of the form
 * k1 v1 k1 v2 ... kj vn
 * ------------------------------------------------------------------------- */
Tcl_Obj *paramListAsListObj(ParamList *hash) {

  HashTableIterator iterator;
  Tcl_Obj          *res = NULL;
  Tcl_Obj          *key = NULL;
  Tcl_Obj          *val = NULL;
  Tcl_Obj          *ele = NULL;
  int              valLen = 0;
  int              i;

  if( hash == NULL ) return NULL;

  assignIteratorToHashTable(hash,&iterator);

  res = Tcl_NewObj();
  if( res == NULL ) return NULL;

  while( nextFromHashIterator(&iterator) != TCL_ERROR ) {

    key = Tcl_NewStringObj(keyOfCurrent(&iterator),-1);
    if( key == NULL ) break;

    val = (Tcl_Obj *)valueOfCurrent(&iterator);
    if( val == NULL ) break;

    if( Tcl_ListObjLength(NULL,val,&valLen) == TCL_ERROR ) break;

    for( i = 0; i < valLen; i++) {

      if( Tcl_ListObjAppendElement(NULL,res,key) == TCL_ERROR ) break;
      if( Tcl_ListObjIndex(NULL,val,i,&ele) == TCL_ERROR ) break;

      if( Tcl_ListObjAppendElement(NULL,res,ele) == TCL_ERROR ) break;
    }
  }

  return res;
}

/* ----------------------------------------------------------------------------
 * listObjAsParamList -- convert Tcl_ListObj to internal representation
 * ------------------------------------------------------------------------- */
int listObjAsParamList(Tcl_Obj *list, ParamList *hash) {

  int     listLen = 0;
  int     i;
  Tcl_Obj *key = NULL;
  Tcl_Obj *val = NULL;

  if( (list == NULL) || (hash == NULL) ) return TCL_ERROR;

  /* --------------------------------------------------------------------------
   * is even numbered ?
   * ----------------------------------------------------------------------- */
  if( Tcl_ListObjLength(NULL,list,&listLen) == TCL_ERROR )
    return TCL_ERROR;
  if( (listLen % 2) != 0 ) {

    /* no. last is single key. append value */
    val = Tcl_NewObj();
    Tcl_ListObjAppendElement(NULL,list,val);
  }

  if( Tcl_ListObjLength(NULL,list,&listLen) == TCL_ERROR )
    return TCL_ERROR;

  for(i = 0; i < listLen; i += 2) {

    key = NULL;
    val = NULL;
    Tcl_ListObjIndex(NULL,list,i,&key);
    if( key == NULL ) return TCL_ERROR;
    Tcl_ListObjIndex(NULL,list,i+1,&val);
    if( val == NULL ) return TCL_ERROR;

    if( paramListAdd(hash,Tcl_GetString(key),val) == TCL_ERROR )
      return TCL_ERROR;
  }

  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * paramListNamesAll -- create list of all existing keys matching pattern
 * ------------------------------------------------------------------------- */
Tcl_Obj *paramListNamesAll(ParamList *hash) {

  HashTableIterator iterator;
  char             *key = NULL;
  Tcl_Obj          *res = NULL;

  if( hash == NULL ) return NULL;

  assignIteratorToHashTable(hash,&iterator);

  res = Tcl_NewObj();
  if( res == NULL ) return NULL;

  while( nextFromHashIterator(&iterator) != TCL_ERROR ) {

    key = keyOfCurrent(&iterator);
    if( key != NULL ) {

      if( Tcl_ListObjAppendElement(NULL,res,Tcl_NewStringObj(key,-1)) == \
	  TCL_ERROR) break;
    }
  }
  return res;
}

/* ----------------------------------------------------------------------------
 * destroyParamList -- free the HashTable with and the values (Tcl_Obj)
 * ------------------------------------------------------------------------- */
void destroyParamList(ParamList *hash) {

  HashTableIterator iterator;
  Tcl_Obj           *tclo = NULL;

  if( hash != NULL ) {

    assignIteratorToHashTable(hash,&iterator);

    while( nextFromHashIterator(&iterator) != TCL_ERROR ) {

      tclo = (Tcl_Obj *)valueOfCurrent(&iterator);
      if( tclo != NULL ) {
	Tcl_DecrRefCount(tclo);
      }
    }

    HashUtlDelFree(hash);
  }
}

/* ----------------------------------------------------------------------------
 * paramGet
 * tries if it is a generic paramlist subcommand
 * returns
 * TCL_OK if command handled
 * TCL_ERROR if there was an error
 * TCL_CONTINUE if command is not handled
 * 
 * ------------------------------------------------------------------------- */
int paramGet(ParamList *paramList, 
	     Tcl_Interp *interp, 
	     int objc, Tcl_Obj *CONST objv[],
	     int hasPrivate) {


  char *arg;
  int opt;

  /* --------------------------------------------------------------------------
   * sanity
   * ----------------------------------------------------------------------- */

  WebAssertData(interp,paramList,"paramList/paramGet",TCL_ERROR);

  /* fixme: what for? */
  /* probably not necessary:
   * WebAssertArgs(interp,objc,objv,paramsubcmd,count,2);
   */

  /* if there are no args -> continue with private parsing */
  if (objc < 2) {
    if (hasPrivate)
      return TCL_CONTINUE;
    else {
      WebAssertObjc(1, 1, "args ...");
    }
  }
  /* see if there is a subcommand */
  arg = Tcl_GetString(objv[1]);
  if (arg[0] == '-') {
    if (Tcl_GetIndexFromObj(interp, objv[1], 
			    paramsubcmd, "subcommand", 0, &opt) == TCL_ERROR)
      if (hasPrivate) {
	/* we ignore the error here cause we might have private commands */
	Tcl_ResetResult(interp); 
	return TCL_CONTINUE;
      } else
	return TCL_ERROR;
    
    /* it's one of our's */
    switch (opt) {
    case PARAM_COUNT:
      WebAssertObjc(objc != 3, 2, "key");
      return paramListCountValue(interp, paramList, objv[2]);
    case PARAM_UNSET:
      WebAssertObjc(objc > 3, 2, "?key?");
      if (objc == 3)
	return paramListDel(paramList, Tcl_GetString(objv[2]));
      else 
	emptyParamList(paramList);
      return TCL_OK;
    case PARAM_SET:
      WebAssertObjc(objc < 3, 2, "key ?value ...?");
      if (objc != 3) {
	int i = 3;
	paramListDel(paramList, Tcl_GetString(objv[2]));
	while (i < objc)
	  paramListAdd(paramList, Tcl_GetString(objv[2]), objv[i++]);
      }  
      /* return value, just like tcl */
      return paramListGetValueToResult(interp, paramList, objv[2], NULL);
    case PARAM_LAPPEND: {
      int i = 3;
      WebAssertObjc(objc < 4, 2, "key value ?value ...?");
      /* we append stuff */
      while (i < objc)
	paramListAdd(paramList, Tcl_GetString(objv[2]), objv[i++]);
      /* return list, just like tcl */
      return paramListGetValueToResult(interp, paramList, objv[2], NULL);
    }
    case PARAM_NAMES: {
      Tcl_Obj *obj = paramListNamesAll(paramList);
      WebAssertObjc(objc != 2, 2, NULL);
      if (obj) {
	Tcl_SetObjResult(interp, obj);
      } else {
	Tcl_ResetResult(interp);
      }
      return TCL_OK;
    }
    default:
      /* fixme: error message */
      return TCL_ERROR;
    }
  } else {
    /* no subcommand -> return value */
    WebAssertObjc(objc > 3, 2, "key ?default?");
    if (objc == 3)
      return paramListGetValueToResult(interp, paramList, objv[1], objv[2]);
    else
      return paramListGetValueToResult(interp, paramList, objv[1], NULL);
  }
  
  /* fixme: error message */
  /* return TCL_ERROR; */
}


int paramListGetValueToResult(Tcl_Interp *interp, ParamList *paramList, Tcl_Obj *key, Tcl_Obj *defaultObj) {
  
  Tcl_Obj *resObj;
  resObj = paramListGetObject(interp, paramList, key);
  if (resObj)
    Tcl_SetObjResult(interp, resObj);
  else
    if (defaultObj)
      Tcl_SetObjResult(interp,Tcl_DuplicateObj(defaultObj));
    else 
      Tcl_ResetResult(interp);
  return TCL_OK;
}

Tcl_Obj *paramListGetObject(Tcl_Interp *interp, ParamList *paramList, Tcl_Obj *key) {
  
  return paramListGetObjectByString(interp, paramList, Tcl_GetString(key));
}

Tcl_Obj *paramListGetObjectByString(Tcl_Interp *interp, ParamList *paramList, char *key) {
  
  Tcl_Obj *resObj;
  resObj = (Tcl_Obj *)getFromHashTable((Tcl_HashTable *)paramList, key);
  if (resObj) {
    if( tclGetListLength(interp,resObj) == 1) {
      /* if the value has only one element, we return that */
      Tcl_Obj *firstEl = NULL;
      if( Tcl_ListObjIndex(interp,resObj,0,&firstEl) == TCL_ERROR )
	return NULL;
      else
	return Tcl_DuplicateObj(firstEl);
    } else
      return Tcl_DuplicateObj(resObj);
  }
  return NULL;
}
		       

int paramListCountValue(Tcl_Interp *interp, ParamList *paramList, Tcl_Obj *key) {
  Tcl_Obj *resObj;
  int n = 0;
  resObj = (Tcl_Obj *)getFromHashTable((Tcl_HashTable *)paramList, Tcl_GetString(key));
  if (resObj) 
    n= tclGetListLength(interp,resObj);
  Tcl_SetObjResult(interp, Tcl_NewIntObj(n));
  return TCL_OK;
}

/* ----------------------------------------------------------------------------
 * emptyParamList -- delete values of param list (Tcl_Obj)
 * ------------------------------------------------------------------------- */
void emptyParamList(ParamList *paramList) {

  /* HashTableIterator iterator; */
  Tcl_Obj           *tclo = NULL;
  Tcl_HashTable *hash = paramList;
  Tcl_HashSearch hs;
  Tcl_HashEntry *he;

  if( hash != NULL ) {
    while (he = Tcl_FirstHashEntry(hash, &hs)) {
      tclo = (Tcl_Obj *)Tcl_GetHashValue(he);
      if( tclo != NULL ) 
	Tcl_DecrRefCount(tclo);
      Tcl_DeleteHashEntry(he);
    }
  }
}
