/*
 *	$Id: zorroutils.h,v 1.5 2000-09-28 18:46:14 geert Exp $
 *
 *	Linux Zorro Utilities -- Declarations
 *
 *	Copyright (C) 1998--2000 Geert Uytterhoeven
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <linux/types.h>

#define ZORROUTILS_VERSION "0.04"

#define PROC_BUS_ZORRO "/proc/bus/zorro"
#define ZORRO_ID_DB "/usr/share/zorro.ids"

/* Types */

typedef __u8 byte;
typedef __u16 word;
typedef __u32 u32;

/* lszorro.c */

void *xmalloc(unsigned int);

/* names.c */

extern int show_numeric_ids;
extern char *zorro_ids;

char *lookup_vendor(word);
char *lookup_device(word, byte, byte);
char *lookup_device_full(word, byte, byte);

/* filter.c */

struct zorro_filter {
  int slot;			/* -1 = ANY */
  int manuf, prod, epc;
};

void filter_init(struct zorro_filter *);
char *filter_parse_slot(struct zorro_filter *, char *);
char *filter_parse_id(struct zorro_filter *, char *);
int filter_match(struct zorro_filter *, byte, word, byte, byte);
