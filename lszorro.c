/*
 *	$Id: lszorro.c,v 1.4 2000-08-12 12:01:19 geert Exp $
 *
 *	Linux Zorro Utilities -- List All Zorro Devices
 *
 *	Copyright (C) 1998--2000 Geert Uytterhoeven
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "zorroutils.h"
#include "zorro.h"

/* Options */

static int verbose;			/* Show detailed information */
static int show_hex;			/* Show contents of config space as hexadecimal numbers */
static struct zorro_filter filter;	/* Device filter */
static int machine_readable;		/* Generate machine-readable output */
static char *zorro_dir = PROC_BUS_ZORRO;

static char options[] = "nvxs:d:i:p:m";

static char help_msg[] = "\
Usage: lszorro [<switches>]\n\
\n\
-v\t\tBe verbose\n\
-n\t\tShow numeric IDs\n\
-x\t\tShow hex-dump of config space\n\
-s [<slot>]\tShow only device in selected slot\n\
-d [<manuf>]:[<prod>]:[<epc>]\tShow only selected devices\n\
-m\t\tProduce machine-readable output\n\
-i <file>\tUse specified ID database instead of " ZORRO_ID_DB "\n\
-p <dir>\tUse specified bus directory instead of " PROC_BUS_ZORRO "\n\
";

/* Our view of the Zorro bus */

struct device {
  struct device *next;
  byte slot;
  word manuf;
  byte prod, epc;
  u32 boardaddr;
  u32 boardsize;
  byte boardtype;
  union {
      struct ConfigDev cd;
      byte raw[0];
  } config;
};
#define cd	config.cd
#define raw	config.raw

static struct device *first_dev, **last_dev = &first_dev;

/* Miscellaneous routines */

void *
xmalloc(unsigned int howmuch)
{
  void *p = malloc(howmuch);
  if (!p)
    {
      fprintf(stderr, "lszorro: Unable to allocate %d bytes of memory\n", howmuch);
      exit(1);
    }
  return p;
}

/* Interface for /proc/bus/zorro */

static void
scan_dev_list(void)
{
  FILE *f;
  byte line[256];
  byte name[256];

  sprintf(name, "%s/devices", zorro_dir);
  if (! (f = fopen(name, "r")))
    {
      perror(name);
      exit(1);
    }
  while (fgets(line, sizeof(line), f))
    {
      struct device *d = xmalloc(sizeof(struct device));
      unsigned int slot, vend, type;

      bzero(d, sizeof(*d));
      sscanf(line, "%x %x %x %x %x",
	     &slot,
	     &vend,
	     &d->boardaddr,
	     &d->boardsize,
	     &type);
      d->slot = slot;
      d->manuf = ZORRO_MANUF(vend);
      d->prod = ZORRO_PROD(vend);
      d->epc = ZORRO_EPC(vend);
      d->boardtype = type;
      if (filter_match(&filter, d->slot, d->manuf, d->prod, d->epc))
	{
	  *last_dev = d;
	  last_dev = &d->next;
	  d->next = NULL;
	}
    }
  fclose(f);
}

static inline void
make_proc_zorro_name(struct device *d, char *p)
{
  sprintf(p, "%s/%02x", zorro_dir, d->slot);
}

static void
scan_config(void)
{
  struct device *d;
  char name[64];
  int fd, res;
  int how_much = sizeof(struct ConfigDev);

  for(d=first_dev; d; d=d->next)
    {
      make_proc_zorro_name(d, name);
      if ((fd = open(name, O_RDONLY)) < 0)
	{
	  fprintf(stderr, "lszorro: Unable to open %s: %m\n", name);
	  exit(1);
	}
      res = read(fd, d->raw, how_much);
      if (res < 0)
	{
	  fprintf(stderr, "lszorro: Error reading %s: %m\n", name);
	  exit(1);
	}
      if (res != how_much)
	{
	  fprintf(stderr, "lszorro: Only %d bytes of config space available to you\n", res);
	  exit(1);
	}
      close(fd);
    }
}

static void
scan_proc(void)
{
  scan_dev_list();
  scan_config();
}

/* Config space accesses */

static inline byte
get_conf_byte(struct device *d, unsigned int pos)
{
  return d->raw[pos];
}

/* Sorting */

static int
compare_them(const void *A, const void *B)
{
  const struct device *a = *(const struct device **)A;
  const struct device *b = *(const struct device **)B;

  if (a->slot < b->slot)
    return -1;
  if (a->slot > b->slot)
    return 1;
  return 0;
}

static void
sort_them(void)
{
  struct device **index, **h;
  int cnt;
  struct device *d;

  cnt = 0;
  for(d=first_dev; d; d=d->next)
    cnt++;
  h = index = alloca(sizeof(struct device *) * cnt);
  for(d=first_dev; d; d=d->next)
    *h++ = d;
  qsort(index, cnt, sizeof(struct device *), compare_them);
  last_dev = &first_dev;
  h = index;
  while (cnt--)
    {
      *last_dev = *h;
      last_dev = &(*h)->next;
      h++;
    }
  *last_dev = NULL;
}

/* Normal output */

static void
show_terse(struct device *d)
{
  printf("%02x: %s\n", d->slot, lookup_device_full(d->manuf, d->prod, d->epc));
}

static void
show_bases(struct device *d)
{
  if (verbose > 1)
    printf("\tAddress: %08x (%08x bytes)\n", d->boardaddr, d->boardsize);
  else
    {
      u32 size = d->boardsize;
      char mag;
      if (size & 0xfffff)
	{
	  size >>= 10;
	  mag = 'K';
	}
      else
	{
	  size >>= 20;
	  mag = 'M';
	}
      printf("\t%08x (%d%c)\n", d->boardaddr, size, mag);
    }
}

static void
show_verbose(struct device *d)
{
  show_terse(d);
  if (verbose > 1)
    {
      const char *zorro;
      switch (d->boardtype & ERT_TYPEMASK)
	{
	case ERT_ZORROII:
	  zorro = "Zorro II";
	  break;
	case ERT_ZORROIII:
	  zorro = "Zorro III";
	  break;
	default:
	  zorro = "Unknown Zorro";
	  break;
	}
      printf("\tType: %s", zorro);
      if (d->boardtype & ERTF_MEMLIST)
	printf(" memory");
      putchar('\n');
    }
  show_bases(d);
  if (verbose > 1)
    {
      printf("\tSerial number: %08x\n", d->cd.cd_Rom.er_SerialNumber);
      printf("\tSlot address: %04x\n", d->cd.cd_SlotAddr);
      printf("\tSlot size: %04x\n", d->cd.cd_SlotSize);
    }
}

static void
show_hex_dump(struct device *d)
{
  int i;
  int limit = sizeof(struct ConfigDev);

  for(i=0; i<limit; i++)
    {
      if (! (i & 15))
	printf("%02x:", i);
      printf(" %02x", get_conf_byte(d, i));
      if ((i & 15) == 15)
	putchar('\n');
    }
}

static void
show_machine(struct device *d)
{
  if (verbose)
    {
      printf("Slot:\t%02x\n", d->slot);
      printf("Vendor:\t%s\n", lookup_vendor(d->manuf));
      printf("Device:\t%s\n", lookup_device(d->manuf, d->prod, d->epc));
    }
  else
    {
      printf("%02x: \"%s\" \"%s\"\n", d->slot, lookup_vendor(d->manuf),
	     lookup_device(d->manuf, d->prod, d->epc));
      putchar('\n');
    }
}

static void
show(void)
{
  struct device *d;

  for(d=first_dev; d; d=d->next)
    {
      if (machine_readable)
	show_machine(d);
      else if (verbose)
	show_verbose(d);
      else
	show_terse(d);
      if (show_hex)
	show_hex_dump(d);
      if (verbose || show_hex)
	putchar('\n');
    }
}

/* Main */

int
main(int argc, char **argv)
{
  int i;
  char *msg;

  if (argc == 2 && !strcmp(argv[1], "--version"))
    {
      puts("lszorro version " ZORROUTILS_VERSION);
      return 0;
    }
  filter_init(&filter);
  while ((i = getopt(argc, argv, options)) != -1)
    switch (i)
      {
      case 'n':
	show_numeric_ids = 1;
	break;
      case 'v':
	verbose++;
	break;
      case 's':
	if ((msg = filter_parse_slot(&filter, optarg)))
	  {
	    fprintf(stderr, "lszorro: -f: %s\n", msg);
	    return 1;
	  }
	break;
      case 'd':
	if ((msg = filter_parse_id(&filter, optarg)))
	  {
	    fprintf(stderr, "lszorro: -d: %s\n", msg);
	    return 1;
	  }
	break;
      case 'x':
	show_hex++;
	break;
      case 'i':
	zorro_ids = optarg;
	break;
      case 'p':
	zorro_dir = optarg;
	break;
      case 'm':
	machine_readable++;
	break;
      default:
      bad:
	fprintf(stderr, help_msg);
	return 1;
      }
  if (optind < argc)
    goto bad;

  scan_proc();
  sort_them();
  show();

  return 0;
}
