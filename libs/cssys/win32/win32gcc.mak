# This is the makefile for MinGW32 compiler (gcc for Win32)

# Friendly names for building environment
DESCRIPTION.win32gcc = Windows with MinGW32 GNU C/C++
DESCRIPTION.OS.win32gcc = Win32

# Choose which drivers you want to build/use
PLUGINS+= cscript/cspython
#PLUGINS+= cscript/cslua
PLUGINS+= sound/renderer/software
PLUGINS+= video/canvas/ddraw
#PLUGINS+= video/canvas/ddraw8
PLUGINS+= sound/loader/mp3
PLUGINS+=net/driver/ensocket
PLUGINS.DYNAMIC += mesh/impexp/3ds
PLUGINS.DYNAMIC += font/server/freefnt2
PLUGINS+=video/format/avi
PLUGINS+=video/format/codecs/opendivx
PLUGINS+=physics/odedynam
#PLUGINS+=video/loader/jng

# if u have the following line uncommented make sure one
# LIBS.OPENGL.SYSTEM is set below or you have a custom
# opengl dll installed as GL.dll (e.g. MESA)
PLUGINS+= video/canvas/openglwin video/renderer/opengl

# uncomment the line below to build the sound driver
PLUGINS+= sound/driver/waveoutsd

#--------------------------------------------------- rootdefines & defines ---#
ifneq (,$(findstring defines,$(MAKESECTION)))

.SUFFIXES: .exe .dll

# Processor type.
PROC=X86

# Operating system
OS=WIN32

# Compiler
COMP=GCC

# Command to update a target
UPD=libs\cssys\win32\winupd.bat $@ DEST

endif # ifneq (,$(findstring defines,$(MAKESECTION)))

#----------------------------------------------------------------- defines ---#
ifeq ($(MAKESECTION),defines)

include mk/dos.mak

# Typical object file extension
O=.o

# Typical prefix for library filenames
LIB_PREFIX=lib

# Extra libraries needed on this system (beside drivers)
LIBS.EXE= $(LFLAGS.l)gdi32

# OpenGL settings for use with OpenGL Drivers...untested
#SGI OPENGL SDK v1.1.1 for Win32
#LIBS.OPENGL.SYSTEM = $(LFLAGS.l)opengl $(LFLAGS.l)glut

# MS OpenGL
LIBS.OPENGL.SYSTEM=$(LFLAGS.l)opengl32 $(LFLAGS.l)glut32

# Socket library
LIBS.SOCKET.SYSTEM=$(LFLAGS.l)wsock32

# Sound library
LIBS.SOUND.SYSTEM=$(LFLAGS.l)dsound $(LFLAGS.l)winmm

# Python library
LIBS.CSPYTHON.SYSTEM=$(LFLAGS.l)python15

# Lua library
LIBS.CSLUA.SYSTEM=$(LFLAGS.l)lua $(LFLAGS.l)lualib

# Freetype library
LIBS.FREETYPE.SYSTEM=$(LFLAGS.l)ttf

# Where can the Zlib library be found on this system?
Z_LIBS=$(LFLAGS.l)z

# Where can the PNG library be found on this system?
PNG_LIBS=$(LFLAGS.l)png

# Where can the JPG library be found on this system?
JPG_LIBS=$(LFLAGS.l)jpeg

# Where can the MNG library be found on this system?
MNG_LIBS=$(LFLAGS.l)mng

# Where can the optional sound libraries be found on this system?
SOUND_LIBS=

# Indicate where special include files can be found.
# for instance where your dx includes are
CFLAGS.INCLUDE=
#$(CFLAGS.I)/dx7asdk/dxf/include

# General flags for the compiler which are used in any case.
CFLAGS.GENERAL=-Wall $(CFLAGS.SYSTEM) -fvtable-thunks -pipe

# Flags for the compiler which are used when optimizing.
CFLAGS.optimize=-s -O3 -ffast-math
ifneq ($(shell gcc --version),2.95.2)
# WARNING: mingw32 2.95.2 has a bug that causes incorrect code
# to be generated if you use -fomit-frame-pointer
CFLAGS.optimize += -fomit-frame-pointer
endif

# Flags for the compiler which are used when debugging.
CFLAGS.debug=-g

# Flags for the compiler which are used when profiling.
CFLAGS.profile=-pg -O -g

# Flags for the compiler which are used when building a shared library.
CFLAGS.DLL=

# General flags for the linker which are used in any case.
LFLAGS.GENERAL=

# Flags for the linker which are used when optimizing.
LFLAGS.optimize=-s

# Flags for the linker which are used when debugging.
LFLAGS.debug=-g

# Flags for the linker which are used when profiling.
LFLAGS.profile=-pg

# Flags for dllwrap in optimize mode
DFLAGS.optimize = -s

# Flags for dllwrap in debug mode
DFLAGS.debug = -g3

# Flags for the linker which are used when building a shared library.
LFLAGS.DLL=$(DFLAGS.$(MODE)) -q --no-export-all-symbols --dllname $*

# Typical extension for objects and static libraries
LIB=.a
define AR
  rm -f $@
  ar
endef
ARFLAGS=cr

# System-dependent flags to pass to NASM
NASMFLAGS.SYSTEM=-f win32 $(CFLAGS.D)EXTERNC_UNDERSCORE

# System dependent source files included into CSSYS library
SRC.SYS_CSSYS = \
  libs/cssys/win32/win32.cpp \
  libs/cssys/win32/dir.cpp \
  libs/cssys/win32/timing32.cpp \
  libs/cssys/win32/loadlib.cpp \
  libs/cssys/win32/instpath.cpp \
  libs/cssys/general/findlib.cpp \
  libs/cssys/general/getopt.cpp \
  libs/cssys/general/printf.cpp \
  libs/cssys/general/runloop.cpp \
  libs/cssys/general/sysinit.cpp

# The C compiler
CC=gcc -c

# The C++ compiler
CXX=g++ -c

# The linker.
LINK=gcc

# Command sequence for creating a directory.
# Note that directories will have forward slashes. Please
# make sure that this command accepts that (or use 'subst' first).
ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL))$(findstring cmd,$(SHELL))$(findstring CMD,$(SHELL)))
  MKDIR = mkdir $(subst /,\,$(@:/=))
else
  MKDIR = mkdir $@
endif

# Extra parameters for 'sed' which are used for doing 'make depend'.
SYS_SED_DEPEND=-e "s/\.ob*j*\:/\$$O:/g"

# Flags for linking a GUI and a console executable
LFLAGS.EXE=-mwindows
# commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
LFLAGS.EXE+=-mconsole
LFLAGS.CONSOLE.EXE=

# Use makedep to build dependencies
DEPEND_TOOL=mkdep

endif # ifeq ($(MAKESECTION),defines)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# If SHELL is the Windows COMMAND or CMD, then we need "bash" for scripts.
# Also, multiple commands are separated with "&" vs ";" w/ bash.
ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL))$(findstring cmd,$(SHELL))$(findstring CMD,$(SHELL)))
  RUN_SCRIPT = bash
  COMMAND_DELIM = &
else
  COMMAND_DELIM = ;
endif

# How to make shared libs for cs-config
LINK.PLUGIN=dllwrap
PLUGIN.POSTFLAGS=-mwindows -mconsole

# How to make a shared AKA dynamic library
#DO.SHARED.PLUGIN.CORE = \
#  dllwrap $(LFLAGS.DLL) $(LFLAGS.@) $(^^) $(L^) $(LIBS) $(LFLAGS) -mwindows 

# uncomment the following to enable workaround for dllwrap bug
DLLWRAPWRAP = $(RUN_SCRIPT) libs/cssys/win32/dllwrapwrap.sh

COMPILE_RES = $(RUN_SCRIPT) libs/cssys/win32/compres.sh
MAKEVERSIONINFO = $(RUN_SCRIPT) libs/cssys/win32/mkverres.sh

DO.SHARED.PLUGIN.CORE = \
  $(MAKEVERSIONINFO) $(OUT)$(@:$(DLL)=-version.rc) \
    "$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
  $(COMPILE_RES) $(MODE) $(OUT)$(@:$(DLL)=-rsrc.o) \
    $(OUT)$(@:$(DLL)=-version.rc) $(COMMAND_DELIM) \
  $(DLLWRAPWRAP) $* $(LFLAGS.DLL) $(LFLAGS.@) $(^^) \
    $(OUT)$(@:$(DLL)=-rsrc.o) $(L^) $(LIBS) $(LFLAGS) -mwindows
#DO.SHARED.PLUGIN.CORE = \
#  $(MAKEVERSIONINFO) $(OUT)$(@:$(DLL)=-version.rc) \
#    "$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
#  $(COMPILE_RES) $(MODE) $(OUT)$(@:$(DLL)=-rsrc.o) $($@.WINRSRC) \
#    $(OUT)$(@:$(DLL)=-version.rc) $(COMMAND_DELIM) \
#  $(DLLWRAPWRAP) $* $(LFLAGS.DLL) $(LFLAGS.@) $(^^) \
#    $(OUT)$(@:$(DLL)=-rsrc.o) $(L^) $(LIBS) $(LFLAGS) -mwindows

# Commenting out the following line will make the -noconsole option work
# but the only way to redirect output will be WITH -noconsole (wacky :-)
# and the console will not start minimized if a shortcut says it should
DO.SHARED.PLUGIN.CORE += -mconsole

DO.LINK.EXE = \
	$(MAKEVERSIONINFO) $(OUT)$(@:$(EXE)=-version.rc) \
	"$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
	$(COMPILE_RES) $(MODE) $(OUT)$(@:$(EXE)=-rsrc.o)  \
	$(OUT)$(@:$(EXE)=-version.rc) $(COMMAND_DELIM) \
	$(LINK) $(LFLAGS) $(LFLAGS.EXE) $(LFLAGS.@) $(^^) \
	$(OUT)$(@:$(EXE)=-rsrc.o) $(L^) $(LIBS) $(LIBS.EXE.PLATFORM)
#DO.LINK.EXE = \
#	$(MAKEVERSIONINFO) $(OUT)$(@:$(EXE)=-version.rc) \
#	"$(DESCRIPTION.$*)" $(COMMAND_DELIM) \
#	$(COMPILE_RES) $(MODE) $(OUT)$(@:$(EXE)=-rsrc.o) $($@.WINRSRC) \
#	$(OUT)$(@:$(EXE)=-version.rc) $(COMMAND_DELIM) \
#	$(LINK) $(LFLAGS) $(LFLAGS.EXE) $(LFLAGS.@) $(^^) \
#	$(OUT)$(@:$(EXE)=-rsrc.o) $(L^) $(LIBS) $(LIBS.EXE.PLATFORM)
DO.LINK.CONSOLE.EXE = $(DO.LINK.EXE)

endif # ifeq ($(MAKESECTION),postdefines)

#-------------------------------------------------------------- confighelp ---#
ifeq ($(MAKESECTION),confighelp)

ifneq (,$(findstring command,$(SHELL))$(findstring COMMAND,$(SHELL)))
"=
|=�
endif

SYSHELP += \
  $(NEWLINE)echo $"  make win32gcc     Prepare for building on $(DESCRIPTION.win32gcc)$"

endif # ifeq ($(MAKESECTION),confighelp)

#--------------------------------------------------------------- configure ---#
ifeq ($(ROOTCONFIG),config)

SYSCONFIG=libs\cssys\win32\winconf.bat mingw32

endif # ifeq ($(ROOTCONFIG),config)
