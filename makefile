P=main
OBJECTS=barneshut.o octree.o
CFLAGS = -g -Wall -Werror -O0 -std=c99
LDLIBS =-lm -lglut32 -lopengl32 -lglu32 -lopencl
CC=gcc

$(P): $(OBJECTS)
