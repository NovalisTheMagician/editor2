APPLICATION := editor2

BUILD_DIR := build
SRC_DIR := src

DEFINES := 
INC_DIRS := $(SRC_DIR)

LIBS := m cimgui_sdl nfd SDL2 glad stdc++
LIB_DIRS := 

CC := gcc
CCFLAGS := -Wall -std=gnu17 -Wstrict-prototypes

LD := gcc
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
    ifeq ($(CONFIG),release)
        LDFLAGS += -mwindows
    else
        LDFLAGS += -mconsole
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBS += asound gtk-3 gdk-3 gdk_pixbuf-2.0 pangocairo-1.0 pango-1.0 cairo gobject-2.0 gmodule-2.0 glib-2.0 Xext X11 GL
        DEFINES += 
    endif
    ifeq ($(UNAME_S),Darwin)
    endif
    ifeq ($(UNAME_S),FreeBSD)
    endif
endif

CPPFLAGS := $(addprefix -I,$(INC_DIRS)) $(addprefix -D,$(DEFINES)) -MMD -MP
LIB_FLAGS := $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(BUILD_DIR) $(APPLICATION)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(APPLICATION): $(OBJS)
	@echo "LD $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIB_FLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	@echo "CC $<"
	@$(CC) $(CPPFLAGS) $(CCFLAGS) -c $< -o $@

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

-include $(OBJS:.o=.d)

