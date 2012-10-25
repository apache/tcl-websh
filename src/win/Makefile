#
# Makefile -- make file for websh for Windows XP
#
# (adopted from Unix Makefile.in)
#
# See the file "license.terms" for information on usage an
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# Based on a version contributed by
# Ruslan Shevchenko, Grad-Soft Ltd, Kiev, Ukraine.
#
# How to use:
#   1.  set configuration variables bellow
#   2.  run nmake <target>
#         where <target> is one of
#         all websh.exe mod_websh.so test apachetest clean install
#
## configuration variables

TCL_PREFIX=C:/Program Files/Tcl
TCL_VERSION=85
HTTPD_PREFIX=C:/Program Files/Apache Software Foundation/Apache2
INSTALL_PREFIX=C:/Program Files/Apache Software Foundation/Websh

## no modifications should be necessary below this line
#

# debug objects?
!IF "$(DEBUG)"==""
DEBUG=0
!ELSE
DEBUG=1
!ENDIF

# local directories
TOP_DIR = ..
SRC_DIR = ../generic

# sources for script.h
SCRIPT_SOURCES = \
	../generic/context.tcl \
	../generic/script.ws3 \
	../generic/cookie.ws3 \
	../generic/sessctx.ws3 

# generic objects
web_OBJECTS = \
	args.obj \
	cfg.obj \
	checksum.obj \
	command.obj \
	conv.obj \
	crypt.obj \
	nca_d.obj \
	dispatch.obj \
	filecounter.obj \
	filelock.obj \
	formdata.obj \
	hashutl.obj \
	htmlify.obj \
	log.obj \
	logtocmd.obj \
	logtochannel.obj \
	logtofile.obj \
	logutl.obj \
	messages.obj \
	messagesCmd.obj \
	paramlist.obj \
	querystring.obj \
	request.obj \
	uricode.obj \
	script.obj \
	url.obj \
	web.obj \
	webout.obj \
	weboutint.obj \
	webutl.obj \
	webutlcmd.obj \
	varchannel.obj \
	modwebsh_cgi.obj \
	request_cgi.obj \
	response_cgi.obj

# Apache objects (for mod_websh.so)
web_OBJECTS_ap = \
	apchannel.obj \
	interpool.obj \
	logtoap.obj \
	mod_websh.obj \
	modwebsh_ap.obj \
	request_ap.obj \
	response_ap.obj


# install directories
bindir = $(INSTALL_PREFIX)/bin
moddir = $(INSTALL_PREFIX)/modules
docdir = $(INSTALL_PREFIX)/htdocs
libdir = $(INSTALL_PREFIX)/lib
cfgdir = $(INSTALL_PREFIX)/conf

# compiler, linker, and other binaries
CC = cl
CCLD = $(CC)
INSTALL = xcopy /y


PACKAGE = web
VERSION = 3.6.0b5

CFLAGS_DEFAULT = /D"WIN32" /D"VERSION=\"$(VERSION)\"" /D"_MBCS" /W3 /EHsc 
CFLAGS_DEBUG = /Od /Yd /Zi /D"_DEBUG"
CFLAGS_OPTIMIZE = /O2 /Ob1 

CLEANFILES = *.obj *.ilk *.pdb *.tlb *.tli *.tlh *.tmp *.rsp *.exe *.dll *.lib *.exp *.so

OBJEXT = obj

EXEC_CFLAGS = /D"_CONSOLE"
!IF "$(DEBUG)" == "1"
SHLIB_CFLAGS = /LDd /MDd /Gy
!ELSE
SHLIB_CFLAGS = /LD /MD /Gy
!ENDIF

# TCL stuff
TCL_INCLUDES = "$(TCL_PREFIX)/include"
TCL_DEFS = /DTHREAD_SAFE=1             
TCL_LIBS = kernel32.lib user32.lib advapi32.lib ws2_32.lib odbc32.lib 
TCL_STUB_LIB_SPEC = "$(TCL_PREFIX)/lib/tclstub$(TCL_VERSION).lib"
TCL_LIB_SPEC = "$(TCL_PREFIX)/lib/tcl$(TCL_VERSION).lib"
TCLSH_PROG = "$(TCL_PREFIX)\bin\tclsh$(TCL_VERSION)"

# Apache stuff
HTTPD_INCLUDES = "$(HTTPD_PREFIX)/include"
!IF EXIST("$(HTTPD_PREFIX)/include/apr_strings.h")
# Apache 2
!IF EXIST("$(HTTPD_PREFIX)/lib/libapr.lib")
# Apache 2.0
HTTPD_LIBS = "$(HTTPD_PREFIX)/lib/libhttpd.lib" \
	"$(HTTPD_PREFIX)/lib/libapr.lib" \
	"$(HTTPD_PREFIX)/lib/libaprutil.lib"
!ELSE
# Apache 2.2
HTTPD_LIBS = "$(HTTPD_PREFIX)/lib/libhttpd.lib" \
	"$(HTTPD_PREFIX)/lib/libapr-1.lib" \
	"$(HTTPD_PREFIX)/lib/libaprutil-1.lib"
!ENDIF
!ELSE
# Apache 1.3
HTTPD_LIBS = "$(HTTPD_PREFIX)/lib/ap.lib" \
	"$(HTTPD_PREFIX)/lib/ApacheOS.lib" \
	"$(HTTPD_PREFIX)/libexec/ApacheCore.lib"
!ENDIF

INCLUDES = -I$(TCL_INCLUDES) -I$(HTTPD_INCLUDES)

EXTRA_CFLAGS = $(TCL_DEFS)

!IF "$(DEBUG)" == "0"
CFLAGS = $(CFLAGS_DEFAULT) $(CFLAGS_OPTIMIZE) $(EXTRA_CFLAGS) $(INCLUDES)
LD_CFLAGS =  $(CFLAGS_DEFAULT) $(CFLAGS_OPTIMIZE)
DEFAULT_MSVCRT=msvcrt.lib
!ELSE
CFLAGS = $(CFLAGS_DEFAULT) $(CFLAGS_DEBUG) $(EXTRA_CFLAGS) $(INCLUDES)
LD_CFLAGS = $(CFLAGS_DEFAULT)
DEFAULT_MSVCRT=msvcrtd.lib
!ENDIF

!IF "$(DEBUG)" == "1"
!MESSAGE ==================================================
!MESSAGE Creating objects with debug information (DEBUG=1)
!MESSAGE ==================================================
!ENDIF

# find apache binary (needed for apachetest)
!IF EXIST("$(HTTPD_PREFIX)/bin/Apache.exe")
HTTPD_BIN = "$(HTTPD_PREFIX)/bin/Apache.exe"
!ELSE
!IF EXIST("$(HTTPD_PREFIX)/bin/httpd.exe")
HTTPD_BIN = "$(HTTPD_PREFIX)/bin/httpd.exe"
!ELSE
!IF EXIST("$(HTTPD_PREFIX)/Apache.exe")
HTTPD_BIN = "$(HTTPD_PREFIX)/Apache.exe"
!ELSE
!MESSAGE Warning: Apache binary not found in "$(HTTPD_PREFIX)/bin"
!ENDIF
!ENDIF
!ENDIF

# =============================================================================
# main targets
# =============================================================================

default:
	@echo.
	@echo -----------------------------------------------------------------
	@echo Target must be one of websh.exe, mod_websh.so, all, clean,
	@echo test, or install
	@echo Use DEBUG=1 to for debug objects (e.g. nmake websh.exe DEBUG=1)
	@echo -----------------------------------------------------------------
	@echo.

all: websh.exe mod_websh.so

script.h: ..\generic\tcldecmt.tcl $(SCRIPT_SOURCES)
	$(TCLSH_PROG) ..\generic\tcldecmt.tcl $(SCRIPT_SOURCES) > $@

script.obj: script.h ../generic/script.c
	$(CC) -I. $(CFLAGS) -c ../generic/script.c /Fo$@

interpool.obj: ../generic/interpool.c
	$(CC) $(CFLAGS) -UUSE_TCL_STUBS -c ../generic/interpool.c /Fo$@

{$(SRC_DIR)}.c{}.obj::
	$(CC) $(CFLAGS) -Fd.\ -c $<


websh.exe: websh$(VERSION).exe websh$(VERSION).dll

mod_websh.so: mod_websh$(VERSION).so websh$(VERSION).dll

websh$(VERSION).exe: tclAppInit.$(OBJEXT) websh$(VERSION).dll
	$(CCLD) $(LD_CFLAGS) $(EXEC_CFLAGS) \
	tclAppInit.$(OBJEXT) websh$(VERSION).lib \
	$(TCL_LIB_SPEC) $(TCL_STUB_LIB_SPEC) $(TCL_LIBS) -o websh$(VERSION).exe \
	/link /nodefaultlib:$(DEFAULT_MSVCRT)

mod_websh$(VERSION).so: $(web_OBJECTS_ap) websh$(VERSION).dll
	$(CCLD) $(LD_CFLAGS) $(SHLIB_CFLAGS) -o mod_websh$(VERSION).so \
	$(web_OBJECTS_ap) websh$(VERSION).lib $(TCL_LIB_SPEC) \
	$(TCL_STUB_LIB_SPEC) $(HTTPD_LIBS) $(TCL_LIBS) \
	/link /dll /nodefaultlib:msvcrt.lib /subsystem:windows

websh$(VERSION).dll: $(web_OBJECTS)
        $(CCLD) $(LD_CFLAGS) $(SHLIB_CFLAGS) -o websh$(VERSION).dll \
	$(web_OBJECTS) $(TCL_LIB_SPEC) $(TCL_STUB_LIB_SPEC) \
	$(TCL_LIBS) /link /dll \
	/nodefaultlib:$(DEFAULT_MSVCRT) /subsystem:windows

test: websh.exe
	set TCL_LIBRARY="$(TCL_PREFIX)/lib"
	set TCLLIBPATH=$(TCL_PACKAGE_PATH)
	set WEB_LIBRARY=$(TOP_DIR)/win/websh$(VERSION).dll
	cd $(TOP_DIR)/tests
	$(TOP_DIR)\win\websh$(VERSION).exe webtest.ws3

apachetest: mod_websh.so websh.exe
	cd $(TOP_DIR)/apachetests
	$(TCLSH_PROG) runtests.tcl $(HTTPD_BIN)

clean:
	-del /Q $(CLEANFILES)
	-del /Q script.h
	-rmdir /S /Q ..\apachetests\logs
	-del /Q ..\apachetests\conf\server.conf
	-del /Q ..\apachetests\conf\websh.conf

distclean: clean

# =============================================================================
# Install Websh
# As there are no symbolic links under Windows, files are installed twice:
# once with version number and once without.
# =============================================================================

install: installdirs installtargets installdocs installsample

installtargets:
	@echo. 
	@echo -----------------------------------------------------------------
	@echo Note that only targets that have been built are installed
	@echo -----------------------------------------------------------------
	@echo. 
	if exist mod_websh$(VERSION).so \
		$(INSTALL) mod_websh$(VERSION).so "$(moddir)"
	if exist mod_websh$(VERSION).lib \
		$(INSTALL) mod_websh$(VERSION).lib "$(libdir)"
	if exist mod_websh$(VERSION).exp \
		$(INSTALL) mod_websh$(VERSION).exp "$(libdir)"
	if exist mod_websh$(VERSION).so \
		copy /Y mod_websh$(VERSION).so "$(moddir)/mod_websh.so"
	if exist websh$(VERSION).exe \
		$(INSTALL) websh$(VERSION).exe "$(bindir)"
	if exist websh$(VERSION).exe \
		copy /Y websh$(VERSION).exe "$(bindir)/websh.exe"
	if exist websh$(VERSION).dll \
		$(INSTALL) websh$(VERSION).dll "$(bindir)"
	if exist websh$(VERSION).lib \
		$(INSTALL) websh$(VERSION).lib "$(libdir)"
	if exist websh$(VERSION).exp \
		$(INSTALL) websh$(VERSION).exp "$(libdir)"
	if exist websh$(VERSION).dll \
		copy /Y websh$(VERSION).dll "$(bindir)/websh.dll"
	copy /Y "$(TOP_DIR)\..\src\win\pkgIndex.tcl" "$(bindir)"

installdocs:
#	$(INSTALL) "$(TOP_DIR)\..\doc\html\*.html" "$(docdir)"

installsample:
	$(INSTALL) "$(TOP_DIR)\..\doc\mod_websh\conf\*.*" "$(cfgdir)"
	$(INSTALL) "$(TOP_DIR)\..\doc\mod_websh\htdocs\*.*" "$(docdir)"
	copy /Y "$(TOP_DIR)\..\doc\mod_websh\README" "$(INSTALL_PREFIX)\README.txt"

installdirs:
	-mkdir "$(bindir)"
	-mkdir "$(libdir)"
	-mkdir "$(moddir)"
	-mkdir "$(docdir)"
	-mkdir "$(cfgdir)"
