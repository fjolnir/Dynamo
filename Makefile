.LIBPATTERNS = lib%.a lib%.dylib lib%.so
vpath %.a /usr/local/lib

vpath %.o build/

ARCH = -arch x86_64

CC         =  gcc
C_FLAGS    += -Wall -Wno-missing-braces -Wno-unused-function
C_FLAGS    += -std=gnu99
C_FLAGS    += -I"/usr/X11/include"
C_FLAGS    += -I"/usr/local/include"
C_FLAGS    += -I"./engine/"
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
LDFLAGS  += $(ARCH)

ENGINE_DYNAMICLIBS := -logg -lmxml -lvorbis -lvorbisfile -lpng

STATICLIBPNG = ../MacSpecific/libpng.a
STATICLIBS := -logg -lmxml -lvorbis -lvorbisfile

ENGINE_SOURCE := $(wildcard engine/*.c) $(wildcard engine/*/*.c)
ENGINE_OBJ    := $(addprefix build/,$(ENGINE_SOURCE:.c=.o))
ENGINE_DYLIB  := libdynamo.dylib

override CFLAGS := $(CFLAGS) $(C_FLAGS)

all: link

build/%.o : %.c
	@echo Building $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm $(TEST_BIN) $(TEST_OBJ) $(ENGINE_DYLIB) $(ENGINE_OBJ)

link: $(ENGINE_OBJ)
	@echo "Linking Dynamo library"
	@$(CC) -o $(ENGINE_DYLIB) -dynamiclib $(ENGINE_DYNAMICLIBS) $(LDFLAGS) $^
