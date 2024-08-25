APPLICATION := editor2

BUILD_DIR := build
SRC_DIR := src
SRC_SUBDIRS := windows dialogs utils map

DEFINES := __USE_XOPEN _GNU_SOURCE CGLM_USE_ANONYMOUS_STRUCT=1
INC_DIRS := $(SRC_DIR)

LIBS := m cimgui_sdl SDL2 glad2 igfd re ftp json-c triangulate stdc++
LIB_DIRS := 

ifdef CC
CC := $(CC)
else
CC := gcc
endif

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

ifeq ($(OS),Windows_NT)
    LIBS += dinput8 dxguid dxerr8 user32 gdi32 winmm imm32 ole32 oleaut32 shell32 setupapi version uuid ws2_32 Iphlpapi comctl32 gdi32 comdlg32 opengl32
    APPLICATION := $(APPLICATION).exe
    LIB_DIRS += $(LIB_GCC_PATH)
    INC_DIRS += $(INC_PATH)
    LDFLAGS += -static
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
        DEFINES += 
    endif
    ifeq ($(UNAME_S),Darwin)
    endif
    ifeq ($(UNAME_S),FreeBSD)
    endif
endif

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) $(addprefix -D,$(DEFINES)) -MMD -MP
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

# SRCS := $(wildcard $(SRC_DIR)/*.c)
SRCS := $(wildcard $(SRC_DIR)/*.c) $(foreach pat,$(SRC_SUBDIRS),$(wildcard $(SRC_DIR)/$(pat)/*.c))
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

RES_DIR := resources
RES_PATH := $(SRC_DIR)/$(RES_DIR)

RES_SRC := $(SRC_DIR)/$(RES_DIR)/resources.c
RES_OBJ := $(BUILD_DIR)/$(RES_DIR)/resources.o
RESOURCES := $(wildcard $(RES_PATH)/*.ttf)

BUILD_DIRS := $(addprefix $(BUILD_DIR)/,$(SRC_SUBDIRS)) $(BUILD_DIR)/$(RES_DIR)

all: $(BUILD_DIR) $(APPLICATION)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIRS)

$(APPLICATION): $(OBJS) $(RES_OBJ)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(RES_OBJ) $(LIB_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	@echo "CC $<"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

$(RES_OBJ): $(RES_SRC) $(RESOURCES) Makefile
	@echo "CC $< (Resources)"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -I$(RES_PATH) -c $< -o $@

.PHONY: clean echo
clean:
	@echo "RM $(BUILD_DIR)/"
	@rm -rf $(BUILD_DIR)
	@echo "RM $(APPLICATION)"
	@rm -f $(APPLICATION)
	@echo "RM ini files"
	@rm -f *.ini

echo:
	@echo "LIBS= $(LIBS)"
	@echo "INC_DIRS= $(INC_DIRS)"
	@echo "CCFLAGS= $(CCFLAGS)"
	@echo "LDFLAGS= $(LDFLAGS)"

-include $(OBJS:.o=.d)
