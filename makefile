CC = g++
CFLAGS = -O2
LIBS = -lGL -lglfw -lGLEW -lglut
INCLUDES = -I./

all: build

build: main.cpp
	$(CC) $(CFLAGS) $(LIBS) $(INCLUDES) -o bsp-viewer main.cpp