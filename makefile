P=main
OBJECTS=barneshut.o octree.o
CFLAGS = -g -Wall -Werror -O0 -std=c99 -D OPEN_CL_FLAG
LDLIBS =-lm -lglut -lGL -lGLU -lOpenCL
CC=gcc

$(P): $(OBJECTS)
