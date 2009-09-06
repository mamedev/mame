/*******************************************************************************

Taito Custom ICs
================

Thanks to Suzuki2go for his videos of Metal Black which made better
emulation of TC0480SCP row and column effects possible.

TODO
----

Tidy ups of new transparency-related code in TC0080VCO and TC0480SCP.
Merge bryan_draw_scanline with its F3 equivalent?

TC0080VCO seems bugged, various sigsevs after games have been reset.

sizeof() s in the TC0110PCR section are probably unnecessary?

                ---

PC080SN
-------
Tilemap generator. Two tilemaps, with gfx data fetched from ROM.
Darius uses 3xPC080SN and has double width tilemaps. (NB: it
has not been verified that Topspeed uses this chip. Possibly
it had a variant with added rowscroll capability.)

Standard memory layout (two 64x64 tilemaps with 8x8 tiles)

0000-3fff BG
4000-41ff BG rowscroll      (only verified to exist on Topspeed)
4200-7fff unknown/unused?
8000-bfff FG    (FG/BG layer order fixed per game; Topspeed has BG on top)
c000-c1ff FG rowscroll      (only verified to exist on Topspeed)
c200-ffff unknown/unused?

Double width memory layout (two 128x64 tilemaps with 8x8 tiles)

0000-7fff BG
8000-ffff FG
(Tile layout is different; tiles and colors are separated:
0x0000-3fff  color / flip words
0x4000-7fff  tile number words)

Control registers

+0x20000 (on from tilemaps)
000-001 BG scroll Y
002-003 FG scroll Y

+0x40000
000-001 BG scroll X
002-003 FG scroll X

+0x50000 control word (written infrequently, only 2 bits used)
       ---------------x flip screen
       ----------x----- 0x20 poked here in Topspeed init, followed
                        by zero (Darius does the same).



PC090OJ
-------

        Information from Raine (todo: reformat)

        OBJECT RAM
        ----------

        - 8 bytes/sprite
        - 256 sprites (0x800 bytes)
        - First sprite has *highest* priority

        -----+--------+-------------------------
        Byte | Bit(s) | Use
        -----+76543210+-------------------------
          0  |.x......| Flip Y Axis
          0  |x.......| Flip X Axis
          1  |....xxxx| Colour Bank
          2  |.......x| Sprite Y
          3  |xxxxxxxx| Sprite Y
          4  |...xxxxx| Sprite Tile
          5  |xxxxxxxx| Sprite Tile
          6  |.......x| Sprite X
          7  |xxxxxxxx| Sprite X
        -----+--------+-------------------------

        SPRITE CONTROL
        --------------

        - Maze of Flott [603D MASK] 201C 200B 200F
        - Earth Joker 001C
        - Cadash 0011 0013 0010 0000

        -----+--------+-------------------------
        Byte | Bit(s) | Use
        -----+76543210+-------------------------
          0  |.......x| ?
          0  |......x.| Write Acknowledge?
          0  |..xxxx..| Colour Bank Offset
          0  |xx......| Unused
          1  |...xxxxx| Unused
          1  |..x.....| BG1:Sprite Priority
          1  |.x......| Priority?
          1  |x.......| Unused
        -----+--------+-------------------------

        OLD SPRITE CONTROL (RASTAN TYPE)
        --------------------------------

        -----+--------+-------------------------
        Byte | Bit(s) | Use
        -----+76543210+-------------------------
          1  |.......x| BG1:Sprite Priority?
          1  |......x.| Write Acknowledge?
          1  |xxx.....| Colour Bank Offset
        -----+--------+-------------------------



TC0080VCO
---------
Combined tilemap and motion object generator. The front tilemap
fetches 3bpp gfx data from ram and only has 8 colors available.
The other tilemaps use ROMs as usual. The same gfx set is used
for both tilemaps and motion objects.

There are two 64x64 tilemaps with 16x16 tiles; the optional
front tilemap is 64x64 with 8x8 tiles.

00000-00fff gfx data for FG0        (lo 2 bits per pixel)
01000-01fff FG0 (64x64)             (two tilenums per word, no color bits)
02000-0bfff chain ram               (sprite tile mapping/colors)
0c000-0dfff BG0 tile numbers (64x64)
0e000-0ffff BG1 tile numbers (64x64)
10000-10fff gfx data for FG0        (hi bit per pixel: Ainferno proves not 4bpp)
11000-11fff unknown / unused ?
12000-1bfff chain ram               (sprite tile mapping/colors)
1c000-1dfff BG0 color / flip bits (64x64)
1e000-1ffff BG1 color / flip bits (64x64)
20000-203ff BG0 rowscroll           (see Dleague title screen *)
20400-207ff spriteram
20800-2080f control registers

[*only used in Dleague AFAIK. Note 0x200 words is not enough for a
64x16 => 0x400 pixel high tilemap. So probably it wraps around and
each rowscroll word affects 2 separate lines. Tacky, but wouldn't be
noticeable unless y zoom more than halved the apparent pixel height
meaning you could see more than half of the total tilemap...]

[There is an oddity with this chip: FG0 areas can be addressed
as chain ram, since the offsets used in spriteram are from the
start of the TC0080VCO address space - not the start of chain ram.
In practice only Dleague seems to do this, for c.10 frames as the
pitcher bowls, and I think it's a coding error. Log it and see.]

[Ainferno and Syvalion are only games using FG0 layer.]

Control registers

000-001 ----xx---------- screen invert
        ------xx-------- unknown (always set)
        ---------------- any others ???

002-003 BG0 scroll X  (0x3ff is the tilemap span)
004-005 BG1 scroll X
006-007 BG0 scroll Y  (0x3ff is the tilemap span)
008-009 BG1 scroll Y
00a-00b unknown (Syvalion - FG0 scroll? - and Recordbr)
00c-00d BG0 zoom (hi byte=X, lo byte=Y *)
00e-00f BG1 zoom (hi byte=X, lo byte=Y *)

[* X zoom normal=0x3f   Y zoom normal=0x7f]

All we know is that as y zoom gets bigger the magnification grows:
this seems to be the only zoom feature actually used in the games.


TC0100SCN
---------
Tilemap generator. The front tilemap fetches gfx data from RAM,
the others use ROMs as usual.

Standard memory layout (three 64x64 tilemaps with 8x8 tiles)

0000-3fff BG0
4000-5fff FG0
6000-6fff gfx data for FG0
7000-7fff unused (probably)
8000-bfff BG1
c000-c3ff BG0 rowscroll (second half unused*)
c400-c7ff BG1 rowscroll (second half unused*)
c800-dfff unused (probably)
e000-e0ff BG0 colscroll [see info below]
e100-ffff unused (probably)

Double width tilemaps memory layout (two 128x64 tilemaps, one 128x32 tilemap)

00000-07fff BG0 (128x64)
08000-0ffff BG1 (128x64)
10000-103ff BG0 rowscroll (second half unused*)
10400-107ff BG1 rowscroll (second half unused*)
10800-108ff BG0 colscroll [evidenced by Warriorb inits from $1634]
10900-10fff unused (probably)
11000-11fff gfx data for FG0
12000-13fff FG0 (128x32)

* Perhaps Taito wanted potential for double height tilemaps on the
  TC0100SCN. The inits state the whole area is "linescroll".

Control registers

000-001 BG0 scroll X
002-003 BG1 scroll X
004-005 FG0 scroll X
006-007 BG0 scroll Y
008-009 BG1 scroll Y
00a-00b FG0 scroll Y
00c-00d ---------------x BG0 disable
        --------------x- BG1 disable
        -------------x-- FG0 disable
        ------------x--- change priority order from BG0-BG1-FG0 to BG1-BG0-FG0
        -----------x---- double width tilemaps + different memory map
                              (cameltru and all the multi-screen games)
        ----------x----- unknown (set in most of the TaitoZ games and Cadash)
00e-00f ---------------x flip screen
        ----------x----- this TC0100SCN is subsidiary [= not the main one]
                              (Multi-screen games only. Could it mean: "write
                               through what is written into main TC0100SCN" ?)
        --x------------- unknown (thunderfox)


Colscroll [standard layout]
=========

The e000-ff area is not divided into two halves, it appears to refer only
to bg1 - the top most layer unless bg0/1 are flipped.

128 words are available in 0xe0?? area. Every word scrolls 8
pixels.

Growl
-----
This uses column scroll in the boat scene [that's just after you have
disposed of the fat men in fezzes] and in the underground lava cavern
scene.

Boat scene: code from $2eb58 appears to be doing see-saw motion for
water layer under boat. $e08c is highest word written, it oscillates
between fffa and 0005. Going back towards $e000 a middle point is reached
which sticks at zero. By $e000 written values oscillate again.

A total of 80 words are being written to [some below 0xe000, I think those
won't do anything, sloppy coding...]

Cavern scene: code from $3178a moves a sequence of 0s, 1s and 0x1ffs
along. These words equate to 0, +1, -1 so will gently ripple bg 0
up and down adding to the shimmering heat effect.

Ninja Kids
----------
This uses column scroll in the fat flame boss scene [that's the end of
round 2] and in the last round in the final confrontation with Satan scene.

Fat flame boss: code at $8eee moves a sequence of 1s and 0s along. This
is similar to the heat shimmer in Growl cavern scene.

Final boss: code at $a024 moves a sine wave of values 0-4 along. When
you are close to getting him the range of values expands to 0-10.

Gunfront
--------
In demo mode when the boss appears with the clouds, a sequence of 40 words
forming sine wave between 0xffd0 and ffe0 is moved along. Bg0 has been
given priority over bg1 so it's the foreground (clouds) affected.

The 40 words will affect 40 8-pixel columns [rows, as this game is
rotated] i.e. what is visible on screen at any point.

Galmedes
--------
Towards end of first round in empty starfield area, about three big ship
sprites cross the screen (scrolling down with the starfield). 16 starfield
columns [rows, as the game is rotated] scroll across with the ship.
$84fc0 and neighbouring routines poke col scroll area.



TC0150ROD
---------
Road generator. Two roads allow for forking. Gfx data fetched from ROM.
Refer to notes below.



TC0280GRD
TC0430GRW
---------
These generate a zooming/rotating tilemap. The TC0280GRD has to be used in
pairs, while the TC0430GRW is a newer, single-chip solution.
Regardless of the hardware differences, the two are functionally identical
except for incxx and incxy, which need to be multiplied by 2 in the TC0280GRD
to bring them to the same scale of the other parameters (maybe the chip has
half-pixel resolution?).

control registers:
000-003 start x
004-005 incxx
006-007 incyx
008-00b start y
00c-00d incxy
00e-00f incyy



TC0360PRI
---------
Priority manager
A higher priority value means higher priority. 0 could mean disable but
I'm not sure. If two inputs have the same priority value, I think the first
one has priority, but I'm not sure of that either.
It seems the chip accepts three inputs from three different sources, and
each one of them can declare to have four different priority levels.

000 Top two bits indicate special blend mode (see taito_f2.c).  Other bits unused?
001 in games with a roz layer, this is the roz palette bank (bottom 6 bits
    affect roz color, top 2 bits affect priority)
002 unknown
003 unknown

004 ----xxxx \       priority level 0 (usually FG1 if present)
    xxxx---- | Input priority level 1 (usually FG0)
005 ----xxxx |   #1  priority level 2 (usually BG0)
    xxxx---- /       priority level 3 (usually BG1)

006 ----xxxx \       priority level 0 (usually sprites with top color bits 00)
    xxxx---- | Input priority level 1 (usually sprites with top color bits 01)
007 ----xxxx |   #2  priority level 2 (usually sprites with top color bits 10)
    xxxx---- /       priority level 3 (usually sprites with top color bits 11)

008 ----xxxx \       priority level 0 (e.g. roz layer if top bits of register 001 are 00)
    xxxx---- | Input priority level 1 (e.g. roz layer if top bits of register 001 are 01)
009 ----xxxx |   #3  priority level 2 (e.g. roz layer if top bits of register 001 are 10)
    xxxx---- /       priority level 3 (e.g. roz layer if top bits of register 001 are 11)

00a unused
00b unused
00c unused
00d unused
00e unused
00f unused



TC0480SCP
---------
Tilemap generator, has four zoomable tilemaps with 16x16 tiles.
It also has a front tilemap with 8x8 tiles which fetches gfx data
from RAM.

BG2 and 3 are "special" layers which have row zoom and source
columnscroll. The selectable layer priority order is a function
of the need to have the "special" layers in particular priority
positions.

Standard memory layout (four 32x32 bg tilemaps, one 64x64 fg tilemap)

0000-0fff BG0
1000-1fff BG1
2000-2fff BG2
3000-3fff BG3
4000-43ff BG0 rowscroll
4400-47ff BG1 rowscroll
4800-4bff BG2 rowscroll
4c00-4fff BG3 rowscroll
5000-53ff BG0 rowscroll low order bytes (see info below)
5400-57ff BG1 rowscroll low order bytes
5800-5bff BG2 rowscroll low order bytes
5c00-5fff BG3 rowscroll low order bytes
6000-63ff BG2 row zoom
6400-67ff BG3 row zoom
6800-6bff BG2 source colscroll
6c00-6fff BG3 source colscroll
7000-bfff unknown/unused?
c000-dfff FG0
e000-ffff gfx data for FG0 (4bpp)

Double width tilemaps memory layout (four 64x32 bg tilemaps, one 64x64 fg tilemap)

0000-1fff BG0
2000-3fff BG1
4000-5fff BG2
6000-7fff BG3
8000-83ff BG0 rowscroll
8400-87ff BG1 rowscroll
8800-8bff BG2 rowscroll
8c00-8fff BG3 rowscroll
9000-93ff BG0 rowscroll low order bytes (used for accuracy with row zoom or layer zoom)
9400-97ff BG1 rowscroll low order bytes [*]
9800-9bff BG2 rowscroll low order bytes
9c00-9fff BG3 rowscroll low order bytes
a000-a3ff BG2 row zoom [+]
a400-a7ff BG3 row zoom
a800-abff BG2 source colscroll
ac00-afff BG3 source colscroll
b000-bfff unknown (Slapshot and Superchs poke in TBA OVER error message in FG0 format)
c000-dfff FG0
e000-ffff gfx data for FG0 (4bpp)

[* Gunbustr suggests that high bytes are irrelevant: it leaves them
all zeroed. Superchs is the only game which uses high bytes that
aren't the low byte of the main rowscroll (Footchmp/Undrfire have
this verified in the code).]

[+ Usual row zoom values are 0 - 0x7f. Gunbustr also uses 0x80-d0
approx. Undrfire keeps to the 0-0x7f range but oddly also uses
the high byte with a mask of 0x3f. Meaning of this high byte is
unknown.]

Bg layers tile word layout

+0x00   %yx..bbbb cccccccc      b=control bits(?) c=color .=unused(?)
+0x02   tilenum
[y=yflip x=xflip b=unknown seen in Metalb]

Control registers

000-001 BG0 x scroll    (layer priority order is definable)
002-003 BG1 x scroll
004-005 BG2 x scroll
006-007 BG3 x scroll
008-009 BG0 y scroll
00a-00b BG1 y scroll
00c-00d BG2 y scroll
00e-00f BG3 y scroll
010-011 BG0 zoom        (high byte = X zoom, low byte = Y zoom,
012-013 BG1 zoom         compression is allowed on Y axis only)
014-015 BG2 zoom
016-017 BG3 zoom
018-019 Text layer x scroll
01a-01b Text layer y scroll
01c-01d Unused (not written)
01e-01f Layer Control register
        x-------    Double width tilemaps (4 bg tilemaps become 64x32, and the
                    memory layout changes). Slapshot changes this on the fly.
        -x------    Flip screen
        --x-----    unknown

                Set in Metalb init by whether a byte in prg ROM $7fffe is zero.
                Subsequently Metalb changes it for some screen layer layouts.
                Footchmp clears it, Hthero sets it [then both leave it alone].
                Deadconx code at $10e2 is interesting, with possible values of:
                0x0, 0x20, 0x40, 0x60 poked in (via ram buffer) to control reg,
                dependent on byte in prg ROM $7fffd and whether screen is flipped.

        ---xxx--    BG layer priority order

        ...000..    0  1  2  3
        ...001..    1  2  3  0  (no evidence of this)
        ...010..    2  3  0  1  (no evidence of this)
        ...011..    3  0  1  2
        ...100..    3  2  1  0
        ...101..    2  1  0  3  [Gunbustr attract and Metalb (c) screen]
        ...110..    1  0  3  2  (no evidence of this)
        ...111..    0  3  2  1

        ------x-    BG3 row zoom enable
        -------x    BG2 row zoom enable

020-021 BG0 dx  (provides extra precision to x-scroll, only changed with xscroll)
022-023 BG1 dx
024-025 BG2 dx
026-027 BG3 dx
028-029 BG0 dy  (provides extra precision to y-scroll, only changed with yscroll)
02a-02b BG1 dy
02c-02d BG2 dy
02e-02f BG3 dy

[see code at $1b4a in Slapshot and $xxxxx in Undrfire for evidence of row areas]


TC0110PCR
---------
Interface to palette RAM, and simple tilemap/sprite priority handler. The
priority order seems to be fixed.
The data bus is 16 bits wide.

000  W selects palette RAM address
002 RW read/write palette RAM
004  W unknown, often written to


TC0220IOC
---------
A simple I/O interface with integrated watchdog.
It has four address inputs, which would suggest 16 bytes of addressing space,
but only the first 8 seem to be used.

000 R  IN00-07 (DSA)
000  W watchdog reset
001 R  IN08-15 (DSB)
002 R  IN16-23 (1P)
002  W unknown. Usually written on startup: initialize?
003 R  IN24-31 (2P)
004 RW coin counters and lockout
005  W unknown
006  W unknown
007 R  INB0-7 (coin)


TC0510NIO
---------
Newer version of the I/O chip

000 R  DSWA
000  W watchdog reset
001 R  DSWB
001  W unknown (ssi)
002 R  1P
003 R  2P
003  W unknown (yuyugogo, qzquest and qzchikyu use it a lot)
004 RW coin counters and lockout
005  W unknown
006  W unknown (koshien and pulirula use it a lot)
007 R  coin


TC0640FIO
---------
Newer version of the I/O chip ?


***************************************************************************/

#include "driver.h"
#include "drawgfxm.h"
#include "taitoic.h"

#define TOPSPEED_ROAD_COLORS


/* These scanline drawing routines lifted from Taito F3: optimise / merge ? */

INLINE void taitoic_drawscanline(
		bitmap_t *bitmap,const rectangle *cliprect,int x,int y,
		const UINT16 *src,int transparent,UINT32 orient,bitmap_t *priority,int pri)
{
	UINT16 *dsti = BITMAP_ADDR16(bitmap, y, x);
	UINT8 *dstp = BITMAP_ADDR8(priority, y, x);
	int length=cliprect->max_x - cliprect->min_x + 1;

	src+=cliprect->min_x;
	dsti+=cliprect->min_x;
	dstp+=cliprect->min_x;
	if (transparent) {
		while (length--) {
			UINT32 spixel = *src++;
			if (spixel<0x7fff) {
				*dsti = spixel;
				*dstp = pri;
			}
			dsti++;
			dstp++;
		}
	} else { /* Not transparent case */
		while (length--) {
			*dsti++ = *src++;
			*dstp++ = pri;
		}
	}
}




/***************************************************************************/

/* Note: various assumptions are made in these routines, typically that
   only CPU#0 is of interest. If in doubt, check the routine. */

static int has_write_handler8(const device_config *cpu, write8_space_func handler)
{
	if (cpu != NULL)
	{
		const address_space *space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM);
		const address_map_entry *entry;

		if (space != NULL && space->map != NULL)
			for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
				if (entry->write.shandler8 == handler)
					return 1;
	}

	return 0;
}

static int has_write_handler(const device_config *cpu, write16_space_func handler)
{
	if (cpu != NULL)
	{
		const address_space *space = cpu_get_address_space(cpu, ADDRESS_SPACE_PROGRAM);
		const address_map_entry *entry;

		if (space != NULL && space->map != NULL)
			for (entry = space->map->entrylist; entry != NULL; entry = entry->next)
				if (entry->write.shandler16 == handler)
					return 1;
	}

	return 0;
}

int TC0100SCN_count(running_machine *machine)
{
	int mask = (has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0100SCN_word_0_w) ||
		has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0100SCN_dual_screen_w) ||
		has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0100SCN_triple_screen_w)) ? 1 : 0;
	mask |= has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0100SCN_word_1_w) << 1;
	mask |= has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0100SCN_word_2_w) << 2;

	/* Catch illegal configurations */
	/* TODO: we should give an appropriate warning */
	assert_always(mask == 0 || mask == 1 || mask == 3 || mask == 7, "Invalid TC0110PCR configuration");
	return BIT(mask, 0) + BIT(mask, 1) + BIT(mask, 2);
}


int TC0110PCR_mask(running_machine *machine)
{
	int mask = (has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0110PCR_word_w) ||
			has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0110PCR_step1_word_w) ||
			has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0110PCR_step1_rbswap_word_w) ||
			has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0110PCR_step1_4bpg_word_w)) ? 1 : 0;
	mask |= has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0110PCR_step1_word_1_w) << 1;
	mask |= has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0110PCR_step1_word_2_w) << 2;
	return mask;
}

int has_TC0150ROD(running_machine *machine)
{
	return	has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0150ROD_word_w) ||
			has_write_handler(cputag_get_cpu(machine, "audiocpu"), TC0150ROD_word_w) ||
			has_write_handler(cputag_get_cpu(machine, "sub"), TC0150ROD_word_w);
}


int has_TC0280GRD(running_machine *machine)
{
	return	has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0280GRD_word_w);
}


int has_TC0360PRI(running_machine *machine)
{
	return	has_write_handler8(cputag_get_cpu(machine, "maincpu"), TC0360PRI_w);
}


int has_TC0430GRW(running_machine *machine)
{
	return	has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0430GRW_word_w);
}


int has_TC0480SCP(running_machine *machine)
{
	return	has_write_handler(cputag_get_cpu(machine, "maincpu"), TC0480SCP_word_w);
}



/***************************************************************************/



#define PC080SN_RAM_SIZE 0x10000
#define PC080SN_MAX_CHIPS 2
static int PC080SN_chips;

static UINT16 PC080SN_ctrl[PC080SN_MAX_CHIPS][8];

static UINT16 *PC080SN_ram[PC080SN_MAX_CHIPS],
				*PC080SN_bg_ram[PC080SN_MAX_CHIPS][2],
				*PC080SN_bgscroll_ram[PC080SN_MAX_CHIPS][2];

static int PC080SN_bgscrollx[PC080SN_MAX_CHIPS][2],PC080SN_bgscrolly[PC080SN_MAX_CHIPS][2];
static int PC080SN_xoffs,PC080SN_yoffs;

static tilemap *PC080SN_tilemap[PC080SN_MAX_CHIPS][2];
static int PC080SN_bg_gfx[PC080SN_MAX_CHIPS];
static int PC080SN_yinvert,PC080SN_dblwidth;

INLINE void common_get_PC080SN_bg_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum)
{
	UINT16 code,attr;

	if (!PC080SN_dblwidth)
	{
		code = (ram[2*tile_index + 1] & 0x3fff);
		attr = ram[2*tile_index];
	}
	else
	{
		code = (ram[tile_index + 0x2000] & 0x3fff);
		attr = ram[tile_index];
	}

	SET_TILE_INFO(
			gfxnum,
			code,
			(attr & 0x1ff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_PC080SN_fg_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum)
{
	UINT16 code,attr;

	if (!PC080SN_dblwidth)
	{
		code = (ram[2*tile_index + 1] & 0x3fff);
		attr = ram[2*tile_index];
	}
	else
	{
		code = (ram[tile_index + 0x2000] & 0x3fff);
		attr = ram[tile_index];
	}

	SET_TILE_INFO(
			gfxnum,
			code,
			(attr & 0x1ff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO( PC080SN_get_bg_tile_info_0 )
{
	common_get_PC080SN_bg_tile_info(machine,tileinfo,tile_index,PC080SN_bg_ram[0][0],PC080SN_bg_gfx[0]);
}

static TILE_GET_INFO( PC080SN_get_fg_tile_info_0 )
{
	common_get_PC080SN_fg_tile_info(machine,tileinfo,tile_index,PC080SN_bg_ram[0][1],PC080SN_bg_gfx[0]);
}

static TILE_GET_INFO( PC080SN_get_bg_tile_info_1 )
{
	common_get_PC080SN_bg_tile_info(machine,tileinfo,tile_index,PC080SN_bg_ram[1][0],PC080SN_bg_gfx[1]);
}

static TILE_GET_INFO( PC080SN_get_fg_tile_info_1 )
{
	common_get_PC080SN_fg_tile_info(machine,tileinfo,tile_index,PC080SN_bg_ram[1][1],PC080SN_bg_gfx[1]);
}

static const tile_get_info_func PC080SN_get_tile_info[PC080SN_MAX_CHIPS][2] =
{
	{ PC080SN_get_bg_tile_info_0, PC080SN_get_fg_tile_info_0 },
	{ PC080SN_get_bg_tile_info_1, PC080SN_get_fg_tile_info_1 }
};

static STATE_POSTLOAD( PC080SN_restore_scroll )
{
	int chip = (FPTR)param;
	int flip;

	PC080SN_bgscrollx[chip][0] = -PC080SN_ctrl[chip][0];
	PC080SN_bgscrollx[chip][1] = -PC080SN_ctrl[chip][1];
	PC080SN_bgscrolly[chip][0] = -PC080SN_ctrl[chip][2];
	PC080SN_bgscrolly[chip][1] = -PC080SN_ctrl[chip][3];

	flip = (PC080SN_ctrl[chip][4] & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	tilemap_set_flip(PC080SN_tilemap[chip][0],flip);
	tilemap_set_flip(PC080SN_tilemap[chip][1],flip);
}


/* opaque parameter is no longer supported and will be removed */

void PC080SN_vh_start(running_machine *machine,int chips,int gfxnum,int x_offset,int y_offset,int y_invert,
				int opaque,int dblwidth)
{
	int i;

	assert(chips <= PC080SN_MAX_CHIPS);
	PC080SN_chips = chips;

	PC080SN_yinvert = y_invert;
	PC080SN_dblwidth = dblwidth;
	PC080SN_xoffs = x_offset;
	PC080SN_yoffs = y_offset;

	for (i = 0;i < chips;i++)
	{
		int xd,yd;

		if (!PC080SN_dblwidth)	/* standard tilemaps */
		{
			PC080SN_tilemap[i][0] = tilemap_create(machine, PC080SN_get_tile_info[i][0],tilemap_scan_rows,8,8,64,64);
			PC080SN_tilemap[i][1] = tilemap_create(machine, PC080SN_get_tile_info[i][1],tilemap_scan_rows,8,8,64,64);
		}
		else	/* double width tilemaps */
		{
			PC080SN_tilemap[i][0] = tilemap_create(machine, PC080SN_get_tile_info[i][0],tilemap_scan_rows,8,8,128,64);
			PC080SN_tilemap[i][1] = tilemap_create(machine, PC080SN_get_tile_info[i][1],tilemap_scan_rows,8,8,128,64);
		}

		PC080SN_ram[i] = auto_alloc_array_clear(machine, UINT16, PC080SN_RAM_SIZE/2);

		PC080SN_bg_ram[i][0]       = PC080SN_ram[i] + 0x0000 /2;
		PC080SN_bg_ram[i][1]       = PC080SN_ram[i] + 0x8000 /2;
		PC080SN_bgscroll_ram[i][0] = PC080SN_ram[i] + 0x4000 /2;
		PC080SN_bgscroll_ram[i][1] = PC080SN_ram[i] + 0xc000 /2;

		state_save_register_item_pointer(machine, "PC080SN", NULL, i, PC080SN_ram[i], PC080SN_RAM_SIZE/2);
		state_save_register_item_array(machine, "PC080SN", NULL, i, PC080SN_ctrl[i]);
		state_save_register_postload(machine, PC080SN_restore_scroll, (void *)(FPTR)i);

		/* use the given gfx set for bg tiles */
		PC080SN_bg_gfx[i] = gfxnum;

		tilemap_set_transparent_pen(PC080SN_tilemap[i][0],0);
		tilemap_set_transparent_pen(PC080SN_tilemap[i][1],0);

		/* I'm setting optional chip #2 with the same offsets (Topspeed) */
		xd = (i == 0) ? -x_offset : -x_offset;
		yd = (i == 0) ? y_offset : y_offset;

		tilemap_set_scrolldx(PC080SN_tilemap[i][0],-16 + xd,-16 - xd);
		tilemap_set_scrolldy(PC080SN_tilemap[i][0],yd,-yd);
		tilemap_set_scrolldx(PC080SN_tilemap[i][1],-16 + xd,-16 - xd);
		tilemap_set_scrolldy(PC080SN_tilemap[i][1],yd,-yd);

		if (!PC080SN_dblwidth)
		{
			tilemap_set_scroll_rows(PC080SN_tilemap[i][0],512);
			tilemap_set_scroll_rows(PC080SN_tilemap[i][1],512);
		}
	}
}

READ16_HANDLER( PC080SN_word_0_r )
{
	return PC080SN_ram[0][offset];
}

READ16_HANDLER( PC080SN_word_1_r )
{
	return PC080SN_ram[1][offset];
}

static void PC080SN_word_w(int chip,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&PC080SN_ram[chip][offset]);
	if (!PC080SN_dblwidth)
	{
		if (offset < 0x2000)
			tilemap_mark_tile_dirty(PC080SN_tilemap[chip][0],offset / 2);
		else if (offset >= 0x4000 && offset < 0x6000)
			tilemap_mark_tile_dirty(PC080SN_tilemap[chip][1],(offset & 0x1fff) / 2);
	}
	else
	{
		if (offset < 0x4000)
			tilemap_mark_tile_dirty(PC080SN_tilemap[chip][0],(offset & 0x1fff));
		else if (offset >= 0x4000 && offset < 0x8000)
			tilemap_mark_tile_dirty(PC080SN_tilemap[chip][1],(offset & 0x1fff));
	}
}

WRITE16_HANDLER( PC080SN_word_0_w )
{
	PC080SN_word_w(0,offset,data,mem_mask);
}

WRITE16_HANDLER( PC080SN_word_1_w )
{
	PC080SN_word_w(1,offset,data,mem_mask);
}

static void PC080SN_xscroll_word_w(int chip,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&PC080SN_ctrl[chip][offset]);

	data = PC080SN_ctrl[chip][offset];

	switch (offset)
	{
		case 0x00:
			PC080SN_bgscrollx[chip][0] = -data;
			break;

		case 0x01:
			PC080SN_bgscrollx[chip][1] = -data;
			break;
	}
}

static void PC080SN_yscroll_word_w(int chip,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&PC080SN_ctrl[chip][offset+2]);

	data = PC080SN_ctrl[chip][offset+2];
	if (PC080SN_yinvert)
		data = -data;

	switch (offset)
	{
		case 0x00:
			PC080SN_bgscrolly[chip][0] = -data;
			break;

		case 0x01:
			PC080SN_bgscrolly[chip][1] = -data;
			break;
	}
}

static void PC080SN_ctrl_word_w(int chip,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&PC080SN_ctrl[chip][offset+4]);

	data = PC080SN_ctrl[chip][offset+4];

	switch (offset)
	{
		case 0x00:
		{
			int flip = (data & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

			tilemap_set_flip(PC080SN_tilemap[chip][0],flip);
			tilemap_set_flip(PC080SN_tilemap[chip][1],flip);
			break;
		}
	}
#if 0
	popmessage("PC080SN ctrl = %4x",data);
#endif
}

WRITE16_HANDLER( PC080SN_xscroll_word_0_w )
{
	PC080SN_xscroll_word_w(0,offset,data,mem_mask);
}

WRITE16_HANDLER( PC080SN_xscroll_word_1_w )
{
	PC080SN_xscroll_word_w(1,offset,data,mem_mask);
}

WRITE16_HANDLER( PC080SN_yscroll_word_0_w )
{
	PC080SN_yscroll_word_w(0,offset,data,mem_mask);
}

WRITE16_HANDLER( PC080SN_yscroll_word_1_w )
{
	PC080SN_yscroll_word_w(1,offset,data,mem_mask);
}

WRITE16_HANDLER( PC080SN_ctrl_word_0_w )
{
	PC080SN_ctrl_word_w(0,offset,data,mem_mask);
}

WRITE16_HANDLER( PC080SN_ctrl_word_1_w )
{
	PC080SN_ctrl_word_w(1,offset,data,mem_mask);
}

/* This routine is needed as an override by Jumping, which
   doesn't set proper scroll values for foreground tilemap */

void PC080SN_set_scroll(int chip,int tilemap_num,int scrollx,int scrolly)
{
	tilemap_set_scrollx(PC080SN_tilemap[chip][tilemap_num],0,scrollx);
	tilemap_set_scrolly(PC080SN_tilemap[chip][tilemap_num],0,scrolly);
}

/* This routine is needed as an override by Jumping */

void PC080SN_set_trans_pen(int chip,int tilemap_num,int pen)
{
	tilemap_set_transparent_pen(PC080SN_tilemap[chip][tilemap_num],pen);
}


void PC080SN_tilemap_update(void)
{
	int chip,j;

	for (chip = 0;chip < PC080SN_chips;chip++)
	{
		tilemap_set_scrolly(PC080SN_tilemap[chip][0],0,PC080SN_bgscrolly[chip][0]);
		tilemap_set_scrolly(PC080SN_tilemap[chip][1],0,PC080SN_bgscrolly[chip][1]);

		if (!PC080SN_dblwidth)
		{
			for (j = 0;j < 256;j++)
				tilemap_set_scrollx(PC080SN_tilemap[chip][0],
						(j + PC080SN_bgscrolly[chip][0]) & 0x1ff,
						PC080SN_bgscrollx[chip][0] - PC080SN_bgscroll_ram[chip][0][j]);
			for (j = 0;j < 256;j++)
				tilemap_set_scrollx(PC080SN_tilemap[chip][1],
						(j + PC080SN_bgscrolly[chip][1]) & 0x1ff,
						PC080SN_bgscrollx[chip][1] - PC080SN_bgscroll_ram[chip][1][j]);
		}
		else
		{
			tilemap_set_scrollx(PC080SN_tilemap[chip][0],0,PC080SN_bgscrollx[chip][0]);
			tilemap_set_scrollx(PC080SN_tilemap[chip][1],0,PC080SN_bgscrollx[chip][1]);
		}
	}
}


static UINT16 topspeed_get_road_pixel_color(UINT16 pixel,UINT16 color)
{
	UINT16 road_body_color,off_road_color,pixel_type;

	/* Color changes based on screenshots from game flyer */
	pixel_type = (pixel % 0x10);
	road_body_color = (pixel &0x7ff0) + 4;
	off_road_color = road_body_color + 1;

	if ((color &0xffe0) == 0xffe0)
	{
		pixel += 10;	/* Tunnel colors */
		road_body_color += 10;
		off_road_color  += 10;
	}
	else
	{
		/* Unsure which way round these bits go */
		if (color &0x10)	road_body_color += 5;
		if (color &0x02)	off_road_color  += 5;
	}

	switch (pixel_type)
	{
		case 0x01:		/* Center lines */
		{
			if (color &0x08)	pixel = road_body_color;
			break;
		}
		case 0x02:		/* Road edge (inner) */
		{
			if (color &0x08)	pixel = road_body_color;
			break;
		}
		case 0x03:		/* Road edge (outer) */
		{
			if (color &0x04)	pixel = road_body_color;
			break;
		}
		case 0x04:		/* Road body */
		{
			pixel = road_body_color;
			break;
		}
		case 0x05:		/* Off road */
			pixel = off_road_color;
		default:
		{}
	}
	return pixel;
}


static void topspeed_custom_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int chip,int layer,int flags,
							UINT32 priority,UINT16 *color_ctrl_ram)
{
	UINT16 *dst16,*src16;
	UINT8 *tsrc;
	UINT16 scanline[1024];	/* won't be called by a wide-screen game, but just in case... */

	bitmap_t *srcbitmap = tilemap_get_pixmap(PC080SN_tilemap[chip][layer]);
	bitmap_t *flagsbitmap = tilemap_get_flagsmap(PC080SN_tilemap[chip][layer]);

	UINT16 a,color;
	int sx,x_index;
	int i,y,y_index,src_y_index,row_index;

	int flip = 0;
	int machine_flip = 0;	/* for  ROT 180 ? */

	int min_x = cliprect->min_x;
	int max_x = cliprect->max_x;
	int min_y = cliprect->min_y;
	int max_y = cliprect->max_y;
	int screen_width = max_x - min_x + 1;
	int width_mask = 0x1ff;	/* underlying tilemap */

	if (!flip)
	{
		sx =       PC080SN_bgscrollx[chip][layer] + 16 - PC080SN_xoffs;
		y_index =  PC080SN_bgscrolly[chip][layer] + min_y - PC080SN_yoffs;
	}
	else	// never used
	{
		sx = 0;
		y_index = 0;
	}

	if (!machine_flip) y = min_y; else y = max_y;

	do
	{
		src_y_index = y_index &0x1ff;	/* tilemaps are 512 px up/down */
		row_index = (src_y_index - PC080SN_bgscrolly[chip][layer]) &0x1ff;
		color = color_ctrl_ram[(row_index + PC080SN_yoffs - 2) &0xff];

		x_index = sx - (PC080SN_bgscroll_ram[chip][layer][row_index]);

		src16 = BITMAP_ADDR16(srcbitmap, src_y_index, 0);
		tsrc  = BITMAP_ADDR8(flagsbitmap, src_y_index, 0);
		dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (i=0; i<screen_width; i++)
			{
				a = src16[x_index & width_mask];
#ifdef TOPSPEED_ROAD_COLORS
				a = topspeed_get_road_pixel_color(a,color);
#endif
				*dst16++ = a;
				x_index++;
			}
		}
		else
		{
			for (i=0; i<screen_width; i++)
			{
				if (tsrc[x_index & width_mask])
				{
					a = src16[x_index & width_mask];
#ifdef TOPSPEED_ROAD_COLORS
					a = topspeed_get_road_pixel_color(a,color);
#endif
					*dst16++ = a;
				}
				else
					*dst16++ = 0x8000;
				x_index++;
			}
		}

		taitoic_drawscanline(bitmap,cliprect,0,y,scanline,(flags & TILEMAP_DRAW_OPAQUE)?0:1,ROT0,machine->priority_bitmap,priority);

		y_index++;
		if (!machine_flip) y++; else y--;
	}
	while ( (!machine_flip && y <= max_y) || (machine_flip && y >= min_y) );
}


void PC080SN_tilemap_draw(bitmap_t *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority)
{
	tilemap_draw(bitmap,cliprect,PC080SN_tilemap[chip][layer],flags,priority);
}

void PC080SN_tilemap_draw_offset(bitmap_t *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority,int xoffs,int yoffs)
{
	int basedx = -16 - PC080SN_xoffs;
	int basedxflip = -16 + PC080SN_xoffs;
	int basedy = PC080SN_yoffs;
	int basedyflip = -PC080SN_yoffs;

	tilemap_set_scrolldx(PC080SN_tilemap[chip][layer], basedx + xoffs, basedxflip + xoffs);
	tilemap_set_scrolldy(PC080SN_tilemap[chip][layer], basedy + yoffs, basedyflip + yoffs);
	tilemap_draw(bitmap,cliprect,PC080SN_tilemap[chip][layer],flags,priority);
	tilemap_set_scrolldx(PC080SN_tilemap[chip][layer], basedx, basedxflip);
	tilemap_set_scrolldy(PC080SN_tilemap[chip][layer], basedy, basedyflip);
}

void PC080SN_tilemap_draw_special(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority,UINT16 *ram)
{
	topspeed_custom_draw(machine,bitmap,cliprect,chip,layer,flags,priority,ram);
}





/***************************************************************************/


#define PC090OJ_RAM_SIZE 0x4000
#define PC090OJ_ACTIVE_RAM_SIZE 0x800

/* NB: PC090OJ_ctrl is the internal register controlling flipping

   PC090OJ_sprite_ctrl is a representation of the hardware OUTSIDE the PC090OJ
   which impacts on sprite plotting, and which varies between games. It
   includes color banking and (optionally) priority. It allows each game to
   control these aspects of the sprites in different ways, while keeping the
   routines here modular.

*/

static UINT16 PC090OJ_ctrl,PC090OJ_buffer,PC090OJ_gfxnum;
UINT16 PC090OJ_sprite_ctrl;

static UINT16 *PC090OJ_ram,*PC090OJ_ram_buffered;

static int PC090OJ_xoffs,PC090OJ_yoffs;



void PC090OJ_vh_start(running_machine *machine,int gfxnum,int x_offset,int y_offset,int use_buffer)
{
	/* use the given gfx set */
	PC090OJ_gfxnum = gfxnum;

	PC090OJ_xoffs = x_offset;
	PC090OJ_yoffs = y_offset;

	PC090OJ_buffer = use_buffer;

	PC090OJ_ram = auto_alloc_array_clear(machine, UINT16, PC090OJ_RAM_SIZE/2);
	PC090OJ_ram_buffered = auto_alloc_array_clear(machine, UINT16, PC090OJ_RAM_SIZE/2);

	state_save_register_global_pointer(machine, PC090OJ_ram, PC090OJ_RAM_SIZE/2);
	state_save_register_global_pointer(machine, PC090OJ_ram_buffered, PC090OJ_RAM_SIZE/2);
	state_save_register_global(machine, PC090OJ_ctrl);
}

READ16_HANDLER( PC090OJ_word_0_r )	// in case we find a game using 2...
{
	return PC090OJ_ram[offset];
}

static void PC090OJ_word_w(offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&PC090OJ_ram[offset]);

	/* If we're not buffering sprite ram, write it straight through... */
	if (!PC090OJ_buffer)
		PC090OJ_ram_buffered[offset] = PC090OJ_ram[offset];

	if (offset == 0xdff)
	{
		/* Bit 0 is flip control, others seem unused */
		PC090OJ_ctrl = data;

#if 0
	popmessage("PC090OJ ctrl = %4x",data);
#endif
	}
}

WRITE16_HANDLER( PC090OJ_word_0_w )	// in case we find a game using 2...
{
	PC090OJ_word_w(offset,data,mem_mask);
}

void PC090OJ_eof_callback(void)
{
	if (PC090OJ_buffer)
	{
		int i;
		for (i=0;i<PC090OJ_ACTIVE_RAM_SIZE/2;i++)
			PC090OJ_ram_buffered[i] = PC090OJ_ram[i];
	}
}


void PC090OJ_draw_sprites(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect,int pri_type)
{
	int offs,priority=0;
	int sprite_colbank = (PC090OJ_sprite_ctrl & 0xf) << 4;	/* top nibble */

	switch (pri_type)
	{
		case 0x00:
			priority = 0;	/* sprites over top bg layer */
			break;

		case 0x01:
			priority = 1;	/* sprites under top bg layer */
			break;

		case 0x02:
			priority = PC090OJ_sprite_ctrl >> 15;	/* variable sprite/tile priority */
	}


	for (offs = 0;offs < PC090OJ_ACTIVE_RAM_SIZE/2;offs += 4)
	{
		int flipx, flipy;
		int x, y;
		int data,code,color;

		data = PC090OJ_ram_buffered[offs+0];
		flipy = (data & 0x8000) >> 15;
		flipx = (data & 0x4000) >> 14;
		color = (data & 0x000f) | sprite_colbank;

		code = PC090OJ_ram_buffered[offs+2] & 0x1fff;
		x = PC090OJ_ram_buffered[offs+3] & 0x1ff;   /* mask verified with Rainbowe board */
		y = PC090OJ_ram_buffered[offs+1] & 0x1ff;   /* mask verified with Rainbowe board */

		/* treat coords as signed */
		if (x>0x140) x -= 0x200;
		if (y>0x140) y -= 0x200;

		if (!(PC090OJ_ctrl & 1))	/* sprites flipscreen */
		{
			x = 320 - x - 16;
			y = 256 - y - 16;
			flipx = !flipx;
			flipy = !flipy;
		}

		x += PC090OJ_xoffs;
		y += PC090OJ_yoffs;

		pdrawgfx_transpen(bitmap,cliprect,machine->gfx[PC090OJ_gfxnum],
				code,
				color,
				flipx,flipy,
				x,y,
				machine->priority_bitmap,
				priority ? 0xfc : 0xf0,0);
	}
}




/******************************************************************************/


#define TC0080VCO_RAM_SIZE 0x21000
#define TC0080VCO_CHAR_RAM_SIZE 0x2000
#define TC0080VCO_TOTAL_CHARS 256

static UINT16 *TC0080VCO_ram,
				*TC0080VCO_bg0_ram_0, *TC0080VCO_bg0_ram_1,
				*TC0080VCO_bg1_ram_0, *TC0080VCO_bg1_ram_1,
				*TC0080VCO_tx_ram_0,  *TC0080VCO_tx_ram_1,
				*TC0080VCO_char_ram,  *TC0080VCO_bgscroll_ram;

/* This sprite related stuff still needs to be accessed in
   video/taito_h */
UINT16 *TC0080VCO_chain_ram_0, *TC0080VCO_chain_ram_1,
				*TC0080VCO_spriteram, *TC0080VCO_scroll_ram;

static UINT16 TC0080VCO_bg0_scrollx,TC0080VCO_bg0_scrolly,
		TC0080VCO_bg1_scrollx,TC0080VCO_bg1_scrolly;

static tilemap *TC0080VCO_tilemap[3];

static int TC0080VCO_bg_gfx,TC0080VCO_tx_gfx;
static int TC0080VCO_bg_xoffs,TC0080VCO_bg_yoffs;
static int TC0080VCO_bg_flip_yoffs;

INT32 TC0080VCO_flipscreen = 0;
static int TC0080VCO_has_tx;


#if 0
static const int TC0080VCO_zoomy_conv_table[] =
{
/*      These are hand-tuned values...      */
/*    +0   +1   +2   +3   +4   +5   +6   +7    +8   +9   +a   +b   +c   +d   +e   +f */
	0x00,0x01,0x01,0x02,0x02,0x03,0x04,0x05, 0x06,0x06,0x07,0x08,0x09,0x0a,0x0a,0x0b,	/* 0x00 */
	0x0b,0x0c,0x0c,0x0d,0x0e,0x0e,0x0f,0x10, 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x16,
	0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e, 0x1f,0x20,0x21,0x22,0x24,0x25,0x26,0x27,
	0x28,0x2a,0x2b,0x2c,0x2e,0x30,0x31,0x32, 0x34,0x36,0x37,0x38,0x3a,0x3c,0x3e,0x3f,

	0x40,0x41,0x42,0x42,0x43,0x43,0x44,0x44, 0x45,0x45,0x46,0x46,0x47,0x47,0x48,0x49,	/* 0x40 */
	0x4a,0x4a,0x4b,0x4b,0x4c,0x4d,0x4e,0x4f, 0x4f,0x50,0x51,0x51,0x52,0x53,0x54,0x55,
	0x56,0x57,0x58,0x59,0x5a,0x5b,0x5c,0x5d, 0x5e,0x5f,0x60,0x61,0x62,0x63,0x64,0x66,
	0x67,0x68,0x6a,0x6b,0x6c,0x6e,0x6f,0x71, 0x72,0x74,0x76,0x78,0x80,0x7b,0x7d,0x7f
};
#endif


static TILE_GET_INFO( TC0080VCO_get_bg0_tile_info_0 )
{
	int color, tile;

	color = TC0080VCO_bg0_ram_1[ tile_index ] & 0x001f;
	tile  = TC0080VCO_bg0_ram_0[ tile_index ] & 0x7fff;

	tileinfo->category = 0;

	SET_TILE_INFO(
			TC0080VCO_bg_gfx,
			tile,
			color,
			TILE_FLIPYX((TC0080VCO_bg0_ram_1[tile_index] & 0x00c0) >> 6));
}

static TILE_GET_INFO( TC0080VCO_get_bg1_tile_info_0 )
{
	int color, tile;

	color = TC0080VCO_bg1_ram_1[ tile_index ] & 0x001f;
	tile  = TC0080VCO_bg1_ram_0[ tile_index ] & 0x7fff;

	tileinfo->category = 0;

	SET_TILE_INFO(
			TC0080VCO_bg_gfx,
			tile,
			color,
			TILE_FLIPYX((TC0080VCO_bg1_ram_1[tile_index] & 0x00c0) >> 6));
}

static TILE_GET_INFO( TC0080VCO_get_tx_tile_info )
{
	int tile;

	if (!TC0080VCO_flipscreen)
	{
		if ( (tile_index & 1) )
			tile = (TC0080VCO_tx_ram_0[tile_index >> 1] & 0x00ff);
		else
			tile = (TC0080VCO_tx_ram_0[tile_index >> 1] & 0xff00) >> 8;
		tileinfo->category = 0;
	}
	else
	{
		if ( (tile_index & 1) )
			tile = (TC0080VCO_tx_ram_0[tile_index >> 1] & 0xff00) >> 8;
		else
			tile = (TC0080VCO_tx_ram_0[tile_index >> 1] & 0x00ff);
		tileinfo->category = 0;
	}

	SET_TILE_INFO(
			TC0080VCO_tx_gfx,
			tile,
			0x40,
			0);		/* 0x20<<1 as 3bpp */
}


/* Is this endian-correct ??? */

static const gfx_layout TC0080VCO_charlayout =
{
	8, 8,	/* 8x8 pixels */
	256,	/* 256 chars */

// can't be 4bpp as it becomes opaque in Ainferno...
//  4,      /* 4 bits per pixel */
//#ifdef LSB_FIRST
//  { 0x10000*8 + 8, 0x10000*8, 8, 0 },
//#else
//  { 0x10000*8, 0x10000*8 + 8, 0, 8 },
//#endif

	3,		/* 3 bits per pixel */
#ifdef LSB_FIRST
	{ 0x10000*8, 8, 0 },
#else
	{ 0x10000*8 + 8, 0, 8 },
#endif
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};


static void TC0080VCO_set_layer_ptrs(void)
{
	TC0080VCO_char_ram      = TC0080VCO_ram + 0x00000/2;	/* continues at +0x10000 */
	TC0080VCO_tx_ram_0      = TC0080VCO_ram + 0x01000/2;
	TC0080VCO_chain_ram_0   = TC0080VCO_ram + 0x00000/2;	/* only used from +0x2000 */

	TC0080VCO_bg0_ram_0     = TC0080VCO_ram + 0x0c000/2;
	TC0080VCO_bg1_ram_0     = TC0080VCO_ram + 0x0e000/2;

	TC0080VCO_tx_ram_1      = TC0080VCO_ram + 0x11000/2;
	TC0080VCO_chain_ram_1   = TC0080VCO_ram + 0x10000/2;	/* only used from +0x12000 */

	TC0080VCO_bg0_ram_1     = TC0080VCO_ram + 0x1c000/2;
	TC0080VCO_bg1_ram_1     = TC0080VCO_ram + 0x1e000/2;
	TC0080VCO_bgscroll_ram  = TC0080VCO_ram + 0x20000/2;
	TC0080VCO_spriteram     = TC0080VCO_ram + 0x20400/2;
	TC0080VCO_scroll_ram    = TC0080VCO_ram + 0x20800/2;
}

static void TC0080VCO_restore_scroll(void)
{
	TC0080VCO_flipscreen = TC0080VCO_scroll_ram[0] & 0x0c00;

	tilemap_set_flip( TC0080VCO_tilemap[0], TC0080VCO_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0 );
	tilemap_set_flip( TC0080VCO_tilemap[1], TC0080VCO_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0 );
	tilemap_set_flip( TC0080VCO_tilemap[2], TC0080VCO_flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0 );

	TC0080VCO_bg0_scrollx = TC0080VCO_scroll_ram[1] &0x03ff;
	TC0080VCO_bg1_scrollx = TC0080VCO_scroll_ram[2] &0x03ff;
	TC0080VCO_bg0_scrolly = TC0080VCO_scroll_ram[3] &0x03ff;
	TC0080VCO_bg1_scrolly = TC0080VCO_scroll_ram[4] &0x03ff;
}


static STATE_POSTLOAD( TC0080VCO_postload )
{
	TC0080VCO_set_layer_ptrs();
	TC0080VCO_restore_scroll();
}

void TC0080VCO_vh_start(running_machine *machine, int gfxnum,int has_fg0,int bg_xoffs,int bg_yoffs,int bg_flip_yoffs)
{
	int gfx_index=0;

	TC0080VCO_bg_xoffs = bg_xoffs;	/* usually 1 */
	TC0080VCO_bg_yoffs = bg_yoffs;	/* usually 1 */
	TC0080VCO_bg_flip_yoffs = bg_flip_yoffs;	/* usually -2 */
	TC0080VCO_has_tx = has_fg0;	/* for debugging only */

	TC0080VCO_tilemap[0] = tilemap_create(machine, TC0080VCO_get_bg0_tile_info_0,tilemap_scan_rows,16,16,64,64);
	TC0080VCO_tilemap[1] = tilemap_create(machine, TC0080VCO_get_bg1_tile_info_0,tilemap_scan_rows,16,16,64,64);
	TC0080VCO_ram = auto_alloc_array_clear(machine, UINT16, TC0080VCO_RAM_SIZE/2);

	TC0080VCO_set_layer_ptrs();

	/* use the given gfx set for bg tiles*/
	TC0080VCO_bg_gfx = gfxnum;

	tilemap_set_transparent_pen( TC0080VCO_tilemap[0],0 );
	tilemap_set_transparent_pen( TC0080VCO_tilemap[1],0 );

	tilemap_set_scrolldx(TC0080VCO_tilemap[0],TC0080VCO_bg_xoffs,512);
	tilemap_set_scrolldx(TC0080VCO_tilemap[1],TC0080VCO_bg_xoffs,512);
	tilemap_set_scrolldy(TC0080VCO_tilemap[0],TC0080VCO_bg_yoffs,TC0080VCO_bg_flip_yoffs);
	tilemap_set_scrolldy(TC0080VCO_tilemap[1],TC0080VCO_bg_yoffs,TC0080VCO_bg_flip_yoffs);

	state_save_register_global_pointer(machine, TC0080VCO_ram, TC0080VCO_RAM_SIZE/2);
	state_save_register_global(machine, TC0080VCO_has_tx);

	/* Perform extra initialisations for text layer */
	{
		TC0080VCO_tilemap[2] = tilemap_create(machine, TC0080VCO_get_tx_tile_info,tilemap_scan_rows,8,8,64,64);

	 	/* find first empty slot to decode gfx */
		for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
			if (machine->gfx[gfx_index] == 0)
				break;
		assert(gfx_index != MAX_GFX_ELEMENTS);

		/* create the char set (gfx will then be updated dynamically from RAM) */
		machine->gfx[gfx_index] = gfx_element_alloc(machine, &TC0080VCO_charlayout, (UINT8 *)TC0080VCO_char_ram, 64, 0);
		TC0080VCO_tx_gfx = gfx_index;

		tilemap_set_scrolldx(TC0080VCO_tilemap[2],0,0);
		tilemap_set_scrolldy(TC0080VCO_tilemap[2],48,-448);

		tilemap_set_transparent_pen( TC0080VCO_tilemap[2],0 );
	}

	state_save_register_postload(machine, TC0080VCO_postload, NULL);

	/* bg0 tilemap scrollable per pixel row */
	tilemap_set_scroll_rows(TC0080VCO_tilemap[0],512);
}


static WRITE16_HANDLER( TC0080VCO_scrollram_w )
{
	switch ( offset )
	{
		case 0x00:			/* screen invert control */
			TC0080VCO_restore_scroll();
			break;

		case 0x01:			/* BG0 scroll X */
			TC0080VCO_bg0_scrollx = data &0x03ff;
			break;

		case 0x02:			/* BG1 scroll X */
			TC0080VCO_bg1_scrollx = data &0x03ff;
			break;

		case 0x03:			/* BG0 scroll Y */
			TC0080VCO_bg0_scrolly = data &0x03ff;
			break;

		case 0x04:			/* BG1 scroll Y */
			TC0080VCO_bg1_scrolly = data &0x03ff;
			break;

		default:
			break;
	}
}

READ16_HANDLER( TC0080VCO_word_r )
{
	return TC0080VCO_ram[offset];
}

WRITE16_HANDLER( TC0080VCO_word_w )
{
	COMBINE_DATA(&TC0080VCO_ram[offset]);

	/* A lot of TC0080VCO writes require no action... */

	if (offset < 0x1000/2)
	{
		gfx_element_mark_dirty(space->machine->gfx[TC0080VCO_tx_gfx], offset / 8);
#if 0
		if (!TC0080VCO_has_tx)
		{
			if (TC0080VCO_ram[offset])
			popmessage("Write non-zero to TC0080VCO char ram\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0x2000/2)	/* fg0 (text layer) */
	{
		tilemap_mark_tile_dirty( TC0080VCO_tilemap[2],(offset &0x07ff) * 2 );
		tilemap_mark_tile_dirty( TC0080VCO_tilemap[2],(offset &0x07ff) * 2 + 1 );
#if 0
		if (!TC0080VCO_has_tx)
		{
			if (TC0080VCO_ram[offset])
			popmessage("Write non-zero to TC0080VCO fg0\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0xc000/2)	/* chain ram */
	{}
	else if (offset < 0xe000/2)	/* bg0 (0) */
		tilemap_mark_tile_dirty(TC0080VCO_tilemap[0],(offset & 0xfff));

	else if (offset < 0x10000/2)	/* bg1 (0) */
		tilemap_mark_tile_dirty(TC0080VCO_tilemap[1],(offset & 0xfff));

	else if (offset < 0x11000/2)
	{
		gfx_element_mark_dirty(space->machine->gfx[TC0080VCO_tx_gfx], (offset - 0x10000/2) / 8);
#if 0
		if (!TC0080VCO_has_tx)
		{
			if (TC0080VCO_ram[offset])
			popmessage("Write non-zero to TC0080VCO char-hi ram\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0x12000/2)	/* unknown/unused */
	{
#if 1
		if (TC0080VCO_ram[offset])
		popmessage("Write non-zero to mystery TC0080VCO area\nPlease report to MAMEDEV");
#endif
	}
	else if (offset < 0x1c000/2)	/* chain ram */
	{}
	else if (offset < 0x1e000/2)	/* bg0 (1) */
		tilemap_mark_tile_dirty(TC0080VCO_tilemap[0],(offset & 0xfff));

	else if (offset < 0x20000/2)	/* bg1 (1) */
		tilemap_mark_tile_dirty(TC0080VCO_tilemap[1],(offset & 0xfff));

	else if (offset < 0x20400/2)	/* bg0 rowscroll */
	{}
	else if (offset < 0x20800/2)	/* sprite ram */
	{}
	else if (offset < 0x20fff/2)
		TC0080VCO_scrollram_w(space,offset-(0x20800/2),TC0080VCO_ram[offset],mem_mask);
}


void TC0080VCO_tilemap_update(running_machine *machine)
{
	int j;

	if (!TC0080VCO_flipscreen)
	{
		for (j = 0;j < 0x400;j++)
			tilemap_set_scrollx(TC0080VCO_tilemap[0],(j+0) & 0x3ff,
				-TC0080VCO_bg0_scrollx - TC0080VCO_bgscroll_ram[j &0x1ff]);
	}
	else
	{
		for (j = 0;j < 0x400;j++)
			tilemap_set_scrollx(TC0080VCO_tilemap[0],(j+0) & 0x3ff,
				-TC0080VCO_bg0_scrollx + TC0080VCO_bgscroll_ram[j &0x1ff]);
	}

	tilemap_set_scrolly(TC0080VCO_tilemap[0],0, TC0080VCO_bg0_scrolly);
	tilemap_set_scrollx(TC0080VCO_tilemap[1],0,-TC0080VCO_bg1_scrollx);
	tilemap_set_scrolly(TC0080VCO_tilemap[1],0, TC0080VCO_bg1_scrolly);
	tilemap_set_scrollx(TC0080VCO_tilemap[2],0,0);	/* no scroll (maybe) */
	tilemap_set_scrolly(TC0080VCO_tilemap[2],0,0);
}


/* NB: orientation_flipx code in following routine has not been tested */

static void TC0080VCO_bg0_tilemap_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority)
{
	UINT16 zoom = TC0080VCO_scroll_ram[6];
	int zx, zy;

	zx = (zoom & 0xff00) >> 8;
	zy =  zoom & 0x00ff;

	if (zx == 0x3f && zy == 0x7f)		/* normal size */
	{
		tilemap_draw(bitmap,cliprect,TC0080VCO_tilemap[0],flags,priority);
	}
	else		/* zoom + rowscroll = custom draw routine */
	{
		UINT16 *dst16,*src16;
		UINT8 *tsrc;
		UINT16 scanline[512];
		bitmap_t *srcbitmap = tilemap_get_pixmap(TC0080VCO_tilemap[0]);
		bitmap_t *flagsbitmap = tilemap_get_flagsmap(TC0080VCO_tilemap[0]);

		int sx,zoomx,zoomy;
		int dx,ex,dy,ey;
		int i,y,y_index,src_y_index,row_index;
		int x_index,x_step;

		int flip = TC0080VCO_flipscreen;
		int machine_flip = 0;	/* for  ROT 180 ? */

		int min_x = cliprect->min_x;
		int max_x = cliprect->max_x;
		int min_y = cliprect->min_y;
		int max_y = cliprect->max_y;
		int screen_width = max_x + 1;
		int width_mask=0x3ff;	/* underlying tilemap */


#if 0
{
	char buf[100];
	sprintf(buf,"xmin= %04x xmax= %04x ymin= %04x ymax= %04x",min_x,max_x,min_y,max_y);
	popmessage(buf);
}
#endif

		if (zx < 63)
		{
			/* no evidence for these calcs? */
			dx = 16 - (zx + 2) / 8;
			ex = (zx + 2) % 8;
			zoomx = ((dx << 3) - ex) << 10;
		}
		else
		{
			/* 256 is speculative, haven't found a game using zoomx yet */
			zoomx = 0x10000 - ((zx - 0x3f) * 256);
		}

		if (zy < 127)
		{
			/* no evidence for these calcs? */
			dy = 16 - (zy + 2) / 16;
			ey = (zy + 2) % 16;
			zoomy = ((dy << 4) - ey) << 9;
		}
		else
		{
			/* confirmed with real board */
			zoomy = 0x10000 - ((zy - 0x7f) * 512);
		}

		if (!flip)
		{
			sx =       (-TC0080VCO_scroll_ram[1] - 1) << 16;
			y_index = (( TC0080VCO_scroll_ram[3] - 1) << 16) + min_y * zoomy;
		}
		else
		{
			/* adjustment for zx is entirely speculative */
			sx =  (( 0x200 + TC0080VCO_scroll_ram[1]) << 16)
					- (max_x + min_x) * (zoomx-0x10000);

			/* 0x130 correct for Dleague. Syvalion correct with 0x1f0.
               min_y is 0x20 and 0x30; max_y is 0x10f and 0x1bf;
               max_y + min_y seems a good bet... */

			y_index = ((-TC0080VCO_scroll_ram[3] - 2) << 16)
					+ min_y * zoomy - (max_y + min_y) * (zoomy-0x10000);
		}

		if (!machine_flip) y = min_y; else y = max_y;

		do
		{
			src_y_index = (y_index>>16) &0x3ff;	/* tilemaps are 1024 px up/down */

			/* row areas are the same in flipscreen, so we must read in reverse */
			row_index = (src_y_index &0x1ff);
			if (flip)	row_index = 0x1ff - row_index;

			x_index = sx - ((TC0080VCO_bgscroll_ram[row_index] << 16));

			src16 = BITMAP_ADDR16(srcbitmap, src_y_index, 0);
			tsrc  = BITMAP_ADDR8(flagsbitmap, src_y_index, 0);
			dst16 = scanline;

			x_step = zoomx;

/*** NEW ***/
			if (flags & TILEMAP_DRAW_OPAQUE)
			{
				for (i=0; i<screen_width; i++)
				{
					*dst16++ = src16[(x_index >> 16) &width_mask];
					x_index += x_step;
				}
			}
			else
			{
				for (i=0; i<screen_width; i++)
				{
					if (tsrc[(x_index >> 16) &width_mask])
						*dst16++ = src16[(x_index >> 16) &width_mask];
					else
						*dst16++ = 0x8000;
					x_index += x_step;
				}
			}
/***********/

//          while (x_index<x_max)
//          {
//              *dst16++ = src16[(x_index >> 16) &width_mask];
//              x_index += x_step;
//          }
//
//                  pdraw_scanline16(bitmap,0,y,screen_width,
//                      scanline,0,0,rot,priority);

/*** NEW ***/
			taitoic_drawscanline(bitmap,cliprect,0,y,scanline,(flags & TILEMAP_DRAW_OPAQUE)?0:1,ROT0,machine->priority_bitmap,priority);
/***********/

			y_index += zoomy;
			if (!machine_flip) y++; else y--;
		}
		while ( (!machine_flip && y <= max_y) || (machine_flip && y >= min_y) );

	}
}


#define PIXEL_OP_COPY_TRANS0_SET_PRIORITY(DEST, PRIORITY, SOURCE) 					\
do																					\
{																					\
	UINT32 srcdata = (SOURCE);														\
	if (srcdata != 0)																\
	{																				\
		(DEST) = SOURCE;															\
		(PRIORITY) = privalue;														\
	}																				\
}																					\
while (0)																			\

static void TC0080VCO_bg1_tilemap_draw(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int flags,UINT32 priority)
{
	UINT8 layer=1;
	UINT16 zoom = TC0080VCO_scroll_ram[6+layer];
	int min_x = cliprect->min_x;
	int max_x = cliprect->max_x;
	int min_y = cliprect->min_y;
	int max_y = cliprect->max_y;
	int zoomx, zoomy;

	zoomx = (zoom & 0xff00) >> 8;
	zoomy =  zoom & 0x00ff;

	if (zoomx == 0x3f && zoomy == 0x7f)		/* normal size */
	{
		tilemap_draw(bitmap,cliprect,TC0080VCO_tilemap[layer],flags,priority);
	}
	else		/* zoomed */
	{
		int zx, zy, dx, dy, ex, ey;
		int sx,sy;

		/* shouldn't we set no_clip before doing this (see TC0480SCP) ? */
		bitmap_t *srcbitmap = tilemap_get_pixmap(TC0080VCO_tilemap[layer]);

		if (zoomx < 63)
		{
			/* no evidence for these calcs? */
			dx = 16 - (zoomx + 2) / 8;
			ex = (zoomx + 2) % 8;
			zx = ((dx << 3) - ex) << 10;
		}
		else
		{
			/* 256 is speculative, haven't found a game using zoomx yet */
			zx = 0x10000 - ((zoomx - 0x3f) * 256);
		}

		if (zoomy < 127)
		{
			/* no evidence for these calcs? */
			dy = 16 - (zoomy + 2) / 16;
			ey = (zoomy + 2) % 16;
			zy = ((dy << 4) - ey) << 9;
		}
		else
		{
			/* confirmed with real board */
			zy = 0x10000 - ((zoomy - 0x7f) * 512);
		}

		if (!TC0080VCO_flipscreen)
		{
			sx = (-TC0080VCO_scroll_ram[layer+1] - 1) << 16;
			sy = ( TC0080VCO_scroll_ram[layer+3] - 1) << 16;
		}
		else
		{
			/* adjustment for zx is entirely speculative */
			sx =  (( 0x200 + TC0080VCO_scroll_ram[layer+1]) << 16)
					- (max_x + min_x) * (zx-0x10000);

			sy =  (( 0x3fe - TC0080VCO_scroll_ram[layer+3]) << 16)
					- (max_y + min_y) * (zy-0x10000);
		}

		{
			bitmap_t *dest = bitmap;
			bitmap_t *src = srcbitmap;
			INT32 startx = sx;
			INT32 starty = sy;
			INT32 incxx = zx;
			INT32 incxy = 0;
			INT32 incyx = 0;
			INT32 incyy = zy;
			int wraparound = 0;
			UINT32 privalue = priority;
			bitmap_t *priority = machine->priority_bitmap;

			if (dest->bpp == 16)
				COPYROZBITMAP_CORE(UINT16, PIXEL_OP_COPY_TRANS0_SET_PRIORITY, UINT8);
			else
				COPYROZBITMAP_CORE(UINT32, PIXEL_OP_COPY_TRANS0_SET_PRIORITY, UINT8);
		}
	}
}


void TC0080VCO_tilemap_draw(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int layer,int flags,UINT32 priority)
{
	int disable = 0x00;	/* possibly layer disable bits do exist ?? */

#if 0
	popmessage("layer disable = %x",disable);
#endif

	switch (layer)
	{
		case 0:
			if (disable & 0x01) return;
			TC0080VCO_bg0_tilemap_draw(machine,bitmap,cliprect,flags,priority);
			break;
		case 1:
			if (disable & 0x02) return;
			TC0080VCO_bg1_tilemap_draw(machine,bitmap,cliprect,flags,priority);
			break;
		case 2:
			if (disable & 0x04) return;
			tilemap_draw(bitmap,cliprect,TC0080VCO_tilemap[2],flags,priority);
			break;
	}
}



/***************************************************************************/


#define TC0100SCN_RAM_SIZE 0x14000	/* enough for double-width tilemaps */
#define TC0100SCN_TOTAL_CHARS 256
#define TC0100SCN_MAX_CHIPS 3
static int TC0100SCN_chips;
static rectangle myclip;

const int TC0100SCN_SINGLE_VDU = 1024;

static UINT16 TC0100SCN_ctrl[TC0100SCN_MAX_CHIPS][8];

static UINT16 *TC0100SCN_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_bg_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_fg_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_tx_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_char_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_bgscroll_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_fgscroll_ram[TC0100SCN_MAX_CHIPS],
				*TC0100SCN_colscroll_ram[TC0100SCN_MAX_CHIPS];

static int TC0100SCN_bgscrollx[TC0100SCN_MAX_CHIPS],TC0100SCN_bgscrolly[TC0100SCN_MAX_CHIPS],
		TC0100SCN_fgscrollx[TC0100SCN_MAX_CHIPS],TC0100SCN_fgscrolly[TC0100SCN_MAX_CHIPS];

/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
static tilemap *TC0100SCN_tilemap[TC0100SCN_MAX_CHIPS][3][2];
static rectangle TC0100SCN_cliprect[TC0100SCN_MAX_CHIPS];

static int TC0100SCN_bg_gfx[TC0100SCN_MAX_CHIPS],TC0100SCN_tx_gfx[TC0100SCN_MAX_CHIPS];
static int TC0100SCN_bg_col_mult,TC0100SCN_bg_tilemask,TC0100SCN_tx_col_mult;
static INT32 TC0100SCN_gfxbank,TC0100SCN_chip_colbank[3],TC0100SCN_colbank[3];
static int TC0100SCN_dblwidth[TC0100SCN_MAX_CHIPS];


INLINE void common_get_bg0_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum,int colbank,int dblwidth)
{
	int code,attr;

	if (!dblwidth)
	{
		/* Mahjong Quest (F2 system) inexplicably has a banking feature */
		code = (ram[2*tile_index + 1] & TC0100SCN_bg_tilemask) + (TC0100SCN_gfxbank << 15);
		attr = ram[2*tile_index];
	}
	else
	{
		code = ram[2*tile_index + 1] & TC0100SCN_bg_tilemask;
		attr = ram[2*tile_index];
	}
	SET_TILE_INFO(
			gfxnum,
			code,
			(((attr * TC0100SCN_bg_col_mult) + TC0100SCN_colbank[0]) & 0xff) + colbank,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_bg1_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum,int colbank,int dblwidth)
{
	int code,attr;

	if (!dblwidth)
	{
		/* Mahjong Quest (F2 system) inexplicably has a banking feature */
		code = (ram[2*tile_index + 1] & TC0100SCN_bg_tilemask) + (TC0100SCN_gfxbank << 15);
		attr = ram[2*tile_index];
	}
	else
	{
		code = ram[2*tile_index + 1] & TC0100SCN_bg_tilemask;
		attr = ram[2*tile_index];
	}
	SET_TILE_INFO(
			gfxnum,
			code,
			(((attr * TC0100SCN_bg_col_mult) + TC0100SCN_colbank[1]) & 0xff) + colbank,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_tx_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum,int colbank,int dblwidth)
{
	int attr = ram[tile_index];

	SET_TILE_INFO(
			gfxnum,
			attr & 0xff,
			((((attr >> 6) &0xfc) * TC0100SCN_tx_col_mult + (TC0100SCN_colbank[2] << 2)) &0x3ff) + colbank*4,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO( TC0100SCN_get_bg_tile_info_0 )
{
	common_get_bg0_tile_info(machine,tileinfo,tile_index,TC0100SCN_bg_ram[0],TC0100SCN_bg_gfx[0],
			TC0100SCN_chip_colbank[0],TC0100SCN_dblwidth[0]);
}

static TILE_GET_INFO( TC0100SCN_get_fg_tile_info_0 )
{
	common_get_bg1_tile_info(machine,tileinfo,tile_index,TC0100SCN_fg_ram[0],TC0100SCN_bg_gfx[0],
			TC0100SCN_chip_colbank[0],TC0100SCN_dblwidth[0]);
}

static TILE_GET_INFO( TC0100SCN_get_tx_tile_info_0 )
{
	common_get_tx_tile_info(machine,tileinfo,tile_index,TC0100SCN_tx_ram[0],TC0100SCN_tx_gfx[0],
			TC0100SCN_chip_colbank[0],TC0100SCN_dblwidth[0]);
}

static TILE_GET_INFO( TC0100SCN_get_bg_tile_info_1 )
{
	common_get_bg0_tile_info(machine,tileinfo,tile_index,TC0100SCN_bg_ram[1],TC0100SCN_bg_gfx[1],
			TC0100SCN_chip_colbank[1],TC0100SCN_dblwidth[1]);
}

static TILE_GET_INFO( TC0100SCN_get_fg_tile_info_1 )
{
	common_get_bg1_tile_info(machine,tileinfo,tile_index,TC0100SCN_fg_ram[1],TC0100SCN_bg_gfx[1],
			TC0100SCN_chip_colbank[1],TC0100SCN_dblwidth[1]);
}

static TILE_GET_INFO( TC0100SCN_get_tx_tile_info_1 )
{
	common_get_tx_tile_info(machine,tileinfo,tile_index,TC0100SCN_tx_ram[1],TC0100SCN_tx_gfx[1],
			TC0100SCN_chip_colbank[1],TC0100SCN_dblwidth[1]);
}

static TILE_GET_INFO( TC0100SCN_get_bg_tile_info_2 )
{
	common_get_bg0_tile_info(machine,tileinfo,tile_index,TC0100SCN_bg_ram[2],TC0100SCN_bg_gfx[2],
			TC0100SCN_chip_colbank[2],TC0100SCN_dblwidth[2]);
}

static TILE_GET_INFO( TC0100SCN_get_fg_tile_info_2 )
{
	common_get_bg1_tile_info(machine,tileinfo,tile_index,TC0100SCN_fg_ram[2],TC0100SCN_bg_gfx[2],
			TC0100SCN_chip_colbank[2],TC0100SCN_dblwidth[2]);
}

static TILE_GET_INFO( TC0100SCN_get_tx_tile_info_2 )
{
	common_get_tx_tile_info(machine,tileinfo,tile_index,TC0100SCN_tx_ram[2],TC0100SCN_tx_gfx[2],
			TC0100SCN_chip_colbank[2],TC0100SCN_dblwidth[2]);
}

/* This array changes with TC0100SCN_MAX_CHIPS */

static const tile_get_info_func TC0100SCN_get_tile_info[TC0100SCN_MAX_CHIPS][3] =
{
	{ TC0100SCN_get_bg_tile_info_0, TC0100SCN_get_fg_tile_info_0, TC0100SCN_get_tx_tile_info_0 },
	{ TC0100SCN_get_bg_tile_info_1, TC0100SCN_get_fg_tile_info_1, TC0100SCN_get_tx_tile_info_1 },
	{ TC0100SCN_get_bg_tile_info_2, TC0100SCN_get_fg_tile_info_2, TC0100SCN_get_tx_tile_info_2 }
};


static const gfx_layout TC0100SCN_charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	2,	/* 2 bits per pixel */
#ifdef LSB_FIRST
	{ 8, 0 },
#else
	{ 0, 8 },
#endif
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8	/* every sprite takes 16 consecutive bytes */
};


void TC0100SCN_set_chip_colbanks(int chip0,int chip1,int chip2)
{
	TC0100SCN_chip_colbank[0] = chip0;	/* palette area for chip 0 */
	TC0100SCN_chip_colbank[1] = chip1;
	TC0100SCN_chip_colbank[2] = chip2;
}

void TC0100SCN_set_colbanks(int bg0,int bg1,int fg)
{
	TC0100SCN_colbank[0] = bg0;
	TC0100SCN_colbank[1] = bg1;
	TC0100SCN_colbank[2] = fg;	/* text */
}

void TC0100SCN_set_bg_tilemask(int mask)
{
	TC0100SCN_bg_tilemask = mask;
}

WRITE16_HANDLER( TC0100SCN_gfxbank_w )   /* Mjnquest banks its 2 sets of scr tiles */
{
    TC0100SCN_gfxbank = (data & 0x1);
}

static void TC0100SCN_set_layer_ptrs(int i)
{
	if (!TC0100SCN_dblwidth[i])
	{
		TC0100SCN_bg_ram[i]        = TC0100SCN_ram[i] + 0x0;
		TC0100SCN_tx_ram[i]        = TC0100SCN_ram[i] + 0x4000 /2;
		TC0100SCN_char_ram[i]      = TC0100SCN_ram[i] + 0x6000 /2;
		TC0100SCN_fg_ram[i]        = TC0100SCN_ram[i] + 0x8000 /2;
		TC0100SCN_bgscroll_ram[i]  = TC0100SCN_ram[i] + 0xc000 /2;
		TC0100SCN_fgscroll_ram[i]  = TC0100SCN_ram[i] + 0xc400 /2;
		TC0100SCN_colscroll_ram[i] = TC0100SCN_ram[i] + 0xe000 /2;
	}
	else
	{
		TC0100SCN_bg_ram[i]        = TC0100SCN_ram[i] + 0x0;
		TC0100SCN_fg_ram[i]        = TC0100SCN_ram[i] + 0x08000 /2;
		TC0100SCN_bgscroll_ram[i]  = TC0100SCN_ram[i] + 0x10000 /2;
		TC0100SCN_fgscroll_ram[i]  = TC0100SCN_ram[i] + 0x10400 /2;
		TC0100SCN_colscroll_ram[i] = TC0100SCN_ram[i] + 0x10800 /2;
		TC0100SCN_char_ram[i]      = TC0100SCN_ram[i] + 0x11000 /2;
		TC0100SCN_tx_ram[i]        = TC0100SCN_ram[i] + 0x12000 /2;
	}
}

/* As we can't pass function calls with params in set...func_postload() calls
   in the vh_start, this slightly obnoxious method is used */

static void TC0100SCN_dirty_tilemaps(int chip)
{
	tilemap_mark_all_tiles_dirty(TC0100SCN_tilemap[chip][0][TC0100SCN_dblwidth[chip]]);
	tilemap_mark_all_tiles_dirty(TC0100SCN_tilemap[chip][1][TC0100SCN_dblwidth[chip]]);
	tilemap_mark_all_tiles_dirty(TC0100SCN_tilemap[chip][2][TC0100SCN_dblwidth[chip]]);
}

static void TC0100SCN_restore_scroll(int chip)
{
	int flip;

	TC0100SCN_bgscrollx[chip] = -TC0100SCN_ctrl[chip][0];
	TC0100SCN_fgscrollx[chip] = -TC0100SCN_ctrl[chip][1];
	tilemap_set_scrollx(TC0100SCN_tilemap[chip][2][0],0,-TC0100SCN_ctrl[chip][2]);
	tilemap_set_scrollx(TC0100SCN_tilemap[chip][2][1],0,-TC0100SCN_ctrl[chip][2]);

	TC0100SCN_bgscrolly[chip] = -TC0100SCN_ctrl[chip][3];
	TC0100SCN_fgscrolly[chip] = -TC0100SCN_ctrl[chip][4];
	tilemap_set_scrolly(TC0100SCN_tilemap[chip][2][0],0,-TC0100SCN_ctrl[chip][5]);
	tilemap_set_scrolly(TC0100SCN_tilemap[chip][2][1],0,-TC0100SCN_ctrl[chip][5]);

	flip = (TC0100SCN_ctrl[chip][7] & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	tilemap_set_flip(TC0100SCN_tilemap[chip][0][0],flip);
	tilemap_set_flip(TC0100SCN_tilemap[chip][1][0],flip);
	tilemap_set_flip(TC0100SCN_tilemap[chip][2][0],flip);
	tilemap_set_flip(TC0100SCN_tilemap[chip][0][1],flip);
	tilemap_set_flip(TC0100SCN_tilemap[chip][1][1],flip);
	tilemap_set_flip(TC0100SCN_tilemap[chip][2][1],flip);
}


static STATE_POSTLOAD( TC0100SCN_postload )
{
	int chip = (FPTR)param;

	TC0100SCN_set_layer_ptrs(chip);
	TC0100SCN_restore_scroll(chip);
}

void TC0100SCN_vh_start(running_machine *machine, int chips,int gfxnum,int x_offset,int y_offset,int flip_xoffs,
		int flip_yoffs,int flip_text_xoffs,int flip_text_yoffs,int multiscrn_xoffs)
{
	int gfx_index,gfxset_offs,i;

	assert(chips <= TC0100SCN_MAX_CHIPS);

	TC0100SCN_chips = chips;

	for (i = 0;i < chips;i++)
	{
		const device_config *screen;
		int xd,yd;
		TC0100SCN_dblwidth[i]=0;

		screen = device_list_find_by_index(machine->config->devicelist, VIDEO_SCREEN, i);
		if (screen == NULL)
			screen = machine->primary_screen;

		/* Single width versions */
		TC0100SCN_tilemap[i][0][0] = tilemap_create(machine, TC0100SCN_get_tile_info[i][0],tilemap_scan_rows,8,8,64,64);
		TC0100SCN_tilemap[i][1][0] = tilemap_create(machine, TC0100SCN_get_tile_info[i][1],tilemap_scan_rows,8,8,64,64);
		TC0100SCN_tilemap[i][2][0] = tilemap_create(machine, TC0100SCN_get_tile_info[i][2],tilemap_scan_rows,8,8,64,64);

		/* Double width versions */
		TC0100SCN_tilemap[i][0][1] = tilemap_create(machine, TC0100SCN_get_tile_info[i][0],tilemap_scan_rows,8,8,128,64);
		TC0100SCN_tilemap[i][1][1] = tilemap_create(machine, TC0100SCN_get_tile_info[i][1],tilemap_scan_rows,8,8,128,64);
		TC0100SCN_tilemap[i][2][1] = tilemap_create(machine, TC0100SCN_get_tile_info[i][2],tilemap_scan_rows,8,8,128,32);

		/* Set up clipping for multi-TC0100SCN games. We assume
           this code won't ever affect single screen games:
           Thundfox is the only one of those with two chips, and
           we're safe as it uses single width tilemaps. */

		myclip = *video_screen_get_visible_area(screen);

		TC0100SCN_cliprect[i] = myclip;

		TC0100SCN_ram[i] = auto_alloc_array_clear(machine, UINT16, TC0100SCN_RAM_SIZE/2);

		TC0100SCN_set_layer_ptrs(i);

		{
			state_save_register_item_pointer(machine, "TC0100SCN", NULL, i, TC0100SCN_ram[i], TC0100SCN_RAM_SIZE/2);
			state_save_register_item_array(machine, "TC0100SCN", NULL, i, TC0100SCN_ctrl[i]);
			state_save_register_item(machine, "TC0100SCN", NULL, i, TC0100SCN_dblwidth[i]);
		}

		state_save_register_postload(machine, TC0100SCN_postload, (void *)(FPTR)i);

		/* find first empty slot to decode gfx */
		for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
			if (machine->gfx[gfx_index] == 0)
				break;
		assert(gfx_index != MAX_GFX_ELEMENTS);

		/* create the char set (gfx will then be updated dynamically from RAM) */
		machine->gfx[gfx_index] = gfx_element_alloc(machine, &TC0100SCN_charlayout, (UINT8 *)TC0100SCN_char_ram[i], 64, 0);
		TC0100SCN_tx_gfx[i] = gfx_index;

		/* use the given gfx set for bg tiles; 2nd/3rd chips will
           use the same gfx set */
		gfxset_offs = i;
		if (i > 1)
			gfxset_offs = 1;
		TC0100SCN_bg_gfx[i] = gfxnum + gfxset_offs;

		tilemap_set_transparent_pen(TC0100SCN_tilemap[i][0][0],0);
		tilemap_set_transparent_pen(TC0100SCN_tilemap[i][1][0],0);
		tilemap_set_transparent_pen(TC0100SCN_tilemap[i][2][0],0);

		tilemap_set_transparent_pen(TC0100SCN_tilemap[i][0][1],0);
		tilemap_set_transparent_pen(TC0100SCN_tilemap[i][1][1],0);
		tilemap_set_transparent_pen(TC0100SCN_tilemap[i][2][1],0);

		/* Standard width tilemaps. I'm setting the optional chip #2
           7 bits higher and 2 pixels to the left than chip #1 because
           that's how thundfox wants it. */

		xd = (i == 0) ?  (-x_offset) : (-x_offset-2);
		yd = (i == 0) ? (8-y_offset) : (1-y_offset);

		tilemap_set_scrolldx(TC0100SCN_tilemap[i][0][0], xd-16, -flip_xoffs -xd-16);
		tilemap_set_scrolldy(TC0100SCN_tilemap[i][0][0], yd,    -flip_yoffs -yd);
		tilemap_set_scrolldx(TC0100SCN_tilemap[i][1][0], xd-16, -flip_xoffs -xd-16);
		tilemap_set_scrolldy(TC0100SCN_tilemap[i][1][0], yd,    -flip_yoffs -yd);
		tilemap_set_scrolldx(TC0100SCN_tilemap[i][2][0], xd-16, -flip_text_xoffs -xd-16-7);
		tilemap_set_scrolldy(TC0100SCN_tilemap[i][2][0], yd,    -flip_text_yoffs -yd);

		/* Double width tilemaps. We must correct offsets for
           extra chips, as MAME sees offsets from LHS of whole
           display not from the edges of individual screens.
           NB flipscreen tilemap offsets are based on Cameltry */

		xd = -x_offset;
		yd = 8-y_offset;

		// the multi-screen games need this, check the alignment grid screens..
		if (chips==2)	/* Dual screen */
		{
			if (i==1)  xd -= (multiscrn_xoffs);
		}
		if (chips==3)	/* Triple screen */
		{
			if (i==1)  xd -= multiscrn_xoffs;
			if (i==2)  xd -= multiscrn_xoffs*2;
		}
		tilemap_set_scrolldx(TC0100SCN_tilemap[i][0][1], xd-16, -flip_xoffs -xd-16);
		tilemap_set_scrolldy(TC0100SCN_tilemap[i][0][1], yd,    -flip_yoffs -yd);
		tilemap_set_scrolldx(TC0100SCN_tilemap[i][1][1], xd-16, -flip_xoffs -xd-16);
		tilemap_set_scrolldy(TC0100SCN_tilemap[i][1][1], yd,    -flip_yoffs -yd);
		tilemap_set_scrolldx(TC0100SCN_tilemap[i][2][1], xd-16, -flip_text_xoffs -xd-16-7);
		tilemap_set_scrolldy(TC0100SCN_tilemap[i][2][1], yd,    -flip_text_yoffs -yd);

		tilemap_set_scroll_rows(TC0100SCN_tilemap[i][0][0],512);
		tilemap_set_scroll_rows(TC0100SCN_tilemap[i][1][0],512);
		tilemap_set_scroll_rows(TC0100SCN_tilemap[i][0][1],512);
		tilemap_set_scroll_rows(TC0100SCN_tilemap[i][1][1],512);

		/* Default is for all used chips to access the same palette area */
		TC0100SCN_chip_colbank[i]=0;
	}

	TC0100SCN_gfxbank= 0;	/* Mjnquest uniquely banks tiles */
	state_save_register_global(machine, TC0100SCN_gfxbank);

	TC0100SCN_bg_tilemask = 0xffff;	/* Mjnquest has 0x7fff tilemask */

	TC0100SCN_bg_col_mult = 1;	/* multiplier for when bg gfx != 4bpp */
	TC0100SCN_tx_col_mult = 1;	/* multiplier needed when bg gfx is 6bpp */

	if (machine->gfx[gfxnum]->color_granularity == 2)	/* Yuyugogo, Yesnoj */
		TC0100SCN_bg_col_mult = 8;

	if (machine->gfx[gfxnum]->color_granularity == 0x40)	/* Undrfire */
		TC0100SCN_tx_col_mult = 4;

//logerror("TC0100SCN bg gfx granularity %04x: multiplier %04x\n",
//machine->gfx[gfxnum]->color_granularity,TC0100SCN_tx_col_mult);

	TC0100SCN_set_colbanks(0,0,0);	/* standard values, only Wgp changes them */
}


READ16_HANDLER( TC0100SCN_word_0_r )
{
	return TC0100SCN_ram[0][offset];
}

READ16_HANDLER( TC0100SCN_word_1_r )
{
	return TC0100SCN_ram[1][offset];
}

READ16_HANDLER( TC0100SCN_word_2_r )
{
	return TC0100SCN_ram[2][offset];
}

static void TC0100SCN_word_w(const address_space *space,int chip,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&TC0100SCN_ram[chip][offset]);
	if (!TC0100SCN_dblwidth[chip])
	{
		if (offset < 0x2000)
			tilemap_mark_tile_dirty(TC0100SCN_tilemap[chip][0][0],offset / 2);
		else if (offset < 0x3000)
			tilemap_mark_tile_dirty(TC0100SCN_tilemap[chip][2][0],(offset & 0x0fff));
		else if (offset < 0x3800)
			gfx_element_mark_dirty(space->machine->gfx[TC0100SCN_tx_gfx[chip]], (offset - 0x3000) / 8);
		else if (offset >= 0x4000 && offset < 0x6000)
			tilemap_mark_tile_dirty(TC0100SCN_tilemap[chip][1][0],(offset & 0x1fff) / 2);
	}
	else	/* Double-width tilemaps have a different memory map */
	{
		if (offset < 0x4000)
			tilemap_mark_tile_dirty(TC0100SCN_tilemap[chip][0][1],offset / 2);
		else if (offset >= 0x4000 && offset < 0x8000)
			tilemap_mark_tile_dirty(TC0100SCN_tilemap[chip][1][1],(offset & 0x3fff) / 2);
		else if (offset >= 0x8800 && offset < 0x9000)
			gfx_element_mark_dirty(space->machine->gfx[TC0100SCN_tx_gfx[chip]], (offset - 0x8800) / 8);
		else if (offset >= 0x9000)
			tilemap_mark_tile_dirty(TC0100SCN_tilemap[chip][2][1],(offset & 0x0fff));
	}
}

WRITE16_HANDLER( TC0100SCN_word_0_w )
{
	TC0100SCN_word_w(space,0,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0100SCN_word_1_w )
{
	TC0100SCN_word_w(space,1,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0100SCN_word_2_w )
{
	TC0100SCN_word_w(space,2,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0100SCN_dual_screen_w )
{
	TC0100SCN_word_0_w(space,offset,data,mem_mask);
	TC0100SCN_word_1_w(space,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0100SCN_triple_screen_w )
{
	TC0100SCN_word_0_w(space,offset,data,mem_mask);
	TC0100SCN_word_1_w(space,offset,data,mem_mask);
	TC0100SCN_word_2_w(space,offset,data,mem_mask);
}


READ16_HANDLER( TC0100SCN_ctrl_word_0_r )
{
	return TC0100SCN_ctrl[0][offset];
}

READ16_HANDLER( TC0100SCN_ctrl_word_1_r )
{
	return TC0100SCN_ctrl[1][offset];
}

READ16_HANDLER( TC0100SCN_ctrl_word_2_r )
{
	return TC0100SCN_ctrl[2][offset];
}


static void TC0100SCN_ctrl_word_w(const address_space *space,int chip,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&TC0100SCN_ctrl[chip][offset]);

	data = TC0100SCN_ctrl[chip][offset];

	switch (offset)
	{
		case 0x00:
			TC0100SCN_bgscrollx[chip] = -data;
			break;

		case 0x01:
			TC0100SCN_fgscrollx[chip] = -data;
			break;

		case 0x02:
			tilemap_set_scrollx(TC0100SCN_tilemap[chip][2][0],0,-data);
			tilemap_set_scrollx(TC0100SCN_tilemap[chip][2][1],0,-data);
			break;

		case 0x03:
			TC0100SCN_bgscrolly[chip] = -data;
			break;

		case 0x04:
			TC0100SCN_fgscrolly[chip] = -data;
			break;

		case 0x05:
			tilemap_set_scrolly(TC0100SCN_tilemap[chip][2][0],0,-data);
			tilemap_set_scrolly(TC0100SCN_tilemap[chip][2][1],0,-data);
			break;

		case 0x06:
		{
			int old_width = TC0100SCN_dblwidth[chip];
			TC0100SCN_dblwidth[chip] = (data &0x10) >> 4;

			if (TC0100SCN_dblwidth[chip] != old_width)	/* tilemap width is changing */
			{
				/* Reinitialise layer pointers */
				TC0100SCN_set_layer_ptrs(chip);

				/* and ensure full redraw of the tilemaps */
				TC0100SCN_dirty_tilemaps(chip);

				/* reset the pointer to the text characters (and dirty them all) */
				gfx_element_set_source(space->machine->gfx[TC0100SCN_tx_gfx[chip]], (UINT8 *)TC0100SCN_char_ram[chip]);
			}

			break;
		}

		case 0x07:
		{
			int flip = (data & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

			tilemap_set_flip(TC0100SCN_tilemap[chip][0][0],flip);
			tilemap_set_flip(TC0100SCN_tilemap[chip][1][0],flip);
			tilemap_set_flip(TC0100SCN_tilemap[chip][2][0],flip);
			tilemap_set_flip(TC0100SCN_tilemap[chip][0][1],flip);
			tilemap_set_flip(TC0100SCN_tilemap[chip][1][1],flip);
			tilemap_set_flip(TC0100SCN_tilemap[chip][2][1],flip);

			break;
		}
	}
}

WRITE16_HANDLER( TC0100SCN_ctrl_word_0_w )
{
	TC0100SCN_ctrl_word_w(space,0,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0100SCN_ctrl_word_1_w )
{
	TC0100SCN_ctrl_word_w(space,1,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0100SCN_ctrl_word_2_w )
{
	TC0100SCN_ctrl_word_w(space,2,offset,data,mem_mask);
}


READ32_HANDLER( TC0100SCN_ctrl_long_r )
{
	return (TC0100SCN_ctrl_word_0_r(space,offset*2,0xffff)<<16)|TC0100SCN_ctrl_word_0_r(space,offset*2+1,0xffff);
}

WRITE32_HANDLER( TC0100SCN_ctrl_long_w )
{
	if (ACCESSING_BITS_16_31) TC0100SCN_ctrl_word_w(space,0,offset*2,data>>16,mem_mask>>16);
	if (ACCESSING_BITS_0_15) TC0100SCN_ctrl_word_w(space,0,(offset*2)+1,data&0xffff,mem_mask&0xffff);
}

READ32_HANDLER( TC0100SCN_long_r )
{
	return (TC0100SCN_word_0_r(space,offset*2,0xffff)<<16)|TC0100SCN_word_0_r(space,offset*2+1,0xffff);
}

WRITE32_HANDLER( TC0100SCN_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		int oldword = TC0100SCN_word_0_r(space,offset*2,0xffff);
		int newword = data>>16;
		if (!ACCESSING_BITS_16_23)
			newword |= (oldword &0x00ff);
		if (!ACCESSING_BITS_24_31)
			newword |= (oldword &0xff00);
		TC0100SCN_word_0_w(space,offset*2,newword,0xffff);
	}
	if (ACCESSING_BITS_0_15)
	{
		int oldword = TC0100SCN_word_0_r(space,(offset*2)+1,0xffff);
		int newword = data&0xffff;
		if (!ACCESSING_BITS_0_7)
			newword |= (oldword &0x00ff);
		if (!ACCESSING_BITS_8_15)
			newword |= (oldword &0xff00);
		TC0100SCN_word_0_w(space,(offset*2)+1,newword,0xffff);
	}
}


void TC0100SCN_tilemap_update(running_machine *machine)
{
	int chip,j;

	for (chip = 0;chip < TC0100SCN_chips;chip++)
	{
		tilemap_set_scrolly(TC0100SCN_tilemap[chip][0][TC0100SCN_dblwidth[chip]],0,TC0100SCN_bgscrolly[chip]);
		tilemap_set_scrolly(TC0100SCN_tilemap[chip][1][TC0100SCN_dblwidth[chip]],0,TC0100SCN_fgscrolly[chip]);

		for (j = 0;j < 256;j++)
			tilemap_set_scrollx(TC0100SCN_tilemap[chip][0][TC0100SCN_dblwidth[chip]],
					(j + TC0100SCN_bgscrolly[chip]) & 0x1ff,
					TC0100SCN_bgscrollx[chip] - TC0100SCN_bgscroll_ram[chip][j]);
		for (j = 0;j < 256;j++)
			tilemap_set_scrollx(TC0100SCN_tilemap[chip][1][TC0100SCN_dblwidth[chip]],
					(j + TC0100SCN_fgscrolly[chip]) & 0x1ff,
					TC0100SCN_fgscrollx[chip] - TC0100SCN_fgscroll_ram[chip][j]);
	}
}

static void TC0100SCN_tilemap_draw_fg(running_machine *machine, bitmap_t *bitmap, const rectangle *cliprect, int chip, tilemap* tmap, int flags, UINT32 priority)
{
	const bitmap_t *src_bitmap = tilemap_get_pixmap(tmap);
	int width_mask, height_mask, x, y, p;
	int column_offset, src_x=0, src_y=0;
	int scrollx_delta = - tilemap_get_scrolldx( tmap );
	int scrolly_delta = - tilemap_get_scrolldy( tmap );

	width_mask=src_bitmap->width - 1;
	height_mask=src_bitmap->height - 1;

	src_y=(TC0100SCN_fgscrolly[chip] + scrolly_delta)&height_mask;
	if (TC0100SCN_ctrl[chip][0x7]&1) // Flipscreen
		src_y=(256-src_y)&height_mask;

	//We use cliprect->max_y and cliprect->max_x to support games which use more than 1 screen

	// Row offsets are 'screen space' 0-255 regardless of Y scroll
	for (y=0; y<=cliprect->max_y; y++) {
		src_x=(TC0100SCN_fgscrollx[chip] - TC0100SCN_fgscroll_ram[chip][(y + scrolly_delta)&0x1ff] + scrollx_delta + cliprect->min_x)&width_mask;
		if (TC0100SCN_ctrl[chip][0x7]&1) // Flipscreen
			src_x=(256 - 64 - src_x)&width_mask;

		// Col offsets are 'tilemap' space 0-511, and apply to blocks of 8 pixels at once
		for (x=0; x<=(cliprect->max_x - cliprect->min_x); x++) {
			column_offset=TC0100SCN_colscroll_ram[chip][(src_x&0x3ff) / 8];
			p=*BITMAP_ADDR16(src_bitmap, (src_y - column_offset)&height_mask, src_x);

			if ((p&0xf)!=0 || (flags & TILEMAP_DRAW_OPAQUE))
			{
				*BITMAP_ADDR16(bitmap, y, x + cliprect->min_x) = p;
				if (machine->priority_bitmap)
				{
					UINT8 *pri = BITMAP_ADDR8(machine->priority_bitmap, y, 0);
					pri[x + cliprect->min_x]|=priority;
				}
			}
			src_x=(src_x+1)&width_mask;
		}
		src_y=(src_y+1)&height_mask;
	}
}

int TC0100SCN_tilemap_draw(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int chip,int layer,int flags,UINT32 priority)
{
	int disable = TC0100SCN_ctrl[chip][6] & 0xf7;
	rectangle clip = *cliprect;
	sect_rect(&clip, &TC0100SCN_cliprect[chip]);

#if 0
if (disable != 0 && disable != 3 && disable != 7)
	popmessage("layer disable = %x",disable);
#endif

	switch (layer)
	{
		case 0:
			if (disable & 0x01) return 1;
			tilemap_draw(bitmap,&clip,TC0100SCN_tilemap[chip][0][TC0100SCN_dblwidth[chip]],flags,priority);
			break;
		case 1:
			if (disable & 0x02) return 1;
			TC0100SCN_tilemap_draw_fg(machine,bitmap,&clip,chip,TC0100SCN_tilemap[chip][1][TC0100SCN_dblwidth[chip]],flags,priority);
			break;
		case 2:
			if (disable & 0x04) return 1;
			tilemap_draw(bitmap,&clip,TC0100SCN_tilemap[chip][2][TC0100SCN_dblwidth[chip]],flags,priority);
			break;
	}
	return 0;
}

int TC0100SCN_bottomlayer(int chip)
{
	return (TC0100SCN_ctrl[chip][6] & 0x8) >> 3;
}


/***************************************************************************/

#define TC0280GRD_RAM_SIZE 0x2000
static UINT16 TC0280GRD_ctrl[8];
static UINT16 *TC0280GRD_ram;
static tilemap *TC0280GRD_tilemap;
static int TC0280GRD_gfxnum,TC0280GRD_base_color;


static TILE_GET_INFO( TC0280GRD_get_tile_info )
{
	int attr = TC0280GRD_ram[tile_index];
	SET_TILE_INFO(
			TC0280GRD_gfxnum,
			attr & 0x3fff,
			((attr & 0xc000) >> 14) + TC0280GRD_base_color,
			0);
}


void TC0280GRD_vh_start(running_machine *machine, int gfxnum)
{
	TC0280GRD_ram = auto_alloc_array(machine, UINT16, TC0280GRD_RAM_SIZE/2);
	TC0280GRD_tilemap = tilemap_create(machine, TC0280GRD_get_tile_info,tilemap_scan_rows,8,8,64,64);

	state_save_register_global_pointer(machine, TC0280GRD_ram, TC0280GRD_RAM_SIZE/2);
	state_save_register_global_array(machine, TC0280GRD_ctrl);

	tilemap_set_transparent_pen(TC0280GRD_tilemap,0);

	TC0280GRD_gfxnum = gfxnum;
}

void TC0430GRW_vh_start(running_machine *machine, int gfxnum)
{
	TC0280GRD_vh_start(machine, gfxnum);
}

READ16_HANDLER( TC0280GRD_word_r )
{
	return TC0280GRD_ram[offset];
}

READ16_HANDLER( TC0430GRW_word_r )
{
	return TC0280GRD_word_r(space,offset,mem_mask);
}

WRITE16_HANDLER( TC0280GRD_word_w )
{
	COMBINE_DATA(&TC0280GRD_ram[offset]);
	tilemap_mark_tile_dirty(TC0280GRD_tilemap,offset);
}

WRITE16_HANDLER( TC0430GRW_word_w )
{
	TC0280GRD_word_w(space,offset,data,mem_mask);
}

WRITE16_HANDLER( TC0280GRD_ctrl_word_w )
{
	COMBINE_DATA(&TC0280GRD_ctrl[offset]);
}

WRITE16_HANDLER( TC0430GRW_ctrl_word_w )
{
	TC0280GRD_ctrl_word_w(space,offset,data,mem_mask);
}

void TC0280GRD_tilemap_update(int base_color)
{
	if (TC0280GRD_base_color != base_color)
	{
		TC0280GRD_base_color = base_color;
		tilemap_mark_all_tiles_dirty(TC0280GRD_tilemap);
	}
}

void TC0430GRW_tilemap_update(int base_color)
{
	TC0280GRD_tilemap_update(base_color);
}

static void zoom_draw(bitmap_t *bitmap,const rectangle *cliprect,int xoffset,int yoffset,UINT32 priority,int xmultiply)
{
	UINT32 startx,starty;
	int incxx,incxy,incyx,incyy;

	/* 24-bit signed */
	startx = ((TC0280GRD_ctrl[0] & 0xff) << 16) + TC0280GRD_ctrl[1];
	if (startx & 0x800000) startx -= 0x1000000;
	incxx = (INT16)TC0280GRD_ctrl[2];
	incxx *= xmultiply;
	incyx = (INT16)TC0280GRD_ctrl[3];
	/* 24-bit signed */
	starty = ((TC0280GRD_ctrl[4] & 0xff) << 16) + TC0280GRD_ctrl[5];
	if (starty & 0x800000) starty -= 0x1000000;
	incxy = (INT16)TC0280GRD_ctrl[6];
	incxy *= xmultiply;
	incyy = (INT16)TC0280GRD_ctrl[7];

	startx -= xoffset * incxx + yoffset * incyx;
	starty -= xoffset * incxy + yoffset * incyy;

	tilemap_draw_roz(bitmap,cliprect,TC0280GRD_tilemap,startx << 4,starty << 4,
			incxx << 4,incxy << 4,incyx << 4,incyy << 4,
			1,	/* copy with wraparound */
			0,priority);
}

void TC0280GRD_zoom_draw(bitmap_t *bitmap,const rectangle *cliprect,int xoffset,int yoffset,UINT32 priority)
{
	zoom_draw(bitmap,cliprect,xoffset,yoffset,priority,2);
}

void TC0430GRW_zoom_draw(bitmap_t *bitmap,const rectangle *cliprect,int xoffset,int yoffset,UINT32 priority)
{
	zoom_draw(bitmap,cliprect,xoffset,yoffset,priority,1);
}


/***************************************************************************/

UINT8 TC0360PRI_regs[16];

void TC0360PRI_vh_start(running_machine *machine)
{
	state_save_register_global_array(machine, TC0360PRI_regs);
}

WRITE8_HANDLER( TC0360PRI_w )
{
	TC0360PRI_regs[offset] = data;

if (offset >= 0x0a)
	popmessage("write %02x to unused TC0360PRI reg %x",data,offset);
#if 0
#define regs TC0360PRI_regs
	popmessage("%02x %02x  %02x %02x  %02x %02x %02x %02x %02x %02x",
		regs[0x00],regs[0x01],regs[0x02],regs[0x03],
		regs[0x04],regs[0x05],regs[0x06],regs[0x07],
		regs[0x08],regs[0x09]);
#endif
}


/***************************************************************************/


#define TC0480SCP_RAM_SIZE 0x10000
#define TC0480SCP_TOTAL_CHARS 256
static UINT16 TC0480SCP_ctrl[0x18];
static UINT16 *TC0480SCP_ram,
		*TC0480SCP_bg_ram[4],
		*TC0480SCP_tx_ram,
		*TC0480SCP_char_ram,
		*TC0480SCP_bgscroll_ram[4],
		*TC0480SCP_rowzoom_ram[4],
		*TC0480SCP_bgcolumn_ram[4];
static int TC0480SCP_bgscrollx[4];
static int TC0480SCP_bgscrolly[4];
int TC0480SCP_pri_reg;

/* We keep two tilemaps for each of the 5 actual tilemaps: one at standard width, one double */
static tilemap *TC0480SCP_tilemap[5][2];
static int TC0480SCP_bg_gfx,TC0480SCP_tx_gfx;
static INT32 TC0480SCP_tile_colbase,TC0480SCP_dblwidth;
static int TC0480SCP_x_offs,TC0480SCP_y_offs;
static int TC0480SCP_text_xoffs,TC0480SCP_text_yoffs;
static int TC0480SCP_flip_xoffs,TC0480SCP_flip_yoffs;


INLINE void common_get_tc0480bg_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum)
{
	int code = ram[2*tile_index + 1] & 0x7fff;
	int attr = ram[2*tile_index];
	SET_TILE_INFO(
			gfxnum,
			code,
			(attr & 0xff) + TC0480SCP_tile_colbase,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_tc0480tx_tile_info(running_machine *machine,tile_data *tileinfo,int tile_index,UINT16 *ram,int gfxnum)
{
	int attr = ram[tile_index];
	SET_TILE_INFO(
			gfxnum,
			attr & 0xff,
			((attr & 0x3f00) >> 8) + TC0480SCP_tile_colbase,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO( TC0480SCP_get_bg0_tile_info )
{
	common_get_tc0480bg_tile_info(machine,tileinfo,tile_index,TC0480SCP_bg_ram[0],TC0480SCP_bg_gfx);
}

static TILE_GET_INFO( TC0480SCP_get_bg1_tile_info )
{
	common_get_tc0480bg_tile_info(machine,tileinfo,tile_index,TC0480SCP_bg_ram[1],TC0480SCP_bg_gfx);
}

static TILE_GET_INFO( TC0480SCP_get_bg2_tile_info )
{
	common_get_tc0480bg_tile_info(machine,tileinfo,tile_index,TC0480SCP_bg_ram[2],TC0480SCP_bg_gfx);
}

static TILE_GET_INFO( TC0480SCP_get_bg3_tile_info )
{
	common_get_tc0480bg_tile_info(machine,tileinfo,tile_index,TC0480SCP_bg_ram[3],TC0480SCP_bg_gfx);
}

static TILE_GET_INFO( TC0480SCP_get_tx_tile_info )
{
	common_get_tc0480tx_tile_info(machine,tileinfo,tile_index,TC0480SCP_tx_ram,TC0480SCP_tx_gfx);
}

static const tile_get_info_func tc480_get_tile_info[5] =
{
	TC0480SCP_get_bg0_tile_info, TC0480SCP_get_bg1_tile_info,
	TC0480SCP_get_bg2_tile_info, TC0480SCP_get_bg3_tile_info,
	TC0480SCP_get_tx_tile_info
};


static const gfx_layout TC0480SCP_charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
#ifdef LSB_FIRST
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4 },
#else
	{ 3*4, 2*4, 1*4, 0*4, 7*4, 6*4, 5*4, 4*4 },
#endif
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every sprite takes 32 consecutive bytes */
};

static void TC0480SCP_set_layer_ptrs(void)
{
	if (!TC0480SCP_dblwidth)
	{
		TC0480SCP_bg_ram[0]	  = TC0480SCP_ram + 0x0000; //0000
		TC0480SCP_bg_ram[1]	  = TC0480SCP_ram + 0x0800; //1000
		TC0480SCP_bg_ram[2]	  = TC0480SCP_ram + 0x1000; //2000
		TC0480SCP_bg_ram[3]	  = TC0480SCP_ram + 0x1800; //3000
		TC0480SCP_bgscroll_ram[0] = TC0480SCP_ram + 0x2000; //4000
		TC0480SCP_bgscroll_ram[1] = TC0480SCP_ram + 0x2200; //4400
		TC0480SCP_bgscroll_ram[2] = TC0480SCP_ram + 0x2400; //4800
		TC0480SCP_bgscroll_ram[3] = TC0480SCP_ram + 0x2600; //4c00
		TC0480SCP_rowzoom_ram[2]  = TC0480SCP_ram + 0x3000; //6000
		TC0480SCP_rowzoom_ram[3]  = TC0480SCP_ram + 0x3200; //6400
		TC0480SCP_bgcolumn_ram[2] = TC0480SCP_ram + 0x3400; //6800
		TC0480SCP_bgcolumn_ram[3] = TC0480SCP_ram + 0x3600; //6c00
		TC0480SCP_tx_ram		  = TC0480SCP_ram + 0x6000; //c000
		TC0480SCP_char_ram	  = TC0480SCP_ram + 0x7000; //e000
	}
	else
	{
		TC0480SCP_bg_ram[0]	  = TC0480SCP_ram + 0x0000; //0000
		TC0480SCP_bg_ram[1]	  = TC0480SCP_ram + 0x1000; //2000
		TC0480SCP_bg_ram[2]	  = TC0480SCP_ram + 0x2000; //4000
		TC0480SCP_bg_ram[3]	  = TC0480SCP_ram + 0x3000; //6000
		TC0480SCP_bgscroll_ram[0] = TC0480SCP_ram + 0x4000; //8000
		TC0480SCP_bgscroll_ram[1] = TC0480SCP_ram + 0x4200; //8400
		TC0480SCP_bgscroll_ram[2] = TC0480SCP_ram + 0x4400; //8800
		TC0480SCP_bgscroll_ram[3] = TC0480SCP_ram + 0x4600; //8c00
		TC0480SCP_rowzoom_ram[2]  = TC0480SCP_ram + 0x5000; //a000
		TC0480SCP_rowzoom_ram[3]  = TC0480SCP_ram + 0x5200; //a400
		TC0480SCP_bgcolumn_ram[2] = TC0480SCP_ram + 0x5400; //a800
		TC0480SCP_bgcolumn_ram[3] = TC0480SCP_ram + 0x5600; //ac00
		TC0480SCP_tx_ram		  = TC0480SCP_ram + 0x6000; //c000
		TC0480SCP_char_ram	  = TC0480SCP_ram + 0x7000; //e000
	}
}

static void TC0480SCP_dirty_tilemaps(void)
{
	tilemap_mark_all_tiles_dirty(TC0480SCP_tilemap[0][TC0480SCP_dblwidth]);
	tilemap_mark_all_tiles_dirty(TC0480SCP_tilemap[1][TC0480SCP_dblwidth]);
	tilemap_mark_all_tiles_dirty(TC0480SCP_tilemap[2][TC0480SCP_dblwidth]);
	tilemap_mark_all_tiles_dirty(TC0480SCP_tilemap[3][TC0480SCP_dblwidth]);
	tilemap_mark_all_tiles_dirty(TC0480SCP_tilemap[4][TC0480SCP_dblwidth]);
}

static void TC0480SCP_restore_scroll(void)
{
	int reg;
	int flip = TC0480SCP_ctrl[0xf] & 0x40;

	tilemap_set_flip(TC0480SCP_tilemap[0][0],flip);
	tilemap_set_flip(TC0480SCP_tilemap[1][0],flip);
	tilemap_set_flip(TC0480SCP_tilemap[2][0],flip);
	tilemap_set_flip(TC0480SCP_tilemap[3][0],flip);
	tilemap_set_flip(TC0480SCP_tilemap[4][0],flip);

	tilemap_set_flip(TC0480SCP_tilemap[0][1],flip);
	tilemap_set_flip(TC0480SCP_tilemap[1][1],flip);
	tilemap_set_flip(TC0480SCP_tilemap[2][1],flip);
	tilemap_set_flip(TC0480SCP_tilemap[3][1],flip);
	tilemap_set_flip(TC0480SCP_tilemap[4][1],flip);

	reg = TC0480SCP_ctrl[0];
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrollx[0] = reg;

	reg = TC0480SCP_ctrl[1] + 4;
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrollx[1] = reg;

	reg = TC0480SCP_ctrl[2] + 8;
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrollx[2] = reg;

	reg = TC0480SCP_ctrl[3] + 12;
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrollx[3] = reg;

	reg = TC0480SCP_ctrl[4];
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrolly[0] = reg;

	reg = TC0480SCP_ctrl[5];
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrolly[1] = reg;

	reg = TC0480SCP_ctrl[6];
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrolly[2] = reg;

	reg = TC0480SCP_ctrl[7];
	if (!flip)  reg = -reg;
	TC0480SCP_bgscrolly[3] = reg;

	reg = TC0480SCP_ctrl[0x0c];
	if (!flip)	reg -= TC0480SCP_text_xoffs;
	if (flip)	reg += TC0480SCP_text_xoffs;
	tilemap_set_scrollx(TC0480SCP_tilemap[4][0], 0, -reg);
	tilemap_set_scrollx(TC0480SCP_tilemap[4][1], 0, -reg);

	reg = TC0480SCP_ctrl[0x0d];
	if (!flip)	reg -= TC0480SCP_text_yoffs;
	if (flip)	reg += TC0480SCP_text_yoffs;
	tilemap_set_scrolly(TC0480SCP_tilemap[4][0], 0, -reg);
	tilemap_set_scrolly(TC0480SCP_tilemap[4][1], 0, -reg);
}


static STATE_POSTLOAD( TC0480SCP_postload )
{
	TC0480SCP_set_layer_ptrs();
	TC0480SCP_restore_scroll();
}

void TC0480SCP_vh_start(running_machine *machine, int gfxnum,int pixels,int x_offset,int y_offset,int text_xoffs,
				int text_yoffs,int flip_xoffs,int flip_yoffs,int col_base)
{
	int gfx_index;

		int i,xd,yd;
		TC0480SCP_tile_colbase = col_base;
		TC0480SCP_text_xoffs = text_xoffs;
		TC0480SCP_text_yoffs = text_yoffs;
		TC0480SCP_flip_xoffs = flip_xoffs;	/* for most games (-1,0) */
		TC0480SCP_flip_yoffs = flip_yoffs;
		TC0480SCP_dblwidth=0;

		/* Single width versions */
		TC0480SCP_tilemap[0][0] = tilemap_create(machine, tc480_get_tile_info[0],tilemap_scan_rows,16,16,32,32);
		TC0480SCP_tilemap[1][0] = tilemap_create(machine, tc480_get_tile_info[1],tilemap_scan_rows,16,16,32,32);
		TC0480SCP_tilemap[2][0] = tilemap_create(machine, tc480_get_tile_info[2],tilemap_scan_rows,16,16,32,32);
		TC0480SCP_tilemap[3][0] = tilemap_create(machine, tc480_get_tile_info[3],tilemap_scan_rows,16,16,32,32);
		TC0480SCP_tilemap[4][0] = tilemap_create(machine, tc480_get_tile_info[4],tilemap_scan_rows,8,8,64,64);

		/* Double width versions */
		TC0480SCP_tilemap[0][1] = tilemap_create(machine, tc480_get_tile_info[0],tilemap_scan_rows,16,16,64,32);
		TC0480SCP_tilemap[1][1] = tilemap_create(machine, tc480_get_tile_info[1],tilemap_scan_rows,16,16,64,32);
		TC0480SCP_tilemap[2][1] = tilemap_create(machine, tc480_get_tile_info[2],tilemap_scan_rows,16,16,64,32);
		TC0480SCP_tilemap[3][1] = tilemap_create(machine, tc480_get_tile_info[3],tilemap_scan_rows,16,16,64,32);
		TC0480SCP_tilemap[4][1] = tilemap_create(machine, tc480_get_tile_info[4],tilemap_scan_rows,8,8,64,64);

		TC0480SCP_ram = auto_alloc_array_clear(machine, UINT16, TC0480SCP_RAM_SIZE/2);

		TC0480SCP_set_layer_ptrs();

		state_save_register_global_pointer(machine, TC0480SCP_ram, TC0480SCP_RAM_SIZE/2);
		state_save_register_global_array(machine, TC0480SCP_ctrl);
		state_save_register_global(machine, TC0480SCP_dblwidth);
		state_save_register_postload(machine, TC0480SCP_postload, NULL);

		/* find first empty slot to decode gfx */
		for (gfx_index = 0; gfx_index < MAX_GFX_ELEMENTS; gfx_index++)
			if (machine->gfx[gfx_index] == 0)
				break;
		assert(gfx_index != MAX_GFX_ELEMENTS);

		/* create the char set (gfx will then be updated dynamically from RAM) */
		machine->gfx[gfx_index] = gfx_element_alloc(machine, &TC0480SCP_charlayout, (UINT8 *)TC0480SCP_char_ram, 64, 0);
		TC0480SCP_tx_gfx = gfx_index;

		/* use the given gfx set for bg tiles */
		TC0480SCP_bg_gfx = gfxnum;

		for (i=0;i<2;i++)
		{
			tilemap_set_transparent_pen(TC0480SCP_tilemap[0][i],0);
			tilemap_set_transparent_pen(TC0480SCP_tilemap[1][i],0);
			tilemap_set_transparent_pen(TC0480SCP_tilemap[2][i],0);
			tilemap_set_transparent_pen(TC0480SCP_tilemap[3][i],0);
			tilemap_set_transparent_pen(TC0480SCP_tilemap[4][i],0);
		}

		TC0480SCP_x_offs = x_offset + pixels;
		TC0480SCP_y_offs = y_offset;

		xd = -TC0480SCP_x_offs;
		yd =  TC0480SCP_y_offs;

		/* Metalb and Deadconx have minor screenflip issues: blue planet
           is off on x axis by 1 and in Deadconx the dark blue screen
           between stages also seems off by 1 pixel. */

		/* It's not possible to get the text scrolldx calculations
           harmonised with the other layers: xd-2, 315-xd is the
           next valid pair:- the numbers diverge from xd, 319-xd */

		/* Single width offsets */
		tilemap_set_scrolldx(TC0480SCP_tilemap[0][0], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[0][0], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[1][0], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[1][0], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[2][0], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[2][0], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[3][0], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[3][0], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[4][0], xd-3, 316-xd);	/* text layer */
		tilemap_set_scrolldy(TC0480SCP_tilemap[4][0], yd,   256-yd);	/* text layer */

		/* Double width offsets */
		tilemap_set_scrolldx(TC0480SCP_tilemap[0][1], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[0][1], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[1][1], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[1][1], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[2][1], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[2][1], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[3][1], xd,   320-xd +TC0480SCP_flip_xoffs );
		tilemap_set_scrolldy(TC0480SCP_tilemap[3][1], yd,   256-yd +TC0480SCP_flip_yoffs );
		tilemap_set_scrolldx(TC0480SCP_tilemap[4][1], xd-3, 317-xd);	/* text layer */
		tilemap_set_scrolldy(TC0480SCP_tilemap[4][1], yd,   256-yd);	/* text layer */

		for (i=0;i<2;i++)
		{
			/* Both sets of bg tilemaps scrollable per pixel row */
			tilemap_set_scroll_rows(TC0480SCP_tilemap[0][i],512);
			tilemap_set_scroll_rows(TC0480SCP_tilemap[1][i],512);
			tilemap_set_scroll_rows(TC0480SCP_tilemap[2][i],512);
			tilemap_set_scroll_rows(TC0480SCP_tilemap[3][i],512);
		}
}

READ32_HANDLER( TC0480SCP_ctrl_long_r )
{
	return (TC0480SCP_ctrl_word_r(space,offset*2,0xffff)<<16)|TC0480SCP_ctrl_word_r(space,offset*2+1,0xffff);
}

/* TODO: byte access ? */

WRITE32_HANDLER( TC0480SCP_ctrl_long_w )
{
	if (ACCESSING_BITS_16_31) TC0480SCP_ctrl_word_w(space,offset*2,data>>16,mem_mask>>16);
	if (ACCESSING_BITS_0_15) TC0480SCP_ctrl_word_w(space,(offset*2)+1,data&0xffff,mem_mask&0xffff);
}

READ32_HANDLER( TC0480SCP_long_r )
{
	return (TC0480SCP_word_r(space,offset*2,0xffff)<<16)|TC0480SCP_word_r(space,offset*2+1,0xffff);
}

WRITE32_HANDLER( TC0480SCP_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		int oldword = TC0480SCP_word_r(space,offset*2,0xffff);
		int newword = data>>16;
		if (!ACCESSING_BITS_16_23)
			newword |= (oldword &0x00ff);
		if (!ACCESSING_BITS_24_31)
			newword |= (oldword &0xff00);
		TC0480SCP_word_w(space,offset*2,newword,0xffff);
	}
	if (ACCESSING_BITS_0_15)
	{
		int oldword = TC0480SCP_word_r(space,(offset*2)+1,0xffff);
		int newword = data&0xffff;
		if (!ACCESSING_BITS_0_7)
			newword |= (oldword &0x00ff);
		if (!ACCESSING_BITS_8_15)
			newword |= (oldword &0xff00);
		TC0480SCP_word_w(space,(offset*2)+1,newword,0xffff);
	}
}

READ16_HANDLER( TC0480SCP_word_r )
{
	return TC0480SCP_ram[offset];
}

static void TC0480SCP_word_write(const address_space *space,offs_t offset,UINT16 data,UINT32 mem_mask)
{
	COMBINE_DATA(&TC0480SCP_ram[offset]);

	if (!TC0480SCP_dblwidth)
	{
		if (offset < 0x2000)
		{
			tilemap_mark_tile_dirty(TC0480SCP_tilemap[(offset /
				0x800)][TC0480SCP_dblwidth],((offset % 0x800) / 2));
		}
		else if (offset < 0x6000)
		{   /* do nothing */
		}
		else if (offset < 0x7000)
		{
			tilemap_mark_tile_dirty(TC0480SCP_tilemap[4][TC0480SCP_dblwidth],
				(offset - 0x6000));
		}
		else if (offset <= 0x7fff)
		{
			gfx_element_mark_dirty(space->machine->gfx[TC0480SCP_tx_gfx], (offset - 0x7000) / 16);
		}
	}
	else
	{
		if (offset < 0x4000)
		{
			tilemap_mark_tile_dirty(TC0480SCP_tilemap[(offset /
				0x1000)][TC0480SCP_dblwidth],((offset % 0x1000) / 2));
		}
		else if (offset < 0x6000)
		{   /* do nothing */
		}
		else if (offset < 0x7000)
		{
			tilemap_mark_tile_dirty(TC0480SCP_tilemap[4][TC0480SCP_dblwidth],
				(offset - 0x6000));
		}
		else if (offset <= 0x7fff)
		{
			gfx_element_mark_dirty(space->machine->gfx[TC0480SCP_tx_gfx], (offset - 0x7000) / 16);
		}
	}
}

WRITE16_HANDLER( TC0480SCP_word_w )
{
	TC0480SCP_word_write(space,offset,data,mem_mask);
}

READ16_HANDLER( TC0480SCP_ctrl_word_r )
{
	return TC0480SCP_ctrl[offset];
}

static void TC0480SCP_ctrl_word_write(offs_t offset,UINT16 data,UINT32 mem_mask)
{
	int flip = TC0480SCP_pri_reg & 0x40;

	COMBINE_DATA(&TC0480SCP_ctrl[offset]);
	data = TC0480SCP_ctrl[offset];

	switch( offset )
	{
		/* The x offsets of the four bg layers are staggered by intervals of 4 pixels */

		case 0x00:   /* bg0 x */
			if (!flip)  data = -data;
			TC0480SCP_bgscrollx[0] = data;
			break;

		case 0x01:   /* bg1 x */
			data += 4;
			if (!flip)  data = -data;
			TC0480SCP_bgscrollx[1] = data;
			break;

		case 0x02:   /* bg2 x */
			data += 8;
			if (!flip)  data = -data;
			TC0480SCP_bgscrollx[2] = data;
			break;

		case 0x03:   /* bg3 x */
			data += 12;
			if (!flip)  data = -data;
			TC0480SCP_bgscrollx[3] = data;
			break;

		case 0x04:   /* bg0 y */
			if (flip)  data = -data;
			TC0480SCP_bgscrolly[0] = data;
			break;

		case 0x05:   /* bg1 y */
			if (flip)  data = -data;
			TC0480SCP_bgscrolly[1] = data;
			break;

		case 0x06:   /* bg2 y */
			if (flip)  data = -data;
			TC0480SCP_bgscrolly[2] = data;
			break;

		case 0x07:   /* bg3 y */
			if (flip)  data = -data;
			TC0480SCP_bgscrolly[3] = data;
			break;

		case 0x08:   /* bg0 zoom */
		case 0x09:   /* bg1 zoom */
		case 0x0a:   /* bg2 zoom */
		case 0x0b:   /* bg3 zoom */
			break;

		case 0x0c:   /* fg (text) x */

			/* Text layer can be offset from bg0 (e.g. Metalb) */
			if (!flip)	data -= TC0480SCP_text_xoffs;
			if (flip)	data += TC0480SCP_text_xoffs;

			tilemap_set_scrollx(TC0480SCP_tilemap[4][0], 0, -data);
			tilemap_set_scrollx(TC0480SCP_tilemap[4][1], 0, -data);
			break;

		case 0x0d:   /* fg (text) y */

			/* Text layer can be offset from bg0 (e.g. Slapshot) */
			if (!flip)	data -= TC0480SCP_text_yoffs;
			if (flip)	data += TC0480SCP_text_yoffs;

			tilemap_set_scrolly(TC0480SCP_tilemap[4][0], 0, -data);
			tilemap_set_scrolly(TC0480SCP_tilemap[4][1], 0, -data);
			break;

		/* offset 0x0e unused */

		case 0x0f:   /* control register */
		{
			int old_width = (TC0480SCP_pri_reg &0x80) >> 7;
			flip = (data & 0x40) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
			TC0480SCP_pri_reg = data;

			tilemap_set_flip(TC0480SCP_tilemap[0][0],flip);
			tilemap_set_flip(TC0480SCP_tilemap[1][0],flip);
			tilemap_set_flip(TC0480SCP_tilemap[2][0],flip);
			tilemap_set_flip(TC0480SCP_tilemap[3][0],flip);
			tilemap_set_flip(TC0480SCP_tilemap[4][0],flip);

			tilemap_set_flip(TC0480SCP_tilemap[0][1],flip);
			tilemap_set_flip(TC0480SCP_tilemap[1][1],flip);
			tilemap_set_flip(TC0480SCP_tilemap[2][1],flip);
			tilemap_set_flip(TC0480SCP_tilemap[3][1],flip);
			tilemap_set_flip(TC0480SCP_tilemap[4][1],flip);

			TC0480SCP_dblwidth = (TC0480SCP_pri_reg &0x80) >> 7;

			if (TC0480SCP_dblwidth != old_width)	/* tilemap width is changing */
			{
				/* Reinitialise layer pointers */
				TC0480SCP_set_layer_ptrs();

				/* and ensure full redraw of tilemaps */
				TC0480SCP_dirty_tilemaps();
			}

			break;
		}

		/* Rest are layer specific delta x and y, used while scrolling that layer */
	}
}



WRITE16_HANDLER( TC0480SCP_ctrl_word_w )
{
	TC0480SCP_ctrl_word_write(offset,data,mem_mask);
}

void TC0480SCP_tilemap_update(running_machine *machine)
{
	int layer, zoom, i, j;
	int flip = TC0480SCP_pri_reg & 0x40;

	for (layer = 0; layer < 4; layer++)
	{
		tilemap_set_scrolly(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth],0,TC0480SCP_bgscrolly[layer]);
		zoom = 0x10000 + 0x7f - TC0480SCP_ctrl[0x08 + layer];

		if (zoom != 0x10000)	/* can't use scroll rows when zooming */
		{
			tilemap_set_scrollx(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth],
					0, TC0480SCP_bgscrollx[layer]);
		}
		else
		{
			for (j = 0;j < 512;j++)
			{
				i = TC0480SCP_bgscroll_ram[layer][j];

				if (!flip)
				tilemap_set_scrollx(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth],
						j & 0x1ff,
						TC0480SCP_bgscrollx[layer] - i);
				if (flip)
				tilemap_set_scrollx(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth],
						j & 0x1ff,
						TC0480SCP_bgscrollx[layer] + i);
			}
		}
	}
}


/*********************************************************************
                BG0,1 LAYER DRAW

TODO
----

Broken for any rotation except ROT0. ROT180 support could probably
be added without too much difficulty: machine_flip is there as a
place-holder for this purpose.

Wouldn't work if y needs to be > 255 (i.e. if some game uses a
bigger than usual vertical visible area). Refer to TC0080VCO
custom draw routine for an example of dealing with this.


Historical Issues
-----------------

1) bg layers got too far left and down, the greater the magnification.
   Largely fixed by adding offsets (to sx&y) which get bigger as we
   zoom in (why we have *zoomx and *zoomy in the calculations).

2) Hthero and Footchmp bg layers behaved differently when zoomed.
   Fixed by bringing tc0480scp_x&y_offs into calculations.

3) Metalb "TAITO" text in attract too far to the right. Fixed by
   bringing (layer*4) into offset calculations. But might be possible
   to avoid this by stepping the scroll deltas for the four layers -
   currently they are the same, and we have to kludge the offsets in
   TC0480SCP_ctrl_word_write.

4) Zoom movement was jagged: improved by bringing in scroll delta
   values... but the results are noticably imperfect.

**********************************************************************/

static void TC0480SCP_bg01_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int layer,int flags,UINT32 priority)
{
	/* X-axis zoom offers expansion only: 0 = no zoom, 0xff = max
       Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max
       (0x1a in Footchmp hiscore = shrunk) */

	int zoomx = 0x10000 - (TC0480SCP_ctrl[0x08 + layer] &0xff00);
	int zoomy = 0x10000 - (((TC0480SCP_ctrl[0x08 + layer] &0xff) - 0x7f) * 512);

	if ((zoomx == 0x10000) && (zoomy == 0x10000))	/* no zoom, simple */
	{
		/* Prevent bad things */
		tilemap_draw(bitmap,cliprect,TC0480SCP_tilemap[layer][TC0480SCP_dblwidth],flags,priority);
	}
	else	/* zoom */
	{
		UINT16 *dst16,*src16;
		UINT8 *tsrc;
		UINT16 scanline[512];
		UINT32 sx;
		bitmap_t *srcbitmap = tilemap_get_pixmap(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth]);
		bitmap_t *flagsbitmap = tilemap_get_flagsmap
							(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth]);
		int flip = TC0480SCP_pri_reg & 0x40;
		int i,y,y_index,src_y_index,row_index;
		int x_index,x_step;
		int machine_flip = 0;	/* for  ROT 180 ? */

		UINT16 screen_width = 512; //cliprect->max_x - cliprect->min_x + 1;
		UINT16 min_y = cliprect->min_y;
		UINT16 max_y = cliprect->max_y;

		int width_mask=0x1ff;
		if (TC0480SCP_dblwidth)	width_mask=0x3ff;


		if (!flip)
		{
			sx = ((TC0480SCP_bgscrollx[layer] + 15 + layer*4) << 16)
				+ ((255-(TC0480SCP_ctrl[0x10 + layer] & 0xff)) << 8);
			sx += (TC0480SCP_x_offs - 15 - layer*4) * zoomx;

			y_index = (TC0480SCP_bgscrolly[layer] << 16)
				+ ((TC0480SCP_ctrl[0x14 + layer] & 0xff) << 8);
			y_index -= (TC0480SCP_y_offs - min_y) * zoomy;
		}
		else	/* TC0480SCP tiles flipscreen */
		{
			sx = ((-TC0480SCP_bgscrollx[layer] + 15 + layer*4 + TC0480SCP_flip_xoffs ) << 16)
				+ ((255-(TC0480SCP_ctrl[0x10 + layer] & 0xff)) << 8);
			sx += (TC0480SCP_x_offs - 15 - layer*4) * zoomx;

			y_index = ((-TC0480SCP_bgscrolly[layer] + TC0480SCP_flip_yoffs) << 16)
				+ ((TC0480SCP_ctrl[0x14 + layer] & 0xff) << 8);
			y_index -= (TC0480SCP_y_offs - min_y) * zoomy;
		}


		if (!machine_flip) y=min_y; else y=max_y;

		do
		{
			src_y_index = (y_index>>16) &0x1ff;

			/* row areas are the same in flipscreen, so we must read in reverse */
			row_index = src_y_index;
			if (flip)	row_index = 0x1ff - row_index;

			x_index = sx - ((TC0480SCP_bgscroll_ram[layer][row_index] << 16))
				- ((TC0480SCP_bgscroll_ram[layer][row_index+0x800] << 8) &0xffff);

			src16 = BITMAP_ADDR16(srcbitmap, src_y_index, 0);
			tsrc  = BITMAP_ADDR8(flagsbitmap, src_y_index, 0);
			dst16 = scanline;

			x_step = zoomx;

			if (flags & TILEMAP_DRAW_OPAQUE)
			{
				for (i=0; i<screen_width; i++)
				{
					*dst16++ = src16[(x_index >> 16) &width_mask];
					x_index += x_step;
				}
			}
			else
			{
				for (i=0; i<screen_width; i++)
				{
					if (tsrc[(x_index >> 16) &width_mask])
						*dst16++ = src16[(x_index >> 16) &width_mask];
					else
						*dst16++ = 0x8000;
					x_index += x_step;
				}
			}

			taitoic_drawscanline(bitmap,cliprect,0,y,scanline,(flags & TILEMAP_DRAW_OPAQUE)?0:1,ROT0,machine->priority_bitmap,priority);

			y_index += zoomy;
			if (!machine_flip) y++; else y--;
		}
		while ( (!machine_flip && y<=max_y) || (machine_flip && y>=min_y) );

	}
}


/****************************************************************
                BG2,3 LAYER DRAW

TODO
----

Broken for any rotation except ROT0. ROT180 support could probably
be added without too much difficulty: machine_flip is there as a
place-holder for this purpose.

Wouldn't work if y needs to be > 255 (i.e. if some game uses a
bigger than usual vertical visible area). Refer to TC0080VCO
custom draw routine for an example of dealing with this.

Low order words for overall layer zoom are not really understood.
In Metalbj initial text screen zoom you can see they ARE words
(not separate bytes); however, I just use the low byte to smooth
the zooming sequences. This is noticeably imperfect on the Y axis.

Verify behaviour of Taito logo (Gunbustr) against real machine
to perfect the row zoom emulation.

What do high bytes of row zoom do - if anything - in UndrFire?
There is still jaggedness to the road in this game and Superchs.


Historical Issues
-----------------

Sometimes BG2/3 were misaligned by 1 pixel horizontally: this
was due to low order byte of 0 causing different (sx >> 16) than
when it was 1-255. To prevent this we use (255-byte) so
(sx >> 16) no longer depends on the low order byte.

In flipscreen we have to bring in extra offsets, since various
games don't have exactly (320-,256-) tilemap scroll deltas in
flipscreen.

****************************************************************/

static void TC0480SCP_bg23_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int layer,int flags,UINT32 priority)
{
	bitmap_t *srcbitmap = tilemap_get_pixmap(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth]);
	bitmap_t *flagsbitmap = tilemap_get_flagsmap
						(TC0480SCP_tilemap[layer][TC0480SCP_dblwidth]);

	UINT16 *dst16,*src16;
	UINT8 *tsrc;
	int i,y,y_index,src_y_index,row_index,row_zoom;
	int sx,x_index,x_step;
	UINT32 zoomx,zoomy;
	UINT16 scanline[512];
	int flipscreen = TC0480SCP_pri_reg & 0x40;
	int machine_flip = 0;	/* for  ROT 180 ? */

	UINT16 screen_width = 512; //cliprect->max_x - cliprect->min_x + 1;
	UINT16 min_y = cliprect->min_y;
	UINT16 max_y = cliprect->max_y;

	int width_mask=0x1ff;
	if (TC0480SCP_dblwidth)	width_mask=0x3ff;

	/* X-axis zoom offers expansion only: 0 = no zoom, 0xff = max
       Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max
       (0x1a in Footchmp hiscore = shrunk) */

	zoomx = 0x10000 - (TC0480SCP_ctrl[0x08 + layer] &0xff00);
	zoomy = 0x10000 - (((TC0480SCP_ctrl[0x08 + layer] &0xff) - 0x7f) * 512);

	if (!flipscreen)
	{
		sx = ((TC0480SCP_bgscrollx[layer] + 15 + layer*4) << 16)
			+ ((255-(TC0480SCP_ctrl[0x10 + layer] & 0xff)) << 8);
		sx += (TC0480SCP_x_offs - 15 - layer*4) * zoomx;

		y_index = (TC0480SCP_bgscrolly[layer] << 16)
			+ ((TC0480SCP_ctrl[0x14 + layer] & 0xff) << 8);
		y_index -= (TC0480SCP_y_offs - min_y) * zoomy;
	}
	else	/* TC0480SCP tiles flipscreen */
	{
		sx = ((-TC0480SCP_bgscrollx[layer] + 15 + layer*4 + TC0480SCP_flip_xoffs ) << 16)
			+ ((255-(TC0480SCP_ctrl[0x10 + layer] & 0xff)) << 8);
		sx += (TC0480SCP_x_offs - 15 - layer*4) * zoomx;

		y_index = ((-TC0480SCP_bgscrolly[layer] + TC0480SCP_flip_yoffs) << 16)
			+ ((TC0480SCP_ctrl[0x14 + layer] & 0xff) << 8);
		y_index -= (TC0480SCP_y_offs - min_y) * zoomy;
	}


	if (!machine_flip) y=min_y; else y=max_y;

	do
	{
		if (!flipscreen)
			src_y_index = ((y_index>>16) + TC0480SCP_bgcolumn_ram[layer][(y -
						TC0480SCP_y_offs) &0x1ff]) &0x1ff;
		else	/* colscroll area is back to front in flipscreen */
			src_y_index = ((y_index>>16) + TC0480SCP_bgcolumn_ram[layer][0x1ff -
						((y - TC0480SCP_y_offs) &0x1ff)]) &0x1ff;

		/* row areas are the same in flipscreen, so we must read in reverse */
		row_index = src_y_index;
		if (flipscreen)	row_index = 0x1ff - row_index;

		if (TC0480SCP_pri_reg & (layer-1))	/* bit0 enables for BG2, bit1 for BG3 */
			row_zoom = TC0480SCP_rowzoom_ram[layer][row_index];
		else
			row_zoom = 0;

		x_index = sx - ((TC0480SCP_bgscroll_ram[layer][row_index] << 16))
			- ((TC0480SCP_bgscroll_ram[layer][row_index+0x800] << 8) &0xffff);

		/* flawed calc ?? */
		x_index -= (TC0480SCP_x_offs - 0x1f + layer*4) * ((row_zoom &0xff) << 8);

/* We used to kludge 270 multiply factor, before adjusting x_index instead */

		x_step = zoomx;
		if (row_zoom)	/* need to reduce x_step */
		{
			if (!(row_zoom &0xff00))
				x_step -= ((row_zoom * 256) &0xffff);
			else	/* Undrfire uses the hi byte, why? */
				x_step -= (((row_zoom &0xff) * 256) &0xffff);
		}

		src16 = BITMAP_ADDR16(srcbitmap, src_y_index, 0);
		tsrc  = BITMAP_ADDR8(flagsbitmap, src_y_index, 0);
		dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (i=0; i<screen_width; i++)
			{
				*dst16++ = src16[(x_index >> 16) &width_mask];
				x_index += x_step;
			}
		}
		else
		{
			for (i=0; i<screen_width; i++)
			{
				if (tsrc[(x_index >> 16) &width_mask])
					*dst16++ = src16[(x_index >> 16) &width_mask];
				else
					*dst16++ = 0x8000;
				x_index += x_step;
			}
		}

		taitoic_drawscanline(bitmap,cliprect,0,y,scanline,(flags & TILEMAP_DRAW_OPAQUE)?0:1,ROT0,machine->priority_bitmap,priority);

		y_index += zoomy;
		if (!machine_flip) y++; else y--;
	}
	while ( (!machine_flip && y<=max_y) || (machine_flip && y>=min_y) );

}



void TC0480SCP_tilemap_draw(running_machine *machine,bitmap_t *bitmap,const rectangle *cliprect,int layer,int flags,UINT32 priority)
{
	/* no layer disable bits */

	switch (layer)
	{
		case 0:
			TC0480SCP_bg01_draw(machine,bitmap,cliprect,0,flags,priority);
			break;
		case 1:
			TC0480SCP_bg01_draw(machine,bitmap,cliprect,1,flags,priority);
			break;
		case 2:
			TC0480SCP_bg23_draw(machine,bitmap,cliprect,2,flags,priority);
			break;
		case 3:
			TC0480SCP_bg23_draw(machine,bitmap,cliprect,3,flags,priority);
			break;
		case 4:
			tilemap_draw(bitmap,cliprect,TC0480SCP_tilemap[4][TC0480SCP_dblwidth],flags,priority);
			break;
	}
}

/* For evidence table of TC0480SCP bg layer priorities, refer to mame55 source */

static const UINT16 TC0480SCP_bg_pri_lookup[8] =
{
	0x0123,
	0x1230,
	0x2301,
	0x3012,
	0x3210,
	0x2103,
	0x1032,
	0x0321
};

int TC0480SCP_get_bg_priority(void)
{
	return TC0480SCP_bg_pri_lookup[(TC0480SCP_pri_reg &0x1c) >> 2];
}


/****************************************************************
                            TC0150ROD
****************************************************************/

static UINT16 *TC0150ROD_ram;
#define TC0150ROD_RAM_SIZE 0x2000

READ16_HANDLER( TC0150ROD_word_r )
{
	return TC0150ROD_ram[offset];
}

WRITE16_HANDLER( TC0150ROD_word_w )
{
	COMBINE_DATA(&TC0150ROD_ram[offset]);
}

void TC0150ROD_vh_start(running_machine *machine)
{
	TC0150ROD_ram = auto_alloc_array(machine, UINT16, TC0150ROD_RAM_SIZE/2);

	state_save_register_global_pointer(machine, TC0150ROD_ram, TC0150ROD_RAM_SIZE/2);
}


/******************************************************************************

    Memory map for TC0150ROD
    ------------------------

    0000-07ff  Road A, bank 0   [all are 256 lines]
    0800-0fff  Road A, bank 1
    1000-17ff  Road B, bank 0
    1800-1fff  Road B, bank 1

    1ffe-1fff  Control word
               ........ xxxxxxxx    Screen line where priority changes
               ......xx ........    Road A RAM page
               ....xx.. ........    Road B RAM page
               0000.... ........    Not used?



    Road ram line layout (thanks to Raine for original table)
    --------------------

    -----+-----------------+----------------------------------------
    Word | Bit(s)          |  Info
    -----+-----------------+----------------------------------------
      0  |x....... ........|  Draw background behind left road edge
      0  |.x...... ........|  Left road edge from road A has priority over road B ?? (+)
      0  |..x..... ........|  Left road edge from road A has priority over road B ?? (*)
      0  |...xx... ........|  Left edge/background palette entry offset
      0  |......xx xxxxxxxx|  Left edge   [pixels from road center] (@)
         |                 |
      1  |x....... ........|  Draw background behind right road edge
      1  |.x...... ........|  Right road edge from road A has priority over road B ??
      1  |..x..... ........|  Right road edge from road A has priority over road B ?? (*)
      1  |...xx... ........|  Right edge/background palette entry offset
      1  |......xx xxxxxxxx|  Right edge   [pixels from road center] (@)
         |                 |
      2  |x....... ........|  Draw background behind road body
      2  |.x...... ........|  Road line body from Road A has higher priority than Road B ??
      2  |..x..... ........|  Road line body from Road A has higher priority than Road B ??
      2  |...xx... ........|  Body/background palette entry offset
      2  |.....xxx xxxxxxxx|  X Offset   [offset is inverted] (^)
         |                 |
      3  |xxxx.... ........|  Color Bank  (selects group of 4 palette entries used for line)
      3  |....xxxx xxxxxxxx|  Road Gfx Tile number (top 2 bits not used by any game)
    -----+-----------------+-----------------------------------------

    @ size of bitmask suggested by Nightstr stage C when boss appears
    ^ bitmask confirmed in ChaseHQ code

    * see Nightstr "stage choice tunnel"
    + see Contcirc track at race start

    These priority bits have a different meaning in road B ram. They appear to mean
    that the relevant part of road B slips under road A. I.e. in road A they raise
    priority, in road B they lower it.

    We need a screenshot of Nightstr "stage choice tunnel" showing exactly what effect
    happens at top and bottom of screen while tunnel bifurcates.


Priority Levels - used by this code to represent the way the TC0150ROD appears to work
---------------

To speed up the code, three bits in the existing pixel-color map are used to store
priority information:

x....... ........ = transparency
.xxx.... ........ = pixel priority
....xxxx xxxxxxxx = pixel pen

There's a problem if any TaitoZ games using twice the palette space turn
up that also use TC0150ROD. This seems unlikely.

Pixel priority levels
---------------------
0 = bottom (used for off-edge i.e. background)
1,2,3,4 = ascending priority levels


Priority bits refer to: (edge a, body a, edge b, body b)

Standard:  bits=(0,0,0,0)
     edge a, body a, edge b, body b
     1       2       3       4

Contcirc:  bits=(0,1,0,0) (body a up by 2)
     edge a, edge b, body b, body a
     1       3       4       4

Nightstr bottom half:  bits=(0,1,1,0) (Contcirc PLUS edge b down by 2)
     edge b, edge a, body b, body a
     1       1       4       4

Nightstr top half:  bits=(1,0,0,1) (edge b down by 1, body a up by 1)
     edge a, edge b, body a, body b
     1       2       3       4

When numbers are the same, A goes on top...

These may need revising once Nightstr / Contcirc screenshots showing road
intersections are obtained.



Road info
---------

Road gfx is 2bpp, each word holds 8 pixels in this format:
xxxxxxxx ........  lo 8 bits
........ xxxxxxxx  hi 8 bits

The line gfx is back to front: this is why we call 'left' 'right' and vice versa in
this code: when the pixels are poked in they are done in reverse order, restoring
the orientation.

Each road gfx tile is 0x200 long in the rom. This comprises TWO road lines each
of 1024x1 pixels.

The first is the "edge" graphic. The second is the road body graphic. This means
separate sets of colors can be used for road edge and road body, giving greater
color variety.

The edge graphic is stored with the edges touching each other. So we must pull LHS
and RHS out separately to draw them.

Gfx lines: generally 0-0x1ff are the standard group (higher tile number indexes
wider lines). However this is just the way the games are: NOT a function of the
TC0150ROD.

Proof of background palette entry offset is in Contcirc in the tunnel on
Monaco level, the flyer screenshot shows different background colors on left
and right.

To investigate the weird road A/B priority system look at Nightstr and also
Contcirc. Contcirc: the "pit entry lane" in road B looks completely wrong if it is
allowed on top of road A body.

Aquajack road requires correct bank selection, or it goes crazy.

Should pen0 in Road A body be transparent? It seems necessary for Bshark round 6
and it makes Aquajack roads look much better. However, in Nightstr stage C
this results in a black band in the middle of the water. Also, it leaves
transparent areas in Dblaxle road which look obviously wrong. For time being a
parameter has been added to select which games use the transparency.

TODO
----

Contcirc: is road really meant to be so ugly when going uphill?

ChaseHQ: take right fork on level 1, at either edge of road you can see black
edge. Not obvious what is going wrong here. What color is it meant to be?

Nightstr: is the choice tunnel split correct? Need verification from machine.

Dblaxle: some stray background lines at top of road on occasional frames.

Sprite/road Y positions sometimes don't match. Maybe we need to use a zoom
lookup table from rom for the TaitoZ sprites.


******************************************************************************/

void TC0150ROD_draw(running_machine *machine, bitmap_t *bitmap,const rectangle *cliprect,int y_offs,int palette_offs,int type,int road_trans,UINT32 low_priority,UINT32 high_priority)
{
#ifdef MAME_DEBUG
	static int dislayer[6];	/* Road Layer toggles to help get road correct */
#endif

	int x_offs = 0xa7;	/* Increasing this shifts road to right */
	UINT16 scanline[512];
	UINT16 roada_line[512],roadb_line[512];
	UINT16 *dst16;
	UINT16 *roada,*roadb;
	UINT16 *roadgfx = (UINT16 *)memory_region(machine, "gfx3");

	UINT16 pixel,color,gfx_word;
	UINT16 roada_clipl,roada_clipr,roada_bodyctrl;
	UINT16 roadb_clipl,roadb_clipr,roadb_bodyctrl;
	UINT16 pri,pixpri;
	UINT8 priorities[6];
	int x_index,roadram_index,roadram2_index,i;
	int xoffset,paloffs,palloffs,palroffs;
	int road_gfx_tilenum,colbank,road_center;
	int road_ctrl = TC0150ROD_ram[0xfff];
	int left_edge,right_edge,begin,end,right_over,left_over;
	int line_needs_drawing,draw_top_road_line,background_only;

	int min_x = cliprect->min_x;
	int max_x = cliprect->max_x;
	int min_y = cliprect->min_y;
	int max_y = cliprect->max_y;
	int screen_width = max_x - min_x + 1;

	int y = min_y;

	int twin_road = 0;

	int road_A_address = y_offs * 4 + ((road_ctrl & 0x0300) << 2);	/* Index into roadram for road A */
	int road_B_address = y_offs * 4 + ((road_ctrl & 0x0c00) << 0);	/* Index into roadram for road B */

	int priority_switch_line = (road_ctrl & 0x00ff) - y_offs;

#ifdef MAME_DEBUG
	if (input_code_pressed_once (machine, KEYCODE_X))
	{
		dislayer[0] ^= 1;
		popmessage("RoadA body: %01x",dislayer[0]);
	}

	if (input_code_pressed_once (machine, KEYCODE_C))
	{
		dislayer[1] ^= 1;
		popmessage("RoadA l-edge: %01x",dislayer[1]);
	}

	if (input_code_pressed_once (machine, KEYCODE_V))
	{
		dislayer[2] ^= 1;
		popmessage("RoadA r-edge: %01x",dislayer[2]);
	}

	if (input_code_pressed_once (machine, KEYCODE_B))
	{
		dislayer[3] ^= 1;
		popmessage("RoadB body: %01x",dislayer[3]);
	}

	if (input_code_pressed_once (machine, KEYCODE_N))
	{
		dislayer[4] ^= 1;
		popmessage("RoadB l-edge: %01x",dislayer[4]);
	}
	if (input_code_pressed_once (machine, KEYCODE_M))
	{
		dislayer[5] ^= 1;
		popmessage("RoadB r-edge: %01x",dislayer[5]);
	}
#endif

#if 0
	if (1)
	{
		char buf3[80];
		sprintf(buf3,"road control: %04x",road_ctrl);
		popmessage(buf3);
	}
#endif

	do
	{
		line_needs_drawing = 0;

		roadram_index  = road_A_address + y * 4;	/* in case there is some switching mechanism (unlikely) */
		roadram2_index = road_B_address + y * 4;

		roada = roada_line;
		roadb = roadb_line;

		for (i=0;i<screen_width;i++)	/* Default transparency fill */
		{
			*roada++ = 0x8000;
			*roadb++ = 0x8000;
		}

		/* l-edge a, r-edge a, body a, l-edge b, r-edge-b, body b */
		priorities[0] = 1;
		priorities[1] = 1;
		priorities[2] = 2;
		priorities[3] = 3;
		priorities[4] = 3;
		priorities[5] = 4;

		roada_clipr    = TC0150ROD_ram[roadram_index];
		roada_clipl    = TC0150ROD_ram[roadram_index+1];
		roada_bodyctrl = TC0150ROD_ram[roadram_index+2];
		roadb_clipr    = TC0150ROD_ram[roadram2_index];
		roadb_clipl    = TC0150ROD_ram[roadram2_index+1];
		roadb_bodyctrl = TC0150ROD_ram[roadram2_index+2];

		/* Not very logical, but seems to work */
		if (roada_bodyctrl & 0x2000)	priorities[2] += 2;
		if (roadb_bodyctrl & 0x2000)	priorities[2] += 1;
		if (roada_clipl    & 0x2000)	priorities[3] -= 1;
		if (roadb_clipl    & 0x2000)	priorities[3] -= 2;
		if (roada_clipr    & 0x2000)	priorities[4] -= 1;
		if (roadb_clipr    & 0x2000)	priorities[4] -= 2;

		if (priorities[4] == 0)	priorities[4]++;	/* Fixes Aquajack LH edge dropping below background */

		if ((roada_bodyctrl &0x8000) || (roadb_bodyctrl &0x8000))
			twin_road ++;

		/********************************************************/
		/*                        ROAD A                        */
            /********************************************************/

		palroffs =(roada_clipr &0x1000) >> 11;
		palloffs =(roada_clipl &0x1000) >> 11;
		xoffset  = roada_bodyctrl &0x7ff;
		paloffs  =(roada_bodyctrl &0x1800) >> 11;
		colbank  =(TC0150ROD_ram[roadram_index+3] &0xf000) >> 10;
		road_gfx_tilenum = TC0150ROD_ram[roadram_index+3] &0x3ff;
		right_over = 0;
		left_over = 0;

		road_center = 0x5ff - ((-xoffset + x_offs) &0x7ff);
		left_edge = road_center - (roada_clipl &0x3ff);		/* start pixel for left edge */
		right_edge = road_center + 1 + (roada_clipr &0x3ff);	/* start pixel for right edge */

		if ((roada_clipl) || (roada_clipr))	line_needs_drawing = 1;

		/* Main road line is drawn from 'begin' to 'end'-1 */

		begin = left_edge + 1;
		if (begin < 0)
		{
			begin = 0;	/* can't begin off edge of screen */
		}

		end = right_edge;
		if (end > screen_width)
		{
			end = screen_width;	/* can't end off edge of screen */
		}

		/* We need to offset start pixel we draw for road edge when edge of
           road is partially or wholly offscreen on the opposite side
           e.g. Contcirc attract */

		if (right_edge < 0)
		{
			right_over = -right_edge;
			right_edge = 0;
		}
		if (left_edge >= screen_width)
		{
			left_over = left_edge - screen_width + 1;
			left_edge = screen_width - 1;
		}

		/* If road is way off to right we only need to plot background */
		background_only = (road_center > (screen_width - 2 + 1024/2)) ? 1 : 0;


		/********* Draw main part of road *********/

		color = ((palette_offs + colbank + paloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[2] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[0])
#endif
		{
		/* Is this calculation imperfect ?  (0xa0 = screen width/2) */
		x_index = (-xoffset + x_offs + begin) &0x7ff;

		roada = roada_line + screen_width - 1 - begin;

		if ((line_needs_drawing) && (begin < end))
		{
			for (i=begin; i<end; i++)
			{
				if (road_gfx_tilenum)	/* fixes Nightstr round C */
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7-(x_index % 8) + 8)) &0x1) * 2 + ((gfx_word >> (7-(x_index % 8))) &0x1);

					if ((pixel) || !(road_trans))
					{
						if (type)	pixel = (pixel-1)&3;
						*roada-- = (color + pixel) | pri;
					}
					else	*roada-- = 0xf000;	/* priority transparency, fixes Bshark round 6 + Aquajack */
				}
				else roada--;

				x_index++;
				x_index &= 0x7ff;
			}
		}
		}


		/********* Draw 'left' road edge *********/

		color = ((palette_offs + colbank + palloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[0] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[2])
#endif
		{
		if (background_only)	/* The "road edge" line is entirely off screen so can't be drawn */
		{
			if (roada_clipl &0x8000)	/* but we may need to fill in the background color */
			{
				roada = roada_line;
				for (i=0;i<screen_width;i++)
				{
					*roada++ = (color + (type ? (3) : (0)));
				}
			}
		}
		else
		{
			if ((left_edge >= 0) && (left_edge < screen_width))
			{
				x_index = (1024/2 - 1 - left_over) &0x7ff;

				roada = roada_line + screen_width - 1 - left_edge;

				if (line_needs_drawing)
				{
					for (i=left_edge; i>=0; i--)
					{
						gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
						pixel = ((gfx_word >> (7-(x_index % 8) + 8)) &0x1) * 2 + ((gfx_word >> (7-(x_index % 8))) &0x1);

						pixpri = (pixel==0) ? (0) : (pri);	/* off edge has low priority */

						if ((pixel==0) && !(roada_clipl &0x8000))
						{
							roada++;
						}
						else
						{
							if (type)	pixel = (pixel-1)&3;
							*roada++ = (color + pixel) | pixpri;
						}

						x_index--;
						x_index &= 0x7ff;
					}
				}
			}
		}
		}


		/********* Draw 'right' road edge *********/

		color = ((palette_offs + colbank + palroffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[1] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[1])
#endif
		{
		if ((right_edge < screen_width) && (right_edge >= 0))
		{
			x_index = (1024/2 + right_over) &0x7ff;

			roada = roada_line + screen_width - 1 - right_edge;

			if (line_needs_drawing)
			{
				for (i=right_edge; i<screen_width; i++)
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7-(x_index % 8) + 8)) &0x1) * 2 + ((gfx_word >> (7-(x_index % 8))) &0x1);

					pixpri = (pixel==0) ? (0) : (pri);	/* off edge has low priority */

					if ((pixel==0) && !(roada_clipr &0x8000))
					{
						roada--;
					}
					else
					{
						if (type)	pixel = (pixel-1)&3;
						*roada-- = (color + pixel) | pixpri;
					}

					x_index++;
					x_index &= 0x7ff;
				}
			}
		}
		}


		/********************************************************/
		/*                        ROAD B                        */
            /********************************************************/

		palroffs = (roadb_clipr &0x1000) >> 11;
		palloffs = (roadb_clipl &0x1000) >> 11;
		xoffset  =  roadb_bodyctrl &0x7ff;
		paloffs  = (roadb_bodyctrl &0x1800) >> 11;
		colbank  = (TC0150ROD_ram[roadram2_index+3] &0xf000) >> 10;
		road_gfx_tilenum = TC0150ROD_ram[roadram2_index+3] &0x3ff;
		right_over = 0;
		left_over = 0;

		road_center = 0x5ff - ((-xoffset + x_offs) &0x7ff);

// ChaseHQ glitches on right when road rejoins:
// de7, de8 causes problems => 5e7/e8
// 5ff - (a7 - 5e7)
// 5ff - 2c0 = 33f / 340 which is not quite > screenwidth + 1024/2: so we subtract 2 more, fixed


// ChaseHQ glitches on right when road rejoins:
// 0a6 and lower => 0x5ff 5fe etc.
// 35c => 575 right road edge wraps back onto other side of screen
// 5ff-54a     thru    5ff-331
// b6          thru    2ce
// 2a6 thru 0 thru 5a7 ??

		left_edge = road_center - (roadb_clipl &0x3ff);		/* start pixel for left edge */
		right_edge = road_center + 1 + (roadb_clipr &0x3ff);	/* start pixel for right edge */

		if (((roadb_clipl) || (roadb_clipr)) && ((road_ctrl &0x800) || (type==2)))
		{
			draw_top_road_line = 1;
			line_needs_drawing = 1;
		}
		else	draw_top_road_line = 0;

		/* Main road line is drawn from 'begin' to 'end'-1 */

		begin = left_edge + 1;
		if (begin < 0)
		{
			begin = 0;	/* can't begin off edge of screen */
		}

		end = right_edge;
		if (end > screen_width)
		{
			end = screen_width;	/* can't end off edge of screen */
		}

		/* We need to offset start pixel we draw for road edge when edge of
           road is partially or wholly offscreen on the opposite side
           e.g. Contcirc attract */

		if (right_edge < 0)
		{
			right_over = -right_edge;
			right_edge = 0;
		}
		if (left_edge >= screen_width)
		{
			left_over = left_edge - screen_width + 1;
			left_edge = screen_width - 1;
		}

		/* If road is way off to right we only need to plot background */
		background_only = (road_center > (screen_width - 2 + 1024/2)) ? 1 : 0;


		/********* Draw main part of road *********/

		color = ((palette_offs + colbank + paloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[5] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[3])
#endif
		{
		/* Is this calculation imperfect ?  (0xa0 = screen width/2) */
		x_index = (-xoffset + x_offs + begin) &0x7ff;

		if (x_index > 0x3ff)	/* Second half of gfx contains the road body line */
		{
			roadb = roadb_line + screen_width - 1 - begin;

			if (draw_top_road_line && road_gfx_tilenum && (begin < end))
			{
				for (i=begin; i<end; i++)
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7-(x_index % 8) + 8)) &0x1) * 2 + ((gfx_word >> (7-(x_index % 8))) &0x1);

					if ((pixel) || !(road_trans))
					{
						if (type)	pixel = (pixel-1)&3;
						*roadb-- = (color + pixel) | pri;
					}
					else	*roadb-- = 0xf000;	/* high priority transparency, fixes Aquajack */

					x_index++;
					x_index &= 0x7ff;
				}
			}
		}
		}


		/********* Draw 'left' road edge *********/

		color = ((palette_offs + colbank + palloffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[3] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[5])
#endif
		{
		if (background_only)	/* The "road edge" line is entirely off screen so can't be drawn */
		{
			if ((roadb_clipl &0x8000) && draw_top_road_line)	/* but we may need to fill in the background color */
			{
				roadb = roadb_line;
				for (i=0;i<screen_width;i++)
				{
					*roadb++ = (color + (type ? (3) : (0)));
				}
			}
		}
		else
		{
			if ((left_edge >= 0) && (left_edge < screen_width))
			{
				x_index = (1024/2 - 1 - left_over) &0x7ff;

				roadb = roadb_line + screen_width - 1 - left_edge;

				if (draw_top_road_line)		// rename to draw_roadb_line !?
				{
					for (i=left_edge; i>=0; i--)
					{
						gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
						pixel = ((gfx_word >> (7-(x_index % 8) + 8)) &0x1) * 2 + ((gfx_word >> (7-(x_index % 8))) &0x1);

						pixpri = (pixel==0) ? (0) : (pri);	/* off edge has low priority */

						if ((pixel==0) && !(roadb_clipl &0x8000))	/* test for background disabled */
						{
							roadb++;
						}
						else
						{
							if (type)	pixel = (pixel-1)&3;
							*roadb++ = (color + pixel) | pixpri;
						}

						x_index--;
						if (x_index < 0)	break;
					}
				}
			}
		}
		}


		/********* Draw 'right' road edge *********/

		color = ((palette_offs + colbank + palroffs) << 4) + ((type) ? (1) : (4));
		pri = priorities[4] << 12;

#ifdef MAME_DEBUG
	if (!dislayer[4])
#endif
		{
		if ((right_edge < screen_width) && (right_edge >= 0))
		{
			x_index = (1024/2 + right_over) &0x7ff;

			roadb = roadb_line + screen_width - 1 - right_edge;

			if (draw_top_road_line)
			{
				for (i=right_edge; i<screen_width; i++)
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7-(x_index % 8) + 8)) &0x1) * 2 + ((gfx_word >> (7-(x_index % 8))) &0x1);

					pixpri = (pixel==0) ? (0) : (pri);	/* off edge has low priority */

					if ((pixel==0) && !(roadb_clipr &0x8000))	/* test for background disabled */
					{
						roadb--;
					}
					else
					{
						if (type)	pixel = (pixel-1)&3;
						*roadb-- =  (color + pixel) | pixpri;
					}

					x_index++;
					if (x_index > 0x3ff)	break;
				}
			}
		}
		}


		/******** Combine the two lines according to pixel priorities ********/

		if (line_needs_drawing)
		{
			dst16 = scanline;

			for (i=0;i<screen_width;i++)
			{
				if (roada_line[i] == 0x8000)	/* road A pixel transparent */
				{
					*dst16++ = roadb_line[i] & 0x8fff;
				}
				else if (roadb_line[i] == 0x8000)	/* road B pixel transparent */
				{
					*dst16++ = roada_line[i] & 0x8fff;
				}
				else	/* two competing pixels, which has highest priority... */
				{
					if ((roadb_line[i] & 0x7000) > (roada_line[i] & 0x7000))
					{
						*dst16++ = roadb_line[i] & 0x8fff;
					}
					else
					{
						*dst16++ = roada_line[i] & 0x8fff;
					}
				}
			}

			taitoic_drawscanline(bitmap,cliprect,0,y,scanline,1,ROT0,machine->priority_bitmap,(y > priority_switch_line)?high_priority:low_priority);
		}

		y++;
	}
	while (y <= max_y);

#if 0
	if (twin_road)	// I don't know what this means, actually...
	{
		char buf2[80];
		sprintf(buf2,"Road twinned for %04x lines",twin_road);
		popmessage(buf2);
	}
#endif
}

/***************************************************************************/


static int TC0110PCR_type = 0;
static int TC0110PCR_addr[3];
static UINT16 *TC0110PCR_ram[3];
#define TC0110PCR_RAM_SIZE 0x2000


static STATE_POSTLOAD( TC0110PCR_restore_colors )
{
	int chip = (FPTR)param;
	int i,color,r=0,g=0,b=0;

	for (i=0; i<(256*16); i++)
	{
		color = TC0110PCR_ram[chip][i];

		switch (TC0110PCR_type)
		{

			case 0x00:
			{
				r = pal5bit(color >>  0);
				g = pal5bit(color >>  5);
				b = pal5bit(color >> 10);
				break;
			}

			case 0x01:
			{
				b = pal5bit(color >>  0);
				g = pal5bit(color >>  5);
				r = pal5bit(color >> 10);
				break;
			}

			case 0x02:
			{
				r = pal4bit(color >> 0);
				g = pal4bit(color >> 4);
				b = pal4bit(color >> 8);
				break;
			}
		}

		palette_set_color(machine, i + (chip << 12),MAKE_RGB(r,g,b));
	}
}


void TC0110PCR_vh_start(running_machine *machine)
{
	TC0110PCR_ram[0] = auto_alloc_array(machine, UINT16, TC0110PCR_RAM_SIZE);

	state_save_register_global_pointer(machine, TC0110PCR_ram[0], TC0110PCR_RAM_SIZE);
	state_save_register_postload(machine, TC0110PCR_restore_colors, (void *)0);

	TC0110PCR_type = 0;	/* default, xBBBBBGGGGGRRRRR */
}

void TC0110PCR_1_vh_start(running_machine *machine)
{
	TC0110PCR_ram[1] = auto_alloc_array(machine, UINT16, TC0110PCR_RAM_SIZE);

	state_save_register_global_pointer(machine, TC0110PCR_ram[1], TC0110PCR_RAM_SIZE);
	state_save_register_postload(machine, TC0110PCR_restore_colors, (void *)1);
}

void TC0110PCR_2_vh_start(running_machine *machine)
{
	TC0110PCR_ram[2] = auto_alloc_array(machine, UINT16, TC0110PCR_RAM_SIZE);

	state_save_register_global_pointer(machine, TC0110PCR_ram[2], TC0110PCR_RAM_SIZE);
	state_save_register_postload(machine, TC0110PCR_restore_colors, (void *)2);
}

READ16_HANDLER( TC0110PCR_word_r )
{
	switch (offset)
	{
		case 1:
			return TC0110PCR_ram[0][(TC0110PCR_addr[0])];

		default:
logerror("PC %06x: warning - read TC0110PCR address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

READ16_HANDLER( TC0110PCR_word_1_r )
{
	switch (offset)
	{
		case 1:
			return TC0110PCR_ram[1][(TC0110PCR_addr[1])];

		default:
logerror("PC %06x: warning - read second TC0110PCR address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

READ16_HANDLER( TC0110PCR_word_2_r )
{
	switch (offset)
	{
		case 1:
			return TC0110PCR_ram[2][(TC0110PCR_addr[2])];

		default:
logerror("PC %06x: warning - read third TC0110PCR address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE16_HANDLER( TC0110PCR_word_w )
{
	switch (offset)
	{
		case 0:
			/* In test mode game writes to odd register number so (data>>1) */
			TC0110PCR_addr[0] = (data >> 1) & 0xfff;
			if (data>0x1fff) logerror ("Write to palette index > 0x1fff\n");
			break;

		case 1:
		{
			TC0110PCR_ram[0][(TC0110PCR_addr[0])] = data & 0xffff;
			palette_set_color_rgb(space->machine,TC0110PCR_addr[0],pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
			break;
		}

		default:
logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

WRITE16_HANDLER( TC0110PCR_step1_word_w )
{
	switch (offset)
	{
		case 0:
			TC0110PCR_addr[0] = data & 0xfff;
			if (data>0xfff) logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
		{
			TC0110PCR_ram[0][(TC0110PCR_addr[0])] = data & 0xffff;
			palette_set_color_rgb(space->machine,TC0110PCR_addr[0],pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
			break;
		}

		default:
logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

WRITE16_HANDLER( TC0110PCR_step1_word_1_w )
{
	switch (offset)
	{
		case 0:
			TC0110PCR_addr[1] = data & 0xfff;
			if (data>0xfff) logerror ("Write to second TC0110PCR palette index > 0xfff\n");
			break;

		case 1:
		{
			TC0110PCR_ram[1][(TC0110PCR_addr[1])] = data & 0xffff;
			/* change a color in the second color area (4096-8191) */
			palette_set_color_rgb(space->machine,TC0110PCR_addr[1] + 4096,pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
			break;
		}

		default:
logerror("PC %06x: warning - write %04x to second TC0110PCR offset %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

WRITE16_HANDLER( TC0110PCR_step1_word_2_w )
{
	switch (offset)
	{
		case 0:
			TC0110PCR_addr[2] = data & 0xfff;
			if (data>0xfff) logerror ("Write to third TC0110PCR palette index > 0xfff\n");
			break;

		case 1:
		{
			TC0110PCR_ram[2][(TC0110PCR_addr[2])] = data & 0xffff;
			/* change a color in the second color area (8192-12288) */
			palette_set_color_rgb(space->machine,TC0110PCR_addr[2] + 8192,pal5bit(data >> 0),pal5bit(data >> 5),pal5bit(data >> 10));
			break;
		}

		default:
logerror("PC %06x: warning - write %04x to third TC0110PCR offset %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

WRITE16_HANDLER( TC0110PCR_step1_rbswap_word_w )
{
	TC0110PCR_type = 1;	/* xRRRRRGGGGGBBBBB */

	switch (offset)
	{
		case 0:
			TC0110PCR_addr[0] = data & 0xfff;
			if (data>0xfff) logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
		{
			TC0110PCR_ram[0][(TC0110PCR_addr[0])] = data & 0xffff;
			palette_set_color_rgb(space->machine,TC0110PCR_addr[0],pal5bit(data >> 10),pal5bit(data >> 5),pal5bit(data >> 0));
			break;
		}

		default:
logerror("PC %06x: warning - write %04x to TC0110PCR offset %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

WRITE16_HANDLER( TC0110PCR_step1_4bpg_word_w )
{
	TC0110PCR_type = 2;	/* xxxxBBBBGGGGRRRR */

	switch (offset)
	{
		case 0:
			TC0110PCR_addr[0] = data & 0xfff;
			if (data>0xfff) logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
		{
			TC0110PCR_ram[0][(TC0110PCR_addr[0])] = data & 0xffff;
			palette_set_color_rgb(space->machine,TC0110PCR_addr[0],pal4bit(data >> 0),pal4bit(data >> 4),pal4bit(data >> 8));
			break;
		}

		default:
logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

/***************************************************************************/


static UINT8 TC0220IOC_regs[8];
static UINT8 TC0220IOC_port;

READ8_HANDLER( TC0220IOC_r )
{
	switch (offset)
	{
		case 0x00:	/* IN00-07 (DSA) */
			return input_port_read(space->machine, "DSWA");

		case 0x01:	/* IN08-15 (DSB) */
			return input_port_read(space->machine, "DSWB");

		case 0x02:	/* IN16-23 (1P) */
			return input_port_read(space->machine, "IN0");

		case 0x03:	/* IN24-31 (2P) */
			return input_port_read(space->machine, "IN1");

		case 0x04:	/* coin counters and lockout */
			return TC0220IOC_regs[4];

		case 0x07:	/* INB0-7 (coin) */
			return input_port_read(space->machine, "IN2");

		default:
logerror("PC %06x: warning - read TC0220IOC address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE8_HANDLER( TC0220IOC_w )
{
	TC0220IOC_regs[offset] = data;

	switch (offset)
	{
		case 0x00:
			watchdog_reset(space->machine);
			break;

		case 0x04:	/* coin counters and lockout, hi nibble irrelevant */
			coin_lockout_w(0,~data & 0x01);
			coin_lockout_w(1,~data & 0x02);
			coin_counter_w(0,data & 0x04);
			coin_counter_w(1,data & 0x08);

//if (data &0xf0)
//logerror("PC %06x: warning - write %02x to TC0220IOC address %02x\n",cpu_get_pc(space->cpu),data,offset);

			break;

		default:
logerror("PC %06x: warning - write %02x to TC0220IOC address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

READ8_HANDLER( TC0220IOC_port_r )
{
	return TC0220IOC_port;
}

WRITE8_HANDLER( TC0220IOC_port_w )
{
	TC0220IOC_port = data;
}

READ8_HANDLER( TC0220IOC_portreg_r )
{
	return TC0220IOC_r(space, TC0220IOC_port);
}

WRITE8_HANDLER( TC0220IOC_portreg_w )
{
	TC0220IOC_w(space, TC0220IOC_port, data);
}


/***************************************************************************/


static UINT8 TC0510NIO_regs[8];

READ8_HANDLER( TC0510NIO_r )
{
	switch (offset)
	{
		case 0x00:	/* DSA */
			return input_port_read(space->machine, "DSWA");

		case 0x01:	/* DSB */
			return input_port_read(space->machine, "DSWB");

		case 0x02:	/* 1P */
			return input_port_read(space->machine, "IN0");

		case 0x03:	/* 2P */
			return input_port_read(space->machine, "IN1");

		case 0x04:	/* coin counters and lockout */
			return TC0510NIO_regs[4];

		case 0x07:	/* coin */
			return input_port_read(space->machine, "IN2");

		default:
logerror("PC %06x: warning - read TC0510NIO address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE8_HANDLER( TC0510NIO_w )
{
	TC0510NIO_regs[offset] = data;

	switch (offset)
	{
		case 0x00:
			watchdog_reset(space->machine);
			break;

		case 0x04:	/* coin counters and lockout */
			coin_lockout_w(0,~data & 0x01);
			coin_lockout_w(1,~data & 0x02);
			coin_counter_w(0,data & 0x04);
			coin_counter_w(1,data & 0x08);
			break;

		default:
logerror("PC %06x: warning - write %02x to TC0510NIO address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

READ16_HANDLER( TC0510NIO_halfword_r )
{
	return TC0510NIO_r(space,offset);
}

WRITE16_HANDLER( TC0510NIO_halfword_w )
{
	if (ACCESSING_BITS_0_7)
		TC0510NIO_w(space,offset,data & 0xff);
	else
	{
		/* driftout writes the coin counters here - bug? */
logerror("CPU #0 PC %06x: warning - write to MSB of TC0510NIO address %02x\n",cpu_get_pc(space->cpu),offset);
		TC0510NIO_w(space,offset,(data >> 8) & 0xff);
	}
}

READ16_HANDLER( TC0510NIO_halfword_wordswap_r )
{
	return TC0510NIO_halfword_r(space,offset ^ 1,mem_mask);
}

WRITE16_HANDLER( TC0510NIO_halfword_wordswap_w )
{
	TC0510NIO_halfword_w(space,offset ^ 1,data,mem_mask);
}


/***************************************************************************/

static UINT8 TC0640FIO_regs[8];

READ8_HANDLER( TC0640FIO_r )
{
	switch (offset)
	{
		case 0x00:	/* DSA */
			return input_port_read(space->machine, "DSWA");

		case 0x01:	/* DSB */
			return input_port_read(space->machine, "DSWB");

		case 0x02:	/* 1P */
			return input_port_read(space->machine, "IN0");

		case 0x03:	/* 2P */
			return input_port_read(space->machine, "IN1");

		case 0x04:	/* coin counters and lockout */
			return TC0640FIO_regs[4];

		case 0x07:	/* coin */
			return input_port_read(space->machine, "IN2");

		default:
logerror("PC %06x: warning - read TC0640FIO address %02x\n",cpu_get_pc(space->cpu),offset);
			return 0xff;
	}
}

WRITE8_HANDLER( TC0640FIO_w )
{
	TC0640FIO_regs[offset] = data;

	switch (offset)
	{
		case 0x00:
			watchdog_reset(space->machine);
			break;

		case 0x04:	/* coin counters and lockout */
			coin_lockout_w(0,~data & 0x01);
			coin_lockout_w(1,~data & 0x02);
			coin_counter_w(0,data & 0x04);
			coin_counter_w(1,data & 0x08);
			break;

		default:
logerror("PC %06x: warning - write %02x to TC0640FIO address %02x\n",cpu_get_pc(space->cpu),data,offset);
			break;
	}
}

READ16_HANDLER( TC0640FIO_halfword_r )
{
	return TC0640FIO_r(space,offset);
}

WRITE16_HANDLER( TC0640FIO_halfword_w )
{
	if (ACCESSING_BITS_0_7)
		TC0640FIO_w(space,offset,data & 0xff);
	else
	{
		TC0640FIO_w(space,offset,(data >> 8) & 0xff);
logerror("CPU #0 PC %06x: warning - write to MSB of TC0640FIO address %02x\n",cpu_get_pc(space->cpu),offset);
	}
}

READ16_HANDLER( TC0640FIO_halfword_byteswap_r )
{
	return TC0640FIO_halfword_r(space,offset,mem_mask) << 8;
}

WRITE16_HANDLER( TC0640FIO_halfword_byteswap_w )
{
	if (ACCESSING_BITS_8_15)
		TC0640FIO_w(space,offset,(data >> 8) & 0xff);
	else
	{
		TC0640FIO_w(space,offset,data & 0xff);
logerror("CPU #0 PC %06x: warning - write to LSB of TC0640FIO address %02x\n",cpu_get_pc(space->cpu),offset);
	}
}


