# This is a subinclude file used to define the rules needed
# to build the 3D infinite rendering driver -- infinite

# Driver description
DESCRIPTION.infinite = Crystal Space infinite 3D driver

#-------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Driver-specific help commands
DRIVERHELP += $(NEWLINE)echo $"  make infinite     Make the $(DESCRIPTION.infinite)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#-------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: infinite

all plugins drivers drivers3d: infinite

infinite:
	$(MAKE_TARGET) MAKE_DLL=yes
infiniteclean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#-------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp libs/cs3d/inf

ifeq ($(USE_SHARED_PLUGINS),yes)
  INF3D=$(OUTDLL)inf3d$(DLL)
  DEP.INF3D=$(CSGEOM.LIB) $(CSGFXLDR.LIB) $(CSUTIL.LIB) $(CSSYS.LIB)
else
  INF3D=$(OUT)$(LIB_PREFIX)inf$(LIB)
  DEP.EXE+=$(INF3D)
  CFLAGS.STATIC_SCF+=$(CFLAGS.D)SCL_INF3D
endif
DESCRIPTION.$(INF3D) = $(DESCRIPTION.infinite)
SRC.INF3D = $(wildcard libs/cs3d/inf/*.cpp) \
  libs/cs3d/common/txtmgr.cpp libs/cs3d/common/dtmesh.cpp \
  libs/cs3d/common/dpmesh.cpp $(wildcard $(SRC.COMMON.DRV2D))
OBJ.INF3D = $(addprefix $(OUT),$(notdir $(subst .asm,$O,$(SRC.INF3D:.cpp=$O))))

endif # ifeq ($(MAKESECTION),postdefines)

#------------------------------------------------------------------ targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: infinite infiniteclean

# Chain rules
all: $(INF3D)
clean: infiniteclean

infinite: $(OUTDIRS) $(INF3D)

# Extra dependencies not generated by "make depend"
$(INF3D): $(OBJ.INF3D) $(DEP.INF3D)
	$(DO.PLUGIN)

infiniteclean:
	$(RM) $(INF3D) $(OBJ.INF3D)

ifdef DO_DEPEND
dep: $(OUTOS)inf3d.dep
$(OUTOS)inf3d.dep: $(SRC.INF3D)
	$(DO.DEP)
else
-include $(OUTOS)inf3d.dep
endif

endif # ifeq ($(MAKESECTION),targets)
