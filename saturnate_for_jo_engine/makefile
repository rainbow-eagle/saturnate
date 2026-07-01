JO_COMPILE_USING_SGL=1

JO_ENGINE_SRC_DIR=../../jo_engine
COMPILER_DIR=../../Compiler

include $(COMPILER_DIR)/COMMON/jo_engine_makefile

#TT_MOT is the version of the animation engine
TT_MOT ?= 1
SYSOBJS =
CCFLAGS += -DTT_DEBUG_MODE -DTT_MOT=$(TT_MOT)
SRCS := $(wildcard src/debug/*.c) \
        $(wildcard src/*.c) \
        $(wildcard src/core/*.c) \
        $(wildcard src/core/format/*.c) \
        $(wildcard src/pool_engine/*.c) \
        $(wildcard src/pool_engine/format/*.c)
OBJS := $(SRCS:.c=.o)

.c.o:
	$(CC) $(CCFLAGS) $(DFLAGS) -c $< -o $@

$(TARGET) : $(SYSOBJS) $(OBJS) $(MAKEFILE) $(LDFILE)
	$(CC) $(LDFLAGS) $(SYSOBJS) $(OBJS) $(LIBS) -o $@

# ------------------------------------------------------------
# Convert 3D models from .blend to .obj to .NYA and from .blend to .MOT
#
# All .obj files located in assets/ are converted to .NYA and .MOT,
# and the output is placed in cd/ASSETS.
# E.g. assets/MODEL.obj produces cd/ASSETS/MODEL.NYA and cd/ASSETS/MODEL.MOT
# E.g. assets/model.obj becomes cd/ASSETS/model.NYA and cd/ASSETS/model.MOT
# but it would fail at runtime because you should use uppercase file names.
# ------------------------------------------------------------

ASSETDIR ?= assets
CDDIR ?= cd
CDASSETDIR ?= $(CDDIR)/ASSETSz
CDASSETDIR ?= $(CDDIR)/ASSETS
CDASSETDIR := $(CDDIR)/ASSETS
SRC_BLEND := $(wildcard $(ASSETDIR)/*.blend)
SRC_OBJ   := $(wildcard $(ASSETDIR)/*.obj)
NYA_FROM_BLEND := $(SRC_BLEND:$(ASSETDIR)/%.blend=$(CDASSETDIR)/%.NYA)
NYA_FROM_OBJ := $(SRC_OBJ:$(ASSETDIR)/%.obj=$(CDASSETDIR)/%.NYA)
MOT_FROM_BLEND := $(SRC_BLEND:$(ASSETDIR)/%.blend=$(CDASSETDIR)/%.MOT)
OBJ_MTL_FROM_BLEND := $(SRC_BLEND:$(ASSETDIR)/%.blend=$(ASSETDIR)/%.obj) \
                $(SRC_BLEND:$(ASSETDIR)/%.blend=$(ASSETDIR)/%.mtl)
ASSETSNYA := $(NYA_FROM_BLEND) $(NYA_FROM_OBJ)
ASSETSMOT := $(SRC_BLEND:$(ASSETDIR)/%.blend=$(CDASSETDIR)/%.NYA)

TOOLDIR := tools
#OBJ2NYA := $(TOOLDIR)/modelConverter/ModelConverter --no-unwrap
OBJ2NYA := dotnet --roll-forward LatestMajor $(TOOLDIR)/modelConverter/ModelConverter.dll --no-unwrap
BLEND2OBJ := blender --background --python $(TOOLDIR)/exportBlenderWavefront.py --
BLEND2MOT := blender --background --factory-startup --debug-all --python $(TOOLDIR)/exportBlenderAnimationAng$(TT_MOT).py -- --reverse

assets: nya mot

nya: $(ASSETSNYA)

mot: $(MOT_FROM_BLEND)

$(CDASSETDIR)/%.NYA: $(ASSETDIR)/%.obj
	$(OBJ2NYA) -i $< -o $@

$(ASSETDIR)/%.obj: $(ASSETDIR)/%.blend
	$(BLEND2OBJ) $< $@

$(CDASSETDIR)/%.MOT: $(ASSETDIR)/%.blend
	-$(BLEND2MOT) $< $@


clean-assets:
	$(RM) $(ASSETSNYA) $(OBJ_MTL_FROM_BLEND) $(MOT_FROM_BLEND)
