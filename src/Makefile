#
# Makefile for websh3 package
# nca-073-9
#
# Copyright (C) 2001 by Netcetera AG.
# All rights reserved.
#
# The copyright to the computer program(s) herein is the property of
# Netcetera AG, Switzerland.  The program(s) may be used and/or copied
# only with the written permission of Netcetera AG or in accordance
# with the terms and conditions stipulated in the agreement/contract
# under which the program(s) have been supplied.
#
# @(#) $Id$
#

VERSIONFILE = version

MAJOR_VERSION=3
MINOR_VERSION=0
BUILDNO = $(shell echo `echo 'source $(VERSIONFILE); if { [catch {puts $$build}] } {puts 0}' | tclsh`)

WEBSH   = websh-$(MAJOR_VERSION).$(MINOR_VERSION).$(BUILDNO)

GENERIC_SOURCES = \
generic/apchannel.c \
generic/args.c \
generic/args.h \
generic/cfg.c \
generic/cfg.h \
generic/checksum.c \
generic/checksum.h \
generic/command.c \
generic/conv.c \
generic/conv.h \
generic/crypt.c \
generic/crypt.h \
generic/dispatch.c \
generic/filecounter.c \
generic/filecounter.h \
generic/filelock.c \
generic/filelock.h \
generic/formdata.c \
generic/hashutl.c \
generic/hashutl.h \
generic/htmlify.c \
generic/initcode.c \
generic/interpool.c \
generic/interpool.h \
generic/log.c \
generic/log.h \
generic/logtoap.c \
generic/logtoap.h \
generic/logtochannel.c \
generic/logtochannel.h \
generic/logtocmd.c \
generic/logtocmd.h \
generic/logtofile.c \
generic/logtofile.h \
generic/logtosyslog.c \
generic/logtosyslog.h \
generic/logutl.c \
generic/macros.h \
generic/messages.c \
generic/messages.h \
generic/messagesCmd.c \
generic/mod_websh.c \
generic/mod_websh.h \
generic/modwebsh.h \
generic/modwebsh_ap.c \
generic/modwebsh_cgi.c \
generic/nca_d.c \
generic/nca_d.h \
generic/paramlist.c \
generic/paramlist.h \
generic/querystring.c \
generic/request.c \
generic/request.h \
generic/request_ap.c \
generic/request_cgi.c \
generic/response_ap.c \
generic/response_cgi.c \
generic/script.c \
generic/tclAppInit.c \
generic/uricode.c \
generic/url.c \
generic/url.h \
generic/web.c \
generic/web.h \
generic/webout.c \
generic/webout.h \
generic/weboutint.c \
generic/webutl.c \
generic/webutl.h \
generic/webutlcmd.c \
generic/webutlcmd.h \
generic/context.tcl \
generic/cookie.ws3 \
generic/script.ws3 \
generic/sessctx.ws3 \
generic/tcldecmt.tcl \
generic/varchannel.c \
generic/varchannel.h

UNIX_SOURCES = \
unix/Makefile.in \
unix/aclocal.m4 \
unix/install-sh \
unix/mkinstalldirs \
unix/tcl.m4

WIN_SOURCES = \
win/websh3.dsw \
win/websh3App.dsp

TESTS = \
tests/cfg.test \
tests/cmdurl.test \
tests/command.test \
tests/context.test \
tests/cookiecontext.test \
tests/dehtmlify.test \
tests/dispatch.test \
tests/encrypt.test \
tests/filecontext.test \
tests/filecounter.test \
tests/filelock.test \
tests/formvar.test \
tests/htmlify.test \
tests/log.test \
tests/logtochannel.test \
tests/logtocmd.test \
tests/logtofile.test \
tests/logtosyslog.test \
tests/messages.test \
tests/mintest.test \
tests/msc.test \
tests/ncad.test \
tests/nocrypt.test \
tests/param.test \
tests/request.test \
tests/script.test \
tests/uricode.test \
tests/webout.test \
tests/webtest.ws3

DOCS = \
ChangeLog \
README \
license.terms \
changes

BUILDFILES = $(GENERIC_SOURCES) \
  $(UNIX_SOURCES) \
  $(WIN_SOURCES) \
  $(TESTS) \
  $(DOCS) \
  Makefile \
  configure.in

TARFILEDEST=/nca/projects/nca-073-9

ttarfile:
	mkdir -p $(TARFILEDEST)/nca-073-9-$(BUILDNO); \
	echo "dir created"; \
	builddir=$(TARFILEDEST)/nca-073-9-$(BUILDNO); \
	tarfile=$$builddir/nca-073-9-$(BUILDNO).tar; \
	if [ -f $$tarfile ] ; then \
	  echo "tarfile $$tarfile already exists, aborting."; \
	  /bin/false; \
	else \
	  echo "writing $$tarfile";\
	  mkdir -p $(WEBSH);\
	  mkdir -p $(WEBSH)/generic;\
	  mkdir -p $(WEBSH)/unix;\
	  mkdir -p $(WEBSH)/tests;\
	  mkdir -p $(WEBSH)/doc;\
	  cp $(GENERIC_SOURCES) $(WEBSH)/generic;\
	  cp $(UNIX_SOURCES) $(WEBSH)/unix;\
	  cp $(TESTS) $(WEBSH)/tests;\
	  cp $(DOCS) $(WEBSH);\
	  cp generic/script.h $(WEBSH)/generic;\
	  cp unix/configure.in $(WEBSH)/unix;\
	  cp unix/configure $(WEBSH)/unix;\
	  cp ../doc/quickref.html $(WEBSH)/doc;\
	  cp ../doc/quickref.txt $(WEBSH)/doc;\
	  chgrp -R user $(WEBSH);\
	  chmod -R u+w,ugo+r,go-wx $(WEBSH);\
	  chmod g+x  $(WEBSH)/unix  $(WEBSH)/doc  $(WEBSH)/tests  $(WEBSH)/generic;\
	  chmod ug+rx $(WEBSH)/unix/install-sh $(WEBSH)/unix/mkinstalldirs $(WEBSH)/unix/configure;\
	  /usr/local/bin/tar cf $$tarfile $(WEBSH); \
	fi;

tarfile: ttarfile

# build:  clean install tagbuild $(TARGETS)
build:  clean unix/configure.in unix/configure tagbuild tarfile


tagbuild: 
	ncabuild nca-073-9 $(VERSIONFILE) $(BUILDFILES)

clean:
	rm -rf $(WEBSH)

unix/configure.in: configure.in
	ncamerge -nf -K PATCHLEVEL $(BUILDNO) -K MAJOR_VERSION $(MAJOR_VERSION) -K MINOR_VERSION $(MINOR_VERSION) -o unix/configure.in configure.in

unix/configure: unix/configure.in
	cd unix; autoconf
