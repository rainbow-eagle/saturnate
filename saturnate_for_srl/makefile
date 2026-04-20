# Configuration
SRL_MALLOC_MEMORY = 204800      # How much memory malloc can use (in bytes)
SRL_MAX_TEXTURES = 100          # Number of VDP1 texture slots
SRL_MODE = NTSC                 # Valid options are PAL or NTSC
SRL_HIGH_RES = 0                # 480i mode
SRL_FRAMERATE = 0               # Framerate control (0=dynamic, 1=< 60/value)
SRL_MAX_CD_BACKGROUND_JOBS = 1  # Maximum number of files GFS can open at once
SRL_MAX_CD_FILES = 256          # Maximum number of files on a CD
SRL_MAX_CD_RETRIES = 5          # Number of times to retry on unsuccessful read

# Sound driver specific configuration
SRL_USE_SGL_SOUND_DRIVER = 0    # Set to 1 if you want to use SGL sound driver, this will copy necessary files into the CD folder
SRL_ENABLE_FREQ_ANALYSIS = 0    # Set to 1 if you want to enable frequency analysis for CD audio, this will load a DSP program into effect slot 1, SGL sound driver must be enabled

# SGL configuration
SGL_MAX_VERTICES = 2500         # Number of vertices that can be used
SGL_MAX_POLYGONS = 1500         # Number of polygons that can be used
SGL_MAX_EVENTS = 1             # Number of events that can be used
SGL_MAX_WORKS = 1             # Number of works that can be used 

# Disk name
CD_NAME = saturnate

# Directory build will be placed into
BUILD_DROP = ./BuildDrop

# SRL installation directory
SRL_INSTALL_ROOT ?= ../..

# Find all .c and .cxx files
SOURCES = $(patsubst ./%,%,$(shell find src/ -name '*.c')) 
SOURCES += $(patsubst ./%,%,$(shell find src/ -name '*.cxx'))

# Include shared makefile
SDK_ROOT = $(SRL_INSTALL_ROOT)/saturnringlib

CCFLAGS += -DTT_DEBUG_MODE

include $(SDK_ROOT)/shared.mk

# ------------------------------------------------------------
# Convert 3D models from .blend to .obj to .NYA and from .blend to .MOT
#
# All .obj files located in assets/ are converted to .NYA and .MOT,
# and the output is placed in srl's asset directory.
# E.g. assets/MODEL.obj produces cd/data/MODEL.NYA and cd/data/MODEL.MOT
# E.g. assets/model.obj becomes cd/data/model.NYA and cd/data/model.MOT
# but it would fail at runtime because you should use uppercase file names.
# ------------------------------------------------------------

ASSETDIR := assets
CDASSETDIR := $(ASSETS_DIR)
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
OBJ2NYA := dotnet --roll-forward LatestMajor $(TOOLDIR)/modelConverter/ModelConverter.dll --no-unwrap
BLEND2OBJ := blender --background --python $(TOOLDIR)/exportBlenderWavefront.py --
BLEND2MOT := blender --background --python $(TOOLDIR)/exportBlenderAnimationAng.py -- --reverse

assets: nya mot

nya: $(ASSETSNYA)

mot: $(MOT_FROM_BLEND)

$(CDASSETDIR)/%.NYA: $(ASSETDIR)/%.obj
	$(OBJ2NYA) -i $< -o $@

$(ASSETDIR)/%.obj: $(ASSETDIR)/%.blend
	$(BLEND2OBJ) $< $@

$(CDASSETDIR)/%.MOT: $(ASSETDIR)/%.blend
	$(BLEND2MOT) $< $@ || true

clean-assets:
	$(RM) $(ASSETSNYA) $(OBJ_MTL_FROM_BLEND) $(MOT_FROM_BLEND)

