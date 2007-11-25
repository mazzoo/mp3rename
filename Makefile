#CC=gcc
#CFLAGS=-O2 -Wall
#CFLAGS=-O2 -Wall -ggdb
CC=diet gcc -nostdlib -nostdinc
CFLAGS=-Os -Wall -ggdb

all:mp3rename

mp3rename: mp3rename.c Makefile
	$(CC) $(CFLAGS) $< -o $@

strip: mp3rename
	strip -R .note -R .comment $^

