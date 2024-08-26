APPLICATION := editor2

BUILD_DIR := build
SRC_DIR := src
SRC_SUBDIRS := windows dialogs utils map

DEFINES := __USE_XOPEN _GNU_SOURCE
INC_DIRS := $(SRC_DIR)

LIBS := m SDL2 ftp json-c stdc++
LIB_DIRS :=

CC := gcc

CCFLAGS := -Wall -std=gnu2x -Wstrict-prototypes

LD := $(CC)
LDFLAGS := -pthread

ifeq ($(CONFIG),release)
    DEFINES += NDEBUG
    CCFLAGS += -O2
else
    DEFINES += _DEBUG
    CCFLAGS += -g
endif

SDL_INC :=
EARCUT_INC :=

ifeq ($(OS),Windows_NT)
    LIBS += dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 setupapi version uuid ws2_32 Iphlpapi comctl32 gdi32 comdlg32 opengl32
    APPLICATION := $(APPLICATION).exe
    LIB_DIRS += $(LIB_GCC_PATH)
    INC_DIRS += $(INC_PATH)
    LDFLAGS += -static
    SDL_INC += -I$(INC_PATH)/SDL2
    EARCUT_INC += -I$(INC_PATH)
    ifeq ($(CONFIG),release)
        LDFLAGS += -mwindows
    else
        LDFLAGS += -mconsole
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBS += GL dl
        CCFLAGS += $(shell pkg-config --cflags sdl2)
        LDFLAGS += $(shell pkg-config --libs sdl2)
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

VENDOR_DIR := extern

# GLAD
GLAD_DIR := $(VENDOR_DIR)/glad
GLAD_SRC := $(GLAD_DIR)/src/gl.c
GLAD_OBJ := $(BUILD_DIR)/$(GLAD_DIR)/gl.o

INC_DIRS += $(GLAD_DIR)/include
BUILD_DIRS += $(BUILD_DIR)/$(GLAD_DIR)

# tiny regular expression
RE_DIR := $(VENDOR_DIR)/re
RE_SRC := $(RE_DIR)/re.c
RE_OBJ := $(BUILD_DIR)/$(RE_DIR)/re.o

INC_DIRS += $(RE_DIR)
BUILD_DIRS += $(BUILD_DIR)/$(RE_DIR)

# imguifiledialog
IGFD_DIR := $(VENDOR_DIR)/imguifiledialog
IGFD_SRC := $(IGFD_DIR)/ImGuiFileDialog.cpp
IGFD_OBJ := $(BUILD_DIR)/$(IGFD_DIR)/ImGuiFileDialog.o

INC_DIRS += $(IGFD_DIR)
BUILD_DIRS += $(BUILD_DIR)/$(IGFD_DIR)

# cimgui
CIMGUI_DIR := $(VENDOR_DIR)/cimgui
CIMGUI_SRCS := $(CIMGUI_DIR)/cimgui.cpp
CIMGUI_SRCS += $(CIMGUI_DIR)/imgui/backends/imgui_impl_sdl2.cpp $(CIMGUI_DIR)/imgui/backends/imgui_impl_opengl3.cpp
CIMGUI_SRCS += $(CIMGUI_DIR)/imgui/imgui.cpp $(CIMGUI_DIR)/imgui/imgui_demo.cpp $(CIMGUI_DIR)/imgui/imgui_draw.cpp $(CIMGUI_DIR)/imgui/imgui_tables.cpp $(CIMGUI_DIR)/imgui/imgui_widgets.cpp
CIMGUI_OBJS := $(patsubst $(CIMGUI_DIR)/%.cpp, $(BUILD_DIR)/$(CIMGUI_DIR)/%.o, $(CIMGUI_SRCS))

INC_DIRS += $(CIMGUI_DIR) $(CIMGUI_DIR)/generator/output $(CIMGUI_DIR)/imgui
BUILD_DIRS += $(BUILD_DIR)/$(CIMGUI_DIR) $(BUILD_DIR)/$(CIMGUI_DIR)/imgui $(BUILD_DIR)/$(CIMGUI_DIR)/imgui/backends

# triang
TRIANG_DIR := $(VENDOR_DIR)/triangulate
TRIANG_SRC := $(TRIANG_DIR)/triangulate.cpp
TRIANG_OBJ := $(BUILD_DIR)/$(TRIANG_DIR)/triangulate.o

INC_DIRS += $(TRIANG_DIR)
BUILD_DIRS += $(BUILD_DIR)/$(TRIANG_DIR)

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) $(addprefix -D,$(DEFINES)) -MMD -MP
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

all: $(BUILD_DIRS) $(APPLICATION)

$(BUILD_DIRS):
	@echo "MD $@"
	@mkdir -p $@

$(APPLICATION): $(OBJS) $(RES_OBJ) $(GLAD_OBJ) $(RE_OBJ) $(IGFD_OBJ) $(CIMGUI_OBJS) $(TRIANG_OBJ)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -o $@ $^ $(LIB_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	@echo "CC $<"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

$(RES_OBJ): $(RES_SRC) $(RESOURCES) Makefile
	@echo "CC $< (Resources)"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -I$(RES_PATH) -c $< -o $@

$(GLAD_OBJ): $(GLAD_SRC)
	@echo "CC $< (External GLAD)"
	@$(CC) -O2 -I$(GLAD_DIR)/include -c $< -o $@

$(RE_OBJ): $(RE_SRC)
	@echo "CC $< (External RE)"
	@$(CC) -O2 -c $< -o $@

$(IGFD_OBJ): $(IGFD_SRC)
	@echo "++ $< (External IGFD)"
	@g++ -O2 -c $< -o $@ -I$(CIMGUI_DIR)/imgui

$(BUILD_DIR)/$(CIMGUI_DIR)/%.o: $(CIMGUI_DIR)/%.cpp
	@echo "++ $< (External CImgui)"
	@g++ -O2 -c $< -o $@ -I$(CIMGUI_DIR)/imgui $(SDL_INC) '-DIMGUI_IMPL_API=extern "C"'

$(TRIANG_OBJ): $(TRIANG_SRC)
	@echo "++ $< (External Triangulate)"
	@g++ -O2 -c $< -o $@ $(EARCUT_INC)

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
