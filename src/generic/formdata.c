/*
 * formdata.c -- x-www-form-urlencoded and multipart/form-data 
 *               (file upload) handling for websh3
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
#include "request.h"
#include "paramlist.h"
#include "webutl.h"
#include "log.h"
#include "macros.h"
#include "varchannel.h"


/* ----------------------------------------------------------------------------
 * parseUrlEncodedFormData --
 *   parse "k1=v1&k2=v2" kind of data, and store in web::formvar structure
 * ------------------------------------------------------------------------- */
int parseUrlEncodedFormData(RequestData *requestData, Tcl_Interp *interp,
			    char *channelName, Tcl_Obj *len) {
  Tcl_Obj     *tclo = NULL;
  int         tRes = 0;
  int         listLen = -1;
  Tcl_Obj     *tmps[2];
  Tcl_Obj     *tmpObj = NULL;
  Tcl_Obj     *formData = NULL;
  Tcl_Channel channel;
  int         mode;
  int         readToEnd = 0;
  int         content_length = 0;

  channel = Web_GetChannelOrVarChannel(interp,channelName,&mode);
  if (channel == NULL) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch -postdata",WEBLOG_WARNING,
	    "error getting channel \"",channelName,"\"",NULL);
    return TCL_ERROR;
  }

  if ( (mode & TCL_READABLE) == 0) {

    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch -postdata",WEBLOG_WARNING,
	    "channel \"",channelName, "\" not open for reading",
	    NULL);

    /* unregister if was a varchannel */
    Web_UnregisterVarChannel(interp,channelName,channel);

    return TCL_ERROR;
  }

  /* fixme: we don't set the channel options back. maybe we should  ;-) */
  Tcl_SetChannelOption(interp,channel,"-translation","binary");

  /* ------------------------------------------------------------------------
   * how much to read ?
   * --------------------------------------------------------------------- */
  if( len == NULL ) {

    readToEnd = 1;

  } else {

    if( strcmp(Tcl_GetString(len),"end") == 0 ) {

      readToEnd = 1;

    } else {

      readToEnd = 0;

      if( Tcl_GetIntFromObj(interp,len,&content_length) != TCL_OK) {

        /* unregister if was a varchannel */
        Web_UnregisterVarChannel(interp,channelName,channel);
        return TCL_ERROR;
      }
    }
  }

  /* ------------------------------------------------------------------------
   * ok, read
   * --------------------------------------------------------------------- */
  formData = Tcl_NewObj();

  if( readToEnd ) {

    /* try to read to the end  */
    /*                                         append flag */
    while( Tcl_ReadChars(channel,formData,4096,1) != -1) {
      if( Tcl_Eof(channel) ) break;
    }

  } else {

    if( Tcl_ReadChars(channel,formData,content_length,1) == TCL_ERROR ) {

      LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
              "web::dispatch -postdata",WEBLOG_WARNING,
              "error reading from \"",channelName, "\"",NULL);

      Tcl_DecrRefCount(formData);

      /* unregister if was a varchannel */
      Web_UnregisterVarChannel(interp,channelName,channel);

      return TCL_ERROR;
    }
  }

  /* unregister if was a varchannel */
  Web_UnregisterVarChannel(interp,channelName,channel);

  tmpObj = Tcl_NewStringObj("web::uri2list ",-1);
  Tcl_AppendObjToObj(tmpObj, formData);

  Tcl_IncrRefCount(tmpObj);
  tRes = Tcl_EvalObjEx(interp,tmpObj,TCL_EVAL_DIRECT);

  Tcl_DecrRefCount(tmpObj);
  Tcl_DecrRefCount(formData);
  
  if( tRes == TCL_ERROR ) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch -postdata", WEBLOG_WARNING,
	    "error parsing formdata", NULL);

    return TCL_ERROR;
  }

  tclo = Tcl_GetObjResult(interp);
  Tcl_IncrRefCount(tclo);
  Tcl_ResetResult(interp);

  /* --------------------------------------------------------------------------
   * only add if list length > 0
   * ----------------------------------------------------------------------- */
  if( (listLen = tclGetListLength(interp,tclo)) == -1 ) {
    /* no Tcl_DecrRefCount(tclo); */
    return TCL_ERROR;
  }
  
  if( listLen > 0) {
    /* ------------------------------------------------------------------------
     * add list to requestData
     * --------------------------------------------------------------------- */
    tRes = listObjAsParamList(tclo,requestData->formVarList);
    Tcl_DecrRefCount(tclo);
    return tRes;
  }

  /* --------------------------------------------------------------------------
   * done
   * ----------------------------------------------------------------------- */
  Tcl_DecrRefCount(tclo);

  return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * parseMultipartFormData
 * ------------------------------------------------------------------------- */
int parseMultipartFormData(RequestData *requestData, Tcl_Interp *interp,
			   char *channelName, char *content_type) {

  Tcl_Channel channel;
  int  mode;
  char *boundary = mimeGetParamFromContDisp(content_type,"boundary");
  int  res = 0;

/*   printf("DBG parseMultipartFormData - starting\n"); fflush(stdout); */

  if (boundary == NULL) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch -postdata",WEBLOG_WARNING,
	    "error accessing boundary from\"",content_type,"\"",NULL);
    return TCL_ERROR;
  }
    
  channel = Web_GetChannelOrVarChannel(interp,channelName,&mode);
  if (channel == NULL) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch -postdata",WEBLOG_WARNING,
	    "error getting channel \"",channelName,"\"",NULL);
    return TCL_ERROR;
  }

  if ( (mode & TCL_READABLE) == 0) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch -postdata",WEBLOG_WARNING,
	    "channel \"",channelName, "\" not open for reading",
	    NULL);

    /* unregister if was a varchannel */
    Web_UnregisterVarChannel(interp,channelName,channel);
    return TCL_ERROR;
  }

  /* fixme: we don't set the channel options back. maybe we should  ;-) */
  Tcl_SetChannelOption(interp,channel,"-translation","binary");


  res = mimeSplitMultipart(interp,channel,boundary,requestData);

  /* unregister if was a varchannel */
  Web_UnregisterVarChannel(interp,channelName,channel);

  return res;
}


/* ----------------------------------------------------------------------------
 * mimeSplitMultipart
 * ------------------------------------------------------------------------- */
int mimeSplitMultipart(Tcl_Interp *interp, Tcl_Channel channel, 
		       const char *boundary, RequestData *requestData) {

  Tcl_Obj *pro = NULL;
  Tcl_Obj *hdr = NULL;
  Tcl_Obj *bdy = NULL;
  int     isLast = TCL_ERROR;
  long    upLoadFileSize = 0;
  long    bytesWritten = 0;
  long    bytesSkipped = 0;
  Tcl_Obj *tmpFileName = NULL;

  MimeContDispData *mimeContDispData = NULL;

  isLast = TCL_ERROR;

/*   printf("DBG mimeSplitMultipart - starting\n"); fflush(stdout); */

  /* --------------------------------------------------------------------------
   * prolog
   * ----------------------------------------------------------------------- */
  pro = Tcl_NewObj();
  if( pro == NULL ) return TCL_ERROR;
  mimeReadBody(channel,pro,boundary, &isLast);
/*   printf("DBG prolog: %s\n",Tcl_GetString(pro)); fflush(stdout); */
  Tcl_DecrRefCount(pro);

  /* --------------------------------------------------------------------------
   * read until last
   * ----------------------------------------------------------------------- */

  /* fixme: only read content_length bytes ... */
  while( isLast == TCL_ERROR ) {

    /* ------------------------------------------------------------------------
     * header
     * --------------------------------------------------------------------- */
    hdr = Tcl_NewObj();
    if( hdr == NULL ) return TCL_ERROR;
    mimeReadHeader(channel,hdr);
    mimeContDispData = mimeGetContDispFromHeader(interp,Tcl_GetString(hdr));
    Tcl_DecrRefCount(hdr);

    /* ------------------------------------------------------------------------
     * body
     * --------------------------------------------------------------------- */
    if( mimeContDispData == NULL ) {
      LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
              "web::dispatch -postdata",
              WEBLOG_ERROR,
              "error accessing 'Content-Disposition'. Check boundary",
              NULL);
      return TCL_ERROR;
    }

    if( (mimeContDispData->name == NULL) || 
	(mimeContDispData->type == NULL) ) {

      destroyMimeContDispData(mimeContDispData);
      return TCL_ERROR;
    }

    if( STRCASECMP(mimeContDispData->type,"form-data") == 0 ) {

      /* ----------------------------------------------------------------------
       * file upload ?
       * ------------------------------------------------------------------- */

      if( mimeContDispData->fileName != NULL ) {

	int fileNameLen = strlen(mimeContDispData->fileName);
	int flag = TCL_OK;
	Tcl_Obj *lobjv[4];
	Tcl_Obj *fileUploadList = NULL;

	WebGetLong(interp,requestData->upLoadFileSize,upLoadFileSize,flag);
	if( flag == TCL_ERROR ) {
	  destroyMimeContDispData(mimeContDispData);
	  return TCL_ERROR;
	}

	bytesWritten = 0;
	bytesSkipped = 0;
          
	tmpFileName = tempFileName(interp,requestData,NULL,NULL);
	if( tmpFileName == NULL ) {
	  LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
		  "web::dispatch -postdata",
		  WEBLOG_ERROR,
		  "cannot request name for temporary file",NULL);
	  destroyMimeContDispData(mimeContDispData);
	  return TCL_ERROR;
	}
	
	bytesWritten = readAndDumpBody(interp,channel, boundary, &isLast, 
				       tmpFileName, upLoadFileSize,
				       &bytesSkipped);

	if( fileNameLen > 0 ) {

	  lobjv[0] = tmpFileName;
	  lobjv[1] = Tcl_NewStringObj(mimeContDispData->fileName,-1);
	  if( upLoadFileSize == 0 )
	    lobjv[2] = Tcl_NewIntObj(-1);
	  else
	    lobjv[2] = Tcl_NewLongObj(bytesSkipped);
	  lobjv[3] = Tcl_NewStringObj(mimeContDispData->content,-1);

	} else {

	  lobjv[0] = Tcl_NewStringObj("",-1);
	  lobjv[1] = Tcl_NewStringObj("",-1);
	  lobjv[2] = Tcl_NewIntObj(-2);
	  lobjv[3] = Tcl_NewStringObj("",-1);
	}

	fileUploadList = Tcl_NewObj();
	Tcl_ListObjReplace(interp,fileUploadList,0,0,4,lobjv);

	if( paramListAdd(requestData->formVarList,
			 mimeContDispData->name,
			 fileUploadList) \
	    == TCL_ERROR ) {
	  LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
		  "web::dispatch -postdata",
		  WEBLOG_ERROR,
		  "cannot add \"",
		  mimeContDispData->name,", ",
		  Tcl_GetString(fileUploadList),
		  "\" to web::formvar data",NULL);

	  Tcl_ListObjReplace(interp,fileUploadList,0,3,0,NULL);
	  Tcl_DecrRefCount(fileUploadList);

	  destroyMimeContDispData(mimeContDispData);
	  return TCL_ERROR;
	}
      } else {

	/* --------------------------------------------------------------------
	 * no filename. is normal field
	 * ----------------------------------------------------------------- */
	bdy = Tcl_NewObj();
	if( bdy == NULL ) {
	  destroyMimeContDispData(mimeContDispData);
	  return TCL_ERROR;
	}
	mimeReadBody(channel,bdy, boundary, &isLast);

/*         printf("DBG mimeSplitMultipart - '%s' -> '%s'\n",mimeContDispData->name,Tcl_GetString(bdy)); fflush(stdout); */

	if( paramListAdd(requestData->formVarList,
			 mimeContDispData->name,bdy) \
	    == TCL_ERROR ) {
	  LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
		  "web::dispatch -postdata",WEBLOG_ERROR,
		  "cannot add \"",
		  mimeContDispData->name,", ",
		  Tcl_GetString(bdy),
		  "\" to web::formvar data",NULL);
	  destroyMimeContDispData(mimeContDispData);
	  Tcl_DecrRefCount(bdy);
	  return TCL_ERROR;
	}
	/* NO Tcl_DecrRefCount(bdy); */
      }
    }
    destroyMimeContDispData(mimeContDispData);
  }
  return TCL_OK;
}


/* ----------------------------------------------------------------------------
 * mimeSplitIsBoundary
 * ------------------------------------------------------------------------- */
int mimeSplitIsBoundary(Tcl_Obj *cur, Tcl_Obj *prev, 
                        const char *boundary, int *isLast) {

  int  bLen  = 0;
  int  cLen  = 0;
  int  pLen  = 0;
  char *line = NULL;

  if( (cur == NULL) || (boundary == NULL) ) return TCL_ERROR;

  /* check for CR on prev line */
  if ( prev != NULL ) {
    line = Tcl_GetStringFromObj(prev,&pLen);

    if( pLen > 0 )
      if(line[pLen-1] != '\r')
        return TCL_ERROR;
  }

  bLen    = strlen(boundary);
  line    = Tcl_GetStringFromObj(cur,&cLen);
  *isLast = TCL_ERROR;

/*   printf("DBG mimeSplitIsBoundary - checking %s\n",line); fflush(stdout); */

  if( cLen  >= 2 + bLen )
    if( strncmp(line,"--",2) == 0 )
      if( strncmp(&(line[2]),boundary,bLen) == 0 ) {
        if( cLen >= (4 + bLen) )
          if( strncmp(&(line[2 + bLen]),"--",2) == 0)
            *isLast = TCL_OK;

        if ( prev != NULL ) {
          /* cut CR from line if it was a boundary */
          Tcl_SetObjLength(prev,pLen-1);
        }

        return TCL_OK;
      }
  return TCL_ERROR;
}

/* ----------------------------------------------------------------------------
 * mimeReadHeader
 * ------------------------------------------------------------------------- */
void mimeReadHeader(Tcl_Channel channel, Tcl_Obj *hdr) {

  Tcl_Obj *tclo = NULL;
  int     first = 0;
  char    *cline = NULL;
  int     len = 0;

  if( (channel == NULL) || (hdr == NULL) ) return;

/*   printf("DBG mimeReadHeader - starting\n"); fflush(stdout); */


  tclo = Tcl_NewObj();
  first = TCL_OK;

  while(Tcl_GetsObj(channel,tclo) != -1 ) {

    cline = Tcl_GetStringFromObj(tclo,&len);

    if( strcmp(cline,"\r") == 0 ) break;

    /* cut CR from line */
    if( len > 0 )
      if( cline[len-1] == '\r')
        Tcl_SetObjLength(tclo,len-1);
        

    if( first == TCL_ERROR )
      Tcl_AppendToObj(hdr,"\n",1);
    else
      first = TCL_ERROR;

    Tcl_AppendObjToObj(hdr,tclo);

    Tcl_DecrRefCount(tclo);
    tclo = Tcl_NewObj();
  }

  Tcl_DecrRefCount(tclo);

/*   printf("DBG mimeReadHeader - hdr: %s\n",Tcl_GetString(hdr)); fflush(stdout); */
}

/* ----------------------------------------------------------------------------
 * readAndDumpBody -- POST/file upload:
 *   get data from channel (typically stdin) and dump to file
 * note: if the first chunk that is read is bigger than upLoadFileSize,
 * note: no bytes will be written, and bytesSkipped = readBytes.
 * ------------------------------------------------------------------------- */
long readAndDumpBody(Tcl_Interp *interp, Tcl_Channel in, 
		     const char *boundary, int *isLast,
		     Tcl_Obj *tmpFileName, long upLoadFileSize, 
		     long *bytesSkipped) {

  Tcl_Channel out;
  Tcl_Obj     *curline  = NULL;
  Tcl_Obj     *prevline = NULL;
  long        tmp = 0;
  long        readBytes = 0;
  long        writtenBytes = 0;
  long        rBytes = 0;
  long        rBytesPrev = 0;
  long        wBytes = 0;
  char        *tmpCurLine = NULL;
  
  /* --------------------------------------------------------------------------
   * sanity
   * ----------------------------------------------------------------------- */
  if( (in == NULL) || (boundary == NULL) || (tmpFileName == NULL ) )
    return 0;

  /* --------------------------------------------------------------------------
   * open file
   * ----------------------------------------------------------------------- */
  if ((out = Tcl_OpenFileChannel(NULL,Tcl_GetString(tmpFileName),
                                 "w",0644))== NULL)
    return 0;

  /* --------------------------------------------------------------------------
   * switch output to "binary"
   * ----------------------------------------------------------------------- */
  if( Tcl_SetChannelOption(interp,out,"-translation","binary") \
      == TCL_ERROR ) {

    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch (file upload)",
	    WEBLOG_INFO,
	    "error setting translation to binary ",NULL);
    return 0;
  }

  /* --------------------------------------------------------------------------
   * first line
   * ----------------------------------------------------------------------- */
  prevline   = Tcl_NewObj();
  rBytesPrev = Tcl_GetsObj(in,prevline);

  if( rBytesPrev != -1 ) {

    /* ------------------------------------------------------------------------
     * read line-by-line, write delayed
     * --------------------------------------------------------------------- */
    curline = Tcl_NewObj();

    while((rBytes = Tcl_GetsObj(in,curline)) != -1 ) {

      int isBoundary = 0;

      /* ----------------------------------------------------------------------
       * test for boundary
       * ------------------------------------------------------------------- */
      if( mimeSplitIsBoundary(curline,prevline,boundary,isLast) == TCL_OK) {

	isBoundary = 1;
	readBytes += rBytesPrev - 1;  /* we truncated CR */

      } else {

	readBytes += rBytesPrev + 1;  /* add newline */
      }

      /* ----------------------------------------------------------------------
       * test for upload limit
       * ------------------------------------------------------------------- */
      if( (upLoadFileSize > 0) && (writtenBytes <= upLoadFileSize) ) {

        /* --------------------------------------------------------------------
         * partial or full write ?
         * ----------------------------------------------------------------- */
        if( (writtenBytes + rBytesPrev) <= upLoadFileSize ) {

          /* still room to write all we read */
      
          if( (wBytes = Tcl_WriteObj(out,prevline)) != -1) {

            writtenBytes += wBytes;
        
	    if(!isBoundary) {

	      if( writtenBytes  <= (upLoadFileSize-1) ) {

              /* still room for newline */

		if( Tcl_Eof(in) == 0 ) /* not yet at end-of-file */
		  if( (tmp = Tcl_WriteChars(out,"\n",1)) != -1 ) 
		    writtenBytes += tmp;
	      }
            }
          }
        } else {

          /* just dump part of input */
	  
          if( (wBytes = Tcl_WriteChars(out,Tcl_GetString(prevline),\
				     upLoadFileSize - writtenBytes)) != -1) {
	  
            writtenBytes += wBytes;
          }
        }
      }

      if(isBoundary) break;

      /* ----------------------------------------------------------------------
       * update prevline
       * ------------------------------------------------------------------- */
      Tcl_DecrRefCount(prevline);
      prevline   = curline;
      curline    = Tcl_NewObj();
      rBytesPrev = rBytes;
    }
    Tcl_DecrRefCount(curline);
  }
  
  Tcl_Close(NULL,out);

  Tcl_DecrRefCount(prevline);

  *bytesSkipped = (readBytes - writtenBytes);

  return writtenBytes;
}

/* ----------------------------------------------------------------------------
 * dumpBody -- POST/file upload in string case: write body to file
 * ------------------------------------------------------------------------- */
long dumpBody(Tcl_Interp *interp, char *body, Tcl_Obj *tmpFileName,
              long upLoadFileSize, long *bytesSkipped) {

  Tcl_Channel out;
  int         len = 0;
  long        wlen = 0;
  long        wBytes = 0;
  
  /* --------------------------------------------------------------------------
   * sanity
   * ----------------------------------------------------------------------- */
  if( (interp == NULL) || (body == NULL) || (tmpFileName == NULL ) )
    return 0;

  /* --------------------------------------------------------------------------
   * open file
   * ----------------------------------------------------------------------- */
  if ((out = Tcl_OpenFileChannel(NULL,Tcl_GetString(tmpFileName),
                                 "w",0644))== NULL)
    return 0;

  /* --------------------------------------------------------------------------
   * switch to -translation "binary"
   * ----------------------------------------------------------------------- */
  if( Tcl_SetChannelOption(interp,out,"-translation","binary") \
      == TCL_ERROR ) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
	    "web::dispatch (file upload)",
	    WEBLOG_INFO,
	    "error setting translation to binary ",NULL);
    return 0;
  }

  /* --------------------------------------------------------------------------
   * how much to write ?
   * ----------------------------------------------------------------------- */
  len = strlen(body);
  if( len > upLoadFileSize ) wlen = upLoadFileSize;
  else                       wlen = len;

  if( (wBytes = Tcl_WriteChars(out,body,wlen)) == -1 ) {
    *bytesSkipped = len;
  }

  Tcl_Close(NULL,out);

  *bytesSkipped = (len - wBytes);

  return wBytes;
}

/* ----------------------------------------------------------------------------
 * mimeReadBody
 * ------------------------------------------------------------------------- */
void mimeReadBody(Tcl_Channel in, Tcl_Obj *bdy, const char *boundary,
  int *isLast) {

  Tcl_Obj *prevline = NULL;
  Tcl_Obj *curline = NULL;
  int     first;

/*   printf("DBG mimeReadBody - starting\n"); fflush(stdout); */

  first = 1;

  prevline = Tcl_NewObj();
  if(Tcl_GetsObj(in,prevline) != -1 ) {

/*     printf("DBG mimeReadBody - first line: %s\n",Tcl_GetString(prevline)); fflush(stdout); */

    if(mimeSplitIsBoundary(prevline,NULL,boundary,isLast) == TCL_OK) {

      Tcl_DecrRefCount(prevline);
      return;
    }

    curline = Tcl_NewObj();

    while(Tcl_GetsObj(in,curline) != -1 ) {

      int isBoundary = 0;

/*       printf("DBG mimeReadBody - prevline: %s\n",Tcl_GetString(prevline)); fflush(stdout); */

      /* ----------------------------------------------------------------------
       * test for boundary
       * ------------------------------------------------------------------- */
      if(mimeSplitIsBoundary(curline,prevline,boundary,isLast) == TCL_OK) {
        isBoundary = 1;
      }

      if (!first) {
        Tcl_AppendToObj(bdy,"\n",1);
      } else {
        first = 0;
      }

      Tcl_AppendObjToObj(bdy,prevline);


/*       printf("DBG mimeReadBody - bdy now: %s\n",Tcl_GetString(bdy)); fflush(stdout); */

      Tcl_DecrRefCount(prevline);
      prevline = curline;
      curline = Tcl_NewObj();

      if(isBoundary) break;
    }
    Tcl_DecrRefCount(curline);
  }
  Tcl_DecrRefCount(prevline);
}

/* ----------------------------------------------------------------------------
 * mimeGetParamFromContDisp --
 *   searches for parameter of name <name> in a string of the form:
 *   Content-Disposition: form-data; name="i1"; filename="test.dat"
 *   actually, it searches case-insensitive for <name> in <contentDisp>
 *   and takes what follows after '=', discarding optional '"'
 *   N o t e: user needs to free memory for char that is returned.
 * ------------------------------------------------------------------------- */
char *mimeGetParamFromContDisp(const char *contentDisp,
			       const char *name) {

  char *tmp = NULL;
  char *val = NULL;
  char *semicolon = NULL;
  char tag = -1;
  int  namLen = 0;
  
  if( (contentDisp == NULL) || (name == NULL) ) return NULL;

  namLen = strlen(name);
  tmp    = myUtfStrStr(contentDisp,name);

  if( tmp == NULL ) return NULL;

  if( strlen(tmp) <= (unsigned int)namLen) return NULL;

  tmp = &(tmp[namLen]);

  if( strlen(tmp) > 2 ) {

    if( tmp[0] == '=' ) {

      tmp = &(tmp[1]);

      if( tmp[0] == '"' ) tmp = &(tmp[1]);

      semicolon = strchrchr(tmp,';','\n',&tag);
      if( semicolon == NULL ) semicolon = tmp + strlen(tmp) + 1;

      if( strlen(tmp) > 0 ) {

	val = allocAndSetN(tmp,semicolon - tmp);

	if( val != NULL ) {
	  if( val[strlen(val) - 1] == '"' ) val[strlen(val) - 1] = 0;

	  val = strWithoutLinebreak(val);

	  return val;
	}
      }
    }
  }
  return NULL;
}


/* ----------------------------------------------------------------------------
 * mimeGetContDispFromHeader
 * ------------------------------------------------------------------------- */
MimeContDispData *mimeGetContDispFromHeader(Tcl_Interp *interp,
					    const char *header) {

  char *start = NULL;
  int  len = 0;
  char *semicolon = NULL;
  char tag;

  char *type = NULL;
  char *name = NULL;
  char *fileName = NULL;
  char *content = NULL;

  MimeContDispData *mimeContDispData = NULL;

  if( header == NULL ) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
            "web::dispatch -postdata",
            WEBLOG_ERROR,
            "error accessing Content-Disposition from multipart/formdata data: no header",
            NULL);
    return NULL;
  }

  /*                                    1         2 */
  /*                          012345678901234567890 */
  start = myUtfStrStr(header,"Content-Disposition: ");
  if( start == NULL ) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
            "web::dispatch -postdata",
            WEBLOG_ERROR,
            "error accessing Content-Disposition from multipart/formdata data: 'Content-Disposition' not found in '",
            header, "'",
            NULL);
    return NULL;
  }

  len = strlen(start);
  if( len < 20 ) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
            "web::dispatch -postdata",
            WEBLOG_ERROR,
            "error accessing Content-Disposition from multipart/formdata data: empty 'Content-Disposition'",
            NULL);
    return NULL;
  }

  semicolon = strchrchr(start,';','\n',&tag);
  if( semicolon == NULL ) {
    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
            "web::dispatch -postdata",
            WEBLOG_ERROR,
            "error accessing Content-Disposition from multipart/formdata data: 'Content-Disposition' not properly terminated in '",
            start, "'",
            NULL);
    return NULL;
  }

  start = webEat(' ',&start[21]);
  type = allocAndSetN(start,semicolon-start);

  name     = mimeGetParamFromContDisp(start,"name");
  fileName = mimeGetParamFromContDisp(start,"filename");
      
  mimeContDispData = newMimeContDispData();
  if( mimeContDispData == NULL) {

    WebFreeIfNotNull(type);
    WebFreeIfNotNull(name);
    WebFreeIfNotNull(fileName);

    LOG_MSG(interp,WRITE_LOG,__FILE__,__LINE__,
            "web::dispatch -postdata",
            WEBLOG_ERROR,
            "error getting memory",
            NULL);
    return NULL;
  }

  mimeContDispData->type = type;
  mimeContDispData->name = name;
  mimeContDispData->fileName = fileName;

  /* --------------------------------------------------------------------------
   * and get the Mime Content-type
   *  patch provided by david@raster.onthe.net.au
   * ----------------------------------------------------------------------- */

  /*                                    1         2 */
  /*                          012345678901234567890 */
  start = myUtfStrStr(header,"Content-Type: ");
  if( start ) {

    len = strlen(start);
    if( len >= 13 ) {
      semicolon = strchr(start,'\n');
      if( semicolon == NULL ) semicolon = &start[len];

      start = webEat(' ',&start[14]);
      content = allocAndSetN(start,semicolon-start);
      mimeContDispData->content = content;
    }
  }
  return mimeContDispData;
}


/* ----------------------------------------------------------------------------
 * newMimeContDispData
 * ------------------------------------------------------------------------- */
MimeContDispData *newMimeContDispData(void) {

  MimeContDispData *data = NULL;

  data = WebAllocInternalData(MimeContDispData);

  if( data != NULL ) {

    data->type = NULL;
    data->name = NULL;
    data->fileName = NULL;
    data->content = NULL;
  }
  return data;
}

/* ----------------------------------------------------------------------------
 * destroyMimeContDispData
 * ------------------------------------------------------------------------- */
void destroyMimeContDispData(MimeContDispData *data) {

  if( data != NULL ) {

    WebFreeIfNotNull(data->type);
    WebFreeIfNotNull(data->name);
    WebFreeIfNotNull(data->fileName);
    WebFreeIfNotNull(data->content);
  }
  WebFreeIfNotNull(data);
}
