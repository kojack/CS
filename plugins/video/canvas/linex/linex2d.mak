# This is a subinclude file used to define the rules needed
# to build the X-windows 2D driver -- linex2d

# Driver description
DESCRIPTION.linex2d = Crystal Space XLib 2D driver for Line3D

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += \
  $(NEWLINE)echo $"  make linex2d      Make the $(DESCRIPTION.linex2d)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: linex2d linex2dclean

all plugins drivers drivers2d: linex2d

linex2d:
	$(MAKE_TARGET) MAKE_DLL=yes
linex2dclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

# We need also the X libs
CFLAGS.LINEX2D+=-I$(X11_PATH)/include
LIBS.LINEX2D+=-L$(X11_PATH)/lib -lXext -lX11 $(X11_EXTRA_LIBS)

ifeq ($(USE_XFREE86VM),yes)
  CFLAGS.LINEX2D+=-DXFREE86VM
  LIBS.LINEX2D+=-lXxf86vm
endif
 
# The 2D Xlib driver
ifeq ($(USE_SHARED_PLUGINS),yes)
  LINEXLIB2D=$(OUTDLL)linex2d$(DLL)
  LIBS.LOCAL.LINEX2D=$(LIBS.LINEX2D)
  DEP.LINEX2D=$(CSGEOM.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  LINEXLIB2D=$(OUT)$(LIB_PREFIX)linex2d$(LIB)
  DEP.EXE+=$(LINEXLIB2D)
  LIBS.EXE+=$(LIBS.LINEX2D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_LINEX2D
endif
DESCRIPTION.$(LINEXLIB2D) = $(DESCRIPTION.linex2d)
SRC.LINEXLIB2D = \
  $(wildcard plugins/video/canvas/linex/*.cpp $(SRC.COMMON.DRV2D))
OBJ.LINEXLIB2D = $(addprefix $(OUT),$(notdir $(SRC.LINEXLIB2D:.cpp=$O)))

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: linex2d linelibxclean

# Chain rules
clean: linelibxclean

linex2d: $(OUTDIRS) $(LINEXLIB2D)

$(OUT)%$O: plugins/video/canvas/linex/%.cpp
	$(DO.COMPILE.CPP) $(CFLAGS.LINEX2D)
 
$(LINEXLIB2D): $(OBJ.LINEXLIB2D) $(DEP.LINEX2D)
	$(DO.PLUGIN) $(LIBS.LOCAL.LINEX2D)

linelibxclean:
	$(RM) $(LINEXLIB2D) $(OBJ.LINEXLIB2D) $(OUTOS)linex2d.dep

ifdef DO_DEPEND
dep: $(OUTOS)linex2d.dep
$(OUTOS)linex2d.dep: $(SRC.LINEXLIB2D)
	$(DO.DEP1) $(CFLAGS.LINEX2D) $(DO.DEP2)
else
-include $(OUTOS)linex2d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
