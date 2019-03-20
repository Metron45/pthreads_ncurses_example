#makefile for main file

cc = gcc

all:
	$(cc) main.c -pthread -lncurses -o main
