# Microsoft Developer Studio Project File - Name="websh3App" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=websh3App - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "websh3App.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "websh3App.mak" CFG="websh3App - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "websh3App - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "websh3App - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "websh3App - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\build\win32\Release"
# PROP Intermediate_Dir "..\..\build\win32\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\..\..\tcl831\include" /I "." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D VERSION=\"30b2\" /FR /YX /FD /D /c
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\..\..\tcl831\lib\tcl83.lib /nologo /subsystem:console /machine:I386 /out:"..\..\build\win32\Release\websh3.exe"

!ELSEIF  "$(CFG)" == "websh3App - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\build\win32\Debug"
# PROP Intermediate_Dir "..\..\build\win32\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\externals\win32" /I "." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /D /GZ /c
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\externals\win32\tcl83.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\..\build\win32\Debug\websh3.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "websh3App - Win32 Release"
# Name "websh3App - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\generic\args.c
# End Source File
# Begin Source File

SOURCE=..\generic\cfg.c
# End Source File
# Begin Source File

SOURCE=..\generic\checksum.c
# End Source File
# Begin Source File

SOURCE=..\generic\command.c
# End Source File
# Begin Source File

SOURCE=..\generic\conv.c
# End Source File
# Begin Source File

SOURCE=..\generic\crypt.c
# End Source File
# Begin Source File

SOURCE=..\generic\dispatch.c
# End Source File
# Begin Source File

SOURCE=..\generic\filecounter.c
# End Source File
# Begin Source File

SOURCE=..\generic\filelock.c
# End Source File
# Begin Source File

SOURCE=..\generic\formdata.c
# End Source File
# Begin Source File

SOURCE=..\generic\hashutl.c
# End Source File
# Begin Source File

SOURCE=..\generic\htmlify.c
# End Source File
# Begin Source File

SOURCE=..\generic\log.c
# End Source File
# Begin Source File

SOURCE=..\generic\logtochannel.c
# End Source File
# Begin Source File

SOURCE=..\generic\logtocmd.c
# End Source File
# Begin Source File

SOURCE=..\generic\logtofile.c
# End Source File
# Begin Source File

SOURCE=..\generic\logutl.c
# End Source File
# Begin Source File

SOURCE=..\generic\modwebsh_cgi.c
# End Source File
# Begin Source File

SOURCE=..\generic\nca_d.c
# End Source File
# Begin Source File

SOURCE=..\generic\paramlist.c
# End Source File
# Begin Source File

SOURCE=..\generic\querystring.c
# End Source File
# Begin Source File

SOURCE=..\generic\request.c
# End Source File
# Begin Source File

SOURCE=..\generic\request_cgi.c
# End Source File
# Begin Source File

SOURCE=..\generic\response_cgi.c
# End Source File
# Begin Source File

SOURCE=..\generic\script.c
# End Source File
# Begin Source File

SOURCE=..\generic\tclAppInit.c
# End Source File
# Begin Source File

SOURCE=..\generic\uricode.c
# End Source File
# Begin Source File

SOURCE=..\generic\url.c
# End Source File
# Begin Source File

SOURCE=..\generic\varchannel.c
# End Source File
# Begin Source File

SOURCE=..\generic\web.c
# End Source File
# Begin Source File

SOURCE=..\generic\webout.c
# End Source File
# Begin Source File

SOURCE=..\generic\weboutint.c
# End Source File
# Begin Source File

SOURCE=..\generic\webutl.c
# End Source File
# Begin Source File

SOURCE=..\generic\webutlcmd.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\generic\args.h
# End Source File
# Begin Source File

SOURCE=..\generic\cfg.h
# End Source File
# Begin Source File

SOURCE=..\generic\checksum.h
# End Source File
# Begin Source File

SOURCE=..\generic\conv.h
# End Source File
# Begin Source File

SOURCE=..\generic\crypt.h
# End Source File
# Begin Source File

SOURCE=..\generic\crypturi.h
# End Source File
# Begin Source File

SOURCE=..\generic\filecounter.h
# End Source File
# Begin Source File

SOURCE=..\generic\filelock.h
# End Source File
# Begin Source File

SOURCE=..\generic\hashlist.h
# End Source File
# Begin Source File

SOURCE=..\generic\hashutl.h
# End Source File
# Begin Source File

SOURCE=..\generic\log.h
# End Source File
# Begin Source File

SOURCE=..\generic\logtochannel.h
# End Source File
# Begin Source File

SOURCE=..\generic\logtocmd.h
# End Source File
# Begin Source File

SOURCE=..\generic\logtofile.h
# End Source File
# Begin Source File

SOURCE=..\generic\logtosyslog.h
# End Source File
# Begin Source File

SOURCE=..\generic\macros.h
# End Source File
# Begin Source File

SOURCE=..\generic\modwebsh.h
# End Source File
# Begin Source File

SOURCE=..\generic\nca_d.h
# End Source File
# Begin Source File

SOURCE=..\generic\paramlist.h
# End Source File
# Begin Source File

SOURCE=..\generic\pluginmgr.h
# End Source File
# Begin Source File

SOURCE=..\generic\request.h
# End Source File
# Begin Source File

SOURCE=.\script.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\tcl831\include\tcl.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\tcl831\include\tcldecls.h
# End Source File
# Begin Source File

SOURCE=..\generic\toSyslog.h
# End Source File
# Begin Source File

SOURCE=..\generic\url.h
# End Source File
# Begin Source File

SOURCE=..\generic\web.h
# End Source File
# Begin Source File

SOURCE=..\generic\webout.h
# End Source File
# Begin Source File

SOURCE=..\generic\weboutint.h
# End Source File
# Begin Source File

SOURCE=..\generic\webutl.h
# End Source File
# Begin Source File

SOURCE=..\generic\webutlcmd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
