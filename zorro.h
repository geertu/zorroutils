/*
 *  $Id: zorro.h,v 1.2 1998-06-08 20:28:17 geert Exp $
 *
 *  Amiga AutoConfig (Zorro) Bus Definitions
 *
 *  Copyright (C) 1995-1998 Geert Uytterhoeven
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License.  See the file COPYING in the main directory of this archive
 *  for more details.
 */

#ifndef _LINUX_ZORRO_H
#define _LINUX_ZORRO_H

#ifndef __ASSEMBLY__

    /*
     *  Known Zorro boards (well, those the kernel has drivers for, the others
     *  are in /etc/zorro.ids)
     *
     *  Each Zorro board has a 32-bit ID of the form
     *
     *      mmmmmmmmmmmmmmmmppppppppeeeeeeee
     *
     *  with
     *
     *      mmmmmmmmmmmmmmmm	16-bit Manufacturer ID (assigned by CBM (sigh))
     *      pppppppp		8-bit Product ID (assigned by manufacturer)
     *      eeeeeeee		8-bit Extended Product ID (currently only used
     *				for some GVP boards)
     */


#define ZORRO_MANUF(id)		((id) >> 16)
#define ZORRO_PROD(id)		(((id) >> 8) & 0xff)
#define ZORRO_EPC(id)		((id) & 0xff)

#define ZORRO_ID(manuf, prod, epc) \
    ((ZORRO_MANUF_##manuf << 16) | ((prod) << 8) | (epc))

typedef __u32 zorro_id;


#define ZORRO_MANUF_COMMODORE_BRAUNSCHWEIG		0x0201
#define  ZORRO_PROD_CBM_A4091_1				ZORRO_ID(COMMODORE_BRAUNSCHWEIG, 0x54, 0)

#define ZORRO_MANUF_COMMODORE_WEST_CHESTER_1		0x0202
#define  ZORRO_PROD_CBM_A590_A2091_1			ZORRO_ID(COMMODORE_WEST_CHESTER_1, 0x02, 0)
#define  ZORRO_PROD_CBM_A590_A2091_2			ZORRO_ID(COMMODORE_WEST_CHESTER_1, 0x03, 0)
#define  ZORRO_PROD_CBM_A4091_2				ZORRO_ID(COMMODORE_WEST_CHESTER_1, 0x54, 0)
#define  ZORRO_PROD_CBM_A2065_1				ZORRO_ID(COMMODORE_WEST_CHESTER_1, 0x5A, 0)
#define  ZORRO_PROD_CBM_A2065_2				ZORRO_ID(COMMODORE_WEST_CHESTER_1, 0x70, 0)

#define ZORRO_MANUF_AMERISTAR				0x041D
#define  ZORRO_PROD_AMERISTAR_A2065			ZORRO_ID(AMERISTAR, 0x01, 0)

#define ZORRO_MANUF_GREAT_VALLEY_PRODUCTS_2		0x07E1
#define  ZORRO_PROD_GVP_EPC_BASE			ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0)
#define  ZORRO_PROD_GVP_GFORCE_040_SCSI_1		ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0x30)
#define  ZORRO_PROD_GVP_A1291				ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0x40)
#define  ZORRO_PROD_GVP_COMBO_030_R4_SCSI		ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0x70)
#define  ZORRO_PROD_GVP_GFORCE_030_SCSI			ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0xb0)
#define  ZORRO_PROD_GVP_A530_SCSI			ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0xd0)
#define  ZORRO_PROD_GVP_COMBO_030_R3_SCSI		ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0xf0)
#define  ZORRO_PROD_GVP_SERIES_II			ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x0B, 0xf8)
#define  ZORRO_PROD_GVP_GFORCE_040_060			ZORRO_ID(GREAT_VALLEY_PRODUCTS_2, 0x16, 0)

#define ZORRO_MANUF_HYDRA_SYSTEMS			0x0849
#define  ZORRO_PROD_HYDRA_SYSTEMS_AMIGANET		ZORRO_ID(HYDRA_SYSTEMS, 0x01, 0)

#define ZORRO_MANUF_VILLAGE_TRONIC			0x0877
#define  ZORRO_PROD_VILLAGE_TRONIC_ARIADNE		ZORRO_ID(VILLAGE_TRONIC, 0xC9, 0)

#define ZORRO_MANUF_MACROSYSTEMS_USA			0x089B
#define  ZORRO_PROD_MACROSYSTEMS_WARP_ENGINE_40xx	ZORRO_ID(MACROSYSTEMS_USA, 0x13, 0)

#define ZORRO_MANUF_PHASE5				0x2140
#define  ZORRO_PROD_PHASE5_BLIZZARD_1230_II_FASTLANE_Z3_CYBERSCSI_CYBERSTORM060	ZORRO_ID(PHASE5, 0x0B, 0)
#define  ZORRO_PROD_PHASE5_BLIZZARD_1220_CYBERSTORM	ZORRO_ID(PHASE5, 0x0C, 0)
#define  ZORRO_PROD_PHASE5_BLIZZARD_1230_IV_1260	ZORRO_ID(PHASE5, 0x11, 0)
#define  ZORRO_PROD_PHASE5_BLIZZARD_2060		ZORRO_ID(PHASE5, 0x18, 0)
#define  ZORRO_PROD_PHASE5_CYBERSTORM_MK_II		ZORRO_ID(PHASE5, 0x19, 0)
#define  ZORRO_PROD_PHASE5_CYBERVISION64		ZORRO_ID(PHASE5, 0x22, 0)
#define  ZORRO_PROD_PHASE5_CYBERVISION64_3D		ZORRO_ID(PHASE5, 0x43, 0)

#define ZORRO_MANUF_MACROSYSTEMS_GERMANY		0x4754
#define  ZORRO_PROD_MACROSYSTEMS_RETINA_Z3		ZORRO_ID(MACROSYSTEMS_GERMANY, 0x10, 0)


    /*
     *  GVP identifies most of its products through the 'extended product code'
     *  (epc). The epc has to be and'ed with the GVP_PRODMASK before the
     *  identification.
     */

#define GVP_PRODMASK			(0xf8)
#define GVP_SCSICLKMASK			(0x01)

enum GVP_flags {
    GVP_IO		= 0x01,
    GVP_ACCEL		= 0x02,
    GVP_SCSI		= 0x04,
    GVP_24BITDMA	= 0x08,
    GVP_25BITDMA	= 0x10,
    GVP_NOBANK		= 0x20,
    GVP_14MHZ		= 0x40,
};


struct Node {
    struct  Node *ln_Succ;	/* Pointer to next (successor) */
    struct  Node *ln_Pred;	/* Pointer to previous (predecessor) */
    __u8    ln_Type;
    __s8    ln_Pri;		/* Priority, for sorting */
    __s8    *ln_Name;		/* ID string, null terminated */
};

struct ExpansionRom {
    /* -First 16 bytes of the expansion ROM */
    __u8  er_Type;		/* Board type, size and flags */
    __u8  er_Product;		/* Product number, assigned by manufacturer */
    __u8  er_Flags;		/* Flags */
    __u8  er_Reserved03;	/* Must be zero ($ff inverted) */
    __u16 er_Manufacturer;	/* Unique ID, ASSIGNED BY COMMODORE-AMIGA! */
    __u32 er_SerialNumber;	/* Available for use by manufacturer */
    __u16 er_InitDiagVec;	/* Offset to optional "DiagArea" structure */
    __u8  er_Reserved0c;
    __u8  er_Reserved0d;
    __u8  er_Reserved0e;
    __u8  er_Reserved0f;
};

/* er_Type board type bits */
#define ERT_TYPEMASK	0xc0
#define ERT_ZORROII	0xc0
#define ERT_ZORROIII	0x80

/* other bits defined in er_Type */
#define ERTB_MEMLIST	5		/* Link RAM into free memory list */
#define ERTF_MEMLIST	(1<<5)

struct ConfigDev {
    struct Node 	cd_Node;
    __u8  		cd_Flags;	/* (read/write) */
    __u8  		cd_Pad; 	/* reserved */
    struct ExpansionRom cd_Rom; 	/* copy of board's expansion ROM */
    void		*cd_BoardAddr;	/* where in memory the board was placed */
    __u32 		cd_BoardSize;	/* size of board in bytes */
    __u16  		cd_SlotAddr;	/* which slot number (PRIVATE) */
    __u16  		cd_SlotSize;	/* number of slots (PRIVATE) */
    void		*cd_Driver;	/* pointer to node of driver */
    struct ConfigDev	*cd_NextCD;	/* linked list of drivers to config */
    __u32 		cd_Unused[4];	/* for whatever the driver wants */
};

#else /* __ASSEMBLY__ */

LN_Succ		= 0
LN_Pred		= LN_Succ+4
LN_Type		= LN_Pred+4
LN_Pri		= LN_Type+1
LN_Name		= LN_Pri+1
LN_sizeof	= LN_Name+4

ER_Type		= 0
ER_Product	= ER_Type+1
ER_Flags	= ER_Product+1
ER_Reserved03	= ER_Flags+1
ER_Manufacturer	= ER_Reserved03+1
ER_SerialNumber	= ER_Manufacturer+2
ER_InitDiagVec	= ER_SerialNumber+4
ER_Reserved0c	= ER_InitDiagVec+2
ER_Reserved0d	= ER_Reserved0c+1
ER_Reserved0e	= ER_Reserved0d+1
ER_Reserved0f	= ER_Reserved0e+1
ER_sizeof	= ER_Reserved0f+1

CD_Node		= 0
CD_Flags	= CD_Node+LN_sizeof
CD_Pad		= CD_Flags+1
CD_Rom		= CD_Pad+1
CD_BoardAddr	= CD_Rom+ER_sizeof
CD_BoardSize	= CD_BoardAddr+4
CD_SlotAddr	= CD_BoardSize+4
CD_SlotSize	= CD_SlotAddr+2
CD_Driver	= CD_SlotSize+2
CD_NextCD	= CD_Driver+4
CD_Unused	= CD_NextCD+4
CD_sizeof	= CD_Unused+(4*4)

#endif /* __ASSEMBLY__ */

#ifndef __ASSEMBLY__

#define ZORRO_NUM_AUTO		16

#ifdef __KERNEL__

extern unsigned int zorro_num_autocon;	/* # of autoconfig devices found */
extern struct ConfigDev zorro_autocon[ZORRO_NUM_AUTO];


    /*
     *  Zorro Functions
     */

extern void zorro_init(void);
extern void zorro_proc_init(void);

extern unsigned int zorro_find(zorro_id id, unsigned int part, unsigned int index);
extern const struct ConfigDev *zorro_get_board(unsigned int key);
extern void zorro_config_board(unsigned int key, unsigned int part);
extern void zorro_unconfig_board(unsigned int key, unsigned int part);


    /*
     *  Bitmask indicating portions of available Zorro II RAM that are unused
     *  by the system. Every bit represents a 64K chunk, for a maximum of 8MB
     *  (128 chunks, physical 0x00200000-0x009fffff).
     *
     *  If you want to use (= allocate) portions of this RAM, you should clear
     *  the corresponding bits.
     */

extern __u32 zorro_unused_z2ram[4];

#define Z2RAM_START		(0x00200000)
#define Z2RAM_END		(0x00a00000)
#define Z2RAM_SIZE		(0x00800000)
#define Z2RAM_CHUNKSIZE		(0x00010000)
#define Z2RAM_CHUNKMASK		(0x0000ffff)
#define Z2RAM_CHUNKSHIFT	(16)


#endif /* !__ASSEMBLY__ */
#endif /* __KERNEL__ */

#endif /* _LINUX_ZORRO_H */
