# Plug-in description
DESCRIPTION.mp3 = Crystal Space mp3 sound loader

#------------------------------------------------------------- rootdefines ---#
ifeq ($(MAKESECTION),rootdefines)

# Plug-in-specific help commands
PLUGINHELP += \
  $(NEWLINE)echo $"  make mp3         Make the $(DESCRIPTION.mp3)$"

endif # ifeq ($(MAKESECTION),rootdefines)

#------------------------------------------------------------- roottargets ---#
ifeq ($(MAKESECTION),roottargets)

.PHONY: mp3 mp3clean
all plugins drivers snddrivers: mp3

mp3:
	$(MAKE_TARGET) MAKE_DLL=yes
mp3clean:
	$(MAKE_CLEAN)

endif # ifeq ($(MAKESECTION),roottargets)

#------------------------------------------------------------- postdefines ---#
ifeq ($(MAKESECTION),postdefines)

vpath %.cpp plugins/sound/loader/mp3 plugins/sound/loader/mp3/mpg123
ifneq ($(OS),WIN32)
  vpath %.cpp plugins/sound/loader/mp3/mpg123/xfermem
endif

ifeq ($(USE_PLUGINS),yes)
  MP3 = $(OUTDLL)sndmp3$(DLL)
  LIB.MP3 = $(foreach d,$(DEP.MP3),$($d.LIB))
  TO_INSTALL.DYNAMIC_LIBS += $(MP3)
else
  MP3 = $(OUT)$(LIB_PREFIX)sndmp3$(LIB)
  DEP.EXE += $(MP3)
  SCF.STATIC += sndmp3
  TO_INSTALL.STATIC_LIBS += $(MP3)
endif

MP3_USE_XFERMEM = yes
ifeq ($(OS),WIN32)
  MP3_USE_XFERMEM = no
endif
ifeq ($(DO_MSVCGEN),yes)
  MP3_USE_XFERMEM = no
endif

ifeq ($(MP3_USE_XFERMEM),yes)
  INC.MP3 = $(wildcard \
    plugins/sound/loader/mp3/*.h \
    plugins/sound/loader/mp3/mpg123/*.h \
    plugins/sound/loader/mp3/mpg123/xfermem/*.h)
  SRC.MP3 = $(wildcard \
    plugins/sound/loader/mp3/*.cpp \
    plugins/sound/loader/mp3/mpg123/*.cpp \
    plugins/sound/loader/mp3/mpg123/xfermem/*.cpp)
else
  INC.MP3 = $(wildcard \
    plugins/sound/loader/mp3/*.h \
    plugins/sound/loader/mp3/mpg123/*.h)
  SRC.MP3 = $(wildcard \
    plugins/sound/loader/mp3/*.cpp \
    plugins/sound/loader/mp3/mpg123/*.cpp)
endif

OBJ.MP3 = $(addprefix $(OUT),$(notdir $(SRC.MP3:.cpp=$O)))
DEP.MP3 = CSUTIL CSSYS CSUTIL

MSVC.DSP += MP3
DSP.MP3.NAME = sndmp3
DSP.MP3.TYPE = plugin

endif # ifeq ($(MAKESECTION),postdefines)

#----------------------------------------------------------------- targets ---#
ifeq ($(MAKESECTION),targets)

.PHONY: mp3 mp3clean

mp3: $(OUTDIRS) $(MP3)

$(MP3): $(OBJ.MP3) $(LIB.MP3)
	$(DO.PLUGIN)

clean: mp3clean
mp3clean:
	$(RM) $(MP3) $(OBJ.MP3)

ifdef DO_DEPEND
dep: $(OUTOS)sndmp3.dep
$(OUTOS)sndmp3.dep: $(SRC.MP3)
	$(DO.DEP)
else
-include $(OUTOS)sndmp3.dep
endif

endif # ifeq ($(MAKESECTION),targets)
