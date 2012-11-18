P=barneshut.o
OBJECTS=octree.o
CFLAGS = -g -Wall -Werror -O3 -std=c99 -c
LDLIBS =
CC=gcc

$(P): $(OBJECTS)
