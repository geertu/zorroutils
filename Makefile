# $Id: Makefile,v 1.2 1998-06-21 20:46:27 geert Exp $
# Makefile for Linux Zorro Utilities
# Copyright (C) 1998 Geert Uytterhoeven <Geert.Uytterhoeven@cs.kuleuven.ac.be>

KERN_H=$(shell if [ ! -f zorro.h ] ; then echo '-DKERNEL_ZORRO_H' ; fi)
OPT=-O3 -fomit-frame-pointer
CFLAGS=$(OPT) -Wall $(KERN_H)

PREFIX=/
MANPREFIX=/usr

all: lszorro

lszorro: lszorro.o names.o filter.o

lszorro.o: lszorro.c zorroutils.h
names.o: names.c zorroutils.h
filter.o: filter.c zorroutils.h

clean:
	rm -f `find . -name "*~" -or -name "*.[oa]" -or -name "\#*\#" -or -name TAGS -or -name core`
	rm -f lszorro

install: all
	install -o root -g root -m 755 -s lszorro $(PREFIX)/sbin
	install -o root -g root -m 644 zorro.ids $(PREFIX)/etc
	install -o root -g root -m 644 lszorro.8 $(MANPREFIX)/man/man8

dist: clean
	cp /usr/src/linux/include/linux/zorro.h .
	sh -c 'X=`pwd` ; X=`basename $$X` ; cd .. ; tar czvvf /tmp/$$X.tar.gz $$X --exclude CVS --exclude tmp'
	rm -f zorro.h
