/*
 *	$Id: filter.c,v 1.2 2000-08-12 12:01:19 geert Exp $
 *
 *	Linux Zorro Utilities -- Device Filtering
 *
 *	Copyright (C) 1998--2000 Geert Uytterhoeven
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "zorroutils.h"

void
filter_init(struct zorro_filter *f)
{
  f->slot = -1;
  f->manuf = f->prod = f->epc = -1;
}

/* Slot filter syntax: [slot] */

char *
filter_parse_slot(struct zorro_filter *f, char *str)
{
  char *e;

  if (str[0] && strcmp(str, "*"))
    {
      long int x = strtol(str, &e, 16);
      if ((e && *e) || (x < 0 || x >= 0xff))
	return "Invalid slot number";
      f->slot = x;
    }
  return NULL;
}

/* ID filter syntax: [manuf]:[prod]:[epc] */

char *
filter_parse_id(struct zorro_filter *f, char *str)
{
  char *s1, *s2, *e;

  if (!*str)
    return NULL;
  s1 = strchr(str, ':');
  if (!s1)
    return "':' expected";
  *s1++ = 0;
  s2 = strchr(s1, ':');
  if (s2)
    *s2++ = 0;
  if (str[0] && strcmp(str, "*"))
    {
      long int x = strtol(str, &e, 16);
      if ((e && *e) || (x < 0 || x >= 0xffff))
	return "Invalid manufacturer ID";
      f->manuf = x;
    }
  if (s1[0] && strcmp(s1, "*"))
    {
      long int x = strtol(s1, &e, 16);
      if ((e && *e) || (x < 0 || x >= 0xff))
	return "Invalid product ID";
      f->prod = x;
    }
  if (s2 && s2[0] && strcmp(s2, "*"))
    {
      long int x = strtol(s2, &e, 16);
      if ((e && *e) || (x < 0 || x >= 0xff))
	return "Invalid extended product ID";
      f->epc = x;
    }
  return NULL;
}

int
filter_match(struct zorro_filter *f, byte slot, word manuf, byte prod, byte epc)
{
  if ((f->slot >= 0 && f->slot != slot) ||
      (f->manuf >= 0 && f->manuf != manuf) ||
      (f->prod >= 0 && f->prod != prod) ||
      (f->epc >= 0 && f->epc != epc))
    return 0;
  return 1;
}
