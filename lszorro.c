/*
 *	$Id: lszorro.c,v 1.2 1998-06-09 20:15:37 geert Exp $
 *
 *	Linux Zorro Utilities -- List All Zorro Devices
 *
 *	Copyright (C) 1998 Geert Uytterhoeven
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "zorroutils.h"

/* Options */

static int verbose;			/* Show detailed information */
static int show_hex;			/* Show contents of config space as hexadecimal numbers */
static struct zorro_filter filter;	/* Device filter */
static int machine_readable;		/* Generate machine-readable output */
static char *zorro_dir = PROC_BUS_ZORRO;

static char options[] = "nvbxs:d:ti:p:m";

static char help_msg[] = "\
Usage: lszorro [<switches>]\n\
\n\
-v\t\tBe verbose\n\
-n\t\tShow numeric IDs\n\
-x\t\tShow hex-dump of config space\n\
-s [<slot>]\tShow only device in selected slot\n\
-d [<manuf>]:[<prod>]:[<epc>]\tShow only selected devices\n\
-m\t\tProduce machine-readable output\n\
-i <file>\tUse specified ID database instead of " ETC_ZORRO_IDS "\n\
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
#define config config.raw

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
      res = read(fd, d->config, how_much);
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
  return d->config[pos];
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
show_bases(struct device *d, int cnt)
{
#if 1
#warning fix this
#else
  word cmd = get_conf_word(d, PCI_COMMAND);
  int i;

  for(i=0; i<6; i++)
    {
      unsigned long pos;
      unsigned int flg = get_conf_long(d, PCI_BASE_ADDRESS_0 + 4*i);
      if (buscentric_view)
	pos = flg;
      else
	pos = d->kernel_base_addr[i];
      if (!pos || pos == 0xffffffff)
	continue;
      if (flg & PCI_BASE_ADDRESS_SPACE_IO)
	{
	  if (cmd & PCI_COMMAND_IO)
	    {
	      if (verbose > 1)
		printf("\tRegion %d: ", i);
	      else
		putchar('\t');
	      printf("I/O ports at %04lx\n", pos & PCI_BASE_ADDRESS_IO_MASK);
	    }
	}
      else if (cmd & PCI_COMMAND_MEMORY)
	{
	  int t = flg & PCI_BASE_ADDRESS_MEM_TYPE_MASK;
	  if (verbose > 1)
	    printf("\tRegion %d: ", i);
	  else
	    putchar('\t');
	  printf("Memory at ");
	  if (t == PCI_BASE_ADDRESS_MEM_TYPE_64)
	    {
	      if (i < cnt - 1)
		{
		  i++;
		  if (!buscentric_view)
		    printf("%08x", get_conf_long(d, PCI_BASE_ADDRESS_0 + 4*i));
		}
	      else
		printf("????????");
	    }
	  printf("%08lx (%s, %sprefetchable)\n",
		 pos & PCI_BASE_ADDRESS_MEM_MASK,
		 (t == PCI_BASE_ADDRESS_MEM_TYPE_32) ? "32-bit" :
		 (t == PCI_BASE_ADDRESS_MEM_TYPE_64) ? "64-bit" :
		 (t == PCI_BASE_ADDRESS_MEM_TYPE_1M) ? "low-1M 32-bit" : "???",
		 (flg & PCI_BASE_ADDRESS_MEM_PREFETCH) ? "" : "non-");
	}
    }
#endif
}

static void
show_verbose(struct device *d)
{
#if 1
#warning fix this
#else
  word status = get_conf_word(d, PCI_STATUS);
  word cmd = get_conf_word(d, PCI_COMMAND);
  word class = get_conf_word(d, PCI_CLASS_DEVICE);
  byte bist = get_conf_byte(d, PCI_BIST);
  byte latency = get_conf_byte(d, PCI_LATENCY_TIMER);
  byte cache_line = get_conf_byte(d, PCI_CACHE_LINE_SIZE);
  byte max_lat, min_gnt;
  byte int_pin = get_conf_byte(d, PCI_INTERRUPT_PIN);
  byte int_line = get_conf_byte(d, PCI_INTERRUPT_LINE);
  unsigned int irq;
  word subsys_v, subsys_d;
#endif

  show_terse(d);

#if 1
#warning fix this
#else
  if (verbose > 1)
    {
      if (subsys_v)
	printf("\tSubsystem ID: %04x:%04x\n", subsys_v, subsys_d);
      printf("\tControl: I/O%c Mem%c BusMaster%c SpecCycle%c MemWINV%c VGASnoop%c ParErr%c Stepping%c SERR%c FastB2B%c\n",
	     (cmd & PCI_COMMAND_IO) ? '+' : '-',
	     (cmd & PCI_COMMAND_MEMORY) ? '+' : '-',
	     (cmd & PCI_COMMAND_MASTER) ? '+' : '-',
	     (cmd & PCI_COMMAND_SPECIAL) ? '+' : '-',
	     (cmd & PCI_COMMAND_INVALIDATE) ? '+' : '-',
	     (cmd & PCI_COMMAND_VGA_PALETTE) ? '+' : '-',
	     (cmd & PCI_COMMAND_PARITY) ? '+' : '-',
	     (cmd & PCI_COMMAND_WAIT) ? '+' : '-',
	     (cmd & PCI_COMMAND_SERR) ? '+' : '-',
	     (cmd & PCI_COMMAND_FAST_BACK) ? '+' : '-');
      printf("\tStatus: 66Mhz%c UDF%c FastB2B%c ParErr%c DEVSEL=%s >TAbort%c <TAbort%c <MAbort%c >SERR%c <PERR%c\n",
	     (status & PCI_STATUS_66MHZ) ? '+' : '-',
	     (status & PCI_STATUS_UDF) ? '+' : '-',
	     (status & PCI_STATUS_FAST_BACK) ? '+' : '-',
	     (status & PCI_STATUS_PARITY) ? '+' : '-',
	     ((status & PCI_STATUS_DEVSEL_MASK) == PCI_STATUS_DEVSEL_SLOW) ? "slow" :
	     ((status & PCI_STATUS_DEVSEL_MASK) == PCI_STATUS_DEVSEL_MEDIUM) ? "medium" :
	     ((status & PCI_STATUS_DEVSEL_MASK) == PCI_STATUS_DEVSEL_FAST) ? "fast" : "??",
	     (status & PCI_STATUS_SIG_TARGET_ABORT) ? '+' : '-',
	     (status & PCI_STATUS_REC_TARGET_ABORT) ? '+' : '-',
	     (status & PCI_STATUS_REC_MASTER_ABORT) ? '+' : '-',
	     (status & PCI_STATUS_SIG_SYSTEM_ERROR) ? '+' : '-',
	     (status & PCI_STATUS_DETECTED_PARITY) ? '+' : '-');
      if (cmd & PCI_COMMAND_MASTER)
	{
	  printf("\tLatency: ");
	  if (min_gnt)
	    printf("%d min, ", min_gnt);
	  if (max_lat)
	    printf("%d max, ", max_lat);
	  printf("%d set", latency);
	  if (cache_line)
	    printf(", cache line size %02x", cache_line);
	  putchar('\n');
	}
      if (int_pin)
	printf("\tInterrupt: pin %c routed to IRQ " IRQ_FORMAT "\n", 'A' + int_pin - 1, irq);
    }
  else
    {
      printf("\tFlags: ");
      if (cmd & PCI_COMMAND_MASTER)
	printf("bus master, ");
      if (cmd & PCI_COMMAND_VGA_PALETTE)
	printf("VGA palette snoop, ");
      if (cmd & PCI_COMMAND_WAIT)
	printf("stepping, ");
      if (cmd & PCI_COMMAND_FAST_BACK)
	printf("fast Back2Back, ");
      if (status & PCI_STATUS_66MHZ)
	printf("66Mhz, ");
      if (status & PCI_STATUS_UDF)
	printf("user-definable features, ");
      printf("%s devsel",
	     ((status & PCI_STATUS_DEVSEL_MASK) == PCI_STATUS_DEVSEL_SLOW) ? "slow" :
	     ((status & PCI_STATUS_DEVSEL_MASK) == PCI_STATUS_DEVSEL_MEDIUM) ? "medium" :
	     ((status & PCI_STATUS_DEVSEL_MASK) == PCI_STATUS_DEVSEL_FAST) ? "fast" : "??");
      if (cmd & PCI_COMMAND_MASTER)
	printf(", latency %d", latency);
      if (int_pin)
	if (d->kernel_irq)
	  printf(", IRQ " IRQ_FORMAT, irq);
	else
	  printf(", IRQ ?");
      putchar('\n');
    }
#endif

  show_bases(d, 6);
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
