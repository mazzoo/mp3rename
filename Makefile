CC=gcc
CFLAGS=-O2 -Wall
#CC=diet gcc -nostdlib -nostdinc
#CFLAGS=-O2 -Wall -ggdb

all:mp3rename

mp3rename: mp3rename.c Makefile
	$(CC) $< -o $@

strip: mp3rename
	strip -R .note -R .comment $^

