vpath %.o build/

CC         =  gcc
C_FLAGS    += -Wall -Wno-missing-braces -Wno-unused-function
C_FLAGS    += -std=gnu99
C_FLAGS    += -I"/opt/X11/include"
C_FLAGS    += -I"/usr/local/include"
C_FLAGS    += -DGLEW_STATIC
C_FLAGS    += -ggdb
C_FLAGS    += -O0
C_FLAGS    += -DDEBUG

LDFLAGS  += -lc
LDFLAGS  += -L/usr/X11/lib
LDFLAGS  += -L/usr/local/lib
LDFLAGS  += -lpng
LDFLAGS  += -logg
LDFLAGS  += -lvorbisfile
LDFLAGS  += -framework GLUT
LDFLAGS  += -framework OpenGL
LDFLAGS  += -framework Accelerate
LDFLAGS  += -framework OpenAL

C_SOURCE   := $(wildcard *.c) $(wildcard engine/*.c) $(wildcard engine/*/*.c)
OBJ        := $(addprefix build/,$(C_SOURCE:.c=.o))
BIN        := test

override CFLAGS := $(CFLAGS) $(C_FLAGS)

all: build_and_run

build/%.o : %.c
	@echo Building $< 
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c $< -o $@

link: $(OBJ)
	@echo Linking to $(BIN)
	@$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

run:
	@./$(BIN)

clean:
	@rm $(BIN) $(OBJ)

build_and_run: link run


