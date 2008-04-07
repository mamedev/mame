/***************************************************************************

These are some of the CPS-B chip numbers:

NAME                                        CPS-B #                     C-board PAL's  B-board PAL's
Forgotten Worlds / Lost Worlds              CPS-B-01  DL-0411-10001     None           LWCH1 & LW10
Ghouls 'n Ghosts                            CPS-B-01  DL-0411-10001     None           DM620 & LW10
Strider                                     CPS-B-01  DL-0411-10001     None           ST24M1 & LW10 or ST22B
Dynasty Wars                                CPS-B-02  DL-0411-10002     ?              ?
Willow                                      CPS-B-03  DL-0411-10003     None           WL24B & LW10
UN Squadron / Area 88                       CPS-B-11  DL-0411-10004     None           AR24B & LW10
Street Fighter II (ETC Rev G)               CPS-B-11  DL-0411-10004     None           STF29 & IOB1
Final Fight (World / Japan)                 CPS-B-04  DL-0411-10005     None           S224B & LW10
Final Fight (US)                            CPS-B-04* DL-0411-10001     None           S224B & LW10
                                            *actually CPS-B-01, the original number was scratched out and "04" stamped over it
Varth                                       CPS-B-04  DL-0411-10005     None           VA24B & LW10
1941                                        CPS-B-05  DL-0411-10006     ?              YI24B & ?
Street Fighter II (US Rev D)                CPS-B-05  DL-0411-10006     C632           STF29 & IOB1
Mercs (US)                                  CPS-B-12  DL-0411-10007     C628           0224B & IOB1
Street Fighter II (Japan Rev C)             CPS-B-12  DL-0411-10007     C632           STF29 & IOB1
Magic Sword (US)                            CPS-B-13  DL-0411-10008     None           MS24B & IOB1
Street Fighter II (Japan)                   CPS-B-13  DL-0411-10008     C632           STF29 & IOB1
Chiki Chiki Boys / Mega Twins               CPS-B-14  DL-0411-10009     ?              CK24B & IOB1
Street Fighter II (US Rev I)                CPS-B-14  DL-0411-10009     C632           STF29 & IOB1
Nemo                                        CPS-B-15  DL-0411-10010     ?              NM24B & ?
Carrier Air Wing                            CPS-B-16  DL-0411-10011     None           CA24B & IOB1
Street Fighter II                           CPS-B-17  DL-0411-10012     C632           STF29 & IOB1
Three Wonders*                              CPS-B-21  DL-0921-10014     IOC1 & C632    RT24B & IOB1
King of Dragons*                            CPS-B-21  DL-0921-10014     IOC1 & C632    KD29B & IOB1
Captain Commando* (US)                      CPS-B-21  DL-0921-10014     IOC1 & C632    CC63B, CCPRG & IOB1
Knights of the Round*                       CPS-B-21  DL-0921-10014     IOC1 & C632    KR63B, BPRG1 & IOB1
Street Fighter II' Champion Edition         CPS-B-21  DL-0921-10014     IOC1 & C632    S9263B, BPRG1 & IOB1
Capcom World 2*                             CPS-B-21  DL-0921-10014     IOC1           Q522B & IOB1
Quiz and Dragons (US)*                      CPS-B-21  DL-0921-10014     IOC1           QD22B & IOB1
Quiz and Dragons (Japan)*                   CPS-B-21  DL-0921-10014     IOC1           ? & IOB1
Warriors of Fate*                           CPS-B-21  DL-0921-10014     IOC1           TK263B, BPRG1 & IOB1
Street Fighter II Turbo Hyper Fighting      CPS-B-21  DL-0921-10014     IOC1 & C632    S9263B, BPRG1 & IOB1
Cadillacs and Dinosaurs*                    CPS-B-21  DL-0921-10014     IOC1           CD63B, BPRG1 & IOB1
Punisher*                                   CPS-B-21  DL-0921-10014     IOC1           PS63B, BPRG1 & IOB1
Saturday Night Slam Masters*                CPS-B-21  DL-0921-10014     IOC1           MB63B, BPRG1 & IOB1
Muscle Bomber Duo*                          CPS-B-21  DL-0921-10014     ?              ?
Pnickies                                    CPS-B-21? DL-0921-10014?    ?              ?
Pang 3                                      CPS-B-21  DL-0921-10014     IOC1 & C632    CP1B1F, CP1B8K & CP1B9KA
Megaman the Power Battle                    CPS-B-21  DL-0921-10014     IOC1 & C632    RCM63B, BPRG1 & IOB1

Street Fighter Zero changer system (Japan)  CPS-B-21  DL-0921-10014     ?              ?

*denotes Suicide Battery

You can set the suicide CPS-B-21 chips to their default layer register and priority bit values
if you pull pins 45 and 46 high (floating the pins seems to work, too). The default is the same
values as Street Fighter 2 CE/Turbo.



OUTPUT PORTS
0x00-0x01     OBJ RAM base (/256)
0x02-0x03     Scroll1 (8x8) RAM base (/256)
0x04-0x05     Scroll2 (16x16) RAM base (/256)
0x06-0x07     Scroll3 (32x32) RAM base (/256)
0x08-0x09     rowscroll RAM base (/256)
0x0a-0x0b     Palette base (/256)
0x0c-0x0d     Scroll 1 X
0x0e-0x0f     Scroll 1 Y
0x10-0x11     Scroll 2 X
0x12-0x13     Scroll 2 Y
0x14-0x15     Scroll 3 X
0x16-0x17     Scroll 3 Y
0x18-0x19     Starfield 1 X
0x1a-0x1b     Starfield 1 Y
0x1c-0x1d     Starfield 2 X
0x1e-0x1f     Starfield 2 Y
0x20-0x21     start offset for the rowscroll matrix
0x22-0x23     unknown but widely used - usually 0x0e. bit 0 enables rowscroll
              on layer 2. bit 15 is flip screen.


Some registers move from game to game.. following example strider
0x66-0x67   Layer control register
            bits 14-15 seem to be unused
                ghouls sets bits 15 in service mode when you press button 2 in
                the input test
            bits 6-13 (4 groups of 2 bits) select layer draw order
            bits 1-5 enable the three tilemap layers and the two starfield
                layers (the bit order changes from game to game).
                Only Forgotten Worlds and Strider use the starfield.
            bit 0 could be rowscroll related. It is set by captain commando,
                varth, mtwins, mssword, cawing while rowscroll is active. However
                kodj and sf2 do NOT set this bit while they are using rowscroll.
                Games known to use row scrolling:
                SF2
                Mega Twins (underwater, cave)
                Carrier Air Wing (hazy background at beginning of mission 8, put 07 at ff8501 to jump there)
                Magic Sword (fire on floor 3; screen distort after continue)
                Varth (title screen)
                Captain Commando (end game sequence)
0x68-0x69   Priority mask \   Tiles in the layer just below sprites can have
0x6a-0x6b   Priority mask |   four priority levels, each one associated with one
0x6c-0x6d   Priority mask |   of these masks. The masks indicate pens in the tile
0x6e-0x6f   Priority mask /   that have priority over sprites.
0x70-0x71   Control register (usually 0x003f). The details of how this register
            works are unknown, but it definitely affects the palette, and games
            always set it together with the palette base register.
            Experiments on the real board show that values different from 0x3f in
            the low 6 bits cause wrong colors. The other bits seem to be unused.
            There is one CPS2 game (Slammasters II) setting this to 0x2f: the
            purpose is unknown.
            The only other places where this register seems to be set to a value
            different from 0x3f is during startup tests. Examples:
            ghouls  0x02 (and palette base is set to 9105; palette base is 9100 normally)
            strider 0x02 (and palette base is set to 9145; palette base is 9140 normally)
            1941    0x02 (and palette base is set to 9145; palette base is 9140 normally)
            unsquad 0x0f
            kod     0x0f
            mtwins  0x0f

Fixed registers
0x80-0x81     Sound command
0x88-0x89     Sound fade


A special note has to be made about tile/sprite codes. Even if all graphics are
stored together in the same ROMs, the hardware knows which part of the ROM space
is 8x8 tiles, 16x16 tiles, 16x16 spites, 32x32 tiles, and all games tested only
draw tiles if their code falls in the valid range. If a tile is out of range, it
is replaced by transparent pixels.
Ideally, this shouldn't be important as far as the emulation is concerned, since
games should only request tiles from valid ranges. In practice, many games contain
bugs which make them try to display out of range tiles. The masking applied by
the hardware therefore needs to be emulated properly, otherwise glitches appear.

The ROM board (B-board) changes from game to game, so the implementation details
may vary, but in general the tile ranges are controlled by a PAL found on the
B-board (see the table at the top of this file).

The A-board passes 23 bits of address to the B-board when requesting gfx ROM data.
The B-board selects 64 bits of data, that is 16 4bpp pixels, and returns half of
them depending on a signal from the C board.
The 23 address bits are laid out this way (note that the top 3 bits select the
tile type; the purpose of the top bit is unknown):

sprite  000ccccccccccccccccyyyy
scroll1 001?ccccccccccccccccyyy
scroll2 010ccccccccccccccccyyyy
scroll3 011ccccccccccccccyyyyyx

where
c is the tile code
y is the y position in the tile
x is the x position in the tile (only applicable to 32x32 tiles)

E.g. on the 89624B-3 board, used by cawing and others, the top 13 bits of the
address are passed to the 16L8B PAL @1A, which outputs the /OE signal for roms
1,3,5,7 (together), 2,4,6,8 (together), 10,12,14,16,20,22,24,26 (together) or
11,13,15,17,21,23,25,27 (together).
Note that in cawing only sockets 1,3,5,7 are populated, so effectively there is
only one bank.

The above would mean that
1) 8x8 and 16x16 tiles have a 16-bit tile code, while
   32x32 tiles have a 14-bit tile code
2) which ROM bank to use is determined by
   bits 15-7 of a 8x8 tile code,
   bits 15-6 of a 16x16 tile code,
   bits 13-4 of a 32x32 tile code

If the PAL decides that the tile code is out of range and doesn't output any /OE
signal, no ROM is read and pullup resistors force the result to all 1 (which
means a transparent tile).

Note that there are several known cases (nemo, cawing, 3wonders, varth, etc.) where
16x16 tiles are used for BOTH sprites and scroll2.

Ideally, the PALs should be dumped and loaded like the ROMs, and the mapping should
be derived from the loaded PALs at run time.
In practice, this is unlikely to happen, so we'll have to tolerate using hardcoded
maps for every game.


Known Bug List
==============
CPS2:
* CPS2 can do raster effects, certainly used by ssf2 (Cammy, DeeJay, T.Hawk levels),
  msh (lava level, early in attract mode) and maybe others (xmcotaj, vsavj).
  IRQ4 is some sort of scanline interrupt used for that purpose.

* Its unknown what CPS2_OBJ_BASE register (0x400000) does but it is not a object base
  register. All games use 0x7000 even if 0x7080 is used at this register (checked on
  real HW). Maybe it sets the object bank used when cps2_objram_bank is set.

* Sprites are currently lagged by one frame to keep sync with backgrounds. This causes
  sprites to stay on screen one frame longer (visable in VSAV attract mode).

Marvel Vs. Capcom
* Sometimes currupt gfx are displayed on the 32x32 layer as the screen flashes at the
  start of super combo moves. The problem seems to be due to tiles being fetched before
  the first 32x32 tile offset and results in data coming from 16x16 or 8x8 tiles instead.

CPS1:
SF2
* Missing chain in the foreground in Ken's level, and sign in Chun Li's level.
  Those graphics are in the backmost layer.

King of Dragons (World).
* Distortion effect missing on character description screen during attract
mode. The game rapidly toggles on and off the layer enable bit.
I think this IS the correct behaviour. The Japanese version does the
distortion as expected.

3wonders
* writes to output ports 42, 44, 46.

qad
* layer enable mask incomplete

varth
* The boot screen of some CPS2 games has a blue background instead of black,
  however that might be the correct behaviour.


Unknown issues
==============

There are often some redundant high bits in the scroll layer's attributes.
I think that these are spare bits that the game uses for to store additional
information, not used by the hardware.
The games seem to use them to mark platforms, kill zones and no-go areas.

***************************************************************************/

#include "driver.h"
#include "cps1.h"

#define VERBOSE 0

#define CPS1_DUMP_VIDEO 0

/********************************************************************

            Configuration table:

********************************************************************/

/* Game specific data */

#define GFXTYPE_SPRITES   (1<<0)
#define GFXTYPE_SCROLL1   (1<<1)
#define GFXTYPE_SCROLL2   (1<<2)
#define GFXTYPE_SCROLL3   (1<<3)
#define GFXTYPE_STARS     (1<<4)
#define GFXTYPE_UNKNOWN   (0)


struct gfx_range
{
	// start and end are as passed by the game (shift adjusted to be all
	// in the same scale a 8x8 tiles): they don't necessarily match the
	// position in ROM.
	int start;
	int end;
	int type;
	int bank;
};

struct CPS1config
{
	const char *name;             /* game driver name */

	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	int cpsb_addr;        /* CPS board B test register address */
	int cpsb_value;       /* CPS board B test register expected value */

	/* some games use as a protection check the ability to do 16-bit multiplies */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	/* It looks like this feature was introduced with 3wonders (CPSB ID = 08xx) */
	int mult_factor1;
	int mult_factor2;
	int mult_result_lo;
	int mult_result_hi;

	int layer_control;
	int priority[4];
	int control_reg;  /* Control register? seems to be always 0x3f */

	/* ideally, the layer enable masks should consist of only one bit, */
	/* but in many cases it is unknown which bit is which. */
	int layer_enable_mask[5];

	const struct gfx_range *bank_mapper;

	int bank_scroll1;
	int bank_scroll2;
	int bank_scroll3;

	/* Some characters aren't visible */
	const int start_scroll2;
	const int end_scroll2;
	const int start_scroll3;
	const int end_scroll3;

	int bootleg_kludge;
};

static const struct CPS1config *cps1_game_config;


/*                 CPSB ID    multiply protection  ctrl     priority masks   unknwn     layer enable masks  */
#define CPS_B_01 0x00,0x0000, 0,0,0,0, /* n/a */   0x66,{0x68,0x6a,0x6c,0x6e},0x70, {0x02,0x04,0x08,0x30,0x30}
#define CPS_B_02 0x60,0x0002, 0,0,0,0, /* n/a */   0x6c,{0x6a,0x68,0x66,0x64},0x62, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_03 0x00,0x0000, 0,0,0,0, /* n/a */   0x70,{0x6e,0x6c,0x6a,0x68},0x66, {0x20,0x10,0x08,0x00,0x00}
#define CPS_B_04 0x60,0x0004, 0,0,0,0, /* n/a */   0x6e,{0x66,0x70,0x68,0x72},0x6a, {0x02,0x0c,0x0c,0x00,0x00}
#define CPS_B_05 0x60,0x0005, 0,0,0,0, /* n/a */   0x68,{0x6a,0x6c,0x6e,0x70},0x72, {0x02,0x08,0x20,0x14,0x14}
#define CPS_B_11 0x72,0x0401, 0,0,0,0, /* n/a */   0x66,{0x68,0x6a,0x6c,0x6e},0x70, {0x08,0x10,0x20,0x00,0x00}
#define CPS_B_12 0x60,0x0402, 0,0,0,0, /* n/a */   0x6c,{0x6a,0x68,0x66,0x64},0x62, {0x02,0x04,0x08,0x00,0x00}
#define CPS_B_13 0x6e,0x0403, 0,0,0,0, /* n/a */   0x62,{0x64,0x66,0x68,0x6a},0x6c, {0x20,0x02,0x04,0x00,0x00}
#define CPS_B_14 0x5e,0x0404, 0,0,0,0, /* n/a */   0x52,{0x54,0x56,0x58,0x5a},0x5c, {0x08,0x20,0x10,0x00,0x00}
#define CPS_B_15 0x4e,0x0405, 0,0,0,0, /* n/a */   0x42,{0x44,0x46,0x48,0x4a},0x4c, {0x04,0x02,0x20,0x00,0x00}
#define CPS_B_16 0x40,0x0406, 0,0,0,0, /* n/a */   0x4c,{0x4a,0x48,0x46,0x44},0x42, {0x10,0x0a,0x0a,0x00,0x00}
#define CPS_B_17 0x48,0x0407, 0,0,0,0, /* n/a */   0x54,{0x52,0x50,0x4e,0x4c},0x4a, {0x08,0x10,0x02,0x00,0x00}
#define CPS_B_18 0xd0,0x0408, 0,0,0,0, /* n/a */   0xdc,{0xda,0xd8,0xd6,0xd4},0xd2, {0x10,0x08,0x02,0x00,0x00}
#define NOBATTRY 0x00,0x0000, 0x40,0x42,0x44,0x46, 0x66,{0x68,0x6a,0x6c,0x6e},0x70, {0x02,0x04,0x08,0x30,0x30}	// pang3 sets layer enable to 0x26 on startup
#define BATTRY_1 0x72,0x0800, 0x4e,0x4c,0x4a,0x48, 0x68,{0x66,0x64,0x62,0x60},0x70, {0x20,0x04,0x08,0x12,0x12}
#define BATTRY_2 0x00,0x0000, 0x5e,0x5c,0x5a,0x58, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x30,0x08,0x30,0x00,0x00}
#define BATTRY_3 0x00,0x0000, 0x46,0x44,0x42,0x40, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x20,0x12,0x12,0x00,0x00}
#define BATTRY_4 0x00,0x0000, 0x46,0x44,0x42,0x40, 0x68,{0x66,0x64,0x62,0x60},0x70, {0x20,0x10,0x02,0x00,0x00}
#define BATTRY_5 0x00,0x0000, 0x4e,0x4c,0x4a,0x48, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x20,0x06,0x06,0x00,0x00}
#define BATTRY_6 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x60,{0x6e,0x6c,0x6a,0x68},0x70, {0x20,0x14,0x14,0x00,0x00}
#define BATTRY_7 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x6c,{0x00,0x00,0x00,0x00},0x52, {0x14,0x02,0x14,0x00,0x00}
#define QSOUND_1 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x62,{0x64,0x66,0x68,0x6a},0x6c, {0x10,0x08,0x04,0x00,0x00}
#define QSOUND_2 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x4a,{0x4c,0x4e,0x40,0x42},0x44, {0x16,0x16,0x16,0x00,0x00}
#define QSOUND_3 0x4e,0x0c00, 0x00,0x00,0x00,0x00, 0x52,{0x54,0x56,0x48,0x4a},0x4c, {0x04,0x02,0x20,0x00,0x00}
#define QSOUND_4 0x6e,0x0c01, 0x00,0x00,0x00,0x00, 0x56,{0x40,0x42,0x68,0x6a},0x6c, {0x04,0x08,0x10,0x00,0x00}
#define QSOUND_5 0x5e,0x0c02, 0x00,0x00,0x00,0x00, 0x6a,{0x6c,0x6e,0x70,0x72},0x5c, {0x04,0x08,0x10,0x00,0x00}
#define HACK_B_1 0x00,0x0000, 0x00,0x00,0x00,0x00, 0x54,{0x52,0x50,0x4e,0x4c},0x5c, {0xff,0xff,0xff,0x00,0x00}


static const struct gfx_range mapper_LWCH1[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x03ff, GFXTYPE_STARS,   0 },
	{ 0x0400, 0x0fff, GFXTYPE_UNKNOWN, 0 },	// empty
	{ 0x1000, 0x3fff, GFXTYPE_SCROLL2, 0 },
	{ 0x4000, 0x7fff, GFXTYPE_SCROLL3, 0 },

	{ 0x0000, 0x7fff, GFXTYPE_SPRITES, 1 },	// 8000-efff physical
	{ 0xf000, 0xffff, GFXTYPE_SCROLL1, 1 },
	{ -1 }
};

static const struct gfx_range mapper_DM620[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x1fff, GFXTYPE_SPRITES, 0 },
	{ 0x2000, 0x3fff, GFXTYPE_SCROLL1, 0 },
	{ 0x4000, 0x7fff, GFXTYPE_SCROLL2, 0 },

	{ 0x8000, 0x9fff, GFXTYPE_SCROLL3, 1 },
	{ 0x2000, 0x7fff, GFXTYPE_SPRITES, 1 },	// a000-ffff physical
	{ -1 }
};

static const struct gfx_range mapper_ST24M1[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x03ff, GFXTYPE_STARS,   0 },
	{ 0x0400, 0x4fff, GFXTYPE_SPRITES, 0 },
	{ 0x5000, 0x7fff, GFXTYPE_SCROLL2, 0 },

	{ 0x0000, 0x6fff, GFXTYPE_SCROLL3, 1 },	// 8000-efff physical
	{ 0x7000, 0x7fff, GFXTYPE_SCROLL1, 1 },	// f000-ffff physical
	{ -1 }
};

static const struct gfx_range mapper_dynwar[] =	// unknown PAL
{
	/* start  end     type             bank */
	{ 0x0000, 0x5fff, GFXTYPE_SPRITES, 0 },
	{ 0x6000, 0x7fff, GFXTYPE_SCROLL1, 0 },

	{ 0x0000, 0x3fff, GFXTYPE_SCROLL3, 1 },	// 8000-bfff physical
	{ 0x4000, 0x7fff, GFXTYPE_SCROLL2, 1 },	// c000-ffff physical
	{ -1 }
};

static const struct gfx_range mapper_WL24B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x4fff, GFXTYPE_SPRITES, 0 },
	{ 0x5000, 0x6fff, GFXTYPE_SCROLL3, 0 },
	{ 0x7000, 0x7fff, GFXTYPE_SCROLL1, 0 },

	{ 0x0000, 0x3fff, GFXTYPE_SCROLL2, 1 },	// 8000-bfff physical
	{ 0xc000, 0xffff, GFXTYPE_UNKNOWN, 1 },	// empty
	{ -1 }
};

static const struct gfx_range mapper_S224B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x43ff, GFXTYPE_SPRITES, 0 },
	{ 0x4400, 0x4bff, GFXTYPE_SCROLL1, 0 },
	{ 0x4c00, 0x5fff, GFXTYPE_SCROLL3, 0 },
	{ 0x6000, 0x7fff, GFXTYPE_SCROLL2, 0 },
	{ -1 }
};

static const struct gfx_range mapper_YI24B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x1fff, GFXTYPE_SPRITES, 0 },
	{ 0x2000, 0x3fff, GFXTYPE_SCROLL3, 0 },
	{ 0x4000, 0x47ff, GFXTYPE_SCROLL1, 0 },
	{ 0x4800, 0x7fff, GFXTYPE_SCROLL2, 0 },
	{ -1 }
};

static const struct gfx_range mapper_AR24B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x2fff, GFXTYPE_SPRITES, 0 },
	{ 0x3000, 0x3fff, GFXTYPE_SCROLL1, 0 },
	{ 0x4000, 0x5fff, GFXTYPE_SCROLL2, 0 },
	{ 0x6000, 0x7fff, GFXTYPE_SCROLL3, 0 },
	{ -1 }
};

static const struct gfx_range mapper_0224B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x0bff, GFXTYPE_SCROLL1, 0 },
	{ 0x0c00, 0x3bff, GFXTYPE_SCROLL2, 0 },
	{ 0x3c00, 0x4bff, GFXTYPE_SCROLL3, 0 },
	{ 0x4c00, 0x7fff, GFXTYPE_SPRITES, 0 },

	{ 0x8000, 0xa7ff, GFXTYPE_SPRITES, 1 },
	{ 0xa800, 0xb7ff, GFXTYPE_SCROLL2, 1 },
	{ 0xb800, 0xbfff, GFXTYPE_SCROLL3, 1 },
	{ 0xc000, 0xffff, GFXTYPE_UNKNOWN, 1 },	// empty
	{ -1 }
};

static const struct gfx_range mapper_MS24B[] =	// verified from the PAL
{
	/* start  end     type             bank */
	{ 0x0000, 0x3fff, GFXTYPE_SPRITES, 0 },
	{ 0x4000, 0x4fff, GFXTYPE_SCROLL1, 0 },
	{ 0x5000, 0x6fff, GFXTYPE_SCROLL2, 0 },
	{ 0x7000, 0x7fff, GFXTYPE_SCROLL3, 0 },
	{ -1 }
};

static const struct gfx_range mapper_CK24B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x2fff, GFXTYPE_SPRITES, 0 },
	{ 0x3000, 0x3fff, GFXTYPE_SCROLL1, 0 },
	{ 0x4000, 0x6fff, GFXTYPE_SCROLL2, 0 },
	{ 0x7000, 0x7fff, GFXTYPE_SCROLL3, 0 },
	{ -1 }
};

static const struct gfx_range mapper_NM24B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x3fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x4000, 0x47ff, GFXTYPE_SCROLL1,                   0 },
	{ 0x4800, 0x67ff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x6800, 0x7fff, GFXTYPE_SCROLL3,                   0 },
	{ -1 }
};

static const struct gfx_range mapper_CA24B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x2fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x3000, 0x4fff, GFXTYPE_SCROLL3,                   0 },
	{ 0x5000, 0x57ff, GFXTYPE_SCROLL1,                   0 },
	{ 0x5800, 0x7fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ -1 }
};

static const struct gfx_range mapper_STF29[] =
{
	/* start   end      type             bank */
	{ 0x00000, 0x07fff, GFXTYPE_SPRITES, 0 },

	{ 0x08000, 0x0ffff, GFXTYPE_SPRITES, 1 },

	{ 0x10000, 0x11fff, GFXTYPE_SPRITES, 2 },
	{ 0x02000, 0x03fff, GFXTYPE_SCROLL3, 2 },	// 12000-13fff physical
	{ 0x04000, 0x04fff, GFXTYPE_SCROLL1, 2 },	// 14000-14fff physical
	{ 0x05000, 0x07fff, GFXTYPE_SCROLL2, 2 },	// 15000-17fff physical
	{ -1 }
};

static const struct gfx_range mapper_RT24B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x53ff, GFXTYPE_SPRITES,                   0 },
	{ 0x5400, 0x6fff, GFXTYPE_SCROLL1,                   0 },
	{ 0x7000, 0x7fff, GFXTYPE_SCROLL3,                   0 },

	{ 0x0000, 0x27ff, GFXTYPE_SCROLL3,                   1 },	// 8000-a7ff physical
	{ 0x2800, 0x53ff, GFXTYPE_SCROLL2,                   1 },	// a800-d3ff physical
	{ 0x5400, 0x7fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 1 },	// d400-ffff physical
	{ -1 }
};

static const struct gfx_range mapper_KD29B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x7fff, GFXTYPE_SPRITES, 0 },

	{ 0x8000, 0x8fff, GFXTYPE_SPRITES, 1 },
	{ 0x9000, 0xbfff, GFXTYPE_SCROLL2, 1 },
	{ 0xc000, 0xd7ff, GFXTYPE_SCROLL1, 1 },
	{ 0xd800, 0xffff, GFXTYPE_SCROLL3, 1 },
	{ -1 }
};

static const struct gfx_range mapper_CC63B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x7fff, GFXTYPE_SPRITES, 0 },

	{ 0x8000, 0x9fff, GFXTYPE_SPRITES, 1 },
	{ 0xa000, 0xbfff, GFXTYPE_SCROLL3, 1 },
	{ 0xc000, 0xcfff, GFXTYPE_SCROLL1, 1 },
	{ 0xd000, 0xffff, GFXTYPE_SCROLL2, 1 },
	{ -1 }
};

static const struct gfx_range mapper_KR63B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x7fff, GFXTYPE_SPRITES, 0 },

	{ 0x8000, 0x87ff, GFXTYPE_SPRITES, 1 },
	{ 0x8800, 0x97ff, GFXTYPE_SCROLL1, 1 },
	{ 0x9800, 0xcfff, GFXTYPE_SCROLL2, 1 },
	{ 0xd000, 0xffff, GFXTYPE_SCROLL3, 1 },
	{ -1 }
};

static const struct gfx_range mapper_S9263B[] =
{
	/* start  end     type             bank */
	{ 0x00000, 0x07fff, GFXTYPE_SPRITES, 0 },

	{ 0x08000, 0x0ffff, GFXTYPE_SPRITES, 1 },

	{ 0x10000, 0x11fff, GFXTYPE_SPRITES, 2 },
	{ 0x02000, 0x03fff, GFXTYPE_SCROLL3, 2 },	// 12000-13fff physical
	{ 0x04000, 0x04fff, GFXTYPE_SCROLL1, 2 },	// 14000-14fff physical
	{ 0x05000, 0x07fff, GFXTYPE_SCROLL2, 2 },	// 15000-17fff physical
	{ -1 }
};

static const struct gfx_range mapper_VA24B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x57ff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x5800, 0x5fff, GFXTYPE_SCROLL1,                   0 },
	{ 0x6000, 0x7fff, GFXTYPE_SCROLL3,                   0 },
	{ -1 }
};

static const struct gfx_range mapper_Q522B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x6fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x7000, 0x77ff, GFXTYPE_SCROLL3,                   0 },
	{ 0x7800, 0x7fff, GFXTYPE_SCROLL1,                   0 },
	{ -1 }
};

static const struct gfx_range mapper_TK263B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x7fff, GFXTYPE_SPRITES,                   0 },

	{ 0x8000, 0xcfff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 1 },
	{ 0xd000, 0xdfff, GFXTYPE_SCROLL1,                   1 },
	{ 0xe000, 0xffff, GFXTYPE_SCROLL3,                   1 },
	{ -1 }
};

static const struct gfx_range mapper_CD63B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x0fff, GFXTYPE_SCROLL1,                   0 },
	{ 0x1000, 0x7fff, GFXTYPE_SPRITES,                   0 },

	{ 0x8000, 0xdfff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 1 },
	{ 0xe000, 0xffff, GFXTYPE_SCROLL3,                   1 },
	{ -1 }
};

static const struct gfx_range mapper_PS63B[] =
{
	/* start  end     type             bank */
	{ 0x0000, 0x0fff, GFXTYPE_SCROLL1, 0 },
	{ 0x1000, 0x7fff, GFXTYPE_SPRITES, 0 },

	{ 0x8000, 0xa7ff, GFXTYPE_SPRITES, 1 },
	{ 0xa800, 0xdbff, GFXTYPE_SCROLL2, 1 },
	{ 0xdc00, 0xffff, GFXTYPE_SCROLL3, 1 },
	{ -1 }
};

static const struct gfx_range mapper_MB63B[] =
{
	/* start  end     type                                 bank */
	{ 0x00000, 0x00fff, GFXTYPE_SCROLL1,                   0 },
	{ 0x01000, 0x07fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },

	{ 0x08000, 0x0ffff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 1 },

	{ 0x10000, 0x167ff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 2 },
	{ 0x16800, 0x17fff, GFXTYPE_SCROLL3,                   2 },
	{ -1 }
};

static const struct gfx_range mapper_QD22B[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x07ff, GFXTYPE_SCROLL1,                   0 },
	{ 0x0800, 0x27ff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x2800, 0x3fff, GFXTYPE_SCROLL3,                   0 },
	{ 0x4000, 0x7fff, GFXTYPE_UNKNOWN,                   0 },	// empty
	{ -1 }
};

static const struct gfx_range mapper_qadj[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x07ff, GFXTYPE_SCROLL1,                   0 },
	{ 0x0800, 0x1fff, GFXTYPE_SCROLL3,                   0 },
	{ 0x2000, 0x7fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ -1 }
};

static const struct gfx_range mapper_qtono2[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x0fff, GFXTYPE_SCROLL1,                   0 },
	{ 0x1000, 0x3fff, GFXTYPE_SCROLL3,                   0 },
	{ 0x4000, 0x7fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },

	{ 0x8000, 0xffff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 1 },
	{ -1 }
};

static const struct gfx_range mapper_RCM63B[] =
{
	/* start  end     type                                 bank */
	{ 0x00000, 0x017ff, GFXTYPE_SCROLL1,                   0 },
	{ 0x01800, 0x07fff, GFXTYPE_SPRITES,                   0 },

	{ 0x08000, 0x0ffff, GFXTYPE_SPRITES,                   1 },

	{ 0x10000, 0x17fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 2 },

	{ 0x18000, 0x19fff, GFXTYPE_SCROLL2,                   3 },
	{ 0x1a000, 0x1ffff, GFXTYPE_SCROLL3,                   3 },
	{ -1 }
};

static const struct gfx_range mapper_pnickj[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x0fff, GFXTYPE_SCROLL1,                   0 },
	{ 0x1000, 0x5fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },
	{ 0x6000, 0x7fff, GFXTYPE_SCROLL3,                   0 },
	{ -1 }
};

static const struct gfx_range mapper_pang3[] =
{
	/* start  end     type                               bank */
	{ 0x0000, 0x7fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 0 },

	{ 0x8000, 0x9fff, GFXTYPE_SPRITES | GFXTYPE_SCROLL2, 1 },
	{ 0xa000, 0xbfff, GFXTYPE_SCROLL1,                   1 },
	{ 0xc000, 0xffff, GFXTYPE_SCROLL3,                   1 },
	{ -1 }
};



/*
  I don't know if CPS2 ROM boards use PALs as well; since all games seem to be
  well behaved, I'll just assume that there is no strong checking of gfx type.
  (sprites are not listed here because they are addressed linearly by the CPS2
  sprite code)
 */
static const struct gfx_range mapper_cps2[] =
{
	/* start   end      type             bank */
	{ 0x00000, 0x07fff, GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 4 },	// 20000-27fff physical
	{ 0x08000, 0x0ffff, GFXTYPE_SCROLL1 | GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 5 },	// 28000-2ffff physical
	{ 0x10000, 0x17fff,                   GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 6 },	// 30000-37fff physical
	{ 0x18000, 0x1ffff,                   GFXTYPE_SCROLL2 | GFXTYPE_SCROLL3, 7 },	// 38000-3ffff physical
	{ -1 }
};



static const struct CPS1config cps1_config_table[]=
{
	/* name       CPSB     gfx mapper  kludge */
	{"forgottn", CPS_B_01, mapper_LWCH1 },
	{"lostwrld", CPS_B_01, mapper_LWCH1 },
	{"ghouls",   CPS_B_01, mapper_DM620 },
	{"ghoulsu",  CPS_B_01, mapper_DM620 },
	{"daimakai", CPS_B_01, mapper_DM620 },
	{"strider",  CPS_B_01, mapper_ST24M1 },
	{"stridrua", CPS_B_01, mapper_ST24M1 },
	{"striderj", CPS_B_01, mapper_ST24M1 },
	{"stridrja", CPS_B_01, mapper_ST24M1 },
	{"dynwar",   CPS_B_02, mapper_dynwar },
	{"dynwarj",  CPS_B_02, mapper_dynwar },
	{"willow",   CPS_B_03, mapper_WL24B },
	{"willowj",  CPS_B_03, mapper_WL24B },
	{"willowje", CPS_B_03, mapper_WL24B },
	{"ffight",   CPS_B_04, mapper_S224B },
	{"ffightu",  CPS_B_04, mapper_S224B },
	{"ffightua", CPS_B_01, mapper_S224B },
	{"ffightub", CPS_B_05, mapper_S224B },
	{"ffightj",  CPS_B_04, mapper_S224B },
	{"ffightj1", CPS_B_02, mapper_S224B },
	{"1941",     CPS_B_05, mapper_YI24B },
	{"1941j",    CPS_B_05, mapper_YI24B },
	{"unsquad",  CPS_B_11, mapper_AR24B },	/* CPSB ID not checked, but it's the same as sf2eg */
	{"area88",   CPS_B_11, mapper_AR24B },	/* CPSB ID not checked, but it's the same as sf2eg */
	{"mercs",    CPS_B_12, mapper_0224B },	/* (uses port 74) */
	{"mercsu",   CPS_B_12, mapper_0224B },	/* (uses port 74) */
	{"mercsua",  CPS_B_12, mapper_0224B },	/* (uses port 74) */
	{"mercsj",   CPS_B_12, mapper_0224B },	/* (uses port 74) */
	{"msword",   CPS_B_13, mapper_MS24B },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mswordr1", CPS_B_13, mapper_MS24B },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mswordu",  CPS_B_13, mapper_MS24B },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mswordj",  CPS_B_13, mapper_MS24B },	/* CPSB ID not checked, but it's the same as sf2j */
	{"mtwins",   CPS_B_14, mapper_CK24B },
	{"chikij",   CPS_B_14, mapper_CK24B },
	{"nemo",     CPS_B_15, mapper_NM24B },
	{"nemoj",    CPS_B_15, mapper_NM24B },
	{"cawing",   CPS_B_16, mapper_CA24B },
	{"cawingr1", CPS_B_16, mapper_CA24B },
	{"cawingu",  CPS_B_16, mapper_CA24B },
	{"cawingj",  CPS_B_16, mapper_CA24B },
	{"sf2",      CPS_B_11, mapper_STF29 },
	{"sf2eb",    CPS_B_17, mapper_STF29 },
	{"sf2ua",    CPS_B_17, mapper_STF29 },
	{"sf2ub",    CPS_B_17, mapper_STF29 },
	{"sf2ud",    CPS_B_05, mapper_STF29 },
	{"sf2ue",    CPS_B_18, mapper_STF29 },
	{"sf2uf",    CPS_B_15, mapper_STF29 },
	{"sf2ui",    CPS_B_14, mapper_STF29 },
	{"sf2uk",    CPS_B_17, mapper_STF29 }, // check CPS_B
	{"sf2j",     CPS_B_13, mapper_STF29 },
	{"sf2ja",    CPS_B_17, mapper_STF29 },
	{"sf2jc",    CPS_B_12, mapper_STF29 },
	/* from here onwards the CPS-B board has suicide battery and multiply protection */
	{"3wonders", BATTRY_1, mapper_RT24B },
	{"3wonderu", BATTRY_1, mapper_RT24B },
	{"wonder3",  BATTRY_1, mapper_RT24B },
	{"kod",      BATTRY_2, mapper_KD29B },
	{"kodu",     BATTRY_2, mapper_KD29B },
	{"kodj",     BATTRY_2, mapper_KD29B },
	{"kodb",     BATTRY_2, mapper_KD29B },	/* bootleg, doesn't use multiply protection */
	{"captcomm", BATTRY_3, mapper_CC63B },
	{"captcomu", BATTRY_3, mapper_CC63B },
	{"captcomj", BATTRY_3, mapper_CC63B },
	{"knights",  BATTRY_4, mapper_KR63B },
	{"knightsu", BATTRY_4, mapper_KR63B },
	{"knightsj", BATTRY_4, mapper_KR63B },
	{"sf2ce",    NOBATTRY, mapper_S9263B },
	{"sf2ceua",  NOBATTRY, mapper_S9263B },
	{"sf2ceub",  NOBATTRY, mapper_S9263B },
	{"sf2ceuc",  NOBATTRY, mapper_S9263B },
	{"sf2cej",   NOBATTRY, mapper_S9263B },
	{"sf2rb",    NOBATTRY, mapper_S9263B },
	{"sf2rb2",   NOBATTRY, mapper_S9263B },
	{"sf2red",   NOBATTRY, mapper_S9263B },
	{"sf2v004",  NOBATTRY, mapper_S9263B },
	{"sf2accp2", NOBATTRY, mapper_S9263B },
	{"sf2m1",    NOBATTRY, mapper_S9263B },
	{"sf2m2",    NOBATTRY, mapper_S9263B },
	{"sf2m3",    NOBATTRY, mapper_S9263B },
	{"sf2m4",    HACK_B_1, mapper_S9263B, 1 },
	{"sf2m5",    NOBATTRY, mapper_S9263B, 1 },
	{"sf2m6",    NOBATTRY, mapper_S9263B, 1 },
	{"sf2m7",    NOBATTRY, mapper_S9263B, 1 },
	{"sf2yyc",   NOBATTRY, mapper_S9263B, 1 },
	{"sf2koryu", NOBATTRY, mapper_S9263B, 1 },
	{"varth",    CPS_B_04, mapper_VA24B },	/* CPSB test has been patched out (60=0008) */
	{"varthr1",  CPS_B_04, mapper_VA24B },	/* CPSB test has been patched out (60=0008) */
	{"varthu",   CPS_B_04, mapper_VA24B },	/* CPSB test has been patched out (60=0008) */
	{"varthj",   BATTRY_5, mapper_VA24B },	/* CPSB test has been patched out (72=0001) */
	{"cworld2j", BATTRY_6, mapper_Q522B },	/* The 0x76 priority values are incorrect values */
	{"wof",      NOBATTRY, mapper_TK263B },	/* bootleg? */
	{"wofa",     NOBATTRY, mapper_TK263B },	/* bootleg? */
	{"wofu",     QSOUND_1, mapper_TK263B },
	{"wofj",     QSOUND_1, mapper_TK263B },
	{"dino",     QSOUND_2, mapper_CD63B },	/* layer enable never used */
	{"dinou",    QSOUND_2, mapper_CD63B },	/* layer enable never used */
	{"dinoj",    QSOUND_2, mapper_CD63B },	/* layer enable never used */
	{"punisher", QSOUND_3, mapper_PS63B },
	{"punishru", QSOUND_3, mapper_PS63B },
	{"punishrj", QSOUND_3, mapper_PS63B },
	{"slammast", QSOUND_4, mapper_MB63B },
	{"slammasu", QSOUND_4, mapper_MB63B },
	{"mbomberj", QSOUND_4, mapper_MB63B },
	{"mbombrd",  QSOUND_5, mapper_MB63B },
	{"mbombrdj", QSOUND_5, mapper_MB63B },
	{"sf2hf",    NOBATTRY, mapper_S9263B },
	{"sf2t",     NOBATTRY, mapper_S9263B },
	{"sf2tj",    NOBATTRY, mapper_S9263B },
	{"qad",      BATTRY_7, mapper_QD22B },	/* TODO: layer enable */
	{"qadj",     NOBATTRY, mapper_qadj },
	{"qtono2",   NOBATTRY, mapper_qtono2 },
	{"megaman",  NOBATTRY, mapper_RCM63B },
	{"rockmanj", NOBATTRY, mapper_RCM63B },
	{"pnickj",   NOBATTRY, mapper_pnickj },
	{"pang3",    NOBATTRY, mapper_pang3 },	/* EEPROM port is among the CPS registers (handled by DRIVER_INIT) */
	{"pang3j",   NOBATTRY, mapper_pang3 },	/* EEPROM port is among the CPS registers (handled by DRIVER_INIT) */
	#ifdef MESS
	{"sfzch",    NOBATTRY, mapper_cps2 },
	#endif

    /* CPS2 games */
	{"cps2",     NOBATTRY, mapper_cps2 },

	{0}		/* End of table */
};


static int cps_version;
int cps1_scanline1;
int cps1_scanline2;
int cps1_scancalls;


static MACHINE_RESET( cps )
{
	const char *gamename = machine->gamedrv->name;
	const struct CPS1config *pCFG=&cps1_config_table[0];

	while(pCFG->name)
	{
		if (strcmp(pCFG->name, gamename) == 0)
		{
			break;
		}
		pCFG++;
	}
	cps1_game_config=pCFG;

    if (!cps1_game_config->name)
    {
        gamename="cps2";
        pCFG=&cps1_config_table[0];
        while(pCFG->name)
        {
            if (strcmp(pCFG->name, gamename) == 0)
            {
                break;
            }
            pCFG++;
        }
        cps1_game_config=pCFG;
   }

	if (strcmp(gamename, "sf2rb" )==0)
	{
		/* Patch out protection check */
		UINT16 *rom = (UINT16 *)memory_region(REGION_CPU1);
		rom[0xe5464/2] = 0x6012;
	}
	if (strcmp(gamename, "sf2rb2" )==0)
	{
		/* Patch out protection check */
		UINT16 *rom = (UINT16 *)memory_region(REGION_CPU1);
		rom[0xe5332/2] = 0x6014;
	}

#if 0
	if (strcmp(gamename, "sf2accp2" )==0)
	{
		/* Patch out a odd branch which would be incorrectly interpreted
           by the cpu core as a 32-bit branch. This branch would make the
           game crash (address error, since it would branch to an odd address)
           if location 180ca6 (outside ROM space) isn't 0. Protection check? */
		UINT16 *rom = (UINT16 *)memory_region(REGION_CPU1);
		rom[0x11756/2] = 0x4e71;
	}
	else if (strcmp(gamename, "ghouls" )==0)
	{
		/* Patch out self-test... it takes forever */
		UINT16 *rom = (UINT16 *)memory_region(REGION_CPU1);
		rom[0x61964/2] = 0x4ef9;
		rom[0x61966/2] = 0x0000;
		rom[0x61968/2] = 0x0400;
	}
#endif
}


int cps1_port(int offset)
{
	return cps1_output[offset/2];
}

INLINE UINT16 *cps1_base(int offset,int boundary)
{
	int base=cps1_port(offset)*256;
	/*
    The scroll RAM must start on a 0x4000 boundary.
    Some games do not do this.
    For example:
       Captain commando     - continue screen will not display
       Muscle bomber games  - will animate garbage during gameplay
    Mask out the irrelevant bits.
    */
	base &= ~(boundary-1);
 	return &cps1_gfxram[(base&0x3ffff)/2];
}



READ16_HANDLER( cps1_output_r )
{
	if (VERBOSE && offset >= 0x18/2) logerror("PC %06x: read output port %02x\n",activecpu_get_pc(),offset*2);

	/* Some games interrogate a couple of registers on bootup. */
	/* These are CPS1 board B self test checks. They wander from game to */
	/* game. */
	if (offset && offset == cps1_game_config->cpsb_addr/2)
		return cps1_game_config->cpsb_value;

	/* some games use as a protection check the ability to do 16-bit multiplies */
	/* with a 32-bit result, by writing the factors to two ports and reading the */
	/* result from two other ports. */
	if (offset && offset == cps1_game_config->mult_result_lo/2)
		return (cps1_output[cps1_game_config->mult_factor1/2] *
				cps1_output[cps1_game_config->mult_factor2/2]) & 0xffff;
	if (offset && offset == cps1_game_config->mult_result_hi/2)
		return (cps1_output[cps1_game_config->mult_factor1/2] *
				cps1_output[cps1_game_config->mult_factor2/2]) >> 16;

	return cps1_output[offset];
}

WRITE16_HANDLER( cps1_output_w )
{
	data = COMBINE_DATA(&cps1_output[offset]);

	/* To mark scanlines for raster effects */
	if(offset == 0x52/2)
	{
		cps1_scanline2 = (data & 0x1ff);
	}
	if(offset == 0x50/2)
	{
		cps1_scanline1 = (data & 0x1ff);
	}


#ifdef MAME_DEBUG
if (cps1_game_config->control_reg && offset == cps1_game_config->control_reg/2 && data != 0x3f)
	logerror("control_reg = %04x",data);
#endif
if (VERBOSE)
{
if (offset > 0x22/2 &&
		offset != cps1_game_config->layer_control/2 &&
		offset != cps1_game_config->priority[0]/2 &&
		offset != cps1_game_config->priority[1]/2 &&
		offset != cps1_game_config->priority[2]/2 &&
		offset != cps1_game_config->priority[3]/2 &&
		offset != cps1_game_config->control_reg/2)
	logerror("PC %06x: write %02x to output port %02x\n",activecpu_get_pc(),data,offset*2);

#ifdef MAME_DEBUG
if (offset == 0x22/2 && (data & ~0x8001) != 0x0e)
	logerror("port 22 = %04x",data);
if (cps1_game_config->priority[0] && offset == cps1_game_config->priority[0]/2 && data != 0x00)
	popmessage("priority0 %04x",data);
#endif
}
}



/* Public variables */
UINT16 *cps1_gfxram;
UINT16 *cps1_output;

size_t cps1_gfxram_size;
size_t cps1_output_size;

/* Offset of each palette entry */
#define cps1_palette_entries (32*6)  /* Number colour schemes in palette */

static const int cps1_scroll_size =0x4000;	/* scroll1, scroll2, scroll3 */
static const int cps1_obj_size    =0x0800;
static const int cps1_other_size  =0x0800;
static const int cps1_palette_align=0x0800;	/* can't be larger than this, breaks ringdest & batcirc otherwise */
static const int cps1_palette_size=cps1_palette_entries*32; /* Size of palette RAM */

static UINT16 *cps1_scroll1;
static UINT16 *cps1_scroll2;
static UINT16 *cps1_scroll3;
static UINT16 *cps1_obj;
static UINT16 *cps1_buffered_obj;
static UINT16 *cps1_palette;
UINT16 *cps1_other;
static UINT16 *cps1_old_palette;

/* Working variables */
static int cps1_last_sprite_offset;     /* Offset of the last sprite */
static int cps1_stars_enabled[2];          /* Layer enabled [Y/N] */

tilemap *cps1_bg_tilemap[3];


int cps1_scroll1x, cps1_scroll1y;
int cps1_scroll2x, cps1_scroll2y;
int cps1_scroll3x, cps1_scroll3y;
static int stars1x, stars1y, stars2x, stars2y;


/* Output ports */
#define CPS1_OBJ_BASE			0x00    /* Base address of objects */
#define CPS1_SCROLL1_BASE       0x02    /* Base address of scroll 1 */
#define CPS1_SCROLL2_BASE       0x04    /* Base address of scroll 2 */
#define CPS1_SCROLL3_BASE       0x06    /* Base address of scroll 3 */
#define CPS1_OTHER_BASE			0x08    /* Base address of other video */
#define CPS1_PALETTE_BASE       0x0a    /* Base address of palette */
#define CPS1_SCROLL1_SCROLLX    0x0c    /* Scroll 1 X */
#define CPS1_SCROLL1_SCROLLY    0x0e    /* Scroll 1 Y */
#define CPS1_SCROLL2_SCROLLX    0x10    /* Scroll 2 X */
#define CPS1_SCROLL2_SCROLLY    0x12    /* Scroll 2 Y */
#define CPS1_SCROLL3_SCROLLX    0x14    /* Scroll 3 X */
#define CPS1_SCROLL3_SCROLLY    0x16    /* Scroll 3 Y */
#define CPS1_STARS1_SCROLLX     0x18    /* Stars 1 X */
#define CPS1_STARS1_SCROLLY     0x1a    /* Stars 1 Y */
#define CPS1_STARS2_SCROLLX     0x1c    /* Stars 2 X */
#define CPS1_STARS2_SCROLLY     0x1e    /* Stars 2 Y */

#define CPS1_ROWSCROLL_OFFS     0x20    /* base of row scroll offsets in other RAM */

#define CPS1_SCROLL2_WIDTH      0x40
#define CPS1_SCROLL2_HEIGHT     0x40


/*
CPS1 VIDEO RENDERER

*/
/* first 0x4000 of gfx ROM are used, but 0x0000-0x1fff is == 0x2000-0x3fff */
static const int stars_rom_size = 0x2000;

/* PSL: CPS2 support */
static const int cps2_obj_size    =0x2000;
UINT16 *cps2_objram1,*cps2_objram2;
UINT16 *cps2_output;

size_t cps2_output_size;
static UINT16 *cps2_buffered_obj;
static int pri_ctrl;				/* Sprite layer priorities */
static int cps2_objram_bank;
static int cps2_last_sprite_offset;     /* Offset of the last sprite */

#define CPS2_OBJ_BASE	0x00	/* Unknown (not base address of objects). Could be bass address of bank used when object swap bit set? */
#define CPS2_OBJ_UK1	0x02	/* Unknown (nearly always 0x807d, or 0x808e when screen flipped) */
#define CPS2_OBJ_PRI	0x04	/* Layers priorities */
#define CPS2_OBJ_UK2	0x06	/* Unknown (usually 0x0000, 0x1101 in ssf2, 0x0001 in 19XX) */
#define CPS2_OBJ_XOFFS	0x08	/* X offset (usually 0x0040) */
#define CPS2_OBJ_YOFFS	0x0a	/* Y offset (always 0x0010) */

INLINE int cps2_port(int offset)
{
    return cps2_output[offset/2];
}




static void cps1_gfx_decode(void)
{
	int size=memory_region_length(REGION_GFX1);
	int i,j,gfxsize;
	UINT8 *cps1_gfx = memory_region(REGION_GFX1);


	gfxsize=size/4;

	for (i = 0;i < gfxsize;i++)
	{
		UINT32 src = cps1_gfx[4*i] + (cps1_gfx[4*i+1]<<8) + (cps1_gfx[4*i+2]<<16) + (cps1_gfx[4*i+3]<<24);
		UINT32 dwval = 0;

		for (j = 0;j < 8;j++)
		{
			int n = 0;
			UINT32 mask = (0x80808080 >> j) & src;

			if (mask & 0x000000ff) n |= 1;
			if (mask & 0x0000ff00) n |= 2;
			if (mask & 0x00ff0000) n |= 4;
			if (mask & 0xff000000) n |= 8;

			dwval |= n << (j * 4);
		}
		cps1_gfx[4*i  ] = dwval>>0;
		cps1_gfx[4*i+1] = dwval>>8;
		cps1_gfx[4*i+2] = dwval>>16;
		cps1_gfx[4*i+3] = dwval>>24;
	}
}

static void unshuffle(UINT64 *buf,int len)
{
	int i;
	UINT64 t;

	if (len == 2) return;

	assert(len % 4 == 0);   /* must not happen */

	len /= 2;

	unshuffle(buf,len);
	unshuffle(buf + len,len);

	for (i = 0;i < len/2;i++)
	{
		t = buf[len/2 + i];
		buf[len/2 + i] = buf[len + i];
		buf[len + i] = t;
	}
}

static void cps2_gfx_decode(void)
{
	const int banksize=0x200000;
	int size=memory_region_length(REGION_GFX1);
	int i;

	for (i = 0;i < size;i += banksize)
		unshuffle((UINT64 *)(memory_region(REGION_GFX1) + i),banksize/8);

	cps1_gfx_decode();
}


DRIVER_INIT( cps1 )
{
	cps1_gfx_decode();
}



DRIVER_INIT( cps2_video )
{
	cps2_gfx_decode();

	cps1_scanline1 = 262;
	cps1_scanline2 = 262;
	cps1_scancalls = 0;
}


#if CPS1_DUMP_VIDEO
static void cps1_dump_video(void)
{
	FILE *fp;
	fp=fopen("SCROLL1.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_scroll1, cps1_scroll_size, 1, fp);
		fclose(fp);
	}
	fp=fopen("SCROLL2.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_scroll2, cps1_scroll_size, 1, fp);
		fclose(fp);
	}
	fp=fopen("SCROLL3.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_scroll3, cps1_scroll_size, 1, fp);
		fclose(fp);
	}

    fp=fopen("OBJ.DMP", "w+b");
    if (fp)
    {
        fwrite(cps1_obj, cps1_obj_size, 1, fp);
        fclose(fp);
    }
    if (cps_version == 2)
    {
        /* PSL: CPS2 support */
        fp=fopen("OBJCPS2.DMP", "w+b");
        if (fp)
        {
            fwrite(cps2_objram1, cps2_obj_size, 1, fp);
            fwrite(cps2_objram2, cps2_obj_size, 1, fp);
            fclose(fp);
        }
        fp=fopen("CPS2OUTP.DMP", "w+b");
        if (fp)
        {
            fwrite(cps2_output, cps2_output_size, 1, fp);
            fclose(fp);
        }

    }


	fp=fopen("OTHER.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_other, cps1_other_size, 1, fp);
		fclose(fp);
	}

	fp=fopen("PALETTE.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_palette, cps1_palette_size, 1, fp);
		fclose(fp);
	}

	fp=fopen("OUTPUT.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_output, cps1_output_size, 1, fp);
		fclose(fp);
	}
	fp=fopen("VIDEO.DMP", "w+b");
	if (fp)
	{
		fwrite(cps1_gfxram, cps1_gfxram_size, 1, fp);
		fclose(fp);
	}

}
#endif


void cps1_get_video_base(void )
{
	int layercontrol, scroll1xoff, scroll2xoff, scroll3xoff;

	/* Re-calculate the VIDEO RAM base */
	if (cps1_scroll1 != cps1_base(CPS1_SCROLL1_BASE,cps1_scroll_size))
	{
		cps1_scroll1 = cps1_base(CPS1_SCROLL1_BASE,cps1_scroll_size);
		tilemap_mark_all_tiles_dirty(cps1_bg_tilemap[0]);
	}
	if (cps1_scroll2 != cps1_base(CPS1_SCROLL2_BASE,cps1_scroll_size))
	{
		cps1_scroll2 = cps1_base(CPS1_SCROLL2_BASE,cps1_scroll_size);
		tilemap_mark_all_tiles_dirty(cps1_bg_tilemap[1]);
	}
	if (cps1_scroll3 != cps1_base(CPS1_SCROLL3_BASE,cps1_scroll_size))
	{
		cps1_scroll3 = cps1_base(CPS1_SCROLL3_BASE,cps1_scroll_size);
		tilemap_mark_all_tiles_dirty(cps1_bg_tilemap[2]);
	}

	/* Some of the sf2 hacks use only sprite port 0x9100 and the scroll layers are offset */
	if (cps1_game_config->bootleg_kludge == 1)
	{
		cps1_output[CPS1_OBJ_BASE/2] = 0x9100;
		cps1_obj=cps1_base(CPS1_OBJ_BASE, cps1_obj_size);
		scroll1xoff = -0x0c;
		scroll2xoff = -0x0e;
		scroll3xoff = -0x10;
	}
	else
	{
		cps1_obj=cps1_base(CPS1_OBJ_BASE, cps1_obj_size);
		scroll1xoff = 0;
		scroll2xoff = 0;
		scroll3xoff = 0;
	}
	cps1_palette=cps1_base(CPS1_PALETTE_BASE,cps1_palette_align);
	cps1_other=cps1_base(CPS1_OTHER_BASE,cps1_other_size);

	/* Get scroll values */
	cps1_scroll1x=cps1_port(CPS1_SCROLL1_SCROLLX) + scroll1xoff;
	cps1_scroll1y=cps1_port(CPS1_SCROLL1_SCROLLY);
	cps1_scroll2x=cps1_port(CPS1_SCROLL2_SCROLLX) + scroll2xoff;
	cps1_scroll2y=cps1_port(CPS1_SCROLL2_SCROLLY);
	cps1_scroll3x=cps1_port(CPS1_SCROLL3_SCROLLX) + scroll3xoff;
	cps1_scroll3y=cps1_port(CPS1_SCROLL3_SCROLLY);
	stars1x =cps1_port(CPS1_STARS1_SCROLLX);
	stars1y =cps1_port(CPS1_STARS1_SCROLLY);
	stars2x =cps1_port(CPS1_STARS2_SCROLLX);
	stars2y =cps1_port(CPS1_STARS2_SCROLLY);

	/* Get layer enable bits */
	layercontrol=cps1_port(cps1_game_config->layer_control);
	tilemap_set_enable(cps1_bg_tilemap[0],layercontrol & cps1_game_config->layer_enable_mask[0]);
	tilemap_set_enable(cps1_bg_tilemap[1],layercontrol & cps1_game_config->layer_enable_mask[1]);
	tilemap_set_enable(cps1_bg_tilemap[2],layercontrol & cps1_game_config->layer_enable_mask[2]);
	cps1_stars_enabled[0] = layercontrol & cps1_game_config->layer_enable_mask[3];
	cps1_stars_enabled[1] = layercontrol & cps1_game_config->layer_enable_mask[4];


#ifdef MAME_DEBUG
{
	int enablemask;

#if 0
if (input_code_pressed(KEYCODE_Z))
{
	if (input_code_pressed(KEYCODE_Q)) cps1_layer_enabled[3]=0;
	if (input_code_pressed(KEYCODE_W)) cps1_layer_enabled[2]=0;
	if (input_code_pressed(KEYCODE_E)) cps1_layer_enabled[1]=0;
	if (input_code_pressed(KEYCODE_R)) cps1_layer_enabled[0]=0;
	if (input_code_pressed(KEYCODE_T))
	{
		popmessage("%d %d %d %d layer %02x",
			(layercontrol>>0x06)&03,
			(layercontrol>>0x08)&03,
			(layercontrol>>0x0a)&03,
			(layercontrol>>0x0c)&03,
			layercontrol&0xc03f
			);
	}

}
#endif

	enablemask = 0;
	if (cps1_game_config->layer_enable_mask[0] == cps1_game_config->layer_enable_mask[1])
		enablemask = cps1_game_config->layer_enable_mask[0];
	if (cps1_game_config->layer_enable_mask[0] == cps1_game_config->layer_enable_mask[2])
		enablemask = cps1_game_config->layer_enable_mask[0];
	if (cps1_game_config->layer_enable_mask[1] == cps1_game_config->layer_enable_mask[2])
		enablemask = cps1_game_config->layer_enable_mask[1];
	if (enablemask)
	{
		if (((layercontrol & enablemask) && (layercontrol & enablemask) != enablemask))
			popmessage("layer %02x",layercontrol&0xc03f);
	}
}
#endif

{
	int enablemask;
	enablemask = cps1_game_config->layer_enable_mask[0] | cps1_game_config->layer_enable_mask[1]
			| cps1_game_config->layer_enable_mask[2]
			| cps1_game_config->layer_enable_mask[3] | cps1_game_config->layer_enable_mask[4];
	if (((layercontrol & ~enablemask) & 0xc03e) != 0)
		popmessage("layer %02x contact MAMEDEV",layercontrol&0xc03f);
}

}


WRITE16_HANDLER( cps1_gfxram_w )
{
	int page = (offset >> 7) & 0x3c0;
	COMBINE_DATA(&cps1_gfxram[offset]);

	if (page == (cps1_port(CPS1_SCROLL1_BASE) & 0x3c0))
		tilemap_mark_tile_dirty(cps1_bg_tilemap[0],offset/2 & 0x0fff);
	if (page == (cps1_port(CPS1_SCROLL2_BASE) & 0x3c0))
		tilemap_mark_tile_dirty(cps1_bg_tilemap[1],offset/2 & 0x0fff);
	if (page == (cps1_port(CPS1_SCROLL3_BASE) & 0x3c0))
		tilemap_mark_tile_dirty(cps1_bg_tilemap[2],offset/2 & 0x0fff);
}



static int gfxrom_bank_mapper(running_machine *machine, int type, int code)
{
	const struct gfx_range *range = cps1_game_config->bank_mapper;
	int shift = 0;

	assert(range);

	switch (type)
	{
		case GFXTYPE_SPRITES: shift = 1; break;
		case GFXTYPE_SCROLL1: shift = 0; break;
		case GFXTYPE_SCROLL2: shift = 1; break;
		case GFXTYPE_SCROLL3: shift = 3; break;
	}

	code <<= shift;

	while (range->start != -1)
	{
		if (code >= range->start && code <= range->end)
		{
			if (range->type & type)
				return ((code & 0x7fff) + (range->bank * 0x8000)) >> shift;
		}

		++range;
	}

#ifdef MAME_DEBUG
//	popmessage("tile %02x/%04x out of range", type,code>>shift);
#endif

	return -1;
}


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

static TILEMAP_MAPPER( tilemap0_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x1f) + ((col & 0x3f) << 5) + ((row & 0x20) << 6);
}

static TILEMAP_MAPPER( tilemap1_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x0f) + ((col & 0x3f) << 4) + ((row & 0x30) << 6);
}

static TILEMAP_MAPPER( tilemap2_scan )
{
	/* logical (col,row) -> memory offset */
	return (row & 0x07) + ((col & 0x3f) << 3) + ((row & 0x38) << 6);
}

static UINT8 empty_tile[32*32/2];

static TILE_GET_INFO( get_tile0_info )
{
	int code = cps1_scroll1[2*tile_index];
	int attr = cps1_scroll1[2*tile_index+1];
	int gfxset;

	code = gfxrom_bank_mapper(machine, GFXTYPE_SCROLL1, code);

	/* allows us to reproduce a problem seen with a ffight board where USA and Japanese
		 roms have been mixed to be reproduced (ffightua) -- it looks like each column
		 should alternate between the left and right side of the 16x16 tiles */
	gfxset = (tile_index & 0x20) >> 5;

	SET_TILE_INFO(
			gfxset,
			code,
			(attr & 0x1f) + 0x20,
			TILE_FLIPYX((attr & 0x60) >> 5));
	tileinfo->group = (attr & 0x0180) >> 7;

	// for out of range tiles, switch to fully transparent data
	// (but still call SET_TILE_INFO, otherwise problems might occur on boot e.g. unsquad)
	if (code == -1)
		tileinfo->pen_data = empty_tile;
}

static TILE_GET_INFO( get_tile1_info )
{
	int code = cps1_scroll2[2*tile_index];
	int attr = cps1_scroll2[2*tile_index+1];

	code = gfxrom_bank_mapper(machine, GFXTYPE_SCROLL2, code);

	SET_TILE_INFO(
			2,
			code,
			(attr & 0x1f) + 0x40,
			TILE_FLIPYX((attr & 0x60) >> 5));
	tileinfo->group = (attr & 0x0180) >> 7;

	// for out of range tiles, switch to fully transparent data
	if (code == -1)
		tileinfo->pen_data = empty_tile;
}

static TILE_GET_INFO( get_tile2_info )
{
	int code = cps1_scroll3[2*tile_index] & 0x3fff;
	int attr = cps1_scroll3[2*tile_index+1];

	code = gfxrom_bank_mapper(machine, GFXTYPE_SCROLL3, code);

	SET_TILE_INFO(
			3,
			code,
			(attr & 0x1f) + 0x60,
			TILE_FLIPYX((attr & 0x60) >> 5));
	tileinfo->group = (attr & 0x0180) >> 7;

	// for out of range tiles, switch to fully transparent data
	// (but still call SET_TILE_INFO, otherwise problems might occur on boot e.g. unsquad)
	if (code == -1)
		tileinfo->pen_data = empty_tile;
}



static void cps1_update_transmasks(void)
{
	int i;

	for (i = 0;i < 4;i++)
	{
		int mask;

		/* Get transparency registers */
		if (cps1_game_config->priority[i])
			mask = cps1_port(cps1_game_config->priority[i]) ^ 0xffff;
		else mask = 0xffff;	/* completely transparent if priority masks not defined (mercs, qad) */

		tilemap_set_transmask(cps1_bg_tilemap[0],i,mask,0x8000);
		tilemap_set_transmask(cps1_bg_tilemap[1],i,mask,0x8000);
		tilemap_set_transmask(cps1_bg_tilemap[2],i,mask,0x8000);
	}
}

static VIDEO_START( cps )
{
	int i;

	MACHINE_RESET_CALL(cps);

	cps1_bg_tilemap[0] = tilemap_create(get_tile0_info,tilemap0_scan, 8, 8,64,64);
	cps1_bg_tilemap[1] = tilemap_create(get_tile1_info,tilemap1_scan,16,16,64,64);
	cps1_bg_tilemap[2] = tilemap_create(get_tile2_info,tilemap2_scan,32,32,64,64);

	/* front masks will change at runtime to handle sprite occluding */
	cps1_update_transmasks();

	memset(empty_tile,0xff,sizeof(empty_tile));

	cps1_old_palette=auto_malloc(cps1_palette_size);
	memset(cps1_old_palette, 0x00, cps1_palette_size);
	for (i = 0;i < cps1_palette_entries*16;i++)
	{
		palette_set_color(machine,i,MAKE_RGB(0,0,0));
	}

    cps1_buffered_obj = auto_malloc (cps1_obj_size);
    memset(cps1_buffered_obj, 0x00, cps1_obj_size);

	if (cps_version==2)
	{
		cps2_buffered_obj = auto_malloc (cps2_obj_size);
		memset(cps2_buffered_obj, 0x00, cps2_obj_size);
	}

	memset(cps1_gfxram, 0, cps1_gfxram_size);   /* Clear GFX RAM */
	memset(cps1_output, 0, cps1_output_size);   /* Clear output ports */

	if (cps_version == 2)
	{
		memset(cps2_objram1, 0, cps2_obj_size);
		memset(cps2_objram2, 0, cps2_obj_size);
	}

	/* Put in some defaults */
	cps1_output[CPS1_OBJ_BASE/2]     = 0x9200;
	cps1_output[CPS1_SCROLL1_BASE/2] = 0x9000;
	cps1_output[CPS1_SCROLL2_BASE/2] = 0x9040;
	cps1_output[CPS1_SCROLL3_BASE/2] = 0x9080;
	cps1_output[CPS1_OTHER_BASE/2]   = 0x9100;
	cps1_output[CPS1_PALETTE_BASE/2] = 0x90c0;

	assert_always(cps1_game_config, "cps1_game_config hasn't been set up yet");

	/* Set up old base */
	cps1_get_video_base();   /* Calculate base pointers */
	cps1_get_video_base();   /* Calculate old base pointers */
}

VIDEO_START( cps1 )
{
	cps_version=1;
	VIDEO_START_CALL(cps);
}

VIDEO_START( cps2 )
{
	cps_version=2;
	VIDEO_START_CALL(cps);
}

/***************************************************************************

  Build palette from palette RAM

  12 bit RGB with a 4 bit brightness value.

***************************************************************************/

void cps1_build_palette(running_machine *machine)
{
	int offset;

	for (offset = 0; offset < cps1_palette_entries*16; offset++)
	{
		int palette = cps1_palette[offset];

		if (palette != cps1_old_palette[offset])
		{
			int red, green, blue, bright;

			bright = 0x10 + (palette>>12);

			red   = ((palette>>8)&0x0f) * bright * 0x11 / 0x1f;
			green = ((palette>>4)&0x0f) * bright * 0x11 / 0x1f;
			blue  = ((palette>>0)&0x0f) * bright * 0x11 / 0x1f;

			palette_set_color (machine, offset, MAKE_RGB(red, green, blue));
			cps1_old_palette[offset] = palette;
		}
	}
}



/***************************************************************************

                                Sprites
                                =======

  Sprites are represented by a number of 8 byte values

  xx xx yy yy nn nn aa aa

  where xxxx = x position
        yyyy = y position
        nnnn = tile number
        aaaa = attribute word
                    0x0001  colour
                    0x0002  colour
                    0x0004  colour
                    0x0008  colour
                    0x0010  colour
                    0x0020  X Flip
                    0x0040  Y Flip
                    0x0080  X & Y offset toggle (used in Marvel vs. Capcom.)
                    0x0100  X block size (in sprites)
                    0x0200  X block size
                    0x0400  X block size
                    0x0800  X block size
                    0x1000  Y block size (in sprites)
                    0x2000  Y block size
                    0x4000  Y block size
                    0x8000  Y block size

  The end of the table (may) be marked by an attribute value of 0xff00.

***************************************************************************/

static void cps1_find_last_sprite(void)    /* Find the offset of last sprite */
{
    int offset=0;
	/* Locate the end of table marker */
    while (offset < cps1_obj_size/2)
	{
        int colour=cps1_buffered_obj[offset+3];
		if ((colour & 0xff00) == 0xff00)
		{
			/* Marker found. This is the last sprite. */
            cps1_last_sprite_offset=offset-4;
			return;
		}
        offset+=4;
	}
	/* Sprites must use full sprite RAM */
    cps1_last_sprite_offset=cps1_obj_size/2-4;
}


static void cps1_render_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect)
{
#define DRAWSPRITE(CODE,COLOR,FLIPX,FLIPY,SX,SY)					\
{																	\
	if (flip_screen_get())											\
		pdrawgfx(bitmap,machine->gfx[2],							\
				CODE,												\
				COLOR,												\
				!(FLIPX),!(FLIPY),									\
				511-16-(SX),255-16-(SY),							\
				cliprect,TRANSPARENCY_PEN,15,0x02);					\
	else															\
		pdrawgfx(bitmap,machine->gfx[2],							\
				CODE,												\
				COLOR,												\
				FLIPX,FLIPY,										\
				SX,SY,												\
				cliprect,TRANSPARENCY_PEN,15,0x02);					\
}


	int i, baseadd;
	UINT16 *base=cps1_buffered_obj;

	/* some sf2 hacks draw the sprites in reverse order */
	if (cps1_game_config->bootleg_kludge == 1)
	{
		base += cps1_last_sprite_offset;
		baseadd = -4;
	}
	else
	{
		baseadd = 4;
	}

	for (i=cps1_last_sprite_offset; i>=0; i-=4)
	{
		int x=*(base+0);
		int y=*(base+1);
		int code  =*(base+2);
		int colour=*(base+3);
		int col=colour&0x1f;

//      x-=0x20;
//      y+=0x20;

		code = gfxrom_bank_mapper(machine, GFXTYPE_SPRITES, code);

		if (code != -1)
		{
			if (colour & 0xff00 )
			{
				/* handle blocked sprites */
				int nx=(colour & 0x0f00) >> 8;
				int ny=(colour & 0xf000) >> 12;
				int nxs,nys,sx,sy;
				nx++;
				ny++;

				if (colour & 0x40)
				{
					/* Y flip */
					if (colour &0x20)
					{
						for (nys=0; nys<ny; nys++)
						{
							for (nxs=0; nxs<nx; nxs++)
							{
								sx = (x+nxs*16) & 0x1ff;
								sy = (y+nys*16) & 0x1ff;

								DRAWSPRITE(
//                                      code+(nx-1)-nxs+0x10*(ny-1-nys),
										(code & ~0xf) + ((code + (nx-1) - nxs) & 0xf) + 0x10*(ny-1-nys),
										(col&0x1f),
										1,1,
										sx,sy);
							}
						}
					}
					else
					{
						for (nys=0; nys<ny; nys++)
						{
							for (nxs=0; nxs<nx; nxs++)
							{
								sx = (x+nxs*16) & 0x1ff;
								sy = (y+nys*16) & 0x1ff;

								DRAWSPRITE(
//                                      code+nxs+0x10*(ny-1-nys),
										(code & ~0xf) + ((code + nxs) & 0xf) + 0x10*(ny-1-nys),
										(col&0x1f),
										0,1,
										sx,sy);
							}
						}
					}
				}
				else
				{
					if (colour &0x20)
					{
						for (nys=0; nys<ny; nys++)
						{
							for (nxs=0; nxs<nx; nxs++)
							{
								sx = (x+nxs*16) & 0x1ff;
								sy = (y+nys*16) & 0x1ff;

								DRAWSPRITE(
//                                      code+(nx-1)-nxs+0x10*nys,
										(code & ~0xf) + ((code + (nx-1) - nxs) & 0xf) + 0x10*nys,
										(col&0x1f),
										1,0,
										sx,sy);
							}
						}
					}
					else
					{
						for (nys=0; nys<ny; nys++)
						{
							for (nxs=0; nxs<nx; nxs++)
							{
								sx = (x+nxs*16) & 0x1ff;
								sy = (y+nys*16) & 0x1ff;

								DRAWSPRITE(
//                                      code+nxs+0x10*nys,
										(code & ~0xf) + ((code + nxs) & 0xf) + 0x10*nys,	// fix 00406: qadj: When playing as the ninja, there is one broekn frame in his animation loop when walking.
										(col&0x1f),
										0,0,
										sx,sy);
							}
						}
					}
				}
			}
			else
			{
				/* Simple case... 1 sprite */
						DRAWSPRITE(
						code,
						(col&0x1f),
						colour&0x20,colour&0x40,
						x & 0x1ff,y & 0x1ff);
			}
		}
		base += baseadd;
	}
#undef DRAWSPRITE
}




WRITE16_HANDLER( cps2_objram_bank_w )
{
	if (ACCESSING_BITS_0_7)
	{
		cps2_objram_bank = data & 1;
	}
}

READ16_HANDLER( cps2_objram1_r )
{
	if (cps2_objram_bank & 1)
		return cps2_objram2[offset];
	else
		return cps2_objram1[offset];
}

READ16_HANDLER( cps2_objram2_r )
{
	if (cps2_objram_bank & 1)
		return cps2_objram1[offset];
	else
		return cps2_objram2[offset];
}

WRITE16_HANDLER( cps2_objram1_w )
{
	if (cps2_objram_bank & 1)
		COMBINE_DATA(&cps2_objram2[offset]);
	else
		COMBINE_DATA(&cps2_objram1[offset]);
}

WRITE16_HANDLER( cps2_objram2_w )
{
	if (cps2_objram_bank & 1)
		COMBINE_DATA(&cps2_objram1[offset]);
	else
		COMBINE_DATA(&cps2_objram2[offset]);
}

static UINT16 *cps2_objbase(void)
{
	int baseptr;
	baseptr = 0x7000;

	if (cps2_objram_bank & 1) baseptr ^= 0x0080;

//popmessage("%04x %d",cps2_port(CPS2_OBJ_BASE),cps2_objram_bank&1);

	if (baseptr == 0x7000)
		return cps2_objram1;
	else //if (baseptr == 0x7080)
		return cps2_objram2;
}


static void cps2_find_last_sprite(void)    /* Find the offset of last sprite */
{
	int offset=0;
	UINT16 *base=cps2_buffered_obj;

	/* Locate the end of table marker */
	while (offset < cps2_obj_size/2)
	{
		if (base[offset+1]>=0x8000
				|| base[offset+3]>=0xff00)
		{
			/* Marker found. This is the last sprite. */
			cps2_last_sprite_offset=offset-4;
			return;
		}

		offset+=4;
	}
	/* Sprites must use full sprite RAM */
	cps2_last_sprite_offset=cps2_obj_size/2-4;
#undef DRAWSPRITE
}

static void cps2_render_sprites(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int *primasks)
{
#define DRAWSPRITE(CODE,COLOR,FLIPX,FLIPY,SX,SY)									\
{																					\
	if (flip_screen_get())															\
		pdrawgfx(bitmap,machine->gfx[2],											\
				CODE,																\
				COLOR,																\
				!(FLIPX),!(FLIPY),													\
				511-16-(SX),255-16-(SY),											\
				cliprect,TRANSPARENCY_PEN,15,primasks[priority]);					\
	else																			\
		pdrawgfx(bitmap,machine->gfx[2],											\
				CODE,																\
				COLOR,																\
				FLIPX,FLIPY,														\
				SX,SY,																\
				cliprect,TRANSPARENCY_PEN,15,primasks[priority]);					\
}

	int i;
	UINT16 *base=cps2_buffered_obj;
	int xoffs = 64-cps2_port(CPS2_OBJ_XOFFS);
	int yoffs = 16-cps2_port(CPS2_OBJ_YOFFS);

#ifdef MAME_DEBUG
	if (input_code_pressed(KEYCODE_Z) && input_code_pressed(KEYCODE_R))
	{
		return;
	}
#endif

	for (i=cps2_last_sprite_offset; i>=0; i-=4)
	{
		int x=base[i+0];
		int y=base[i+1];
		int priority=(x>>13)&0x07;
		int code  = base[i+2]+((y & 0x6000) <<3);
		int colour= base[i+3];
		int col=colour&0x1f;

		if(colour & 0x80)
		{
			x += cps2_port(CPS2_OBJ_XOFFS);  /* fix the offset of some games */
			y += cps2_port(CPS2_OBJ_YOFFS);  /* like Marvel vs. Capcom ending credits */
		}

		if (colour & 0xff00 )
		{
			/* handle blocked sprites */
			int nx=(colour & 0x0f00) >> 8;
			int ny=(colour & 0xf000) >> 12;
			int nxs,nys,sx,sy;
			nx++;
			ny++;

			if (colour & 0x40)
			{
				/* Y flip */
				if (colour &0x20)
				{
					for (nys=0; nys<ny; nys++)
					{
						for (nxs=0; nxs<nx; nxs++)
						{
							sx = (x+nxs*16+xoffs) & 0x3ff;
							sy = (y+nys*16+yoffs) & 0x3ff;
							DRAWSPRITE(
									code+(nx-1)-nxs+0x10*(ny-1-nys),
									(col&0x1f),
									1,1,
									sx,sy);
						}
					}
				}
				else
				{
					for (nys=0; nys<ny; nys++)
					{
						for (nxs=0; nxs<nx; nxs++)
						{
							sx = (x+nxs*16+xoffs) & 0x3ff;
							sy = (y+nys*16+yoffs) & 0x3ff;

							DRAWSPRITE(
									code+nxs+0x10*(ny-1-nys),
									(col&0x1f),
									0,1,
									sx,sy);
						}
					}
				}
			}
			else
			{
				if (colour &0x20)
				{
					for (nys=0; nys<ny; nys++)
					{
						for (nxs=0; nxs<nx; nxs++)
						{
							sx = (x+nxs*16+xoffs) & 0x3ff;
							sy = (y+nys*16+yoffs) & 0x3ff;

							DRAWSPRITE(
									code+(nx-1)-nxs+0x10*nys,
									(col&0x1f),
									1,0,
									sx,sy);
						}
					}
				}
				else
				{
					for (nys=0; nys<ny; nys++)
					{
						for (nxs=0; nxs<nx; nxs++)
						{
							sx = (x+nxs*16+xoffs) & 0x3ff;
							sy = (y+nys*16+yoffs) & 0x3ff;

							DRAWSPRITE(
//                                      code+nxs+0x10*nys,
									(code & ~0xf) + ((code + nxs) & 0xf) + 0x10*nys,	//  pgear fix
									(col&0x1f),
									0,0,
									sx,sy);
						}
					}
				}
			}
		}
		else
		{
			/* Simple case... 1 sprite */
			DRAWSPRITE(
					code,
					(col&0x1f),
					colour&0x20,colour&0x40,
					(x+xoffs) & 0x3ff,(y+yoffs) & 0x3ff);
		}
	}
}




static void cps1_render_stars(const device_config *screen, bitmap_t *bitmap,const rectangle *cliprect)
{
	int offs;
	UINT8 *stars_rom = memory_region(REGION_GFX2);

	if (!stars_rom && (cps1_stars_enabled[0] || cps1_stars_enabled[1]))
	{
#ifdef MAME_DEBUG
		popmessage("stars enabled but no stars ROM");
#endif
		return;
	}

	if (cps1_stars_enabled[0])
	{
		for (offs = 0;offs < stars_rom_size/2;offs++)
		{
			int col = stars_rom[8*offs+4];
			if (col != 0x0f)
			{
				int sx = (offs / 256) * 32;
				int sy = (offs % 256);
				sx = (sx - stars2x + (col & 0x1f)) & 0x1ff;
				sy = (sy - stars2y) & 0xff;
				if (flip_screen_get())
				{
					sx = 511 - sx;
					sy = 255 - sy;
				}

				col = ((col & 0xe0) >> 1) + (video_screen_get_frame_number(screen)/16 & 0x0f);

				if (sx >= cliprect->min_x && sx <= cliprect->max_x &&
					sy >= cliprect->min_y && sy <= cliprect->max_y)
					*BITMAP_ADDR16(bitmap, sy, sx) = 0xa00 + col;
			}
		}
	}

	if (cps1_stars_enabled[1])
	{
		for (offs = 0;offs < stars_rom_size/2;offs++)
		{
			int col = stars_rom[8*offs];
			if (col != 0x0f)
			{
				int sx = (offs / 256) * 32;
				int sy = (offs % 256);
				sx = (sx - stars1x + (col & 0x1f)) & 0x1ff;
				sy = (sy - stars1y) & 0xff;
				if (flip_screen_get())
				{
					sx = 511 - sx;
					sy = 255 - sy;
				}

				col = ((col & 0xe0) >> 1) + (video_screen_get_frame_number(screen)/16 & 0x0f);

				if (sx >= cliprect->min_x && sx <= cliprect->max_x &&
					sy >= cliprect->min_y && sy <= cliprect->max_y)
					*BITMAP_ADDR16(bitmap, sy, sx) = 0x800 + col;
			}
		}
	}
}


static void cps1_render_layer(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int layer,int primask)
{
	switch (layer)
	{
		case 0:
			cps1_render_sprites(machine,bitmap,cliprect);
			break;
		case 1:
		case 2:
		case 3:
			tilemap_draw(bitmap,cliprect,cps1_bg_tilemap[layer-1],TILEMAP_DRAW_LAYER1,primask);
			break;
	}
}

static void cps1_render_high_layer(bitmap_t *bitmap, const rectangle *cliprect, int layer)
{
	switch (layer)
	{
		case 0:
			/* there are no high priority sprites */
			break;
		case 1:
		case 2:
		case 3:
			tilemap_draw(NULL,cliprect,cps1_bg_tilemap[layer-1],TILEMAP_DRAW_LAYER0,1);
			break;
	}
}


/***************************************************************************

    Refresh screen

***************************************************************************/

VIDEO_UPDATE( cps1 )
{
	int layercontrol,l0,l1,l2,l3;
	int videocontrol=cps1_port(0x22);


	flip_screen_set(videocontrol & 0x8000);

	layercontrol = cps1_output[cps1_game_config->layer_control/2];

	/* Get video memory base registers */
	cps1_get_video_base();

	/* Find the offset of the last sprite in the sprite table */
	cps1_find_last_sprite();
	if (cps_version == 2)
	{
		cps2_find_last_sprite();
	}
	/* Build palette */
	cps1_build_palette(screen->machine);

	cps1_update_transmasks();

	tilemap_set_scrollx(cps1_bg_tilemap[0],0,cps1_scroll1x);
	tilemap_set_scrolly(cps1_bg_tilemap[0],0,cps1_scroll1y);
	if (videocontrol & 0x01)	/* linescroll enable */
	{
		int scrly=-cps1_scroll2y;
		int i;
		int otheroffs;

		tilemap_set_scroll_rows(cps1_bg_tilemap[1],1024);

		otheroffs = cps1_port(CPS1_ROWSCROLL_OFFS);

		for (i = 0;i < 256;i++)
			tilemap_set_scrollx(cps1_bg_tilemap[1],(i - scrly) & 0x3ff,cps1_scroll2x + cps1_other[(i + otheroffs) & 0x3ff]);
	}
	else
	{
		tilemap_set_scroll_rows(cps1_bg_tilemap[1],1);
		tilemap_set_scrollx(cps1_bg_tilemap[1],0,cps1_scroll2x);
	}
	tilemap_set_scrolly(cps1_bg_tilemap[1],0,cps1_scroll2y);
	tilemap_set_scrollx(cps1_bg_tilemap[2],0,cps1_scroll3x);
	tilemap_set_scrolly(cps1_bg_tilemap[2],0,cps1_scroll3y);


	/* Blank screen */
	fillbitmap(bitmap,0xbff,cliprect);

	cps1_render_stars(screen, bitmap,cliprect);

	/* Draw layers (0 = sprites, 1-3 = tilemaps) */
	l0 = (layercontrol >> 0x06) & 03;
	l1 = (layercontrol >> 0x08) & 03;
	l2 = (layercontrol >> 0x0a) & 03;
	l3 = (layercontrol >> 0x0c) & 03;
	fillbitmap(priority_bitmap,0,cliprect);

	if (cps_version == 1)
	{
		cps1_render_layer(screen->machine,bitmap,cliprect,l0,0);
		if (l1 == 0) cps1_render_high_layer(bitmap,cliprect,l0); /* prepare mask for sprites */
		cps1_render_layer(screen->machine,bitmap,cliprect,l1,0);
		if (l2 == 0) cps1_render_high_layer(bitmap,cliprect,l1); /* prepare mask for sprites */
		cps1_render_layer(screen->machine,bitmap,cliprect,l2,0);
		if (l3 == 0) cps1_render_high_layer(bitmap,cliprect,l2); /* prepare mask for sprites */
		cps1_render_layer(screen->machine,bitmap,cliprect,l3,0);
	}
	else
	{
		int l0pri,l1pri,l2pri,l3pri;
		int primasks[8],i;
		l0pri = (pri_ctrl >> 4*l0) & 0x0f;
		l1pri = (pri_ctrl >> 4*l1) & 0x0f;
		l2pri = (pri_ctrl >> 4*l2) & 0x0f;
		l3pri = (pri_ctrl >> 4*l3) & 0x0f;

#if 0
if (	(cps2_port(CPS2_OBJ_BASE) != 0x7080 && cps2_port(CPS2_OBJ_BASE) != 0x7000) ||
		cps2_port(CPS2_OBJ_UK1) != 0x807d ||
		(cps2_port(CPS2_OBJ_UK2) != 0x0000 && cps2_port(CPS2_OBJ_UK2) != 0x1101 && cps2_port(CPS2_OBJ_UK2) != 0x0001) ||
	popmessage("base %04x uk1 %04x uk2 %04x",
			cps2_port(CPS2_OBJ_BASE),
			cps2_port(CPS2_OBJ_UK1),
			cps2_port(CPS2_OBJ_UK2));

if (0 && input_code_pressed(KEYCODE_Z))
	popmessage("order: %d (%d) %d (%d) %d (%d) %d (%d)",l0,l0pri,l1,l1pri,l2,l2pri,l3,l3pri);
#endif

		/* take out the CPS1 sprites layer */
		if (l0 == 0) { l0 = l1; l1 = 0; l0pri = l1pri; }
		if (l1 == 0) { l1 = l2; l2 = 0; l1pri = l2pri; }
		if (l2 == 0) { l2 = l3; l3 = 0; l2pri = l3pri; }

		{
			int mask0 = 0xaa;
			int mask1 = 0xcc;
			if(l0pri>l1pri) mask0 &= ~0x88;
			if(l0pri>l2pri) mask0 &= ~0xa0;
			if(l1pri>l2pri) mask1 &= ~0xc0;

			primasks[0] = 0xff;
			for (i = 1;i < 8;i++)
			{
				if (i <= l0pri && i <= l1pri && i <= l2pri)
				{
					primasks[i] = 0xfe;
					continue;
				}
				primasks[i] = 0;
				if (i <= l0pri) primasks[i] |= mask0;
				if (i <= l1pri) primasks[i] |= mask1;
				if (i <= l2pri) primasks[i] |= 0xf0;
			}
		}

		cps1_render_layer(screen->machine,bitmap,cliprect,l0,1);
		cps1_render_layer(screen->machine,bitmap,cliprect,l1,2);
		cps1_render_layer(screen->machine,bitmap,cliprect,l2,4);
		cps2_render_sprites(screen->machine,bitmap,cliprect,primasks);
	}

#if CPS1_DUMP_VIDEO
	if (input_code_pressed(KEYCODE_F))
	{
		cps1_dump_video();
	}
#endif
	return 0;
}

VIDEO_EOF( cps1 )
{
	/* Get video memory base registers */
	cps1_get_video_base();

	if (cps_version == 1)
	{
		/* CPS1 sprites have to be delayed one frame */
		memcpy(cps1_buffered_obj, cps1_obj, cps1_obj_size);
	}
}

void cps2_set_sprite_priorities(void)
{
	pri_ctrl = cps2_port(CPS2_OBJ_PRI);
}

void cps2_objram_latch(void)
{
	cps2_set_sprite_priorities();
	memcpy(cps2_buffered_obj, cps2_objbase(), cps2_obj_size);
}

