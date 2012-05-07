.LIBPATTERNS = lib%.a lib%.dylib lib%.so
vpath %.a /usr/local/lib

vpath %.o build/

ARCH = -arch x86_64

CC         =  gcc
C_FLAGS    += -Wall -Wno-missing-braces -Wno-unused-function
C_FLAGS    += -std=gnu99
C_FLAGS    += -I"/usr/X11/include"
C_FLAGS    += -I"/usr/local/include"
C_FLAGS    += -I"./Dependencies/"
C_FLAGS    += -ggdb
C_FLAGS    += -O0
C_FLAGS    += -DTWODEEDENG_DEBUG
C_FLAGS    += $(shell sdl-config --cflags)
C_FLAGS    += $(ARCH)

LDFLAGS  += -lc
LDFLAGS  += -lz
LDFLAGS  += -L.
LDFLAGS  += -L/usr/X11/lib
LDFLAGS  += -framework Cocoa
LDFLAGS  += -framework OpenGL
LDFLAGS  += -framework Accelerate
LDFLAGS  += -framework OpenAL
LDFLAGS  += -framework AudioToolbox
LDFLAGS  += $(ARCH)

ENGINE_DYNAMICLIBS := -logg -lvorbis -lvorbisfile

STATICLIBS := -logg -lvorbis -lvorbisfile

ENGINE_SOURCE := $(wildcard Dependencies/*/*.c) \
Source/array.c \
Source/background.c \
Source/dictionary.c \
Source/drawutils.c \
Source/gametimer.c \
Source/input.c \
Source/json.c \
Source/linkedlist.c \
Source/networking.c \
Source/object.c \
Source/ogg_loader.c \
Source/png_loader.c \
Source/primitive_types.c \
Source/renderer.c \
Source/scene.c \
Source/shader.c \
Source/sprite.c \
Source/texture.c \
Source/texture_atlas.c \
Source/tmx_map.c \
Source/util.c \
Source/sound_apple.m \


ENGINE_OBJ    := $(addprefix build/,$(addsuffix .o,$(ENGINE_SOURCE)))
ENGINE_DYLIB  := libdynamo.dylib

override CFLAGS := $(CFLAGS) $(C_FLAGS)

all: link

build/%.o: %
	@echo Building $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm $(TEST_BIN) $(TEST_OBJ) $(ENGINE_DYLIB) $(ENGINE_OBJ)

link: $(ENGINE_OBJ)
	@echo "Linking Dynamo library"
	@$(CC) -o $(ENGINE_DYLIB) -dynamiclib $(ENGINE_DYNAMICLIBS) $(LDFLAGS) $^
