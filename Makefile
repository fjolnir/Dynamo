# Makefile for OS X

.LIBPATTERNS = lib%.a lib%.dylib lib%.so
vpath %.a /usr/local/lib

vpath %.o build/

ARCH = -arch x86_64

CC         =  gcc
CFLAGS    += -Wall -Wno-missing-braces -Wno-unused-function
CFLAGS    += -std=gnu99
CFLAGS    += -I"/usr/X11/include"
CFLAGS    += -I"/usr/local/include"
CFLAGS    += -I"./Dependencies/"
CFLAGS    += -ggdb
CFLAGS    += -O0
CFLAGS    += -DDYNAMO_DEBUG
CFLAGS    += $(ARCH)

LDFLAGS  += -lc
LDFLAGS  += -lz
LDFLAGS  += -L.
LDFLAGS  += -L/usr/X11/lib
LDFLAGS  += -framework Cocoa
LDFLAGS  += -framework OpenGL
LDFLAGS  += -framework Accelerate
LDFLAGS  += -framework OpenAL
LDFLAGS  += -framework AudioToolbox
LDFLAGS  += -framework ApplicationServices

LDFLAGS  += $(ARCH)

DYLIBS := -logg -lvorbis -lvorbisfile

STATICLIBS := -logg -lvorbis -lvorbisfile

SOURCE := $(wildcard Dependencies/*/*.c) \
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
Source/sound_apple.m

OBJ    := $(addprefix build/,$(addsuffix .o,$(SOURCE)))
PRODUCT  := libdynamo.dylib

all: link

build/%.o: %
	@echo Building $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm $(TEST_BIN) $(TEST_OBJ) $(PRODUCT) $(OBJ)

link: $(OBJ)
	@echo "Linking Dynamo library"
	@$(CC) -o $(PRODUCT) -dynamiclib $(DYLIBS) $(LDFLAGS) $^
