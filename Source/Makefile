.LIBPATTERNS = lib%.a lib%.dylib lib%.so
vpath %.a /usr/local/lib

vpath %.o build/

ARCH = -arch x86_64

CC         =  gcc
C_FLAGS    += -Wall -Wno-missing-braces -Wno-unused-function
C_FLAGS    += -std=gnu99
C_FLAGS    += -I"/usr/X11/include"
C_FLAGS    += -I"/usr/local/include"
C_FLAGS    += -ggdb
C_FLAGS    += -O0
C_FLAGS    += -DTWODEEDENG_DEBUG
C_FLAGS    += $(shell sdl-config --cflags)
C_FLAGS    += $(ARCH)

LDFLAGS  += -lc
LDFLAGS  += -lz
LDFLAGS  += -L/usr/X11/lib
LDFLAGS  += $(shell sdl-config --static-libs)
LDFLAGS  += -framework Cocoa
LDFLAGS  += -framework OpenGL
LDFLAGS  += -framework Accelerate
LDFLAGS  += -framework OpenAL
LDFLAGS  += $(ARCH)

DYNAMICLIBS := -logg -lmxml -lvorbis -lvorbisfile -lpng

STATICLIBPNG = ../MacSpecific/libpng.a
STATICLIBS := -logg -lmxml -lvorbis -lvorbisfile

ENGINE_SOURCE := $(wildcard engine/*.c) $(wildcard engine/*/*.c)
ENGINE_OBJ := $(addprefix build/,$(ENGINE_SOURCE:.c=.o))
C_SOURCE   := $(wildcard *.c) $(C_SOURCE)
OBJ        := $(addprefix build/,$(C_SOURCE:.c=.o))
BIN        := test
DYLIB      := dynamo.dylib

override CFLAGS := $(CFLAGS) $(C_FLAGS)

all: build_and_run
lib: build_dylib

build/%.o : %.c
	@echo Building $<
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

link: $(OBJ)
	@echo Linking to $(BIN)
	@$(CC) -o $(BIN) $(DYNAMICLIBS) $(LDFLAGS) $^

link-static: $(OBJ) $(STATICLIBS)
	@echo Linking (static binary) to $(BIN)
	@$(CC) -o $(BIN) $(LDFLAGS) $(STATICLIBPNG) $^

run:
	@./$(BIN)

clean:
	@rm $(BIN) $(OBJ) $(DYLIB)

build_and_run: link run

build_dylib: $(ENGINE_OBJ)
	@$(CC) -o $(DYLIB) -dynamiclib $(DYNAMICLIBS) $(LDFLAGS) $^
