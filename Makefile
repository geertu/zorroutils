# $Id: Makefile,v 1.4 2000-08-12 12:01:19 geert Exp $
# Makefile for Linux Zorro Utilities
# Copyright (C) 1998--2000 Geert Uytterhoeven <geert@linux-m68k.org>

CC=gcc
OPT=-O3 -fomit-frame-pointer
CFLAGS=$(OPT) -Wall

ROOT=/
PREFIX=/usr

all: lszorro

lszorro: lszorro.o names.o filter.o

lszorro.o: lszorro.c zorroutils.h zorro.h
names.o: names.c zorroutils.h
filter.o: filter.c zorroutils.h

clean:
	rm -f `find . -name "*~" -or -name "*.[oa]" -or -name "\#*\#" -or -name TAGS -or -name core`
	rm -f lszorro

install: all
	install -o root -g root -m 755 -s lszorro $(ROOT)/sbin
	install -o root -g root -m 644 zorro.ids $(PREFIX)/share
	install -o root -g root -m 644 lszorro.8 $(PREFIX)/man/man8
	# Remove relics from old versions
	rm -f $(ROOT)/etc/zorro.ids

dist: clean
	sh -c 'X=`pwd` ; X=`basename $$X` ; cd .. ; tar czvvf /tmp/$$X.tar.gz $$X --exclude CVS --exclude tmp'
