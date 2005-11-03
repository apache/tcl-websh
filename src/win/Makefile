#
# Makefile -- make file def for websh for Windows XP/NT
#
# (adopted from Unix Makefile.in)
#
# See the file "license.terms" for information on usage an
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#
# @author: Ruslan Shevchenko, Grad-Soft Ltd, Kiev, Ukraine.
#
# How to use:
#   1.  set configure variables bellow
#   2.  run nmake <target>
#         where target is one of websh.exe webshd.exe
#                                mod_websh.so mod_webshd.so
#                                clean install install-all
#
#  Note: when you want build multiple targets at once, you must run clean between them.
#  INCORRECT:  
#     nmake websh.exe
#     nmake mod_websh.so
#  CORRECT:
#     nmake websh.exe
#     nmake clean
#     nmake mod_websh.so          
#
## configuration variables

TCL_PREFIX=C:/Program Files/Tcl
TCL_VERSION=83
HTTPD_PREFIX=C:/Program Files/Apache Group/Apache2
INSTALL_PREFIX=C:/Program Files/Apache Group/Websh

## no modifications should be necessary below this line
#

MAKEOPTS=$(MAKEOPTS) /NOLOGO

!IF "$(INTERNAL)"=="1"

!IF "$(DEBUG)"==""
DEBUG=0
!ELSE
DEBUG=1
!ENDIF

!IF "$(CTARGET)"=="websh.exe" 
BUILD_WEBSH=1          
HTTPD_MODULE=0
!ELSEIF "$(CTARGET)"=="mod_websh.so"
BUILD_WEBSH=0
HTTPD_MODULE=1
!ELSEIF "$(CTARGET)"=="test"
BUILD_WEBSH=1          
HTTPD_MODULE=0
!ELSEIF "$(CTARGET)"=="clean"
!ELSEIF "$(CTARGET)"=="install"
!ELSE                    
!MESSAGE Unknown target $(CTARGET). Supported targets are:
!MESSAGE    websh.exe mod_websh.so clean test install
!MESSAGE Use DEBUG=1 to create debug objects
!ERROR  stop
!ENDIF

!ELSE

default:
	@echo Target must be one of: websh.exe mod_websh.so all clean test install
	@echo Use DEBUG=1 to create debug objects
websh.exe:
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=websh.exe websh.exe
mod_websh.so:
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=mod_websh.so mod_websh.so
clean:
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=clean clean
install:
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=install install
test:
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=test test
all:
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=clean clean
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=websh.exe websh.exe
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=clean cleanobj
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=mod_websh.so mod_websh.so
	@$(MAKE) $(MAKEOPTS) INTERNAL=1 CTARGET=clean cleanobj

!ENDIF


!IF "$(INTERNAL)" == "1"

TOP_DIR = ..
SRC_DIR = ../generic

# sources for script.h
SCRIPT_SOURCES = \
	../generic/context.tcl \
	../generic/script.ws3 \
	../generic/cookie.ws3 \
	../generic/sessctx.ws3 

# generic objects
web_OBJECTS_gen = \
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
	varchannel.obj

# CGI objects (for websh.exe)
web_OBJECTS_cgi = \
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


web_OBJECTS = $(web_OBJECTS_gen) $(web_OBJECTS_cgi)
web_ap_OBJECTS = $(web_OBJECTS_gen) $(web_OBJECTS_ap)

GENERIC_HDRS=

SHELL =

prefix = $(INSTALL_PREFIX)
exec_prefix = $(prefix)

bindir = $(prefix)/bin
sbindir = $(prefix)/sbin
libexecdir = $(prefix)/libexec
datadir = $(prefix)/share
sysconfdir = $(prefix)/etc
sharedstatedir = $(prefix)/com
localstatedir = $(prefix)/var
libdir = $(prefix)/lib
infodir = $(prefix)/info
mandir = $(prefix)/man
includedir = $(prefix)/include
oldincludedir = /usr/include


INSTALL = copy
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL)
INSTALL_SCRIPT = $(INSTALL)
INSTALL_STRIP_FLAG =

NORMAL_INSTALL = :
PRE_INSTALL = :
POST_INSTALL = :
NORMAL_UNINSTALL = :
PRE_UNINSTALL = :
POST_UNINSTALL = :

PACKAGE = web
VERSION = 3.6.0b2
CC = cl
!IF "$(BUILD_WEBSH)" == "1"
CFLAGS_DEBUG = /Od /Yd /Zi /MLd /D"_DEBUG"
!ELSE
CFLAGS_DEBUG = /Od /Yd /Zi /LDd /D"_DEBUG"
!ENDIF
!IF "$(BUILD_WEBSH)" == "1"
CFLAGS_DEFAULT = /D"WIN32" /D"_CONSOLE" /D"VERSION=\"$(VERSION)\"" /D"_MBCS" /W3 /EHsc 
!ELSE
CFLAGS_DEFAULT = /D"WIN32" /D"VERSION=\"$(VERSION)\"" /D"_MBCS" /W3 /EHsc 
!ENDIF
CFLAGS_OPTIMIZE = /O2 /Ob1 
CLEANFILES = *.obj *.ilk *.pdb *.tlb *.tli *.tlh *.tmp *.rsp *.exe *.dll *.lib *.exp *.so
EXEEXT =                
LDFLAGS_DEBUG = /link /verbose
LDFLAGS_DEFAULT = 
LDFLAGS_OPTIMIZE = 

# TODO
MAKE_SHARED_LIB = link /dll /out:$@ $**
MAKE_STATIC_LIB = lib /out:$@ $**

OBJEXT = obj
RANLIB = :
!IF "$(BUILD_WEBSH)" == "1"
SHLIB_CFLAGS = 
!ELSE
!IF "$(DEBUG)" == "1"
SHLIB_CFLAGS = /LDd /MDd /Gy
!ELSE
SHLIB_CFLAGS =  /LD /MD /Gy
!ENDIF
!ENDIF

# TCL stuff
TCL_INCLUDES = "$(TCL_PREFIX)/include"
TCL_DEFS = /DTHREAD_SAFE=1             
TCL_EXTRA_CFLAGS =
TCL_LD_FLAGS = 
TCL_LIBS = kernel32.lib user32.lib advapi32.lib ws2_32.lib odbc32.lib 
TCL_SHLIB_LD_LIBS = kernel32.dll user32.dll advapi32.dll ws2_32.dll
TCL_SRC_DIR = 
TCL_DBGX = 
TCL_STUB_LIB_FILE = "$(TCL_PREFIX)/lib/tclstub$(TCL_VERSION).lib"
TCL_STUB_LIB_SPEC = "$(TCL_PREFIX)/lib/tclstub$(TCL_VERSION).lib"
TCLSH_PROG = "$(TCL_PREFIX)\bin\tclsh$(TCL_VERSION)"

TCL_LIB_SPEC = $(TCL_STUB_LIB_SPEC) "$(TCL_PREFIX)/lib/tcl$(TCL_VERSION).lib"

# Apache stuff
HTTPD_LIBS = "$(HTTPD_PREFIX)/lib/libhttpd.lib"  "$(HTTPD_PREFIX)/lib/libapr.lib"
HTTPD_INCLUDES = "$(HTTPD_PREFIX)/include"

SHARED_LIB_SUFFIX = .dll
UNSHARED_LIB_SUFFIX = .lib
#


INCLUDES = -I$(TCL_INCLUDES) -I$(HTTPD_INCLUDES)

EXTRA_CFLAGS = $(TCL_DEFS) $(PROTO_FLAGS) $(SECURITY_FLAGS) $(MEM_DEBUG_FLAGS) $(KEYSYM_FLAGS) $(NO_DEPRECATED_FLAGS)

!IF "$(BUILD_WEBSH)" == "1"
DEFS = -DUSE_TCL_STUBS=1 $(EXTRA_CFLAGS)
!ELSE
DEFS = $(EXTRA_CFLAGS)
!ENDIF

CPPFLAGS = 
LIBS = 
AR = ar
!IF "$(DEBUG)" == "0"
CFLAGS = $(CFLAGS_DEFAULT) $(CFLAGS_OPTIMIZE) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) 
LD_CFLAGS =  $(CFLAGS_DEFAULT) $(CFLAGS_OPTIMIZE)  $(AM_CFLAGS) 

DEFAULT_MSVCRT=msvcrt.lib
!ELSE
CFLAGS = $(CFLAGS_DEFAULT) $(CFLAGS_DEBUG) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) 
LD_CFLAGS = $(CFLAGS_DEFAULT) $(AM_CFLAGS) 
DEFAULT_MSVCRT=msvcrtd.lib
!ENDIF

!IF "$(DEBUG)" == "1"
!MESSAGE ==================================================
!MESSAGE Creating objects with debug information (DEBUG=1)
!MESSAGE ==================================================
!ENDIF

CCLD = $(CC)
LINK = $(CCLD) $(AM_CFLAGS) $(CFLAGS) $(LDFLAGS) -o $@

# =============================================================================
# main targets
# =============================================================================

script.h: ..\generic\tcldecmt.tcl $(SCRIPT_SOURCES)
	$(TCLSH_PROG) ..\generic\tcldecmt.tcl $(SCRIPT_SOURCES) > $@

script.obj: script.h ../generic/script.c
	$(CC) -I. $(CFLAGS) -c ../generic/script.c /Fo$@

{$(SRC_DIR)}.c{}.obj::
	$(CC) $(CFLAGS) -Fd.\ -c $<


websh.exe: websh$(VERSION).exe

mod_websh.so: mod_websh$(VERSION).so


websh$(VERSION).exe: tclAppInit.$(OBJEXT) $(web_OBJECTS)
	$(CCLD) $(LD_CFLAGS) tclAppInit.$(OBJEXT) $(web_OBJECTS) \
	$(TCL_LIB_SPEC) $(TCL_LIBS) -o websh$(VERSION).exe \
	/link /nodefaultlib:$(DEFAULT_MSVCRT)

mod_websh$(VERSION).so: $(web_ap_OBJECTS)
	$(CCLD) $(LD_CFLAGS) $(SHLIB_CFLAGS) -o mod_websh$(VERSION).so \
	$(web_ap_OBJECTS) $(TCL_LIB_SPEC) $(HTTPD_LIBS) $(TCL_LIBS) \
	/link /dll /nodefaultlib:msvcrt.lib /subsystem:windows

# does not work yet
websh$(VERSION).dll:  $(web_OBJECTS)
        $(CCLD) $(LD_CFLAGS) $(SHLIB_CFLAGS) -o websh$(VERSION).dll \
	$(web_OBJECTS) $(TCL_LIB_SPEC) $(TCL_LIBS) /link /dll \
        /nodefaultlib:$(DEFAULT_MSVCRT) /subsystem:windows

test: websh$(VERSION).exe $(web_OBJECTS)
	set TCL_LIBRARY="$(TCL_PREFIX)/library"
	set TCLLIBPATH=$(TCL_PACKAGE_PATH)
	set WEB_LIBRARY=$(TOP_DIR)/win/websh$(VERSION).dll
	cd $(TOP_DIR)/tests
	$(TOP_DIR)\win\websh$(VERSION).exe webtest.ws3

clean:
	-del /Q *.$(OBJEXT)
	-del /Q script.h
	-del /Q $(CLEANFILES)

# used between targets to account for different flags
cleanobj:
	-del /Q *.$(OBJEXT)

distclean: clean

# =============================================================================
# Install binary object libraries.  On Windows this includes both .dll and
# .lib files.  Because the .lib files are not explicitly listed anywhere,
# we need to deduce their existence from the .dll file of the same name.
# Additionally, the .dll files go into the bin directory, but the .lib
# files go into the lib directory.  On Unix platforms, all library files
# go into the lib directory.  In addition, this will generate the pkgIndex.tcl
# file in the install location (assuming it can find a usable tclsh8.2 shell)
#
# =============================================================================

#install: install-binaries install-libraries install-doc

install:
	@echo Not properly implemented yet

install-binaries: install-lib-binaries install-bin-binaries

install-libraries:


install-lib-binaries: installdirs
	if exist libwebshcbroker.dll  $(INSTALL_DATA)  libwebshcbroker.dll "$(libdir)"
	if exist mod_websh.so $(INSTALL_DATA) mod_websh.so "$(HTTPD_PREFIX)/modules"
	if exist mod_webshd.so $(INSTALL_DATA) mod_webshd.so "$(HTTPD_PREFIX)/modules"


# =============================================================================
# Install documentation.  Unix manpages should go in the $(mandir)
# directory.
# =============================================================================

#TODO.
install-doc:


# =============================================================================
# Install binary executables (e.g. .exe files)
#
# You should not have to modify this target.
# =============================================================================

install-bin-binaries: installdirs
	if exist websh.exe   $(INSTALL_PROGRAM) websh.exe "$(bindir)"
	if exist webshd.exe  $(INSTALL_PROGRAM) webshd.exe "$(bindir)"


uninstall-binaries:
	if exist $(bindir)/websh.exe   del /q "$(bindir)/websh.exe"
	if exist $(bindir)/webshd.exe   del /q "$(bindir)/webshd.exe"

installdirs:
	-mkdir  "$(libdir)"
	-mkdir  "$(bindir)"


# INTERNAL flag
!ENDIF
