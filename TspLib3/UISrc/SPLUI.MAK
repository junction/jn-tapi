#########################################################################
# TSP++ 3.0 MAKEFILE
# Copyright (C) 2000-2001 JulMar Technology, Inc.
#########################################################################

!IF "$(CFG)" != "Release" && "$(CFG)" != "Debug" && "$(CFG)" != "DebugUnicode" && "$(CFG)" != "ReleaseUnicode"
CFG=Release
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe
LIB32=link.exe -lib

!IF "$(TAPI_CURRENT_VERSION)" != ""
TAPIVER=/D "TAPI_CURRENT_VERSION=$(TAPI_CURRENT_VERSION)"
!ELSE
TAPIVER=
!ENDIF

!IF  "$(CFG)" == "Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : "..\LIB\SPLUI.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W4 /WX /GR /GX /O1 /I "..\INCLUDE" $(TAPIVER) /D "STRICT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\SPLUI.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
LIB32_FLAGS=/nologo /out:"..\LIB\SPLUI.lib" 

LIB32_OBJS= \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\address.obj" \
	"$(INTDIR)\device.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\phone.obj" \
	"$(INTDIR)\serialize.obj" \
	"$(INTDIR)\Spdll.obj" \
	"$(INTDIR)\UISUPP.OBJ"

"..\LIB\SPLUI.lib" : "$(OUTDIR)" $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Debug"

ALL : "..\LIB\SPLUId.lib"

OUTDIR=.\Debug
INTDIR=.\Debug

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W4 /WX /GR /GX /Zi /Od /Gf /I "..\INCLUDE" $(TAPIVER) /D "STRICT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\SPLUI.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
LIB32_FLAGS=/nologo /out:"..\LIB\SPLUId.lib" 

LIB32_OBJS= \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\address.obj" \
	"$(INTDIR)\device.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\phone.obj" \
	"$(INTDIR)\serialize.obj" \
	"$(INTDIR)\Spdll.obj" \
	"$(INTDIR)\UISUPP.OBJ"

"..\LIB\SPLUId.lib" : "$(OUTDIR)" $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ReleaseUnicode"

ALL : "..\LIB\SPLUIU.lib"

OUTDIR=.\ReleaseUnicode
INTDIR=.\ReleaseUnicode

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W4 /WX /GR /GX /O2 /I "..\INCLUDE" $(TAPIVER) /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_UNICODE" /D "STRICT" /Fp"$(INTDIR)\SPLUI.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
LIB32_FLAGS=/nologo /out:"..\LIB\SPLUIU.lib" 

LIB32_OBJS= \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\address.obj" \
	"$(INTDIR)\device.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\phone.obj" \
	"$(INTDIR)\serialize.obj" \
	"$(INTDIR)\Spdll.obj" \
	"$(INTDIR)\UISUPP.OBJ"

"..\LIB\SPLUIU.lib" : "$(OUTDIR)" $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DebugUnicode"

ALL : "..\LIB\SPLUIUD.lib"

OUTDIR=.\DebugUnicode
INTDIR=.\DebugUnicode

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W4 /WX /GR /GX /Zi /Od /Gf /I "..\INCLUDE" $(TAPIVER) /D "STRICT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_UNICODE" /Fp"$(INTDIR)\SPLUI.pch" /YX"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
LIB32_FLAGS=/nologo /out:"..\LIB\SPLUIUD.lib" 

LIB32_OBJS= \
	"$(INTDIR)\stdafx.obj" \
	"$(INTDIR)\address.obj" \
	"$(INTDIR)\device.obj" \
	"$(INTDIR)\line.obj" \
	"$(INTDIR)\phone.obj" \
	"$(INTDIR)\serialize.obj" \
	"$(INTDIR)\Spdll.obj" \
	"$(INTDIR)\UISUPP.OBJ"

"..\LIB\SPLUIUD.lib" : "$(OUTDIR)" $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.\address.cpp : \
	"..\INCLUDE\spbstrm.h"\
	

.\device.cpp : \
	"..\INCLUDE\spbstrm.h"\
	

.\line.cpp : \
	"..\INCLUDE\spbstrm.h"\
	

.\phone.cpp : \
	"..\INCLUDE\spbstrm.h"\
	

.\serialize.cpp : \
	"..\INCLUDE\regstream.h"\
	"..\INCLUDE\spbstrm.h"\
	

.\stdafx.cpp : \
	"..\INCLUDE\splibui.h"\
	"..\INCLUDE\uiaddr.inl"\
	"..\INCLUDE\uiagent.inl"\
	"..\Include\uidevice.inl"\
	"..\INCLUDE\uiline.inl"\
	"..\INCLUDE\uiphone.inl"\
	".\STDAFX.H"\
	
