CC = g++
CFLAGS = -O2
LIBS = -lGL -lglfw -lGLEW -lglut
INCLUDES = -I./src/

all: build

build: src/main.cpp
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -o bsp-viewer src/main.cpp