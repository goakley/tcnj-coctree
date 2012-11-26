P=main
OBJECTS=barneshut.o octree.o
CFLAGS = -g -Wall -Werror -O0 -std=c99
LDLIBS =-lm
CC=gcc

$(P): $(OBJECTS)
