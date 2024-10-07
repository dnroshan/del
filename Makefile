CC := gcc
CFLAGS := -W -Wall -Wextra -Wpedantic -O3

prefix ?= /usr/local

del: del.c
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean
clean: del
	rm $^;

.PHONY: install
install: del
	cp $^ $(prefix)/bin;
