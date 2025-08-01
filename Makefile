APPLICATION := editor2

BUILD_DIR := build
SRC_DIR := src
SRC_SUBDIRS := windows dialogs utils map scripts asset_sources

DEFINES := __USE_XOPEN _GNU_SOURCE
INC_DIRS := $(SRC_DIR)

LIBS := m SDL2 stdc++
LIB_DIRS :=

RES := windres

#CC := gcc
C++ := g++

CCFLAGS := -Wall -Wextra -std=gnu23 -Wstrict-prototypes

LD := $(CC)
LDFLAGS := -pthread -fuse-ld=gold

ifeq ($(CONFIG),release)
    DEFINES += NDEBUG
    CCFLAGS += -O2
else
    DEFINES += _DEBUG
    CCFLAGS += -g
endif

SDL_INC :=

ifeq ($(OS),Windows_NT)
    LIBS += dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 setupapi version uuid ws2_32 Iphlpapi comctl32 gdi32 comdlg32 opengl32
    APPLICATION := $(APPLICATION).exe
    LIB_DIRS += $(LIB_GCC_PATH)
    INC_DIRS += $(INC_PATH)
    LDFLAGS += -static
    SDL_INC += -I$(INC_PATH)/SDL2
    ifeq ($(CONFIG),release)
        LDFLAGS += -mwindows
    else
        LDFLAGS += -mconsole
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBS += GL dl
        CCFLAGS += $(shell pkg-config --cflags sdl2) -fsanitize=address
        LDFLAGS += $(shell pkg-config --libs sdl2) -fsanitize=address
        SDL_INC += $(shell pkg-config --cflags sdl2)
        DEFINES +=
    endif
    ifeq ($(UNAME_S),Darwin)
    endif
    ifeq ($(UNAME_S),FreeBSD)
    endif
endif

# SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(wildcard $(SRC_DIR)/*.c) $(foreach pat,$(SRC_SUBDIRS),$(wildcard $(SRC_DIR)/$(pat)/*.c))
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

RES_DIR := resources
RES_PATH := $(SRC_DIR)/$(RES_DIR)

RES_SRC := $(SRC_DIR)/$(RES_DIR)/resources.c
RES_OBJ := $(BUILD_DIR)/$(RES_DIR)/resources.o
RESOURCES := $(wildcard $(RES_PATH)/*.ttf) $(wildcard $(RES_PATH)/*.vs) $(wildcard $(RES_PATH)/*.fs) # TODO add more resources as dependencies (e.g. images, etc...)

BUILD_DIRS := $(addprefix $(BUILD_DIR)/,$(SRC_SUBDIRS)) $(BUILD_DIR)/$(RES_DIR)

# 3rd party libraries

EXTERN_DIR := extern

# GLAD
GLAD_DIR := $(EXTERN_DIR)/glad
GLAD_SRC := $(GLAD_DIR)/src/gl.c
GLAD_OBJ := $(BUILD_DIR)/$(GLAD_DIR)/gl.o

INC_DIRS += $(GLAD_DIR)/include
BUILD_DIRS += $(BUILD_DIR)/$(GLAD_DIR)

# tsoding
INC_DIRS += $(EXTERN_DIR)/tsoding

# tiny regular expression
RE_DIR := $(EXTERN_DIR)/re
RE_SRC := $(RE_DIR)/re.c
RE_OBJ := $(BUILD_DIR)/$(RE_DIR)/re.o

INC_DIRS += $(RE_DIR)
BUILD_DIRS += $(BUILD_DIR)/$(RE_DIR)

# imguifiledialog
IGFD_DIR := $(EXTERN_DIR)/imguifiledialog
IGFD_SRC := $(IGFD_DIR)/ImGuiFileDialog.cpp
IGFD_OBJ := $(BUILD_DIR)/$(IGFD_DIR)/ImGuiFileDialog.o

INC_DIRS += $(IGFD_DIR)
BUILD_DIRS += $(BUILD_DIR)/$(IGFD_DIR)

# cimgui
CIMGUI_DIR := $(EXTERN_DIR)/cimgui
CIMGUI_SRCS := $(CIMGUI_DIR)/cimgui.cpp
CIMGUI_SRCS += $(CIMGUI_DIR)/imgui/backends/imgui_impl_sdl2.cpp $(CIMGUI_DIR)/imgui/backends/imgui_impl_opengl3.cpp
CIMGUI_SRCS += $(CIMGUI_DIR)/imgui/imgui.cpp $(CIMGUI_DIR)/imgui/imgui_demo.cpp $(CIMGUI_DIR)/imgui/imgui_draw.cpp $(CIMGUI_DIR)/imgui/imgui_tables.cpp $(CIMGUI_DIR)/imgui/imgui_widgets.cpp
CIMGUI_OBJS := $(patsubst $(CIMGUI_DIR)/%.cpp, $(BUILD_DIR)/$(CIMGUI_DIR)/%.o, $(CIMGUI_SRCS))

INC_DIRS += $(CIMGUI_DIR) $(CIMGUI_DIR)/generator/output $(CIMGUI_DIR)/imgui
BUILD_DIRS += $(BUILD_DIR)/$(CIMGUI_DIR) $(BUILD_DIR)/$(CIMGUI_DIR)/imgui $(BUILD_DIR)/$(CIMGUI_DIR)/imgui/backends

# triang
TRIANG_DIR := $(EXTERN_DIR)/triangulate
TRIANG_SRC := $(TRIANG_DIR)/triangulate.cpp
TRIANG_OBJ := $(BUILD_DIR)/$(TRIANG_DIR)/triangulate.o

INC_DIRS += $(TRIANG_DIR)
BUILD_DIRS += $(BUILD_DIR)/$(TRIANG_DIR)

EARCUT_INC := $(TRIANG_DIR)/earcut/include

# incbin
INCBIN_DIR := $(EXTERN_DIR)/incbin
INC_DIRS += $(INCBIN_DIR)

# cglm
CGLM_DIR := $(EXTERN_DIR)/cglm
INC_DIRS += $(CGLM_DIR)/include

# lua
LUA_DIR := $(EXTERN_DIR)/lua-5.4.8
LUA_SRCS := $(LUA_DIR)/src/lapi.c $(LUA_DIR)/src/lcode.c $(LUA_DIR)/src/lctype.c $(LUA_DIR)/src/ldebug.c $(LUA_DIR)/src/ldo.c $(LUA_DIR)/src/ldump.c $(LUA_DIR)/src/lfunc.c $(LUA_DIR)/src/lgc.c $(LUA_DIR)/src/llex.c $(LUA_DIR)/src/lmem.c $(LUA_DIR)/src/lobject.c $(LUA_DIR)/src/lopcodes.c $(LUA_DIR)/src/lparser.c $(LUA_DIR)/src/lstate.c $(LUA_DIR)/src/lstring.c $(LUA_DIR)/src/ltable.c $(LUA_DIR)/src/ltm.c $(LUA_DIR)/src/lundump.c $(LUA_DIR)/src/lvm.c $(LUA_DIR)/src/lzio.c
LUA_SRCS += $(LUA_DIR)/src/lauxlib.c $(LUA_DIR)/src/lbaselib.c $(LUA_DIR)/src/lcorolib.c $(LUA_DIR)/src/ldblib.c $(LUA_DIR)/src/liolib.c $(LUA_DIR)/src/lmathlib.c $(LUA_DIR)/src/loadlib.c $(LUA_DIR)/src/loslib.c $(LUA_DIR)/src/lstrlib.c $(LUA_DIR)/src/ltablib.c $(LUA_DIR)/src/lutf8lib.c $(LUA_DIR)/src/linit.c
LUA_OBJS := $(patsubst $(LUA_DIR)/src/%.c, $(BUILD_DIR)/$(LUA_DIR)/%.o, $(LUA_SRCS))

INC_DIRS += $(LUA_DIR)/src
BUILD_DIRS += $(BUILD_DIR)/$(LUA_DIR)

# ftplib
FTP_DIR := $(EXTERN_DIR)/ftplib-4.0-1
FTP_SRC := $(FTP_DIR)/src/ftplib.c
FTP_OBJ := $(BUILD_DIR)/$(FTP_DIR)/ftplib.o

INC_DIRS += $(FTP_DIR)/src
BUILD_DIRS += $(BUILD_DIR)/$(FTP_DIR)

# stb
INC_DIR := $(EXTERN_DIR)/stb

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) $(addprefix -D,$(DEFINES)) -MMD -MP
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

RC_SRC :=
RC_OBJ :=
ifeq ($(OS),Windows_NT)
    RC_SRC += $(SRC_DIR)/$(RES_DIR)/resource.rc
    RC_OBJ += $(BUILD_DIR)/$(RES_DIR)/resource.o
endif

all: $(BUILD_DIRS) $(APPLICATION)

$(BUILD_DIRS):
	@echo "MD $@"
	@mkdir -p $@

$(APPLICATION): $(OBJS) $(RES_OBJ) $(RC_OBJ) $(GLAD_OBJ) $(RE_OBJ) $(IGFD_OBJ) $(CIMGUI_OBJS) $(TRIANG_OBJ) $(LUA_OBJS) $(FTP_OBJ)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIB_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	@echo "CC $<"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

$(RES_OBJ): $(RES_SRC) $(RESOURCES) Makefile
	@echo "CC $< (Resources)"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -I$(RES_PATH) -c $< -o $@

$(RC_OBJ): $(RC_SRC)
	@echo "RES $<"
	@$(RES) $< $@

$(GLAD_OBJ): $(GLAD_SRC)
	@echo "CC $< (External GLAD)"
	@$(CC) -O2 -I$(GLAD_DIR)/include -c $< -o $@

$(RE_OBJ): $(RE_SRC)
	@echo "CC $< (External RE)"
	@$(CC) -O2 -c $< -o $@

$(IGFD_OBJ): $(IGFD_SRC)
	@echo "++ $< (External IGFD)"
	@$(C++) -O2 -c $< -o $@ -I$(CIMGUI_DIR)/imgui -DUSE_PLACES_FEATURE -DUSE_PLACES_DEVICES -DUSE_PLACES_BOOKMARKS

$(BUILD_DIR)/$(CIMGUI_DIR)/%.o: $(CIMGUI_DIR)/%.cpp
	@echo "++ $< (External CImgui)"
	@$(C++) -O2 -c $< -o $@ -I$(CIMGUI_DIR)/imgui $(SDL_INC) '-DIMGUI_IMPL_API=extern "C"'

$(TRIANG_OBJ): $(TRIANG_SRC)
	@echo "++ $< (External Triangulate)"
	@$(C++) -O2 -c $< -o $@ -I$(EARCUT_INC)

$(BUILD_DIR)/$(LUA_DIR)/%.o: $(LUA_DIR)/src/%.c
	@echo "CC $< (External Lua)"
	@$(CC) -O2 -c $< -o $@ -DLUA_COMPAT_5_3

$(FTP_OBJ): $(FTP_SRC)
	@echo "CC $< (External ftplib)"
	@$(CC) -O2 -c $< -o $@ -D_FILE_OFFSET_BITS=64

.PHONY: clean echo
clean:
	@echo "RM $(BUILD_DIR)/"
	@rm -rf $(BUILD_DIR)
	@echo "RM $(APPLICATION)"
	@rm -f $(APPLICATION)

echo:
	@echo "LIBS= $(LIBS)"
	@echo "INC_DIRS= $(INC_DIRS)"
	@echo "CCFLAGS= $(CCFLAGS)"
	@echo "LDFLAGS= $(LDFLAGS)"

-include $(OBJS:.o=.d)
