/*******************************************************************************

Taito Custom ICs
================

Thanks to Suzuki2go for his videos of Metal Black which made better
emulation of TC0480SCP row and column effects possible.

TODO
----

Tidy ups of new transparency-related code in tc0080vco and TC0480SCP.
Merge bryan_draw_scanline with its F3 equivalent?

tc0080vco seems bugged, various sigsevs after games have been reset.

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



pc090oj
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



tc0080vco
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
start of the tc0080vco address space - not the start of chain ram.
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


***************************************************************************/

#include "emu.h"
#include "drawgfxm.h"
#include "taitoic.h"

#define TOPSPEED_ROAD_COLORS


/* These scanline drawing routines lifted from Taito F3: optimise / merge ? */

INLINE void taitoic_drawscanline( bitmap_ind16 &bitmap, const rectangle &cliprect, int x, int y,
		const UINT16 *src, int transparent, UINT32 orient, bitmap_ind8 &priority, int pri)
{
	UINT16 *dsti = &bitmap.pix16(y, x);
	UINT8 *dstp = &priority.pix8(y, x);
	int length = cliprect.width();

	src += cliprect.min_x;
	dsti += cliprect.min_x;
	dstp += cliprect.min_x;
	if (transparent)
	{
		while (length--)
		{
			UINT32 spixel = *src++;

			if (spixel < 0x7fff)
			{
				*dsti = spixel;
				*dstp = pri;
			}

			dsti++;
			dstp++;
		}
	}
	else	/* Not transparent case */
	{
		while (length--)
		{
			*dsti++ = *src++;
			*dstp++ = pri;
		}
	}
}


/***************************************************************************/
/*                                                                         */
/*                                 pc080sn                                 */
/*                                                                         */
/***************************************************************************/

typedef struct _pc080sn_state pc080sn_state;
struct _pc080sn_state
{
	UINT16         ctrl[8];

	UINT16 *       ram;
	UINT16 *       bg_ram[2];
	UINT16 *       bgscroll_ram[2];

	int            bgscrollx[2], bgscrolly[2];
	int            xoffs, yoffs;

	tilemap_t        *tilemap[2];
	int            bg_gfx;
	int            yinvert, dblwidth;
};

#define PC080SN_RAM_SIZE 0x10000

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE pc080sn_state *pc080sn_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == PC080SN);

	return (pc080sn_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const pc080sn_interface *pc080sn_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == PC080SN));
	return (const pc080sn_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

INLINE void common_get_pc080sn_bg_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	UINT16 code, attr;

	if (!pc080sn->dblwidth)
	{
		code = (ram[2 * tile_index + 1] & 0x3fff);
		attr = ram[2 * tile_index];
	}
	else
	{
		code = (ram[tile_index + 0x2000] & 0x3fff);
		attr = ram[tile_index];
	}

	SET_TILE_INFO_DEVICE(
			gfxnum,
			code,
			(attr & 0x1ff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_pc080sn_fg_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	UINT16 code,attr;

	if (!pc080sn->dblwidth)
	{
		code = (ram[2 * tile_index + 1] & 0x3fff);
		attr = ram[2 * tile_index];
	}
	else
	{
		code = (ram[tile_index + 0x2000] & 0x3fff);
		attr = ram[tile_index];
	}

	SET_TILE_INFO_DEVICE(
			gfxnum,
			code,
			(attr & 0x1ff),
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO_DEVICE( pc080sn_get_bg_tile_info )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	common_get_pc080sn_bg_tile_info(device, tileinfo, tile_index, pc080sn->bg_ram[0], pc080sn->bg_gfx);
}

static TILE_GET_INFO_DEVICE( pc080sn_get_fg_tile_info )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	common_get_pc080sn_fg_tile_info(device, tileinfo, tile_index, pc080sn->bg_ram[1], pc080sn->bg_gfx);
}


READ16_DEVICE_HANDLER( pc080sn_word_r )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	return pc080sn->ram[offset];
}

WRITE16_DEVICE_HANDLER( pc080sn_word_w )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);

	COMBINE_DATA(&pc080sn->ram[offset]);

	if (!pc080sn->dblwidth)
	{
		if (offset < 0x2000)
			pc080sn->tilemap[0]->mark_tile_dirty(offset / 2);
		else if (offset >= 0x4000 && offset < 0x6000)
			pc080sn->tilemap[1]->mark_tile_dirty((offset & 0x1fff) / 2);
	}
	else
	{
		if (offset < 0x4000)
			pc080sn->tilemap[0]->mark_tile_dirty((offset & 0x1fff));
		else if (offset >= 0x4000 && offset < 0x8000)
			pc080sn->tilemap[1]->mark_tile_dirty((offset & 0x1fff));
	}
}

WRITE16_DEVICE_HANDLER( pc080sn_xscroll_word_w )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);

	COMBINE_DATA(&pc080sn->ctrl[offset]);

	data = pc080sn->ctrl[offset];

	switch (offset)
	{
		case 0x00:
			pc080sn->bgscrollx[0] = -data;
			break;

		case 0x01:
			pc080sn->bgscrollx[1] = -data;
			break;
	}
}

WRITE16_DEVICE_HANDLER( pc080sn_yscroll_word_w )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);

	COMBINE_DATA(&pc080sn->ctrl[offset + 2]);

	data = pc080sn->ctrl[offset + 2];

	if (pc080sn->yinvert)
		data = -data;

	switch (offset)
	{
		case 0x00:
			pc080sn->bgscrolly[0] = -data;
			break;

		case 0x01:
			pc080sn->bgscrolly[1] = -data;
			break;
	}
}

WRITE16_DEVICE_HANDLER( pc080sn_ctrl_word_w )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);

	COMBINE_DATA(&pc080sn->ctrl[offset + 4]);

	data = pc080sn->ctrl[offset + 4];

	switch (offset)
	{
		case 0x00:
		{
			int flip = (data & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

			pc080sn->tilemap[0]->set_flip(flip);
			pc080sn->tilemap[1]->set_flip(flip);
			break;
		}
	}
#if 0
	popmessage("pc080sn ctrl = %4x", data);
#endif
}


/* This routine is needed as an override by Jumping, which
   doesn't set proper scroll values for foreground tilemap */

void pc080sn_set_scroll( device_t *device, int tilemap_num, int scrollx, int scrolly )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);

	pc080sn->tilemap[tilemap_num]->set_scrollx(0, scrollx);
	pc080sn->tilemap[tilemap_num]->set_scrolly(0, scrolly);
}

/* This routine is needed as an override by Jumping */

void pc080sn_set_trans_pen( device_t *device, int tilemap_num, int pen )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	pc080sn->tilemap[tilemap_num]->set_transparent_pen(pen);
}


void pc080sn_tilemap_update( device_t *device )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	int j;

	pc080sn->tilemap[0]->set_scrolly(0, pc080sn->bgscrolly[0]);
	pc080sn->tilemap[1]->set_scrolly(0, pc080sn->bgscrolly[1]);

	if (!pc080sn->dblwidth)
	{
		for (j = 0; j < 256; j++)
			pc080sn->tilemap[0]->set_scrollx((j + pc080sn->bgscrolly[0]) & 0x1ff,	pc080sn->bgscrollx[0] - pc080sn->bgscroll_ram[0][j]);

		for (j = 0; j < 256; j++)
			pc080sn->tilemap[1]->set_scrollx((j + pc080sn->bgscrolly[1]) & 0x1ff, pc080sn->bgscrollx[1] - pc080sn->bgscroll_ram[1][j]);
	}
	else
	{
		pc080sn->tilemap[0]->set_scrollx(0, pc080sn->bgscrollx[0]);
		pc080sn->tilemap[1]->set_scrollx(0, pc080sn->bgscrollx[1]);
	}
}


static UINT16 topspeed_get_road_pixel_color( UINT16 pixel, UINT16 color )
{
	UINT16 road_body_color, off_road_color, pixel_type;

	/* Color changes based on screenshots from game flyer */
	pixel_type = (pixel % 0x10);
	road_body_color = (pixel & 0x7ff0) + 4;
	off_road_color = road_body_color + 1;

	if ((color & 0xffe0) == 0xffe0)
	{
		pixel += 10;	/* Tunnel colors */
		road_body_color += 10;
		off_road_color  += 10;
	}
	else
	{
		/* Unsure which way round these bits go */
		if (color & 0x10)	road_body_color += 5;
		if (color & 0x02)	off_road_color  += 5;
	}

	switch (pixel_type)
	{
	case 0x01:		/* Center lines */
		if (color & 0x08)
			pixel = road_body_color;
		break;
	case 0x02:		/* Road edge (inner) */
		if (color & 0x08)
			pixel = road_body_color;
		break;
	case 0x03:		/* Road edge (outer) */
		if (color & 0x04)
			pixel = road_body_color;
		break;
	case 0x04:		/* Road body */
		pixel = road_body_color;
		break;
	case 0x05:		/* Off road */
		pixel = off_road_color;
		break;
	default:
		{}
	}
	return pixel;
}


static void topspeed_custom_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags,
							UINT32 priority, UINT16 *color_ctrl_ram )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	UINT16 *dst16, *src16;
	UINT8 *tsrc;
	UINT16 scanline[1024];	/* won't be called by a wide-screen game, but just in case... */

	bitmap_ind16 &srcbitmap = pc080sn->tilemap[layer]->pixmap();
	bitmap_ind8 &flagsbitmap = pc080sn->tilemap[layer]->flagsmap();

	UINT16 a, color;
	int sx, x_index;
	int i, y, y_index, src_y_index, row_index;

	int flip = 0;
	int machine_flip = 0;	/* for  ROT 180 ? */

	int min_x = cliprect.min_x;
	int max_x = cliprect.max_x;
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	int screen_width = max_x - min_x + 1;
	int width_mask = 0x1ff;	/* underlying tilemap */

	if (!flip)
	{
		sx = pc080sn->bgscrollx[layer] + 16 - pc080sn->xoffs;
		y_index = pc080sn->bgscrolly[layer] + min_y - pc080sn->yoffs;
	}
	else	// never used
	{
		sx = 0;
		y_index = 0;
	}

	if (!machine_flip)
		y = min_y;
	else
		y = max_y;

	do
	{
		src_y_index = y_index & 0x1ff;	/* tilemaps are 512 px up/down */
		row_index = (src_y_index - pc080sn->bgscrolly[layer]) & 0x1ff;
		color = color_ctrl_ram[(row_index + pc080sn->yoffs - 2) & 0xff];

		x_index = sx - (pc080sn->bgscroll_ram[layer][row_index]);

		src16 = &srcbitmap.pix16(src_y_index);
		tsrc  = &flagsbitmap.pix8(src_y_index);
		dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (i = 0; i < screen_width; i++)
			{
				a = src16[x_index & width_mask];
#ifdef TOPSPEED_ROAD_COLORS
				a = topspeed_get_road_pixel_color(a, color);
#endif
				*dst16++ = a;
				x_index++;
			}
		}
		else
		{
			for (i = 0; i < screen_width; i++)
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

		taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1, ROT0, device->machine().priority_bitmap, priority);
		y_index++;

		if (!machine_flip)
			y++;
		else
			y--;
	}
	while ((!machine_flip && y <= max_y) || (machine_flip && y >= min_y));
}

void pc080sn_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	pc080sn->tilemap[layer]->draw(bitmap, cliprect, flags, priority);
}

void pc080sn_tilemap_draw_offset( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, int xoffs, int yoffs )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	int basedx = -16 - pc080sn->xoffs;
	int basedxflip = -16 + pc080sn->xoffs;
	int basedy = pc080sn->yoffs;
	int basedyflip = -pc080sn->yoffs;

	pc080sn->tilemap[layer]->set_scrolldx(basedx + xoffs, basedxflip + xoffs);
	pc080sn->tilemap[layer]->set_scrolldy(basedy + yoffs, basedyflip + yoffs);
	pc080sn->tilemap[layer]->draw(bitmap, cliprect, flags, priority);
	pc080sn->tilemap[layer]->set_scrolldx(basedx, basedxflip);
	pc080sn->tilemap[layer]->set_scrolldy(basedy, basedyflip);
}

void pc080sn_tilemap_draw_special( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority, UINT16 *ram )
{
	topspeed_custom_draw(device, bitmap, cliprect, layer, flags, priority, ram);
}


static void pc080sn_restore_scroll(pc080sn_state *pc080sn)
{
	int flip;

	pc080sn->bgscrollx[0] = -pc080sn->ctrl[0];
	pc080sn->bgscrollx[1] = -pc080sn->ctrl[1];
	pc080sn->bgscrolly[0] = -pc080sn->ctrl[2];
	pc080sn->bgscrolly[1] = -pc080sn->ctrl[3];

	flip = (pc080sn->ctrl[4] & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	pc080sn->tilemap[0]->set_flip(flip);
	pc080sn->tilemap[1]->set_flip(flip);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( pc080sn )
{
	pc080sn_state *pc080sn = pc080sn_get_safe_token(device);
	const pc080sn_interface *intf = pc080sn_get_interface(device);

	/* use the given gfx set for bg tiles */
	pc080sn->bg_gfx = intf->gfxnum;

	pc080sn->yinvert = intf->y_invert;
	pc080sn->dblwidth = intf->dblwidth;
	pc080sn->xoffs = intf->x_offset;
	pc080sn->yoffs = intf->y_offset;

	if (!pc080sn->dblwidth)	/* standard tilemaps */
	{
		pc080sn->tilemap[0] = tilemap_create_device(device, pc080sn_get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
		pc080sn->tilemap[1] = tilemap_create_device(device, pc080sn_get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	}
	else	/* double width tilemaps */
	{
		pc080sn->tilemap[0] = tilemap_create_device(device, pc080sn_get_bg_tile_info, tilemap_scan_rows, 8, 8, 128, 64);
		pc080sn->tilemap[1] = tilemap_create_device(device, pc080sn_get_fg_tile_info, tilemap_scan_rows, 8, 8, 128, 64);
	}

	pc080sn->tilemap[0]->set_transparent_pen(0);
	pc080sn->tilemap[1]->set_transparent_pen(0);

	pc080sn->tilemap[0]->set_scrolldx(-16 + pc080sn->xoffs, -16 - pc080sn->xoffs);
	pc080sn->tilemap[0]->set_scrolldy(pc080sn->yoffs, -pc080sn->yoffs);
	pc080sn->tilemap[1]->set_scrolldx(-16 + pc080sn->xoffs, -16 - pc080sn->xoffs);
	pc080sn->tilemap[1]->set_scrolldy(pc080sn->yoffs, -pc080sn->yoffs);

	if (!pc080sn->dblwidth)
	{
		pc080sn->tilemap[0]->set_scroll_rows(512);
		pc080sn->tilemap[1]->set_scroll_rows(512);
	}

	pc080sn->ram = auto_alloc_array_clear(device->machine(), UINT16, PC080SN_RAM_SIZE / 2);

	pc080sn->bg_ram[0]       = pc080sn->ram + 0x0000 /2;
	pc080sn->bg_ram[1]       = pc080sn->ram + 0x8000 /2;
	pc080sn->bgscroll_ram[0] = pc080sn->ram + 0x4000 /2;
	pc080sn->bgscroll_ram[1] = pc080sn->ram + 0xc000 /2;

	device->save_pointer(NAME(pc080sn->ram), PC080SN_RAM_SIZE / 2);
	device->save_item(NAME(pc080sn->ctrl));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(pc080sn_restore_scroll), pc080sn));
}


/***************************************************************************/
/*                                                                         */
/*                                 PC090OJ                                 */
/*                                                                         */
/***************************************************************************/

typedef struct _pc090oj_state pc090oj_state;
struct _pc090oj_state
{
/* NB: pc090oj_ctrl is the internal register controlling flipping

   pc090oj_sprite_ctrl is a representation of the hardware OUTSIDE the pc090oj
   which impacts on sprite plotting, and which varies between games. It
   includes color banking and (optionally) priority. It allows each game to
   control these aspects of the sprites in different ways, while keeping the
   routines here modular.

*/

	UINT16     ctrl, buffer, gfxnum;
	UINT16     sprite_ctrl;

	UINT16 *   ram;
	UINT16 *   ram_buffered;

	int        xoffs, yoffs;
};

#define PC090OJ_RAM_SIZE 0x4000
#define PC090OJ_ACTIVE_RAM_SIZE 0x800

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE pc090oj_state *pc090oj_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == PC090OJ);

	return (pc090oj_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const pc090oj_interface *pc090oj_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == PC090OJ));
	return (const pc090oj_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void pc090oj_set_sprite_ctrl( device_t *device, UINT16 sprctrl )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);
	pc090oj->sprite_ctrl = sprctrl;
}

READ16_DEVICE_HANDLER( pc090oj_word_r )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);
	return pc090oj->ram[offset];
}

WRITE16_DEVICE_HANDLER( pc090oj_word_w )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);
	COMBINE_DATA(&pc090oj->ram[offset]);

	/* If we're not buffering sprite ram, write it straight through... */
	if (!pc090oj->buffer)
		pc090oj->ram_buffered[offset] = pc090oj->ram[offset];

	if (offset == 0xdff)
	{
		/* Bit 0 is flip control, others seem unused */
		pc090oj->ctrl = data;

#if 0
	popmessage("pc090oj ctrl = %4x", data);
#endif
	}
}

void pc090oj_eof_callback( device_t *device )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);
	if (pc090oj->buffer)
	{
		int i;
		for (i = 0; i < PC090OJ_ACTIVE_RAM_SIZE / 2; i++)
			pc090oj->ram_buffered[i] = pc090oj->ram[i];
	}
}


void pc090oj_draw_sprites( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int pri_type )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);
	int offs, priority = 0;
	int sprite_colbank = (pc090oj->sprite_ctrl & 0xf) << 4;	/* top nibble */

	switch (pri_type)
	{
		case 0x00:
			priority = 0;	/* sprites over top bg layer */
			break;

		case 0x01:
			priority = 1;	/* sprites under top bg layer */
			break;

		case 0x02:
			priority = pc090oj->sprite_ctrl >> 15;	/* variable sprite/tile priority */
	}

	for (offs = 0; offs < PC090OJ_ACTIVE_RAM_SIZE / 2; offs += 4)
	{
		int flipx, flipy;
		int x, y;
		int data, code, color;

		data = pc090oj->ram_buffered[offs];
		flipy = (data & 0x8000) >> 15;
		flipx = (data & 0x4000) >> 14;
		color = (data & 0x000f) | sprite_colbank;

		code = pc090oj->ram_buffered[offs + 2] & 0x1fff;
		x = pc090oj->ram_buffered[offs + 3] & 0x1ff;   /* mask verified with Rainbowe board */
		y = pc090oj->ram_buffered[offs + 1] & 0x1ff;   /* mask verified with Rainbowe board */

		/* treat coords as signed */
		if (x > 0x140) x -= 0x200;
		if (y > 0x140) y -= 0x200;

		if (!(pc090oj->ctrl & 1))	/* sprites flipscreen */
		{
			x = 320 - x - 16;
			y = 256 - y - 16;
			flipx = !flipx;
			flipy = !flipy;
		}

		x += pc090oj->xoffs;
		y += pc090oj->yoffs;

		pdrawgfx_transpen(bitmap,cliprect,device->machine().gfx[pc090oj->gfxnum],
				code,
				color,
				flipx,flipy,
				x,y,
				device->machine().priority_bitmap,
				priority ? 0xfc : 0xf0,0);
	}
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( pc090oj )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);
	const pc090oj_interface *intf = pc090oj_get_interface(device);

	/* use the given gfx set */
	pc090oj->gfxnum = intf->gfxnum;

	pc090oj->xoffs = intf->x_offset;
	pc090oj->yoffs = intf->y_offset;

	pc090oj->buffer = intf->use_buffer;


	pc090oj->ram = auto_alloc_array_clear(device->machine(), UINT16, PC090OJ_RAM_SIZE / 2);
	pc090oj->ram_buffered = auto_alloc_array_clear(device->machine(), UINT16, PC090OJ_RAM_SIZE / 2);

	device->save_pointer(NAME(pc090oj->ram), PC090OJ_RAM_SIZE / 2);
	device->save_pointer(NAME(pc090oj->ram_buffered), PC090OJ_RAM_SIZE / 2);
	device->save_item(NAME(pc090oj->ctrl));
	device->save_item(NAME(pc090oj->sprite_ctrl));	// should this be set in intf?!?
}

static DEVICE_RESET( pc090oj )
{
	pc090oj_state *pc090oj = pc090oj_get_safe_token(device);

	pc090oj->ctrl = 0;
}


/***************************************************************************/
/*                                                                         */
/*                            TC0080VCO                                    */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0080vco_state tc0080vco_state;
struct _tc0080vco_state
{
	UINT16 *       ram;
	UINT16 *       bg0_ram_0;
	UINT16 *       bg0_ram_1;
	UINT16 *       bg1_ram_0;
	UINT16 *       bg1_ram_1;
	UINT16 *       tx_ram_0;
	UINT16 *       tx_ram_1;
	UINT16 *       char_ram;
	UINT16 *       bgscroll_ram;

/* FIXME: This sprite related stuff still needs to be accessed in
   video/taito_h */
	UINT16 *       chain_ram_0;
	UINT16 *       chain_ram_1;
	UINT16 *       spriteram;
	UINT16 *       scroll_ram;

	UINT16         bg0_scrollx;
	UINT16         bg0_scrolly;
	UINT16         bg1_scrollx;
	UINT16         bg1_scrolly;

	tilemap_t        *tilemap[3];

	int            bg_gfx, tx_gfx;
	int            bg_xoffs, bg_yoffs;
	int            bg_flip_yoffs;

	INT32          flipscreen;
	int            has_tx;
};

#define TC0080VCO_RAM_SIZE 0x21000
#define TC0080VCO_CHAR_RAM_SIZE 0x2000
#define TC0080VCO_TOTAL_CHARS 256

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0080vco_state *tc0080vco_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0080VCO);

	return (tc0080vco_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0080vco_interface *tc0080vco_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0080VCO));
	return (const tc0080vco_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

#if 0
static const int tc0080vco_zoomy_conv_table[] =
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


static TILE_GET_INFO_DEVICE( tc0080vco_get_bg0_tile_info )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	int color, tile;

	color = tc0080vco->bg0_ram_1[tile_index] & 0x001f;
	tile  = tc0080vco->bg0_ram_0[tile_index] & 0x7fff;

	tileinfo.category = 0;

	SET_TILE_INFO_DEVICE(
			tc0080vco->bg_gfx,
			tile,
			color,
			TILE_FLIPYX((tc0080vco->bg0_ram_1[tile_index] & 0x00c0) >> 6));
}

static TILE_GET_INFO_DEVICE( tc0080vco_get_bg1_tile_info )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	int color, tile;

	color = tc0080vco->bg1_ram_1[tile_index] & 0x001f;
	tile  = tc0080vco->bg1_ram_0[tile_index] & 0x7fff;

	tileinfo.category = 0;

	SET_TILE_INFO_DEVICE(
			tc0080vco->bg_gfx,
			tile,
			color,
			TILE_FLIPYX((tc0080vco->bg1_ram_1[tile_index] & 0x00c0) >> 6));
}

static TILE_GET_INFO_DEVICE( tc0080vco_get_tx_tile_info )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	int tile;

	if (!tc0080vco->flipscreen)
	{
		if ((tile_index & 1))
			tile = (tc0080vco->tx_ram_0[tile_index >> 1] & 0x00ff);
		else
			tile = (tc0080vco->tx_ram_0[tile_index >> 1] & 0xff00) >> 8;
		tileinfo.category = 0;
	}
	else
	{
		if ((tile_index & 1))
			tile = (tc0080vco->tx_ram_0[tile_index >> 1] & 0xff00) >> 8;
		else
			tile = (tc0080vco->tx_ram_0[tile_index >> 1] & 0x00ff);
		tileinfo.category = 0;
	}

	SET_TILE_INFO_DEVICE(
			tc0080vco->tx_gfx,
			tile,
			0x40,
			0);		/* 0x20<<1 as 3bpp */
}


/* Is this endian-correct ??? */

#define XOR(a) WORD_XOR_BE(a)

static const gfx_layout tc0080vco_charlayout =
{
	8, 8,	/* 8x8 pixels */
	256,	/* 256 chars */
	3,		/* 3 bits per pixel */
	{ 0x10000*8 + XOR(2)*4, XOR(0)*4, XOR(2)*4 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 16*0, 16*1, 16*2, 16*3, 16*4, 16*5, 16*6, 16*7 },
	16*8
};

static WRITE16_DEVICE_HANDLER( tc0080vco_scrollram_w )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);

	switch (offset)
	{
		case 0x00:			/* screen invert control */
			tc0080vco->flipscreen = tc0080vco->scroll_ram[0] & 0x0c00;

			tc0080vco->tilemap[0]->set_flip(tc0080vco->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
			tc0080vco->tilemap[1]->set_flip(tc0080vco->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
			tc0080vco->tilemap[2]->set_flip(tc0080vco->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

			tc0080vco->bg0_scrollx = tc0080vco->scroll_ram[1] & 0x03ff;
			tc0080vco->bg1_scrollx = tc0080vco->scroll_ram[2] & 0x03ff;
			tc0080vco->bg0_scrolly = tc0080vco->scroll_ram[3] & 0x03ff;
			tc0080vco->bg1_scrolly = tc0080vco->scroll_ram[4] & 0x03ff;
			break;

		case 0x01:			/* BG0 scroll X */
			tc0080vco->bg0_scrollx = data & 0x03ff;
			break;

		case 0x02:			/* BG1 scroll X */
			tc0080vco->bg1_scrollx = data & 0x03ff;
			break;

		case 0x03:			/* BG0 scroll Y */
			tc0080vco->bg0_scrolly = data & 0x03ff;
			break;

		case 0x04:			/* BG1 scroll Y */
			tc0080vco->bg1_scrolly = data & 0x03ff;
			break;

		default:
			break;
	}
}

READ16_DEVICE_HANDLER( tc0080vco_word_r )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	return tc0080vco->ram[offset];
}

WRITE16_DEVICE_HANDLER( tc0080vco_word_w )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);

	COMBINE_DATA(&tc0080vco->ram[offset]);

	/* A lot of tc0080vco writes require no action... */

	if (offset < 0x1000 / 2)
	{
		gfx_element_mark_dirty(device->machine().gfx[tc0080vco->tx_gfx], offset / 8);
#if 0
		if (!tc0080vco->has_tx)
		{
			if (tc0080vco->ram[offset])
			popmessage("Write non-zero to tc0080vco char ram\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0x2000 / 2)	/* fg0 (text layer) */
	{
		tc0080vco->tilemap[2]->mark_tile_dirty((offset & 0x07ff) * 2);
		tc0080vco->tilemap[2]->mark_tile_dirty((offset & 0x07ff) * 2 + 1);
#if 0
		if (!tc0080vco->has_tx)
		{
			if (tc0080vco->ram[offset])
			popmessage("Write non-zero to tc0080vco fg0\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0xc000 / 2)	/* chain ram */
	{}
	else if (offset < 0xe000 / 2)	/* bg0 (0) */
		tc0080vco->tilemap[0]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x10000 / 2)	/* bg1 (0) */
		tc0080vco->tilemap[1]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x11000 / 2)
	{
		gfx_element_mark_dirty(device->machine().gfx[tc0080vco->tx_gfx], (offset - 0x10000 / 2) / 8);
#if 0
		if (!tc0080vco->has_tx)
		{
			if (tc0080vco->ram[offset])
			popmessage("Write non-zero to tc0080vco char-hi ram\nPlease report to MAMEDEV");
		}
#endif
	}
	else if (offset < 0x12000 / 2)	/* unknown/unused */
	{
#if 1
		if (tc0080vco->ram[offset])
		popmessage("Write non-zero to mystery tc0080vco area\nPlease report to MAMEDEV");
#endif
	}
	else if (offset < 0x1c000 / 2)	/* chain ram */
	{}
	else if (offset < 0x1e000 / 2)	/* bg0 (1) */
		tc0080vco->tilemap[0]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x20000 / 2)	/* bg1 (1) */
		tc0080vco->tilemap[1]->mark_tile_dirty((offset & 0xfff));

	else if (offset < 0x20400 / 2)	/* bg0 rowscroll */
	{}
	else if (offset < 0x20800 / 2)	/* sprite ram */
	{}
	else if (offset < 0x20fff / 2)
		tc0080vco_scrollram_w(device, offset - (0x20800 / 2), tc0080vco->ram[offset], mem_mask);
}

void tc0080vco_tilemap_update( device_t *device )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	int j;

	if (!tc0080vco->flipscreen)
	{
		for (j = 0; j < 0x400; j++)
			tc0080vco->tilemap[0]->set_scrollx((j + 0) & 0x3ff, -tc0080vco->bg0_scrollx - tc0080vco->bgscroll_ram[j & 0x1ff]);
	}
	else
	{
		for (j = 0; j < 0x400; j++)
			tc0080vco->tilemap[0]->set_scrollx((j + 0) & 0x3ff, -tc0080vco->bg0_scrollx + tc0080vco->bgscroll_ram[j & 0x1ff]);
	}

	tc0080vco->tilemap[0]->set_scrolly(0,  tc0080vco->bg0_scrolly);
	tc0080vco->tilemap[1]->set_scrollx(0, -tc0080vco->bg1_scrollx);
	tc0080vco->tilemap[1]->set_scrolly(0,  tc0080vco->bg1_scrolly);
	tc0080vco->tilemap[2]->set_scrollx(0, 0);	/* no scroll (maybe) */
	tc0080vco->tilemap[2]->set_scrolly(0, 0);
}


/* NB: orientation_flipx code in following routine has not been tested */

static void tc0080vco_bg0_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	UINT16 zoom = tc0080vco->scroll_ram[6];
	int zx, zy;

	zx = (zoom & 0xff00) >> 8;
	zy = zoom & 0x00ff;

	if (zx == 0x3f && zy == 0x7f)		/* normal size */
	{
		tc0080vco->tilemap[0]->draw(bitmap, cliprect, flags, priority);
	}
	else		/* zoom + rowscroll = custom draw routine */
	{
		UINT16 *dst16, *src16;
		UINT8 *tsrc;
		UINT16 scanline[512];
		bitmap_ind16 &srcbitmap = tc0080vco->tilemap[0]->pixmap();
		bitmap_ind8 &flagsbitmap = tc0080vco->tilemap[0]->flagsmap();

		int sx, zoomx, zoomy;
		int dx, ex, dy, ey;
		int i, y, y_index, src_y_index, row_index;
		int x_index, x_step;

		int flip = tc0080vco->flipscreen;
		int machine_flip = 0;	/* for  ROT 180 ? */

		int min_x = cliprect.min_x;
		int max_x = cliprect.max_x;
		int min_y = cliprect.min_y;
		int max_y = cliprect.max_y;
		int screen_width = max_x + 1;
		int width_mask = 0x3ff;	/* underlying tilemap */


#if 0
{
	char buf[100];
	sprintf(buf, "xmin= %04x xmax= %04x ymin= %04x ymax= %04x", min_x, max_x, min_y, max_y);
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
			sx = (-tc0080vco->scroll_ram[1] - 1) << 16;
			y_index = (( tc0080vco->scroll_ram[3] - 1) << 16) + min_y * zoomy;
		}
		else
		{
			/* adjustment for zx is entirely speculative */
			sx =  (( 0x200 + tc0080vco->scroll_ram[1]) << 16) - (max_x + min_x) * (zoomx - 0x10000);

			/* 0x130 correct for Dleague. Syvalion correct with 0x1f0. min_y is 0x20 and 0x30; max_y is 0x10f and 0x1bf; max_y + min_y seems a good bet... */
			y_index = ((-tc0080vco->scroll_ram[3] - 2) << 16) + min_y * zoomy - (max_y + min_y) * (zoomy - 0x10000);
		}

		if (!machine_flip)
			y = min_y;
		else
			y = max_y;

		do
		{
			src_y_index = (y_index >> 16) & 0x3ff;	/* tilemaps are 1024 px up/down */

			/* row areas are the same in flipscreen, so we must read in reverse */
			row_index = (src_y_index & 0x1ff);
			if (flip)
				row_index = 0x1ff - row_index;

			x_index = sx - ((tc0080vco->bgscroll_ram[row_index] << 16));

			src16 = &srcbitmap.pix16(src_y_index);
			tsrc  = &flagsbitmap.pix8(src_y_index);
			dst16 = scanline;

			x_step = zoomx;

			if (flags & TILEMAP_DRAW_OPAQUE)
			{
				for (i = 0; i < screen_width; i++)
				{
					*dst16++ = src16[(x_index >> 16) & width_mask];
					x_index += x_step;
				}
			}
			else
			{
				for (i = 0; i < screen_width; i++)
				{
					if (tsrc[(x_index >> 16) & width_mask])
						*dst16++ = src16[(x_index >> 16) & width_mask];
					else
						*dst16++ = 0x8000;
					x_index += x_step;
				}
			}

			taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1 , ROT0, device->machine().priority_bitmap, priority);

			y_index += zoomy;

			if (!machine_flip)
				y++;
			else
				y--;
		}
		while ((!machine_flip && y <= max_y) || (machine_flip && y >= min_y));
	}
}


#define PIXEL_OP_COPY_TRANS0_SET_PRIORITY(DEST, PRIORITY, SOURCE)					\
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

static void tc0080vco_bg1_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int flags, UINT32 priority )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	UINT8 layer = 1;
	UINT16 zoom = tc0080vco->scroll_ram[6 + layer];
	int min_x = cliprect.min_x;
	int max_x = cliprect.max_x;
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	int zoomx, zoomy;

	zoomx = (zoom & 0xff00) >> 8;
	zoomy =  zoom & 0x00ff;

	if (zoomx == 0x3f && zoomy == 0x7f)		/* normal size */
	{
		tc0080vco->tilemap[layer]->draw(bitmap, cliprect, flags, priority);
	}
	else		/* zoomed */
	{
		int zx, zy, dx, dy, ex, ey;
		int sx,sy;

		/* shouldn't we set no_clip before doing this (see TC0480SCP) ? */
		bitmap_ind16 &srcbitmap = tc0080vco->tilemap[layer]->pixmap();

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

		if (!tc0080vco->flipscreen)
		{
			sx = (-tc0080vco->scroll_ram[layer + 1] - 1) << 16;
			sy = ( tc0080vco->scroll_ram[layer + 3] - 1) << 16;
		}
		else
		{
			/* adjustment for zx is entirely speculative */
			sx =  (( 0x200 + tc0080vco->scroll_ram[layer + 1]) << 16) - (max_x + min_x) * (zx - 0x10000);
			sy =  (( 0x3fe - tc0080vco->scroll_ram[layer + 3]) << 16) - (max_y + min_y) * (zy - 0x10000);
		}

		{
			bitmap_ind16 &dest = bitmap;
			bitmap_ind16 &src = srcbitmap;
			INT32 startx = sx;
			INT32 starty = sy;
			INT32 incxx = zx;
			INT32 incxy = 0;
			INT32 incyx = 0;
			INT32 incyy = zy;
			int wraparound = 0;
			UINT32 privalue = priority;
			bitmap_ind8 &priority = device->machine().priority_bitmap;

			if (dest.bpp() == 16)
				COPYROZBITMAP_CORE(UINT16, PIXEL_OP_COPY_TRANS0_SET_PRIORITY, UINT8);
			else
				COPYROZBITMAP_CORE(UINT32, PIXEL_OP_COPY_TRANS0_SET_PRIORITY, UINT8);
		}
	}
}


void tc0080vco_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	int disable = 0x00;	/* possibly layer disable bits do exist ?? */

#if 0
	popmessage("layer disable = %x", disable);
#endif

	switch (layer)
	{
		case 0:
			if (disable & 0x01)
				return;
			tc0080vco_bg0_tilemap_draw(device, bitmap, cliprect, flags, priority);
			break;
		case 1:
			if (disable & 0x02)
				return;
			tc0080vco_bg1_tilemap_draw(device, bitmap, cliprect, flags, priority);
			break;
		case 2:
			if (disable & 0x04)
				return;
			tc0080vco->tilemap[2]->draw(bitmap, cliprect, flags, priority);
			break;
	}
}

/* FIXME: maybe it would be better to provide pointers to these RAM regions
which can be accessed directly by the drivers... */
READ16_DEVICE_HANDLER( tc0080vco_cram_0_r )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	return tc0080vco->chain_ram_0[offset];
}

READ16_DEVICE_HANDLER( tc0080vco_cram_1_r )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	return tc0080vco->chain_ram_1[offset];
}

READ16_DEVICE_HANDLER( tc0080vco_sprram_r )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	return tc0080vco->spriteram[offset];
}

READ16_DEVICE_HANDLER( tc0080vco_scrram_r )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	return tc0080vco->scroll_ram[offset];
}

READ_LINE_DEVICE_HANDLER( tc0080vco_flipscreen_r )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	return tc0080vco->flipscreen;
}


static void tc0080vco_postload(tc0080vco_state *tc0080vco)
{
	tc0080vco->flipscreen = tc0080vco->scroll_ram[0] & 0x0c00;

	tc0080vco->tilemap[0]->set_flip(tc0080vco->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tc0080vco->tilemap[1]->set_flip(tc0080vco->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);
	tc0080vco->tilemap[2]->set_flip(tc0080vco->flipscreen ? TILEMAP_FLIPX | TILEMAP_FLIPY : 0);

	tc0080vco->bg0_scrollx = tc0080vco->scroll_ram[1] & 0x03ff;
	tc0080vco->bg1_scrollx = tc0080vco->scroll_ram[2] & 0x03ff;
	tc0080vco->bg0_scrolly = tc0080vco->scroll_ram[3] & 0x03ff;
	tc0080vco->bg1_scrolly = tc0080vco->scroll_ram[4] & 0x03ff;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0080vco )
{
	tc0080vco_state *tc0080vco = tc0080vco_get_safe_token(device);
	const tc0080vco_interface *intf = tc0080vco_get_interface(device);

	/* use the given gfx sets for bg/tx tiles*/
	tc0080vco->bg_gfx = intf->gfxnum;
	tc0080vco->tx_gfx = intf->txnum;

	tc0080vco->bg_xoffs = intf->bg_xoffs;	/* usually 1 */
	tc0080vco->bg_yoffs = intf->bg_yoffs;	/* usually 1 */
	tc0080vco->bg_flip_yoffs = intf->bg_flip_yoffs;	/* usually -2 */
	tc0080vco->has_tx = intf->has_fg0;	/* for debugging only */

	tc0080vco->tilemap[0] = tilemap_create_device(device, tc0080vco_get_bg0_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	tc0080vco->tilemap[1] = tilemap_create_device(device, tc0080vco_get_bg1_tile_info, tilemap_scan_rows, 16, 16, 64, 64);

	tc0080vco->tilemap[0]->set_transparent_pen(0);
	tc0080vco->tilemap[1]->set_transparent_pen(0);

	tc0080vco->tilemap[0]->set_scrolldx(tc0080vco->bg_xoffs, 512);
	tc0080vco->tilemap[1]->set_scrolldx(tc0080vco->bg_xoffs, 512);
	tc0080vco->tilemap[0]->set_scrolldy(tc0080vco->bg_yoffs, tc0080vco->bg_flip_yoffs);
	tc0080vco->tilemap[1]->set_scrolldy(tc0080vco->bg_yoffs, tc0080vco->bg_flip_yoffs);

	/* bg0 tilemap scrollable per pixel row */
	tc0080vco->tilemap[0]->set_scroll_rows(512);

	/* Perform extra initialisations for text layer */
	tc0080vco->tilemap[2] = tilemap_create_device(device, tc0080vco_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	tc0080vco->tilemap[2]->set_scrolldx(0, 0);
	tc0080vco->tilemap[2]->set_scrolldy(48, -448);

	tc0080vco->tilemap[2]->set_transparent_pen(0);

	tc0080vco->ram = auto_alloc_array_clear(device->machine(), UINT16, TC0080VCO_RAM_SIZE / 2);

	tc0080vco->char_ram      = tc0080vco->ram + 0x00000 / 2;	/* continues at +0x10000 */
	tc0080vco->tx_ram_0      = tc0080vco->ram + 0x01000 / 2;
	tc0080vco->chain_ram_0   = tc0080vco->ram + 0x00000 / 2;	/* only used from +0x2000 */

	tc0080vco->bg0_ram_0     = tc0080vco->ram + 0x0c000 / 2;
	tc0080vco->bg1_ram_0     = tc0080vco->ram + 0x0e000 / 2;

	tc0080vco->tx_ram_1      = tc0080vco->ram + 0x11000 / 2;
	tc0080vco->chain_ram_1   = tc0080vco->ram + 0x10000 / 2;	/* only used from +0x12000 */

	tc0080vco->bg0_ram_1     = tc0080vco->ram + 0x1c000 / 2;
	tc0080vco->bg1_ram_1     = tc0080vco->ram + 0x1e000 / 2;
	tc0080vco->bgscroll_ram  = tc0080vco->ram + 0x20000 / 2;
	tc0080vco->spriteram     = tc0080vco->ram + 0x20400 / 2;
	tc0080vco->scroll_ram    = tc0080vco->ram + 0x20800 / 2;

	/* create the char set (gfx will then be updated dynamically from RAM) */
	device->machine().gfx[tc0080vco->tx_gfx] = gfx_element_alloc(device->machine(), &tc0080vco_charlayout, (UINT8 *)tc0080vco->char_ram, 64, 0);

	device->save_pointer(NAME(tc0080vco->ram), TC0080VCO_RAM_SIZE / 2);
	device->machine().save().register_postload(save_prepost_delegate(FUNC(tc0080vco_postload), tc0080vco));
}

/***************************************************************************/
/*                                                                         */
/*                              TC0100SCN                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0100scn_state tc0100scn_state;
struct _tc0100scn_state
{
	UINT16       ctrl[8];

	UINT16 *     ram;
	UINT16 *     bg_ram;
	UINT16 *     fg_ram;
	UINT16 *     tx_ram;
	UINT16 *     char_ram;
	UINT16 *     bgscroll_ram;
	UINT16 *     fgscroll_ram;
	UINT16 *     colscroll_ram;

	int          bgscrollx, bgscrolly, fgscrollx, fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t      *tilemap[3][2];
	rectangle    cliprect;

	int          bg_gfx, tx_gfx;
	int          bg_col_mult, bg_tilemask, tx_col_mult;
	INT32        gfxbank, colbank;
	INT32        bg0_colbank, bg1_colbank, tx_colbank;
	int          dblwidth;

	screen_device *screen;
};

#define TC0100SCN_RAM_SIZE        0x14000	/* enough for double-width tilemaps */
#define TC0100SCN_TOTAL_CHARS     256

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0100scn_state *tc0100scn_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0100SCN);

	return (tc0100scn_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0100scn_interface *tc0100scn_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0100SCN));
	return (const tc0100scn_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

INLINE void common_get_bg0_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum, int colbank, int dblwidth )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	int code, attr;

	if (!dblwidth)
	{
		/* Mahjong Quest (F2 system) inexplicably has a banking feature */
		code = (ram[2 * tile_index + 1] & tc0100scn->bg_tilemask) + (tc0100scn->gfxbank << 15);
		attr = ram[2 * tile_index];
	}
	else
	{
		code = ram[2 * tile_index + 1] & tc0100scn->bg_tilemask;
		attr = ram[2 * tile_index];
	}

	SET_TILE_INFO_DEVICE(
			gfxnum,
			code,
			(((attr * tc0100scn->bg_col_mult) + tc0100scn->bg0_colbank) & 0xff) + colbank,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_bg1_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum, int colbank, int dblwidth )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	int code, attr;

	if (!dblwidth)
	{
		/* Mahjong Quest (F2 system) inexplicably has a banking feature */
		code = (ram[2 * tile_index + 1] & tc0100scn->bg_tilemask) + (tc0100scn->gfxbank << 15);
		attr = ram[2 * tile_index];
	}
	else
	{
		code = ram[2 * tile_index + 1] & tc0100scn->bg_tilemask;
		attr = ram[2 * tile_index];
	}

	SET_TILE_INFO_DEVICE(
			gfxnum,
			code,
			(((attr * tc0100scn->bg_col_mult) + tc0100scn->bg1_colbank) & 0xff) + colbank,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_tx_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum, int colbank, int dblwidth )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	int attr = ram[tile_index];

	SET_TILE_INFO_DEVICE(
			gfxnum,
			attr & 0xff,
			((((attr >> 6) & 0xfc) * tc0100scn->tx_col_mult + (tc0100scn->tx_colbank << 2)) & 0x3ff) + colbank * 4,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO_DEVICE( tc0100scn_get_bg_tile_info )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	common_get_bg0_tile_info(device, tileinfo, tile_index, tc0100scn->bg_ram, tc0100scn->bg_gfx, tc0100scn->colbank, tc0100scn->dblwidth);
}

static TILE_GET_INFO_DEVICE( tc0100scn_get_fg_tile_info )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	common_get_bg1_tile_info(device, tileinfo, tile_index, tc0100scn->fg_ram, tc0100scn->bg_gfx, tc0100scn->colbank, tc0100scn->dblwidth);
}

static TILE_GET_INFO_DEVICE( tc0100scn_get_tx_tile_info )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	common_get_tx_tile_info(device, tileinfo, tile_index, tc0100scn->tx_ram, tc0100scn->tx_gfx, tc0100scn->colbank, tc0100scn->dblwidth);
}

static const gfx_layout tc0100scn_charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	2,	/* 2 bits per pixel */
	{ XOR(0)*4, XOR(2)*4 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8	/* every sprite takes 16 consecutive bytes */
};


void tc0100scn_set_colbank( device_t *device, int col )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	tc0100scn->colbank = col;
}

void tc0100scn_set_colbanks( device_t *device, int bg0, int bg1, int tx )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	tc0100scn->bg0_colbank = bg0;
	tc0100scn->bg1_colbank = bg1;
	tc0100scn->tx_colbank = tx;
}

void tc0100scn_set_bg_tilemask( device_t *device, int mask )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	tc0100scn->bg_tilemask = mask;
}

WRITE16_DEVICE_HANDLER( tc0100scn_gfxbank_w )   /* Mjnquest banks its 2 sets of scr tiles */
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	tc0100scn->gfxbank = (data & 0x1);
}

static void tc0100scn_set_layer_ptrs( tc0100scn_state *tc0100scn )
{
	if (!tc0100scn->dblwidth)
	{
		tc0100scn->bg_ram        = tc0100scn->ram + 0x0;
		tc0100scn->tx_ram        = tc0100scn->ram + 0x4000 /2;
		tc0100scn->char_ram      = tc0100scn->ram + 0x6000 /2;
		tc0100scn->fg_ram        = tc0100scn->ram + 0x8000 /2;
		tc0100scn->bgscroll_ram  = tc0100scn->ram + 0xc000 /2;
		tc0100scn->fgscroll_ram  = tc0100scn->ram + 0xc400 /2;
		tc0100scn->colscroll_ram = tc0100scn->ram + 0xe000 /2;
	}
	else
	{
		tc0100scn->bg_ram        = tc0100scn->ram + 0x0;
		tc0100scn->fg_ram        = tc0100scn->ram + 0x08000 /2;
		tc0100scn->bgscroll_ram  = tc0100scn->ram + 0x10000 /2;
		tc0100scn->fgscroll_ram  = tc0100scn->ram + 0x10400 /2;
		tc0100scn->colscroll_ram = tc0100scn->ram + 0x10800 /2;
		tc0100scn->char_ram      = tc0100scn->ram + 0x11000 /2;
		tc0100scn->tx_ram        = tc0100scn->ram + 0x12000 /2;
	}
}

static void tc0100scn_dirty_tilemaps( device_t *device )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);

	tc0100scn->tilemap[0][tc0100scn->dblwidth]->mark_all_dirty();
	tc0100scn->tilemap[1][tc0100scn->dblwidth]->mark_all_dirty();
	tc0100scn->tilemap[2][tc0100scn->dblwidth]->mark_all_dirty();
}

static void tc0100scn_restore_scroll( tc0100scn_state *tc0100scn )
{
	int flip;

	tc0100scn->bgscrollx = -tc0100scn->ctrl[0];
	tc0100scn->fgscrollx = -tc0100scn->ctrl[1];
	tc0100scn->tilemap[2][0]->set_scrollx(0, -tc0100scn->ctrl[2]);
	tc0100scn->tilemap[2][1]->set_scrollx(0, -tc0100scn->ctrl[2]);

	tc0100scn->bgscrolly = -tc0100scn->ctrl[3];
	tc0100scn->fgscrolly = -tc0100scn->ctrl[4];
	tc0100scn->tilemap[2][0]->set_scrolly(0, -tc0100scn->ctrl[5]);
	tc0100scn->tilemap[2][1]->set_scrolly(0, -tc0100scn->ctrl[5]);

	flip = (tc0100scn->ctrl[7] & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
	tc0100scn->tilemap[0][0]->set_flip(flip);
	tc0100scn->tilemap[1][0]->set_flip(flip);
	tc0100scn->tilemap[2][0]->set_flip(flip);
	tc0100scn->tilemap[0][1]->set_flip(flip);
	tc0100scn->tilemap[1][1]->set_flip(flip);
	tc0100scn->tilemap[2][1]->set_flip(flip);
}


static void tc0100scn_postload(tc0100scn_state *tc0100scn)
{
	tc0100scn_set_layer_ptrs(tc0100scn);
	tc0100scn_restore_scroll(tc0100scn);

	tc0100scn->tilemap[0][0]->mark_all_dirty();
	tc0100scn->tilemap[1][0]->mark_all_dirty();
	tc0100scn->tilemap[2][0]->mark_all_dirty();
	tc0100scn->tilemap[0][1]->mark_all_dirty();
	tc0100scn->tilemap[1][1]->mark_all_dirty();
	tc0100scn->tilemap[2][1]->mark_all_dirty();
}

READ16_DEVICE_HANDLER( tc0100scn_word_r )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	return tc0100scn->ram[offset];
}

WRITE16_DEVICE_HANDLER( tc0100scn_word_w )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);

	COMBINE_DATA(&tc0100scn->ram[offset]);
	if (!tc0100scn->dblwidth)
	{
		if (offset < 0x2000)
			tc0100scn->tilemap[0][0]->mark_tile_dirty(offset / 2);
		else if (offset < 0x3000)
			tc0100scn->tilemap[2][0]->mark_tile_dirty((offset & 0x0fff));
		else if (offset < 0x3800)
			gfx_element_mark_dirty(device->machine().gfx[tc0100scn->tx_gfx], (offset - 0x3000) / 8);
		else if (offset >= 0x4000 && offset < 0x6000)
			tc0100scn->tilemap[1][0]->mark_tile_dirty((offset & 0x1fff) / 2);
	}
	else	/* Double-width tilemaps have a different memory map */
	{
		if (offset < 0x4000)
			tc0100scn->tilemap[0][1]->mark_tile_dirty(offset / 2);
		else if (offset >= 0x4000 && offset < 0x8000)
			tc0100scn->tilemap[1][1]->mark_tile_dirty((offset & 0x3fff) / 2);
		else if (offset >= 0x8800 && offset < 0x9000)
			gfx_element_mark_dirty(device->machine().gfx[tc0100scn->tx_gfx], (offset - 0x8800) / 8);
		else if (offset >= 0x9000)
			tc0100scn->tilemap[2][1]->mark_tile_dirty((offset & 0x0fff));
	}
}

READ16_DEVICE_HANDLER( tc0100scn_ctrl_word_r )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	return tc0100scn->ctrl[offset];
}

WRITE16_DEVICE_HANDLER( tc0100scn_ctrl_word_w )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);

	COMBINE_DATA(&tc0100scn->ctrl[offset]);

	data = tc0100scn->ctrl[offset];

	switch (offset)
	{
		case 0x00:
			tc0100scn->bgscrollx = -data;
			break;

		case 0x01:
			tc0100scn->fgscrollx = -data;
			break;

		case 0x02:
			tc0100scn->tilemap[2][0]->set_scrollx(0, -data);
			tc0100scn->tilemap[2][1]->set_scrollx(0, -data);
			break;

		case 0x03:
			tc0100scn->bgscrolly = -data;
			break;

		case 0x04:
			tc0100scn->fgscrolly = -data;
			break;

		case 0x05:
			tc0100scn->tilemap[2][0]->set_scrolly(0, -data);
			tc0100scn->tilemap[2][1]->set_scrolly(0, -data);
			break;

		case 0x06:
		{
			int old_width = tc0100scn->dblwidth;
			tc0100scn->dblwidth = (data & 0x10) >> 4;

			if (tc0100scn->dblwidth != old_width)	/* tilemap width is changing */
			{
				/* Reinitialise layer pointers */
				tc0100scn_set_layer_ptrs(tc0100scn);

				/* and ensure full redraw of the tilemaps */
				tc0100scn_dirty_tilemaps(device);

				/* reset the pointer to the text characters (and dirty them all) */
				gfx_element_set_source(device->machine().gfx[tc0100scn->tx_gfx], (UINT8 *)tc0100scn->char_ram);
			}

			break;
		}

		case 0x07:
		{
			int flip = (data & 0x01) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;

			tc0100scn->tilemap[0][0]->set_flip(flip);
			tc0100scn->tilemap[1][0]->set_flip(flip);
			tc0100scn->tilemap[2][0]->set_flip(flip);
			tc0100scn->tilemap[0][1]->set_flip(flip);
			tc0100scn->tilemap[1][1]->set_flip(flip);
			tc0100scn->tilemap[2][1]->set_flip(flip);

			break;
		}
	}
}


READ32_DEVICE_HANDLER( tc0100scn_ctrl_long_r )
{
	return (tc0100scn_ctrl_word_r(device, offset * 2, 0xffff) << 16) | tc0100scn_ctrl_word_r(device, offset * 2 + 1, 0xffff);
}

WRITE32_DEVICE_HANDLER( tc0100scn_ctrl_long_w )
{
	if (ACCESSING_BITS_16_31)
		tc0100scn_ctrl_word_w(device, offset * 2, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		tc0100scn_ctrl_word_w(device, (offset * 2) + 1, data & 0xffff, mem_mask & 0xffff);
}

READ32_DEVICE_HANDLER( tc0100scn_long_r )
{
	return (tc0100scn_word_r(device, offset * 2, 0xffff) << 16) | tc0100scn_word_r(device, offset * 2 + 1, 0xffff);
}

WRITE32_DEVICE_HANDLER( tc0100scn_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		int oldword = tc0100scn_word_r(device, offset * 2, 0xffff);
		int newword = data >> 16;
		if (!ACCESSING_BITS_16_23)
			newword |= (oldword & 0x00ff);
		if (!ACCESSING_BITS_24_31)
			newword |= (oldword & 0xff00);
		tc0100scn_word_w(device, offset * 2, newword, 0xffff);
	}
	if (ACCESSING_BITS_0_15)
	{
		int oldword = tc0100scn_word_r(device, (offset * 2) + 1, 0xffff);
		int newword = data& 0xffff;
		if (!ACCESSING_BITS_0_7)
			newword |= (oldword & 0x00ff);
		if (!ACCESSING_BITS_8_15)
			newword |= (oldword & 0xff00);
		tc0100scn_word_w(device, (offset * 2) + 1, newword, 0xffff);
	}
}


void tc0100scn_tilemap_update( device_t *device )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	int j;

	tc0100scn->tilemap[0][tc0100scn->dblwidth]->set_scrolly(0, tc0100scn->bgscrolly);
	tc0100scn->tilemap[1][tc0100scn->dblwidth]->set_scrolly(0, tc0100scn->fgscrolly);

	for (j = 0; j < 256; j++)
		tc0100scn->tilemap[0][tc0100scn->dblwidth]->set_scrollx((j + tc0100scn->bgscrolly) & 0x1ff, tc0100scn->bgscrollx - tc0100scn->bgscroll_ram[j]);
	for (j = 0; j < 256; j++)
		tc0100scn->tilemap[1][tc0100scn->dblwidth]->set_scrollx((j + tc0100scn->fgscrolly) & 0x1ff, tc0100scn->fgscrollx - tc0100scn->fgscroll_ram[j]);
}

static void tc0100scn_tilemap_draw_fg( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, UINT32 priority )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	const bitmap_ind16 &src_bitmap = tmap->pixmap();
	int width_mask, height_mask, x, y, p;
	int column_offset, src_x = 0, src_y = 0;
	int scrollx_delta = - tmap->scrolldx();
	int scrolly_delta = - tmap->scrolldy();

	width_mask = src_bitmap.width() - 1;
	height_mask = src_bitmap.height() - 1;

	src_y = (tc0100scn->fgscrolly + scrolly_delta) & height_mask;
	if (tc0100scn->ctrl[0x7] & 1) // Flipscreen
		src_y = (256 - src_y) & height_mask;

	//We use cliprect.max_y and cliprect.max_x to support games which use more than 1 screen

	// Row offsets are 'screen space' 0-255 regardless of Y scroll
	for (y = 0; y <= cliprect.max_y; y++)
	{
		src_x = (tc0100scn->fgscrollx - tc0100scn->fgscroll_ram[(y + scrolly_delta) & 0x1ff] + scrollx_delta + cliprect.min_x) & width_mask;
		if (tc0100scn->ctrl[0x7] & 1) // Flipscreen
			src_x = (256 - 64 - src_x) & width_mask;

		// Col offsets are 'tilemap' space 0-511, and apply to blocks of 8 pixels at once
		for (x = 0; x < cliprect.width(); x++)
		{
			column_offset = tc0100scn->colscroll_ram[(src_x & 0x3ff) / 8];
			p = src_bitmap.pix16((src_y - column_offset) & height_mask, src_x);

			if ((p & 0xf)!= 0 || (flags & TILEMAP_DRAW_OPAQUE))
			{
				bitmap.pix16(y, x + cliprect.min_x) = p;
				if (device->machine().priority_bitmap.valid())
				{
					UINT8 *pri = &device->machine().priority_bitmap.pix8(y);
					pri[x + cliprect.min_x] |= priority;
				}
			}
			src_x = (src_x + 1) & width_mask;
		}
		src_y = (src_y + 1) & height_mask;
	}
}

int tc0100scn_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	int disable = tc0100scn->ctrl[6] & 0xf7;
	rectangle clip = cliprect;
	clip &= tc0100scn->cliprect;

#if 0
if (disable != 0 && disable != 3 && disable != 7)
	popmessage("layer disable = %x",disable);
#endif

	switch (layer)
	{
		case 0:
			if (disable & 0x01)
				return 1;
			tc0100scn->tilemap[0][tc0100scn->dblwidth]->draw(bitmap, clip, flags, priority);
			break;
		case 1:
			if (disable & 0x02)
				return 1;
			tc0100scn_tilemap_draw_fg(device, bitmap, clip, tc0100scn->tilemap[1][tc0100scn->dblwidth], flags, priority);
			break;
		case 2:
			if (disable & 0x04)
				return 1;
			tc0100scn->tilemap[2][tc0100scn->dblwidth]->draw(bitmap, clip, flags, priority);
			break;
	}
	return 0;
}

int tc0100scn_bottomlayer( device_t *device )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	return (tc0100scn->ctrl[6] & 0x8) >> 3;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0100scn )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	const tc0100scn_interface *intf = tc0100scn_get_interface(device);
	int xd, yd;

	tc0100scn->screen = device->machine().device<screen_device>(intf->screen);

	/* Set up clipping for multi-TC0100SCN games. We assume
       this code won't ever affect single screen games:
       Thundfox is the only one of those with two chips, and
       we're safe as it uses single width tilemaps. */

	tc0100scn->cliprect = tc0100scn->screen->visible_area();

	/* use the given gfx sets for bg/tx tiles*/
	tc0100scn->bg_gfx = intf->gfxnum;	/* 2nd/3rd chips will use the same gfx set */
	tc0100scn->tx_gfx = intf->txnum;

	/* Single width versions */
	tc0100scn->tilemap[0][0] = tilemap_create_device(device, tc0100scn_get_bg_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	tc0100scn->tilemap[1][0] = tilemap_create_device(device, tc0100scn_get_fg_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	tc0100scn->tilemap[2][0] = tilemap_create_device(device, tc0100scn_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	/* Double width versions */
	tc0100scn->tilemap[0][1] = tilemap_create_device(device, tc0100scn_get_bg_tile_info, tilemap_scan_rows, 8, 8, 128, 64);
	tc0100scn->tilemap[1][1] = tilemap_create_device(device, tc0100scn_get_fg_tile_info, tilemap_scan_rows, 8, 8, 128, 64);
	tc0100scn->tilemap[2][1] = tilemap_create_device(device, tc0100scn_get_tx_tile_info, tilemap_scan_rows, 8, 8, 128, 32);

	tc0100scn->tilemap[0][0]->set_transparent_pen(0);
	tc0100scn->tilemap[1][0]->set_transparent_pen(0);
	tc0100scn->tilemap[2][0]->set_transparent_pen(0);

	tc0100scn->tilemap[0][1]->set_transparent_pen(0);
	tc0100scn->tilemap[1][1]->set_transparent_pen(0);
	tc0100scn->tilemap[2][1]->set_transparent_pen(0);

	/* Standard width tilemaps. I'm setting the optional chip #2
       7 bits higher and 2 pixels to the left than chip #1 because
       that's how thundfox wants it. */

	xd = (intf->multiscrn_hack == 0) ?  (-intf->x_offset) : (-intf->x_offset - 2);
	yd = (intf->multiscrn_hack == 0) ?  (8 - intf->y_offset) : (1 - intf->y_offset);

	tc0100scn->tilemap[0][0]->set_scrolldx(xd - 16, -intf->flip_xoffs - xd - 16);
	tc0100scn->tilemap[0][0]->set_scrolldy(yd,      -intf->flip_yoffs - yd);
	tc0100scn->tilemap[1][0]->set_scrolldx(xd - 16, -intf->flip_xoffs - xd - 16);
	tc0100scn->tilemap[1][0]->set_scrolldy(yd,      -intf->flip_yoffs - yd);
	tc0100scn->tilemap[2][0]->set_scrolldx(xd - 16, -intf->flip_text_xoffs - xd - 16 - 7);
	tc0100scn->tilemap[2][0]->set_scrolldy(yd,      -intf->flip_text_yoffs - yd);

	/* Double width tilemaps. We must correct offsets for
       extra chips, as MAME sees offsets from LHS of whole
       display not from the edges of individual screens.
       NB flipscreen tilemap offsets are based on Cameltry */

	xd = -intf->x_offset - intf->multiscrn_xoffs;
	yd = 8 - intf->y_offset;

	tc0100scn->tilemap[0][1]->set_scrolldx(xd - 16, -intf->flip_xoffs - xd - 16);
	tc0100scn->tilemap[0][1]->set_scrolldy(yd,      -intf->flip_yoffs - yd);
	tc0100scn->tilemap[1][1]->set_scrolldx(xd - 16, -intf->flip_xoffs - xd - 16);
	tc0100scn->tilemap[1][1]->set_scrolldy(yd,      -intf->flip_yoffs - yd);
	tc0100scn->tilemap[2][1]->set_scrolldx(xd - 16, -intf->flip_text_xoffs - xd - 16 - 7);
	tc0100scn->tilemap[2][1]->set_scrolldy(yd,      -intf->flip_text_yoffs - yd);

	tc0100scn->tilemap[0][0]->set_scroll_rows(512);
	tc0100scn->tilemap[1][0]->set_scroll_rows(512);
	tc0100scn->tilemap[0][1]->set_scroll_rows(512);
	tc0100scn->tilemap[1][1]->set_scroll_rows(512);

	tc0100scn->bg_tilemask = 0xffff;	/* Mjnquest has 0x7fff tilemask */

	tc0100scn->bg_col_mult = 1;	/* multiplier for when bg gfx != 4bpp */
	tc0100scn->tx_col_mult = 1;	/* multiplier needed when bg gfx is 6bpp */

	if (device->machine().gfx[intf->gfxnum]->color_granularity == 2)	/* Yuyugogo, Yesnoj */
		tc0100scn->bg_col_mult = 8;

	if (device->machine().gfx[intf->gfxnum]->color_granularity == 0x40)	/* Undrfire */
		tc0100scn->tx_col_mult = 4;

//logerror("TC0100SCN bg gfx granularity %04x: multiplier %04x\n", device->machine().gfx[intf->gfxnum]->color_granularity, tc0100scn->tx_col_mult);

	tc0100scn->ram = auto_alloc_array_clear(device->machine(), UINT16, TC0100SCN_RAM_SIZE / 2);

	tc0100scn_set_layer_ptrs(tc0100scn);

	tc0100scn_set_colbanks(device, 0, 0, 0);	/* standard values, only Wgp & multiscreen games change them */
									/* we call this here, so that they can be modified at VIDEO_START*/

	/* create the char set (gfx will then be updated dynamically from RAM) */
	device->machine().gfx[tc0100scn->tx_gfx] = gfx_element_alloc(device->machine(), &tc0100scn_charlayout, (UINT8 *)tc0100scn->char_ram, 64, 0);

	device->save_pointer(NAME(tc0100scn->ram), TC0100SCN_RAM_SIZE / 2);
	device->save_item(NAME(tc0100scn->ctrl));
	device->save_item(NAME(tc0100scn->dblwidth));
	device->save_item(NAME(tc0100scn->gfxbank));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(tc0100scn_postload), tc0100scn));
}


static DEVICE_RESET( tc0100scn )
{
	tc0100scn_state *tc0100scn = tc0100scn_get_safe_token(device);
	int i;

	tc0100scn->dblwidth = 0;
	tc0100scn->colbank = 0;
	tc0100scn->gfxbank = 0;	/* Mjnquest uniquely banks tiles */

	for (i = 0; i < 8; i++)
		tc0100scn->ctrl[i] = 0;

}

/***************************************************************************/
/*                                                                         */
/*                      TC0280GRD / TC0430GRW                              */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0280grd_state tc0280grd_state;
struct _tc0280grd_state
{
	UINT16 *       ram;

	tilemap_t      *tilemap;

	UINT16         ctrl[8];
	int            gfxnum, base_color;
};

#define TC0280GRD_RAM_SIZE 0x2000

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0280grd_state *tc0280grd_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0280GRD) || (device->type() == TC0430GRW));

	return (tc0280grd_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0280grd_interface *tc0280grd_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0280GRD) || (device->type() == TC0430GRW));
	return (const tc0280grd_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static TILE_GET_INFO_DEVICE( tc0280grd_get_tile_info )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	int attr = tc0280grd->ram[tile_index];
	SET_TILE_INFO_DEVICE(
			tc0280grd->gfxnum,
			attr & 0x3fff,
			((attr & 0xc000) >> 14) + tc0280grd->base_color,
			0);
}

READ16_DEVICE_HANDLER( tc0280grd_word_r )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	return tc0280grd->ram[offset];
}

WRITE16_DEVICE_HANDLER( tc0280grd_word_w )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	COMBINE_DATA(&tc0280grd->ram[offset]);
	tc0280grd->tilemap->mark_tile_dirty(offset);
}

WRITE16_DEVICE_HANDLER( tc0280grd_ctrl_word_w )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	COMBINE_DATA(&tc0280grd->ctrl[offset]);
}

READ16_DEVICE_HANDLER( tc0430grw_word_r )
{
	return tc0280grd_word_r(device, offset, mem_mask);
}

WRITE16_DEVICE_HANDLER( tc0430grw_word_w )
{
	tc0280grd_word_w(device, offset, data, mem_mask);
}

WRITE16_DEVICE_HANDLER( tc0430grw_ctrl_word_w )
{
	tc0280grd_ctrl_word_w(device, offset, data, mem_mask);
}

void tc0280grd_tilemap_update( device_t *device, int base_color )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	if (tc0280grd->base_color != base_color)
	{
		tc0280grd->base_color = base_color;
		tc0280grd->tilemap->mark_all_dirty();
	}
}

void tc0430grw_tilemap_update( device_t *device, int base_color )
{
	tc0280grd_tilemap_update(device, base_color);
}

static void zoom_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority, int xmultiply )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	UINT32 startx, starty;
	int incxx, incxy, incyx, incyy;

	/* 24-bit signed */
	startx = ((tc0280grd->ctrl[0] & 0xff) << 16) + tc0280grd->ctrl[1];

	if (startx & 0x800000)
		startx -= 0x1000000;

	incxx = (INT16)tc0280grd->ctrl[2];
	incxx *= xmultiply;
	incyx = (INT16)tc0280grd->ctrl[3];

	/* 24-bit signed */
	starty = ((tc0280grd->ctrl[4] & 0xff) << 16) + tc0280grd->ctrl[5];

	if (starty & 0x800000)
		starty -= 0x1000000;

	incxy = (INT16)tc0280grd->ctrl[6];
	incxy *= xmultiply;
	incyy = (INT16)tc0280grd->ctrl[7];

	startx -= xoffset * incxx + yoffset * incyx;
	starty -= xoffset * incxy + yoffset * incyy;

	tc0280grd->tilemap->draw_roz(bitmap, cliprect, startx << 4, starty << 4,
			incxx << 4, incxy << 4, incyx << 4, incyy << 4,
			1,	/* copy with wraparound */
			0, priority);
}

void tc0280grd_zoom_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority )
{
	zoom_draw(device, bitmap, cliprect, xoffset, yoffset, priority, 2);
}

void tc0430grw_zoom_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffset, int yoffset, UINT32 priority )
{
	zoom_draw(device, bitmap, cliprect, xoffset, yoffset, priority, 1);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0280grd )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	const tc0280grd_interface *intf = tc0280grd_get_interface(device);

	tc0280grd->gfxnum = intf->gfxnum;

	tc0280grd->tilemap = tilemap_create_device(device, tc0280grd_get_tile_info, tilemap_scan_rows, 8, 8, 64, 64);
	tc0280grd->tilemap->set_transparent_pen(0);

	tc0280grd->ram = auto_alloc_array(device->machine(), UINT16, TC0280GRD_RAM_SIZE / 2);

	device->save_pointer(NAME(tc0280grd->ram), TC0280GRD_RAM_SIZE / 2);
	device->save_item(NAME(tc0280grd->ctrl));
}

static DEVICE_RESET( tc0280grd )
{
	tc0280grd_state *tc0280grd = tc0280grd_get_safe_token(device);
	int i;

	for (i = 0; i < 8; i++)
		tc0280grd->ctrl[i] = 0;

}

/***************************************************************************/
/*                                                                         */
/*                              TC0360PRI                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0360pri_state tc0360pri_state;
struct _tc0360pri_state
{
	UINT8   regs[16];
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0360pri_state *tc0360pri_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0360PRI);

	return (tc0360pri_state *)downcast<legacy_device_base *>(device)->token();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

WRITE8_DEVICE_HANDLER( tc0360pri_w )
{
	tc0360pri_state *tc0360pri = tc0360pri_get_safe_token(device);
	tc0360pri->regs[offset] = data;

if (offset >= 0x0a)
	popmessage("write %02x to unused TC0360PRI reg %x", data, offset);
#if 0
#define regs tc0360pri->regs
	popmessage("%02x %02x  %02x %02x  %02x %02x %02x %02x %02x %02x",
		regs[0x00], regs[0x01], regs[0x02], regs[0x03],
		regs[0x04], regs[0x05], regs[0x06], regs[0x07],
		regs[0x08], regs[0x09]);
#endif
}

READ8_DEVICE_HANDLER( tc0360pri_r )
{
	tc0360pri_state *tc0360pri = tc0360pri_get_safe_token(device);
	return tc0360pri->regs[offset];
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0360pri )
{
	tc0360pri_state *tc0360pri = tc0360pri_get_safe_token(device);
	device->save_item(NAME(tc0360pri->regs));
}

static DEVICE_RESET( tc0360pri )
{
	tc0360pri_state *tc0360pri = tc0360pri_get_safe_token(device);
	int i;

	for (i = 0; i < 16; i++)
		tc0360pri->regs[i] = 0;

}

/***************************************************************************/
/*                                                                         */
/*                              TC0480SCP                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0480scp_state tc0480scp_state;
struct _tc0480scp_state
{
	UINT16           ctrl[0x18];

	UINT16 *         ram;
	UINT16 *         bg_ram[4];
	UINT16 *         tx_ram;
	UINT16 *         char_ram;
	UINT16 *         bgscroll_ram[4];
	UINT16 *         rowzoom_ram[4];
	UINT16 *         bgcolumn_ram[4];
	int              bgscrollx[4];
	int              bgscrolly[4];
	int              pri_reg;

	/* We keep two tilemaps for each of the 5 actual tilemaps: one at standard width, one double */
	tilemap_t         *tilemap[5][2];
	int             bg_gfx, tx_gfx;
	INT32           tile_colbase, dblwidth;
	int             x_offs, y_offs;
	int             text_xoffs, text_yoffs;
	int             flip_xoffs, flip_yoffs;
};

#define TC0480SCP_RAM_SIZE 0x10000
#define TC0480SCP_TOTAL_CHARS 256

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0480scp_state *tc0480scp_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0480SCP);

	return (tc0480scp_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0480scp_interface *tc0480scp_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0480SCP));
	return (const tc0480scp_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/


INLINE void common_get_tc0480bg_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	int code = ram[2 * tile_index + 1] & 0x7fff;
	int attr = ram[2 * tile_index];
	SET_TILE_INFO_DEVICE(
			gfxnum,
			code,
			(attr & 0xff) + tc0480scp->tile_colbase,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

INLINE void common_get_tc0480tx_tile_info( device_t *device, tile_data &tileinfo, int tile_index, UINT16 *ram, int gfxnum )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	int attr = ram[tile_index];
	SET_TILE_INFO_DEVICE(
			gfxnum,
			attr & 0xff,
			((attr & 0x3f00) >> 8) + tc0480scp->tile_colbase,
			TILE_FLIPYX((attr & 0xc000) >> 14));
}

static TILE_GET_INFO_DEVICE( tc0480scp_get_bg0_tile_info )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	common_get_tc0480bg_tile_info(device, tileinfo, tile_index, tc0480scp->bg_ram[0], tc0480scp->bg_gfx );
}

static TILE_GET_INFO_DEVICE( tc0480scp_get_bg1_tile_info )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	common_get_tc0480bg_tile_info(device, tileinfo, tile_index, tc0480scp->bg_ram[1], tc0480scp->bg_gfx);
}

static TILE_GET_INFO_DEVICE( tc0480scp_get_bg2_tile_info )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	common_get_tc0480bg_tile_info(device, tileinfo, tile_index, tc0480scp->bg_ram[2], tc0480scp->bg_gfx);
}

static TILE_GET_INFO_DEVICE( tc0480scp_get_bg3_tile_info )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	common_get_tc0480bg_tile_info(device, tileinfo, tile_index, tc0480scp->bg_ram[3], tc0480scp->bg_gfx);
}

static TILE_GET_INFO_DEVICE( tc0480scp_get_tx_tile_info )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	common_get_tc0480tx_tile_info(device, tileinfo, tile_index, tc0480scp->tx_ram, tc0480scp->tx_gfx);
}

static const gfx_layout tc0480scp_charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ XOR(3)*4, XOR(2)*4, XOR(1)*4, XOR(0)*4, XOR(7)*4, XOR(6)*4, XOR(5)*4, XOR(4)*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8	/* every sprite takes 32 consecutive bytes */
};


static void tc0480scp_dirty_tilemaps( tc0480scp_state *tc0480scp )
{
	tc0480scp->tilemap[0][tc0480scp->dblwidth]->mark_all_dirty();
	tc0480scp->tilemap[1][tc0480scp->dblwidth]->mark_all_dirty();
	tc0480scp->tilemap[2][tc0480scp->dblwidth]->mark_all_dirty();
	tc0480scp->tilemap[3][tc0480scp->dblwidth]->mark_all_dirty();
	tc0480scp->tilemap[4][tc0480scp->dblwidth]->mark_all_dirty();
}


static void tc0480scp_set_layer_ptrs( tc0480scp_state *tc0480scp )
{
	if (!tc0480scp->dblwidth)
	{
		tc0480scp->bg_ram[0]	   = tc0480scp->ram + 0x0000; //0000
		tc0480scp->bg_ram[1]	   = tc0480scp->ram + 0x0800; //1000
		tc0480scp->bg_ram[2]	   = tc0480scp->ram + 0x1000; //2000
		tc0480scp->bg_ram[3]	   = tc0480scp->ram + 0x1800; //3000
		tc0480scp->bgscroll_ram[0] = tc0480scp->ram + 0x2000; //4000
		tc0480scp->bgscroll_ram[1] = tc0480scp->ram + 0x2200; //4400
		tc0480scp->bgscroll_ram[2] = tc0480scp->ram + 0x2400; //4800
		tc0480scp->bgscroll_ram[3] = tc0480scp->ram + 0x2600; //4c00
		tc0480scp->rowzoom_ram[2]  = tc0480scp->ram + 0x3000; //6000
		tc0480scp->rowzoom_ram[3]  = tc0480scp->ram + 0x3200; //6400
		tc0480scp->bgcolumn_ram[2] = tc0480scp->ram + 0x3400; //6800
		tc0480scp->bgcolumn_ram[3] = tc0480scp->ram + 0x3600; //6c00
		tc0480scp->tx_ram		   = tc0480scp->ram + 0x6000; //c000
		tc0480scp->char_ram	   = tc0480scp->ram + 0x7000; //e000
	}
	else
	{
		tc0480scp->bg_ram[0]	   = tc0480scp->ram + 0x0000; //0000
		tc0480scp->bg_ram[1]	   = tc0480scp->ram + 0x1000; //2000
		tc0480scp->bg_ram[2]	   = tc0480scp->ram + 0x2000; //4000
		tc0480scp->bg_ram[3]	   = tc0480scp->ram + 0x3000; //6000
		tc0480scp->bgscroll_ram[0] = tc0480scp->ram + 0x4000; //8000
		tc0480scp->bgscroll_ram[1] = tc0480scp->ram + 0x4200; //8400
		tc0480scp->bgscroll_ram[2] = tc0480scp->ram + 0x4400; //8800
		tc0480scp->bgscroll_ram[3] = tc0480scp->ram + 0x4600; //8c00
		tc0480scp->rowzoom_ram[2]  = tc0480scp->ram + 0x5000; //a000
		tc0480scp->rowzoom_ram[3]  = tc0480scp->ram + 0x5200; //a400
		tc0480scp->bgcolumn_ram[2] = tc0480scp->ram + 0x5400; //a800
		tc0480scp->bgcolumn_ram[3] = tc0480scp->ram + 0x5600; //ac00
		tc0480scp->tx_ram		   = tc0480scp->ram + 0x6000; //c000
		tc0480scp->char_ram	   = tc0480scp->ram + 0x7000; //e000
	}
}

READ16_DEVICE_HANDLER( tc0480scp_word_r )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	return tc0480scp->ram[offset];
}

WRITE16_DEVICE_HANDLER( tc0480scp_word_w )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);

	COMBINE_DATA(&tc0480scp->ram[offset]);

	if (!tc0480scp->dblwidth)
	{
		if (offset < 0x2000)
		{
			tc0480scp->tilemap[(offset / 0x800)][tc0480scp->dblwidth]->mark_tile_dirty(((offset % 0x800) / 2));
		}
		else if (offset < 0x6000)
		{   /* do nothing */
		}
		else if (offset < 0x7000)
		{
			tc0480scp->tilemap[4][tc0480scp->dblwidth]->mark_tile_dirty((offset - 0x6000));
		}
		else if (offset <= 0x7fff)
		{
			gfx_element_mark_dirty(device->machine().gfx[tc0480scp->tx_gfx], (offset - 0x7000) / 16);
		}
	}
	else
	{
		if (offset < 0x4000)
		{
			tc0480scp->tilemap[(offset / 0x1000)][tc0480scp->dblwidth]->mark_tile_dirty(((offset % 0x1000) / 2));
		}
		else if (offset < 0x6000)
		{   /* do nothing */
		}
		else if (offset < 0x7000)
		{
			tc0480scp->tilemap[4][tc0480scp->dblwidth]->mark_tile_dirty((offset - 0x6000));
		}
		else if (offset <= 0x7fff)
		{
			gfx_element_mark_dirty(device->machine().gfx[tc0480scp->tx_gfx], (offset - 0x7000) / 16);
		}
	}
}

READ16_DEVICE_HANDLER( tc0480scp_ctrl_word_r )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	return tc0480scp->ctrl[offset];
}

WRITE16_DEVICE_HANDLER( tc0480scp_ctrl_word_w )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	int flip = tc0480scp->pri_reg & 0x40;

	COMBINE_DATA(&tc0480scp->ctrl[offset]);
	data = tc0480scp->ctrl[offset];

	switch (offset)
	{
		/* The x offsets of the four bg layers are staggered by intervals of 4 pixels */
		case 0x00:   /* bg0 x */
			if (!flip)  data = -data;
			tc0480scp->bgscrollx[0] = data;
			break;

		case 0x01:   /* bg1 x */
			data += 4;
			if (!flip)  data = -data;
			tc0480scp->bgscrollx[1] = data;
			break;

		case 0x02:   /* bg2 x */
			data += 8;
			if (!flip)  data = -data;
			tc0480scp->bgscrollx[2] = data;
			break;

		case 0x03:   /* bg3 x */
			data += 12;
			if (!flip)  data = -data;
			tc0480scp->bgscrollx[3] = data;
			break;

		case 0x04:   /* bg0 y */
			if (flip)  data = -data;
			tc0480scp->bgscrolly[0] = data;
			break;

		case 0x05:   /* bg1 y */
			if (flip)  data = -data;
			tc0480scp->bgscrolly[1] = data;
			break;

		case 0x06:   /* bg2 y */
			if (flip)  data = -data;
			tc0480scp->bgscrolly[2] = data;
			break;

		case 0x07:   /* bg3 y */
			if (flip)  data = -data;
			tc0480scp->bgscrolly[3] = data;
			break;

		case 0x08:   /* bg0 zoom */
		case 0x09:   /* bg1 zoom */
		case 0x0a:   /* bg2 zoom */
		case 0x0b:   /* bg3 zoom */
			break;

		case 0x0c:   /* fg (text) x */

			/* Text layer can be offset from bg0 (e.g. Metalb) */
			if (!flip)	data -= tc0480scp->text_xoffs;
			if (flip)	data += tc0480scp->text_xoffs;

			tc0480scp->tilemap[4][0]->set_scrollx(0, -data);
			tc0480scp->tilemap[4][1]->set_scrollx(0, -data);
			break;

		case 0x0d:   /* fg (text) y */

			/* Text layer can be offset from bg0 (e.g. Slapshot) */
			if (!flip)	data -= tc0480scp->text_yoffs;
			if (flip)	data += tc0480scp->text_yoffs;

			tc0480scp->tilemap[4][0]->set_scrolly(0, -data);
			tc0480scp->tilemap[4][1]->set_scrolly(0, -data);
			break;

		/* offset 0x0e unused */

		case 0x0f:   /* control register */
		{
			int old_width = (tc0480scp->pri_reg & 0x80) >> 7;
			flip = (data & 0x40) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0;
			tc0480scp->pri_reg = data;

			tc0480scp->tilemap[0][0]->set_flip(flip);
			tc0480scp->tilemap[1][0]->set_flip(flip);
			tc0480scp->tilemap[2][0]->set_flip(flip);
			tc0480scp->tilemap[3][0]->set_flip(flip);
			tc0480scp->tilemap[4][0]->set_flip(flip);

			tc0480scp->tilemap[0][1]->set_flip(flip);
			tc0480scp->tilemap[1][1]->set_flip(flip);
			tc0480scp->tilemap[2][1]->set_flip(flip);
			tc0480scp->tilemap[3][1]->set_flip(flip);
			tc0480scp->tilemap[4][1]->set_flip(flip);

			tc0480scp->dblwidth = (tc0480scp->pri_reg & 0x80) >> 7;

			if (tc0480scp->dblwidth != old_width)	/* tilemap width is changing */
			{
				/* Reinitialise layer pointers */
				tc0480scp_set_layer_ptrs(tc0480scp);

				/* and ensure full redraw of tilemaps */
				tc0480scp_dirty_tilemaps(tc0480scp);
			}

			break;
		}

		/* Rest are layer specific delta x and y, used while scrolling that layer */
	}
}


READ32_DEVICE_HANDLER( tc0480scp_ctrl_long_r )
{
	return (tc0480scp_ctrl_word_r(device, offset * 2, 0xffff) << 16) | tc0480scp_ctrl_word_r(device, offset * 2 + 1, 0xffff);
}

/* TODO: byte access ? */

WRITE32_DEVICE_HANDLER( tc0480scp_ctrl_long_w )
{
	if (ACCESSING_BITS_16_31)
		tc0480scp_ctrl_word_w(device, offset * 2, data >> 16, mem_mask >> 16);
	if (ACCESSING_BITS_0_15)
		tc0480scp_ctrl_word_w(device, (offset * 2) + 1, data & 0xffff, mem_mask & 0xffff);
}

READ32_DEVICE_HANDLER( tc0480scp_long_r )
{
	return (tc0480scp_word_r(device, offset * 2, 0xffff) << 16) | tc0480scp_word_r(device, offset * 2 + 1, 0xffff);
}

WRITE32_DEVICE_HANDLER( tc0480scp_long_w )
{
	if (ACCESSING_BITS_16_31)
	{
		int oldword = tc0480scp_word_r(device, offset * 2, 0xffff);
		int newword = data >> 16;
		if (!ACCESSING_BITS_16_23)
			newword |= (oldword & 0x00ff);
		if (!ACCESSING_BITS_24_31)
			newword |= (oldword & 0xff00);
		tc0480scp_word_w(device, offset * 2, newword, 0xffff);
	}
	if (ACCESSING_BITS_0_15)
	{
		int oldword = tc0480scp_word_r(device, (offset * 2) + 1, 0xffff);
		int newword = data & 0xffff;
		if (!ACCESSING_BITS_0_7)
			newword |= (oldword & 0x00ff);
		if (!ACCESSING_BITS_8_15)
			newword |= (oldword & 0xff00);
		tc0480scp_word_w(device, (offset * 2) + 1, newword, 0xffff);
	}
}


void tc0480scp_tilemap_update( device_t *device )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	int layer, zoom, i, j;
	int flip = tc0480scp->pri_reg & 0x40;

	for (layer = 0; layer < 4; layer++)
	{
		tc0480scp->tilemap[layer][tc0480scp->dblwidth]->set_scrolly(0, tc0480scp->bgscrolly[layer]);
		zoom = 0x10000 + 0x7f - tc0480scp->ctrl[0x08 + layer];

		if (zoom != 0x10000)	/* can't use scroll rows when zooming */
		{
			tc0480scp->tilemap[layer][tc0480scp->dblwidth]->set_scrollx(0, tc0480scp->bgscrollx[layer]);
		}
		else
		{
			for (j = 0; j < 512; j++)
			{
				i = tc0480scp->bgscroll_ram[layer][j];

				if (!flip)
					tc0480scp->tilemap[layer][tc0480scp->dblwidth]->set_scrollx(j & 0x1ff, tc0480scp->bgscrollx[layer] - i);
				else
					tc0480scp->tilemap[layer][tc0480scp->dblwidth]->set_scrollx(j & 0x1ff, tc0480scp->bgscrollx[layer] + i);
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
bigger than usual vertical visible area). Refer to tc0080vco
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

static void tc0480scp_bg01_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	/* X-axis zoom offers expansion only: 0 = no zoom, 0xff = max
       Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max
       (0x1a in Footchmp hiscore = shrunk) */

	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	int zoomx = 0x10000 - (tc0480scp->ctrl[0x08 + layer] & 0xff00);
	int zoomy = 0x10000 - (((tc0480scp->ctrl[0x08 + layer] & 0xff) - 0x7f) * 512);

	if ((zoomx == 0x10000) && (zoomy == 0x10000))	/* no zoom, simple */
	{
		/* Prevent bad things */
		tc0480scp->tilemap[layer][tc0480scp->dblwidth]->draw(bitmap, cliprect, flags, priority);
	}
	else	/* zoom */
	{
		UINT16 *dst16, *src16;
		UINT8 *tsrc;
		UINT16 scanline[512];
		UINT32 sx;
		bitmap_ind16 &srcbitmap = tc0480scp->tilemap[layer][tc0480scp->dblwidth]->pixmap();
		bitmap_ind8 &flagsbitmap = tc0480scp->tilemap[layer][tc0480scp->dblwidth]->flagsmap();
		int flip = tc0480scp->pri_reg & 0x40;
		int i, y, y_index, src_y_index, row_index;
		int x_index, x_step;
		int machine_flip = 0;	/* for  ROT 180 ? */

		UINT16 screen_width = 512; //cliprect.width();
		UINT16 min_y = cliprect.min_y;
		UINT16 max_y = cliprect.max_y;

		int width_mask = 0x1ff;
		if (tc0480scp->dblwidth)
			width_mask = 0x3ff;

		if (!flip)
		{
			sx = ((tc0480scp->bgscrollx[layer] + 15 + layer * 4) << 16) + ((255 - (tc0480scp->ctrl[0x10 + layer] & 0xff)) << 8);
			sx += (tc0480scp->x_offs - 15 - layer * 4) * zoomx;

			y_index = (tc0480scp->bgscrolly[layer] << 16) + ((tc0480scp->ctrl[0x14 + layer] & 0xff) << 8);
			y_index -= (tc0480scp->y_offs - min_y) * zoomy;
		}
		else	/* TC0480SCP tiles flipscreen */
		{
			sx = ((-tc0480scp->bgscrollx[layer] + 15 + layer * 4 + tc0480scp->flip_xoffs ) << 16) + ((255-(tc0480scp->ctrl[0x10 + layer] & 0xff)) << 8);
			sx += (tc0480scp->x_offs - 15 - layer * 4) * zoomx;

			y_index = ((-tc0480scp->bgscrolly[layer] + tc0480scp->flip_yoffs) << 16) + ((tc0480scp->ctrl[0x14 + layer] & 0xff) << 8);
			y_index -= (tc0480scp->y_offs - min_y) * zoomy;
		}

		if (!machine_flip)
			y = min_y;
		else
			y = max_y;

		do
		{
			src_y_index = (y_index >> 16) & 0x1ff;

			/* row areas are the same in flipscreen, so we must read in reverse */
			row_index = src_y_index;
			if (flip)
				row_index = 0x1ff - row_index;

			x_index = sx - ((tc0480scp->bgscroll_ram[layer][row_index] << 16)) - ((tc0480scp->bgscroll_ram[layer][row_index + 0x800] << 8) & 0xffff);

			src16 = &srcbitmap.pix16(src_y_index);
			tsrc = &flagsbitmap.pix8(src_y_index);
			dst16 = scanline;

			x_step = zoomx;

			if (flags & TILEMAP_DRAW_OPAQUE)
			{
				for (i = 0; i < screen_width; i++)
				{
					*dst16++ = src16[(x_index >> 16) & width_mask];
					x_index += x_step;
				}
			}
			else
			{
				for (i = 0; i < screen_width; i++)
				{
					if (tsrc[(x_index >> 16) & width_mask])
						*dst16++ = src16[(x_index >> 16) & width_mask];
					else
						*dst16++ = 0x8000;
					x_index += x_step;
				}
			}

			taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1, ROT0, device->machine().priority_bitmap, priority);

			y_index += zoomy;
			if (!machine_flip)
				y++;
			else
				y--;
		}
		while ((!machine_flip && y <= max_y) || (machine_flip && y >= min_y));

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
bigger than usual vertical visible area). Refer to tc0080vco
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

static void tc0480scp_bg23_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	bitmap_ind16 &srcbitmap = tc0480scp->tilemap[layer][tc0480scp->dblwidth]->pixmap();
	bitmap_ind8 &flagsbitmap = tc0480scp->tilemap[layer][tc0480scp->dblwidth]->flagsmap();

	UINT16 *dst16, *src16;
	UINT8 *tsrc;
	int i, y, y_index, src_y_index, row_index, row_zoom;
	int sx, x_index, x_step;
	UINT32 zoomx, zoomy;
	UINT16 scanline[512];
	int flipscreen = tc0480scp->pri_reg & 0x40;
	int machine_flip = 0;	/* for  ROT 180 ? */

	UINT16 screen_width = 512; //cliprect.width();
	UINT16 min_y = cliprect.min_y;
	UINT16 max_y = cliprect.max_y;

	int width_mask = 0x1ff;
	if (tc0480scp->dblwidth)
		width_mask = 0x3ff;

	/* X-axis zoom offers expansion only: 0 = no zoom, 0xff = max
       Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max
       (0x1a in Footchmp hiscore = shrunk) */

	zoomx = 0x10000 - (tc0480scp->ctrl[0x08 + layer] & 0xff00);
	zoomy = 0x10000 - (((tc0480scp->ctrl[0x08 + layer] & 0xff) - 0x7f) * 512);

	if (!flipscreen)
	{
		sx = ((tc0480scp->bgscrollx[layer] + 15 + layer * 4) << 16) + ((255-(tc0480scp->ctrl[0x10 + layer] & 0xff)) << 8);
		sx += (tc0480scp->x_offs - 15 - layer * 4) * zoomx;

		y_index = (tc0480scp->bgscrolly[layer] << 16) + ((tc0480scp->ctrl[0x14 + layer] & 0xff) << 8);
		y_index -= (tc0480scp->y_offs - min_y) * zoomy;
	}
	else	/* TC0480SCP tiles flipscreen */
	{
		sx = ((-tc0480scp->bgscrollx[layer] + 15 + layer * 4 + tc0480scp->flip_xoffs ) << 16) + ((255 - (tc0480scp->ctrl[0x10 + layer] & 0xff)) << 8);
		sx += (tc0480scp->x_offs - 15 - layer * 4) * zoomx;

		y_index = ((-tc0480scp->bgscrolly[layer] + tc0480scp->flip_yoffs) << 16) + ((tc0480scp->ctrl[0x14 + layer] & 0xff) << 8);
		y_index -= (tc0480scp->y_offs - min_y) * zoomy;
	}


	if (!machine_flip)
		y = min_y;
	else
		y = max_y;

	do
	{
		if (!flipscreen)
			src_y_index = ((y_index>>16) + tc0480scp->bgcolumn_ram[layer][(y - tc0480scp->y_offs) & 0x1ff]) & 0x1ff;
		else	/* colscroll area is back to front in flipscreen */
			src_y_index = ((y_index>>16) + tc0480scp->bgcolumn_ram[layer][0x1ff - ((y - tc0480scp->y_offs) & 0x1ff)]) & 0x1ff;

		/* row areas are the same in flipscreen, so we must read in reverse */
		row_index = src_y_index;
		if (flipscreen)
			row_index = 0x1ff - row_index;

		if (tc0480scp->pri_reg & (layer - 1))	/* bit0 enables for BG2, bit1 for BG3 */
			row_zoom = tc0480scp->rowzoom_ram[layer][row_index];
		else
			row_zoom = 0;

		x_index = sx - ((tc0480scp->bgscroll_ram[layer][row_index] << 16)) - ((tc0480scp->bgscroll_ram[layer][row_index + 0x800] << 8) & 0xffff);

		/* flawed calc ?? */
		x_index -= (tc0480scp->x_offs - 0x1f + layer * 4) * ((row_zoom & 0xff) << 8);

/* We used to kludge 270 multiply factor, before adjusting x_index instead */

		x_step = zoomx;
		if (row_zoom)	/* need to reduce x_step */
		{
			if (!(row_zoom & 0xff00))
				x_step -= ((row_zoom * 256) & 0xffff);
			else	/* Undrfire uses the hi byte, why? */
				x_step -= (((row_zoom & 0xff) * 256) & 0xffff);
		}

		src16 = &srcbitmap.pix16(src_y_index);
		tsrc = &flagsbitmap.pix8(src_y_index);
		dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (i = 0; i < screen_width; i++)
			{
				*dst16++ = src16[(x_index >> 16) & width_mask];
				x_index += x_step;
			}
		}
		else
		{
			for (i = 0; i < screen_width; i++)
			{
				if (tsrc[(x_index >> 16) & width_mask])
					*dst16++ = src16[(x_index >> 16) & width_mask];
				else
					*dst16++ = 0x8000;
				x_index += x_step;
			}
		}

		taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? 0 : 1, ROT0, device->machine().priority_bitmap, priority);

		y_index += zoomy;
		if (!machine_flip)
			y++;
		else
			y--;
	}
	while ((!machine_flip && y<=max_y) || (machine_flip && y>=min_y));
}


void tc0480scp_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, UINT32 priority )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);

	/* no layer disable bits */
	switch (layer)
	{
		case 0:
			tc0480scp_bg01_draw(device, bitmap, cliprect, 0, flags, priority);
			break;
		case 1:
			tc0480scp_bg01_draw(device, bitmap, cliprect, 1, flags, priority);
			break;
		case 2:
			tc0480scp_bg23_draw(device, bitmap, cliprect, 2, flags, priority);
			break;
		case 3:
			tc0480scp_bg23_draw(device, bitmap, cliprect, 3, flags, priority);
			break;
		case 4:
			tc0480scp->tilemap[4][tc0480scp->dblwidth]->draw(bitmap, cliprect, flags, priority);
			break;
	}
}

/* For evidence table of TC0480SCP bg layer priorities, refer to mame55 source */

static const UINT16 tc0480scp_bg_pri_lookup[8] =
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

int tc0480scp_get_bg_priority( device_t *device )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	return tc0480scp_bg_pri_lookup[(tc0480scp->pri_reg & 0x1c) >> 2];
}

// undrfire.c also needs to directly access the priority reg
READ8_DEVICE_HANDLER( tc0480scp_pri_reg_r )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	return tc0480scp->pri_reg;
}

static void tc0480scp_postload(tc0480scp_state *tc0480scp)
{
	int reg;
	int flip = tc0480scp->ctrl[0xf] & 0x40;

	tc0480scp_set_layer_ptrs(tc0480scp);

	tc0480scp->tilemap[0][0]->set_flip(flip);
	tc0480scp->tilemap[1][0]->set_flip(flip);
	tc0480scp->tilemap[2][0]->set_flip(flip);
	tc0480scp->tilemap[3][0]->set_flip(flip);
	tc0480scp->tilemap[4][0]->set_flip(flip);

	tc0480scp->tilemap[0][1]->set_flip(flip);
	tc0480scp->tilemap[1][1]->set_flip(flip);
	tc0480scp->tilemap[2][1]->set_flip(flip);
	tc0480scp->tilemap[3][1]->set_flip(flip);
	tc0480scp->tilemap[4][1]->set_flip(flip);

	reg = tc0480scp->ctrl[0];
	if (!flip)  reg = -reg;
	tc0480scp->bgscrollx[0] = reg;

	reg = tc0480scp->ctrl[1] + 4;
	if (!flip)  reg = -reg;
	tc0480scp->bgscrollx[1] = reg;

	reg = tc0480scp->ctrl[2] + 8;
	if (!flip)  reg = -reg;
	tc0480scp->bgscrollx[2] = reg;

	reg = tc0480scp->ctrl[3] + 12;
	if (!flip)  reg = -reg;
	tc0480scp->bgscrollx[3] = reg;

	reg = tc0480scp->ctrl[4];
	if (!flip)  reg = -reg;
	tc0480scp->bgscrolly[0] = reg;

	reg = tc0480scp->ctrl[5];
	if (!flip)  reg = -reg;
	tc0480scp->bgscrolly[1] = reg;

	reg = tc0480scp->ctrl[6];
	if (!flip)  reg = -reg;
	tc0480scp->bgscrolly[2] = reg;

	reg = tc0480scp->ctrl[7];
	if (!flip)  reg = -reg;
	tc0480scp->bgscrolly[3] = reg;

	reg = tc0480scp->ctrl[0x0c];
	if (!flip)	reg -= tc0480scp->text_xoffs;
	if (flip)	reg += tc0480scp->text_xoffs;
	tc0480scp->tilemap[4][0]->set_scrollx(0, -reg);
	tc0480scp->tilemap[4][1]->set_scrollx(0, -reg);

	reg = tc0480scp->ctrl[0x0d];
	if (!flip)	reg -= tc0480scp->text_yoffs;
	if (flip)	reg += tc0480scp->text_yoffs;
	tc0480scp->tilemap[4][0]->set_scrolly(0, -reg);
	tc0480scp->tilemap[4][1]->set_scrolly(0, -reg);
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0480scp )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	const tc0480scp_interface *intf = tc0480scp_get_interface(device);
	int i, xd, yd;

	/* use the given gfx set for bg/tx tiles */
	tc0480scp->bg_gfx = intf->gfxnum;
	tc0480scp->tx_gfx = intf->txnum;

	tc0480scp->tile_colbase = intf->col_base;
	tc0480scp->text_xoffs = intf->text_xoffs;
	tc0480scp->text_yoffs = intf->text_yoffs;
	tc0480scp->flip_xoffs = intf->flip_xoffs;	/* for most games (-1,0) */
	tc0480scp->flip_yoffs = intf->flip_yoffs;
	tc0480scp->x_offs = intf->x_offset + intf->pixels;
	tc0480scp->y_offs = intf->y_offset;

	/* Single width versions */
	tc0480scp->tilemap[0][0] = tilemap_create_device(device, tc0480scp_get_bg0_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tc0480scp->tilemap[1][0] = tilemap_create_device(device, tc0480scp_get_bg1_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tc0480scp->tilemap[2][0] = tilemap_create_device(device, tc0480scp_get_bg2_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tc0480scp->tilemap[3][0] = tilemap_create_device(device, tc0480scp_get_bg3_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
	tc0480scp->tilemap[4][0] = tilemap_create_device(device, tc0480scp_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	/* Double width versions */
	tc0480scp->tilemap[0][1] = tilemap_create_device(device, tc0480scp_get_bg0_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	tc0480scp->tilemap[1][1] = tilemap_create_device(device, tc0480scp_get_bg1_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	tc0480scp->tilemap[2][1] = tilemap_create_device(device, tc0480scp_get_bg2_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	tc0480scp->tilemap[3][1] = tilemap_create_device(device, tc0480scp_get_bg3_tile_info, tilemap_scan_rows, 16, 16, 64, 32);
	tc0480scp->tilemap[4][1] = tilemap_create_device(device, tc0480scp_get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 64);

	for (i = 0; i < 2; i++)
	{
		tc0480scp->tilemap[0][i]->set_transparent_pen(0);
		tc0480scp->tilemap[1][i]->set_transparent_pen(0);
		tc0480scp->tilemap[2][i]->set_transparent_pen(0);
		tc0480scp->tilemap[3][i]->set_transparent_pen(0);
		tc0480scp->tilemap[4][i]->set_transparent_pen(0);
	}

	xd = -tc0480scp->x_offs;
	yd =  tc0480scp->y_offs;

	/* Metalb and Deadconx have minor screenflip issues: blue planet
       is off on x axis by 1 and in Deadconx the dark blue screen
       between stages also seems off by 1 pixel. */

	/* It's not possible to get the text scrolldx calculations
       harmonised with the other layers: xd-2, 315-xd is the
       next valid pair:- the numbers diverge from xd, 319-xd */

	/* Single width offsets */
	tc0480scp->tilemap[0][0]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[0][0]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[1][0]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[1][0]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[2][0]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[2][0]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[3][0]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[3][0]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[4][0]->set_scrolldx(xd - 3, 316 - xd);	/* text layer */
	tc0480scp->tilemap[4][0]->set_scrolldy(yd,     256 - yd);	/* text layer */

	/* Double width offsets */
	tc0480scp->tilemap[0][1]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[0][1]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[1][1]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[1][1]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[2][1]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[2][1]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[3][1]->set_scrolldx(xd,     320 - xd + tc0480scp->flip_xoffs);
	tc0480scp->tilemap[3][1]->set_scrolldy(yd,     256 - yd + tc0480scp->flip_yoffs);
	tc0480scp->tilemap[4][1]->set_scrolldx(xd - 3, 317 - xd);	/* text layer */
	tc0480scp->tilemap[4][1]->set_scrolldy(yd,     256 - yd);	/* text layer */

	for (i = 0; i < 2; i++)
	{
		/* Both sets of bg tilemaps scrollable per pixel row */
		tc0480scp->tilemap[0][i]->set_scroll_rows(512);
		tc0480scp->tilemap[1][i]->set_scroll_rows(512);
		tc0480scp->tilemap[2][i]->set_scroll_rows(512);
		tc0480scp->tilemap[3][i]->set_scroll_rows(512);
	}

	tc0480scp->ram = auto_alloc_array_clear(device->machine(), UINT16, TC0480SCP_RAM_SIZE / 2);

	tc0480scp_set_layer_ptrs(tc0480scp);

	/* create the char set (gfx will then be updated dynamically from RAM) */
	device->machine().gfx[tc0480scp->tx_gfx] = gfx_element_alloc(device->machine(), &tc0480scp_charlayout, (UINT8 *)tc0480scp->char_ram, 64, 0);

	device->save_pointer(NAME(tc0480scp->ram), TC0480SCP_RAM_SIZE / 2);
	device->save_item(NAME(tc0480scp->ctrl));
	device->save_item(NAME(tc0480scp->dblwidth));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(tc0480scp_postload), tc0480scp));
}

static DEVICE_RESET( tc0480scp )
{
	tc0480scp_state *tc0480scp = tc0480scp_get_safe_token(device);
	int i;

	tc0480scp->dblwidth = 0;

	for (i = 0; i < 0x18; i++)
		tc0480scp->ctrl[i] = 0;

}

/***************************************************************************/
/*                                                                         */
/*                              TC0150ROD                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0150rod_state tc0150rod_state;
struct _tc0150rod_state
{
	UINT16 *        ram;

	const char      *gfx_region;	/* gfx region for the road */
};

#define TC0150ROD_RAM_SIZE 0x2000

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0150rod_state *tc0150rod_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0150ROD);

	return (tc0150rod_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0150rod_interface *tc0150rod_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0150ROD));
	return (const tc0150rod_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ16_DEVICE_HANDLER( tc0150rod_word_r )
{
	tc0150rod_state *tc0150rod = tc0150rod_get_safe_token(device);
	return tc0150rod->ram[offset];
}

WRITE16_DEVICE_HANDLER( tc0150rod_word_w )
{
	tc0150rod_state *tc0150rod = tc0150rod_get_safe_token(device);
	COMBINE_DATA(&tc0150rod->ram[offset]);
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

void tc0150rod_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs, int palette_offs, int type, int road_trans, UINT32 low_priority, UINT32 high_priority )
{
	tc0150rod_state *tc0150rod = tc0150rod_get_safe_token(device);

#ifdef MAME_DEBUG
	static int dislayer[6];	/* Road Layer toggles to help get road correct */
#endif

	int x_offs = 0xa7;	/* Increasing this shifts road to right */
	UINT16 scanline[512];
	UINT16 roada_line[512], roadb_line[512];
	UINT16 *dst16;
	UINT16 *roada, *roadb;
	UINT16 *roadgfx = (UINT16 *)device->machine().root_device().memregion(tc0150rod->gfx_region)->base();

	UINT16 pixel, color, gfx_word;
	UINT16 roada_clipl, roada_clipr, roada_bodyctrl;
	UINT16 roadb_clipl, roadb_clipr, roadb_bodyctrl;
	UINT16 pri, pixpri;
	UINT8 priorities[6];
	int x_index, roadram_index, roadram2_index, i;
	int xoffset, paloffs, palloffs, palroffs;
	int road_gfx_tilenum, colbank, road_center;
	int road_ctrl = tc0150rod->ram[0xfff];
	int left_edge, right_edge, begin, end, right_over, left_over;
	int line_needs_drawing, draw_top_road_line, background_only;

	int min_x = cliprect.min_x;
	int max_x = cliprect.max_x;
	int min_y = cliprect.min_y;
	int max_y = cliprect.max_y;
	int screen_width = max_x - min_x + 1;

	int y = min_y;
#if 0
	int twin_road = 0;
#endif

	int road_A_address = y_offs * 4 + ((road_ctrl & 0x0300) << 2);	/* Index into roadram for road A */
	int road_B_address = y_offs * 4 + ((road_ctrl & 0x0c00) << 0);	/* Index into roadram for road B */

	int priority_switch_line = (road_ctrl & 0x00ff) - y_offs;

#ifdef MAME_DEBUG
	if (device->machine().input().code_pressed_once (KEYCODE_X))
	{
		dislayer[0] ^= 1;
		popmessage("RoadA body: %01x",dislayer[0]);
	}

	if (device->machine().input().code_pressed_once (KEYCODE_C))
	{
		dislayer[1] ^= 1;
		popmessage("RoadA l-edge: %01x",dislayer[1]);
	}

	if (device->machine().input().code_pressed_once (KEYCODE_V))
	{
		dislayer[2] ^= 1;
		popmessage("RoadA r-edge: %01x",dislayer[2]);
	}

	if (device->machine().input().code_pressed_once (KEYCODE_B))
	{
		dislayer[3] ^= 1;
		popmessage("RoadB body: %01x",dislayer[3]);
	}

	if (device->machine().input().code_pressed_once (KEYCODE_N))
	{
		dislayer[4] ^= 1;
		popmessage("RoadB l-edge: %01x",dislayer[4]);
	}
	if (device->machine().input().code_pressed_once (KEYCODE_M))
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

		for (i = 0; i < screen_width; i++)	/* Default transparency fill */
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

		roada_clipr    = tc0150rod->ram[roadram_index];
		roada_clipl    = tc0150rod->ram[roadram_index + 1];
		roada_bodyctrl = tc0150rod->ram[roadram_index + 2];
		roadb_clipr    = tc0150rod->ram[roadram2_index];
		roadb_clipl    = tc0150rod->ram[roadram2_index + 1];
		roadb_bodyctrl = tc0150rod->ram[roadram2_index + 2];

		/* Not very logical, but seems to work */
		if (roada_bodyctrl & 0x2000)	priorities[2] += 2;
		if (roadb_bodyctrl & 0x2000)	priorities[2] += 1;
		if (roada_clipl    & 0x2000)	priorities[3] -= 1;
		if (roadb_clipl    & 0x2000)	priorities[3] -= 2;
		if (roada_clipr    & 0x2000)	priorities[4] -= 1;
		if (roadb_clipr    & 0x2000)	priorities[4] -= 2;

		if (priorities[4] == 0)	priorities[4]++;	/* Fixes Aquajack LH edge dropping below background */

#if 0
		if ((roada_bodyctrl & 0x8000) || (roadb_bodyctrl & 0x8000))
			twin_road++;
#endif

		/********************************************************/
		/*                        ROAD A                        */
            /********************************************************/

		palroffs =(roada_clipr & 0x1000) >> 11;
		palloffs =(roada_clipl & 0x1000) >> 11;
		xoffset  = roada_bodyctrl & 0x7ff;
		paloffs  =(roada_bodyctrl & 0x1800) >> 11;
		colbank  =(tc0150rod->ram[roadram_index + 3] & 0xf000) >> 10;
		road_gfx_tilenum = tc0150rod->ram[roadram_index + 3] & 0x3ff;
		right_over = 0;
		left_over = 0;

		road_center = 0x5ff - ((-xoffset + x_offs) & 0x7ff);
		left_edge = road_center - (roada_clipl & 0x3ff);		/* start pixel for left edge */
		right_edge = road_center + 1 + (roada_clipr & 0x3ff);	/* start pixel for right edge */

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
		x_index = (-xoffset + x_offs + begin) & 0x7ff;

		roada = roada_line + screen_width - 1 - begin;

		if ((line_needs_drawing) && (begin < end))
		{
			for (i = begin; i < end; i++)
			{
				if (road_gfx_tilenum)	/* fixes Nightstr round C */
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					if ((pixel) || !(road_trans))
					{
						if (type)	pixel = (pixel - 1) & 3;
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
			if (roada_clipl & 0x8000)	/* but we may need to fill in the background color */
			{
				roada = roada_line;
				for (i = 0; i < screen_width; i++)
				{
					*roada++ = (color + (type ? (3) : (0)));
				}
			}
		}
		else
		{
			if ((left_edge >= 0) && (left_edge < screen_width))
			{
				x_index = (1024 / 2 - 1 - left_over) & 0x7ff;

				roada = roada_line + screen_width - 1 - left_edge;

				if (line_needs_drawing)
				{
					for (i = left_edge; i >= 0; i--)
					{
						gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
						pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

						pixpri = (pixel == 0) ? (0) : (pri);	/* off edge has low priority */

						if ((pixel == 0) && !(roada_clipl & 0x8000))
						{
							roada++;
						}
						else
						{
							if (type)	pixel = (pixel - 1)&3;
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
			x_index = (1024 / 2 + right_over) & 0x7ff;

			roada = roada_line + screen_width - 1 - right_edge;

			if (line_needs_drawing)
			{
				for (i = right_edge; i < screen_width; i++)
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					pixpri = (pixel == 0) ? (0) : (pri);	/* off edge has low priority */

					if ((pixel == 0) && !(roada_clipr & 0x8000))
					{
						roada--;
					}
					else
					{
						if (type)	pixel = (pixel - 1) & 3;
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

		palroffs = (roadb_clipr & 0x1000) >> 11;
		palloffs = (roadb_clipl & 0x1000) >> 11;
		xoffset  =  roadb_bodyctrl & 0x7ff;
		paloffs  = (roadb_bodyctrl & 0x1800) >> 11;
		colbank  = (tc0150rod->ram[roadram2_index + 3] & 0xf000) >> 10;
		road_gfx_tilenum = tc0150rod->ram[roadram2_index + 3] & 0x3ff;
		right_over = 0;
		left_over = 0;

		road_center = 0x5ff - ((-xoffset + x_offs) & 0x7ff);

// ChaseHQ glitches on right when road rejoins:
// de7, de8 causes problems => 5e7/e8
// 5ff - (a7 - 5e7)
// 5ff - 2c0 = 33f / 340 which is not quite > screenwidth + 1024/2: so we subtract 2 more, fixed


// ChaseHQ glitches on right when road rejoins:
// 0a6 and lower => 0x5ff 5fe etc.
// 35c => 575 right road edge wraps back onto other side of screen
// 5ff-54a       through    5ff-331
// b6            through    2ce
// 2a6 through 0 through    5a7 ??

		left_edge = road_center - (roadb_clipl & 0x3ff);		/* start pixel for left edge */
		right_edge = road_center + 1 + (roadb_clipr & 0x3ff);	/* start pixel for right edge */

		if (((roadb_clipl) || (roadb_clipr)) && ((road_ctrl & 0x800) || (type == 2)))
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
		x_index = (-xoffset + x_offs + begin) & 0x7ff;

		if (x_index > 0x3ff)	/* Second half of gfx contains the road body line */
		{
			roadb = roadb_line + screen_width - 1 - begin;

			if (draw_top_road_line && road_gfx_tilenum && (begin < end))
			{
				for (i = begin; i < end; i++)
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					if ((pixel) || !(road_trans))
					{
						if (type)	pixel = (pixel - 1) & 3;
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
			if ((roadb_clipl & 0x8000) && draw_top_road_line)	/* but we may need to fill in the background color */
			{
				roadb = roadb_line;
				for (i = 0; i < screen_width; i++)
				{
					*roadb++ = (color + (type ? (3) : (0)));
				}
			}
		}
		else
		{
			if ((left_edge >= 0) && (left_edge < screen_width))
			{
				x_index = (1024 / 2 - 1 - left_over) & 0x7ff;

				roadb = roadb_line + screen_width - 1 - left_edge;

				if (draw_top_road_line)		// rename to draw_roadb_line !?
				{
					for (i = left_edge; i >= 0; i--)
					{
						gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
						pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

						pixpri = (pixel == 0) ? (0) : (pri);	/* off edge has low priority */

						if ((pixel == 0) && !(roadb_clipl & 0x8000))	/* test for background disabled */
						{
							roadb++;
						}
						else
						{
							if (type)	pixel = (pixel - 1) & 3;
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
			x_index = (1024 / 2 + right_over) & 0x7ff;

			roadb = roadb_line + screen_width - 1 - right_edge;

			if (draw_top_road_line)
			{
				for (i = right_edge; i < screen_width; i++)
				{
					gfx_word = roadgfx[(road_gfx_tilenum << 8) + (x_index >> 3)];
					pixel = ((gfx_word >> (7 - (x_index % 8) + 8)) & 0x1) * 2 + ((gfx_word >> (7 - (x_index % 8))) & 0x1);

					pixpri = (pixel == 0) ? (0) : (pri);	/* off edge has low priority */

					if ((pixel == 0) && !(roadb_clipr & 0x8000))	/* test for background disabled */
					{
						roadb--;
					}
					else
					{
						if (type)	pixel = (pixel - 1) & 3;
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

			for (i = 0; i < screen_width; i++)
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

			taitoic_drawscanline(bitmap, cliprect, 0, y, scanline, 1, ROT0, device->machine().priority_bitmap, (y > priority_switch_line) ? high_priority : low_priority);
		}

		y++;
	}
	while (y <= max_y);

#if 0
	if (twin_road)	// I don't know what this means, actually...
	{
		char buf2[80];
		sprintf(buf2, "Road twinned for %04x lines", twin_road);
		popmessage(buf2);
	}
#endif
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0150rod )
{
	tc0150rod_state *tc0150rod = tc0150rod_get_safe_token(device);
	const tc0150rod_interface *intf = tc0150rod_get_interface(device);

	tc0150rod->gfx_region = intf->gfx_region;

	tc0150rod->ram = auto_alloc_array(device->machine(), UINT16, TC0150ROD_RAM_SIZE / 2);

	device->save_pointer(NAME(tc0150rod->ram), TC0150ROD_RAM_SIZE / 2);

}


/***************************************************************************/
/*                                                                         */
/*                              TC0110PCR                                  */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0110pcr_state tc0110pcr_state;
struct _tc0110pcr_state
{
	running_machine &machine() const { assert(m_machine != NULL); return *m_machine; }
	UINT16 *     ram;
	int          type;
	int          addr;
	int          pal_offs;
	running_machine *m_machine;
};

#define TC0110PCR_RAM_SIZE 0x2000

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0110pcr_state *tc0110pcr_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0110PCR);

	return (tc0110pcr_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0110pcr_interface *tc0110pcr_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0110PCR));
	return (const tc0110pcr_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static void tc0110pcr_restore_colors(tc0110pcr_state *tc0110pcr)
{
	int i, color, r = 0, g = 0, b = 0;

	for (i = 0; i < (256 * 16); i++)
	{
		color = tc0110pcr->ram[i];

		switch (tc0110pcr->type)
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

		palette_set_color(tc0110pcr->machine(), i + (tc0110pcr->pal_offs << 12), MAKE_RGB(r, g, b));
	}
}


READ16_DEVICE_HANDLER( tc0110pcr_word_r )
{
	tc0110pcr_state *tc0110pcr = tc0110pcr_get_safe_token(device);

	switch (offset)
	{
		case 1:
			return tc0110pcr->ram[tc0110pcr->addr];

		default:
//logerror("PC %06x: warning - read TC0110PCR address %02x\n",cpu_get_pc(&space->device()),offset);
			return 0xff;
	}
}

WRITE16_DEVICE_HANDLER( tc0110pcr_word_w )
{
	tc0110pcr_state *tc0110pcr = tc0110pcr_get_safe_token(device);

	switch (offset)
	{
		case 0:
			/* In test mode game writes to odd register number so (data>>1) */
			tc0110pcr->addr = (data >> 1) & 0xfff;
			if (data > 0x1fff)
				logerror ("Write to palette index > 0x1fff\n");
			break;

		case 1:
			tc0110pcr->ram[tc0110pcr->addr] = data & 0xffff;
			palette_set_color_rgb(device->machine(), tc0110pcr->addr, pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
			break;

		default:
//logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",cpu_get_pc(&space->device()),data,offset);
			break;
	}
}

WRITE16_DEVICE_HANDLER( tc0110pcr_step1_word_w )
{
	tc0110pcr_state *tc0110pcr = tc0110pcr_get_safe_token(device);

	switch (offset)
	{
		case 0:
			tc0110pcr->addr = data & 0xfff;
			if (data > 0xfff)
				logerror ("Write to palette index (color area %d) > 0xfff\n", tc0110pcr->pal_offs);
			break;

		case 1:
			tc0110pcr->ram[tc0110pcr->addr] = data & 0xffff;
			palette_set_color_rgb(device->machine(), tc0110pcr->addr + (tc0110pcr->pal_offs << 12), pal5bit(data >> 0), pal5bit(data >> 5), pal5bit(data >> 10));
			break;

		default:
//logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",cpu_get_pc(&space->device()),data,offset);
			break;
	}
}

WRITE16_DEVICE_HANDLER( tc0110pcr_step1_rbswap_word_w )
{
	tc0110pcr_state *tc0110pcr = tc0110pcr_get_safe_token(device);

	tc0110pcr->type = 1;	/* xRRRRRGGGGGBBBBB */

	switch (offset)
	{
		case 0:
			tc0110pcr->addr = data & 0xfff;
			if (data > 0xfff)
				logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
			tc0110pcr->ram[tc0110pcr->addr] = data & 0xffff;
			palette_set_color_rgb(device->machine(), tc0110pcr->addr, pal5bit(data >> 10), pal5bit(data >> 5), pal5bit(data >> 0));
			break;

		default:
//logerror("PC %06x: warning - write %04x to TC0110PCR offset %02x\n",cpu_get_pc(&space->device()),data,offset);
			break;
	}
}

WRITE16_DEVICE_HANDLER( tc0110pcr_step1_4bpg_word_w )
{
	tc0110pcr_state *tc0110pcr = tc0110pcr_get_safe_token(device);

	tc0110pcr->type = 2;	/* xxxxBBBBGGGGRRRR */

	switch (offset)
	{
		case 0:
			tc0110pcr->addr = data & 0xfff;
			if (data > 0xfff)
				logerror ("Write to palette index > 0xfff\n");
			break;

		case 1:
			tc0110pcr->ram[tc0110pcr->addr] = data & 0xffff;
			palette_set_color_rgb(device->machine(), tc0110pcr->addr, pal4bit(data >> 0), pal4bit(data >> 4), pal4bit(data >> 8));
			break;

		default:
//logerror("PC %06x: warning - write %04x to TC0110PCR address %02x\n",cpu_get_pc(&space->device()),data,offset);
			break;
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0110pcr )
{
	tc0110pcr_state *tc0110pcr = tc0110pcr_get_safe_token(device);
	const tc0110pcr_interface *intf = tc0110pcr_get_interface(device);

	tc0110pcr->m_machine = &device->machine();

	tc0110pcr->pal_offs = intf->pal_offs;

	tc0110pcr->ram = auto_alloc_array(device->machine(), UINT16, TC0110PCR_RAM_SIZE);

	device->save_pointer(NAME(tc0110pcr->ram), TC0110PCR_RAM_SIZE);
	device->save_item(NAME(tc0110pcr->type));
	device->machine().save().register_postload(save_prepost_delegate(FUNC(tc0110pcr_restore_colors), tc0110pcr));
}

static DEVICE_RESET( tc0110pcr )
{
	tc0110pcr_state *tc0110pcr =  tc0110pcr_get_safe_token(device);
	tc0110pcr->type = 0;	/* default, xBBBBBGGGGGRRRRR */
}


/***************************************************************************/
/*                                                                         */
/*                                TC0180VCU                                */
/*                                                                         */
/***************************************************************************/

typedef struct _tc0180vcu_state tc0180vcu_state;
struct _tc0180vcu_state
{
	UINT16         ctrl[0x10];

	UINT16 *       ram;
	UINT16 *       scrollram;

	tilemap_t        *tilemap[3];

	UINT16         bg_rambank[2], fg_rambank[2], tx_rambank;
	UINT8          framebuffer_page;
	UINT8          video_control;

	int            bg_color_base;
	int            fg_color_base;
	int            tx_color_base;
};

#define TC0180VCU_RAM_SIZE          0x10000
#define TC0180VCU_SCROLLRAM_SIZE    0x0800

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0180vcu_state *tc0180vcu_get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0180VCU);

	return (tc0180vcu_state *)downcast<legacy_device_base *>(device)->token();
}

INLINE const tc0180vcu_interface *tc0180vcu_get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0180VCU));
	return (const tc0180vcu_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

/* TC0180VCU control registers:
* offset:
* 0 - -----xxx bg ram page 0 (tile codes)
*     -xxx---- bg ram page 1 (attributes)
* 1 - -----xxx fg ram page 0 (tile codes)
*     -xxx---- fg ram page 1 (attributes)
* 2 - xxxxxxxx number of independent foreground scrolling blocks (see below)
* 3 - xxxxxxxx number of independent background scrolling blocks
* 4 - --xxxxxx text tile bank 0
* 5 - --xxxxxx text tile bank 1
* 6 - ----xxxx text ram page
* 7 - xxxxxxxx video control: pixelram page and enable, screen flip, sprite to foreground priority (see below)
* 8 to f - unused (always zero)
*
******************************************************************************************
*
* offset 6 - text video page register:
*            This location controls which page of video text ram to view
* hitice:
*     0x08 (00001000) - show game text: credits XX, player1 score
*     0x09 (00001001) - show FBI logo
* rambo3:
*     0x08 (00001000) - show game text
*     0x09 (00001001) - show taito logo
*     0x0a (00001010) - used in pair with 0x09 to smooth screen transitions (attract mode)
*
* Is bit 3 (0x08) video text enable/disable ?
*
******************************************************************************************
*
* offset 7 - video control register:
*            bit 0 (0x01) 1 = don't erase sprite frame buffer "after the beam"
*            bit 3 (0x08) sprite to foreground priority
*                         1 = bg, fg, obj, tx
*                         0 = bg, obj1, fg, obj0, tx (obj0/obj1 selected by bit 0 of color code)
*            bit 4 (0x10) screen flip (active HI) (this one is for sure)
*            bit 5 (0x20) could be global video enable switch (Hit the Ice clears this
*                         bit, clears videoram portions and sets this bit)
*            bit 6 (0x40) frame buffer page to show when bit 7 is set
*            bit 7 (0x80) don't flip frame buffer every vblank, use the page selected by bit 6
*
*/

READ8_DEVICE_HANDLER( tc0180vcu_get_fb_page )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	return tc0180vcu->framebuffer_page;
}

WRITE8_DEVICE_HANDLER( tc0180vcu_set_fb_page )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	tc0180vcu->framebuffer_page = data;
}

READ8_DEVICE_HANDLER( tc0180vcu_get_videoctrl )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	return tc0180vcu->video_control;
}

static void tc0180vcu_video_control( device_t *device, UINT8 data )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
#if 0
	if (data != tc0180vcu->video_control)
		popmessage("video control = %02x", data);
#endif

	tc0180vcu->video_control = data;

	if (tc0180vcu->video_control & 0x80)
		tc0180vcu->framebuffer_page = (~tc0180vcu->video_control & 0x40) >> 6;

	device->machine().tilemap().set_flip_all((tc0180vcu->video_control & 0x10) ? (TILEMAP_FLIPX | TILEMAP_FLIPY) : 0 );
}

READ16_DEVICE_HANDLER( tc0180vcu_ctrl_r )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	return tc0180vcu->ctrl[offset];
}

WRITE16_DEVICE_HANDLER( tc0180vcu_ctrl_w )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	UINT16 oldword = tc0180vcu->ctrl[offset];

	COMBINE_DATA (&tc0180vcu->ctrl[offset]);

	if (oldword != tc0180vcu->ctrl[offset])
	{
		if (ACCESSING_BITS_8_15)
		{
			switch(offset)
			{
			case 0:
				tc0180vcu->tilemap[1]->mark_all_dirty();
				tc0180vcu->fg_rambank[0] = (((tc0180vcu->ctrl[offset] >> 8) & 0x0f) << 12);
				tc0180vcu->fg_rambank[1] = (((tc0180vcu->ctrl[offset] >> 12) & 0x0f) << 12);
				break;
			case 1:
				tc0180vcu->tilemap[0]->mark_all_dirty();
				tc0180vcu->bg_rambank[0] = (((tc0180vcu->ctrl[offset] >> 8) & 0x0f) << 12);
				tc0180vcu->bg_rambank[1] = (((tc0180vcu->ctrl[offset] >> 12) & 0x0f) << 12);
				break;
			case 4:
			case 5:
				tc0180vcu->tilemap[2]->mark_all_dirty();
				break;
			case 6:
				tc0180vcu->tilemap[2]->mark_all_dirty();
				tc0180vcu->tx_rambank = (((tc0180vcu->ctrl[offset] >> 8) & 0x0f) << 11);
				break;
			case 7:
				tc0180vcu_video_control(device, (tc0180vcu->ctrl[offset] >> 8) & 0xff);
				break;
			default:
				break;
			}
		}
	}
}

static TILE_GET_INFO_DEVICE( get_bg_tile_info )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	int tile  = tc0180vcu->ram[tile_index + tc0180vcu->bg_rambank[0]];
	int color = tc0180vcu->ram[tile_index + tc0180vcu->bg_rambank[1]];

	SET_TILE_INFO_DEVICE(
		1,
		tile,
		tc0180vcu->bg_color_base + (color & 0x3f),
		TILE_FLIPYX((color & 0x00c0) >> 6));
}

static TILE_GET_INFO_DEVICE( get_fg_tile_info )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	int tile  = tc0180vcu->ram[tile_index + tc0180vcu->fg_rambank[0]];
	int color = tc0180vcu->ram[tile_index + tc0180vcu->fg_rambank[1]];

	SET_TILE_INFO_DEVICE(
		1,
		tile,
		tc0180vcu->fg_color_base + (color & 0x3f),
		TILE_FLIPYX((color & 0x00c0) >> 6));
}

static TILE_GET_INFO_DEVICE( get_tx_tile_info )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	int tile = tc0180vcu->ram[tile_index + tc0180vcu->tx_rambank];

	SET_TILE_INFO_DEVICE(
		0,
		(tile & 0x07ff) | ((tc0180vcu->ctrl[4 + ((tile & 0x800) >> 11)]>>8) << 11),
		tc0180vcu->tx_color_base + ((tile >> 12) & 0x0f),
		0);
}

READ16_DEVICE_HANDLER( tc0180vcu_scroll_r )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	return tc0180vcu->scrollram[offset];
}

WRITE16_DEVICE_HANDLER( tc0180vcu_scroll_w )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	COMBINE_DATA(&tc0180vcu->scrollram[offset]);
}

READ16_DEVICE_HANDLER( tc0180vcu_word_r )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	return tc0180vcu->ram[offset];
}

WRITE16_DEVICE_HANDLER( tc0180vcu_word_w )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	COMBINE_DATA(&tc0180vcu->ram[offset]);

	if ((offset & 0x7000) == tc0180vcu->fg_rambank[0] || (offset & 0x7000) == tc0180vcu->fg_rambank[1])
		tc0180vcu->tilemap[1]->mark_tile_dirty(offset & 0x0fff);

	if ((offset & 0x7000) == tc0180vcu->bg_rambank[0] || (offset & 0x7000) == tc0180vcu->bg_rambank[1])
		tc0180vcu->tilemap[0]->mark_tile_dirty(offset & 0x0fff);

	if ((offset & 0x7800) == tc0180vcu->tx_rambank)
		tc0180vcu->tilemap[2]->mark_tile_dirty(offset & 0x7ff);
}

void tc0180vcu_tilemap_draw( device_t *device, bitmap_ind16 &bitmap, const rectangle &cliprect, int tmap_num, int plane )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);

	assert(tmap_num < 3);

	if (tmap_num == 2)
		tc0180vcu->tilemap[2]->draw(bitmap, cliprect, 0, 0);	/* not much to do for tx_tilemap */
	else
	{
		/*plane = 0 fg tilemap*/
		/*plane = 1 bg tilemap*/
		rectangle my_clip;
		int i;
		int scrollx, scrolly;
		int lines_per_block;	/* number of lines scrolled by the same amount (per one scroll value) */
		int number_of_blocks;	/* number of such blocks per _screen_ (256 lines) */

		lines_per_block = 256 - (tc0180vcu->ctrl[2 + plane] >> 8);
		number_of_blocks = 256 / lines_per_block;

		my_clip.min_x = cliprect.min_x;
		my_clip.max_x = cliprect.max_x;

		for (i = 0; i < number_of_blocks; i++)
		{
			scrollx = tc0180vcu->scrollram[plane * 0x200 + i * 2 * lines_per_block];
			scrolly = tc0180vcu->scrollram[plane * 0x200 + i * 2 * lines_per_block + 1];

			my_clip.min_y = i * lines_per_block;
			my_clip.max_y = (i + 1) * lines_per_block - 1;

			if (tc0180vcu->video_control & 0x10)   /*flip screen*/
			{
				my_clip.min_y = bitmap.height() - 1 - (i + 1) * lines_per_block - 1;
				my_clip.max_y = bitmap.height() - 1 - i * lines_per_block;
			}

			my_clip &= cliprect;

			if (my_clip.min_y <= my_clip.max_y)
			{
				tc0180vcu->tilemap[tmap_num]->set_scrollx(0, -scrollx);
				tc0180vcu->tilemap[tmap_num]->set_scrolly(0, -scrolly);
				tc0180vcu->tilemap[tmap_num]->draw(bitmap, my_clip, 0, 0);
			}
		}
	}
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0180vcu )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	const tc0180vcu_interface *intf = tc0180vcu_get_interface(device);

	tc0180vcu->bg_color_base = intf->bg_color_base;
	tc0180vcu->fg_color_base = intf->fg_color_base;
	tc0180vcu->tx_color_base = intf->tx_color_base;

	tc0180vcu->tilemap[0] = tilemap_create_device(device, get_bg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	tc0180vcu->tilemap[1] = tilemap_create_device(device, get_fg_tile_info, tilemap_scan_rows, 16, 16, 64, 64);
	tc0180vcu->tilemap[2] = tilemap_create_device(device, get_tx_tile_info, tilemap_scan_rows, 8, 8, 64, 32);

	tc0180vcu->tilemap[1]->set_transparent_pen(0);
	tc0180vcu->tilemap[2]->set_transparent_pen(0);

	tc0180vcu->tilemap[0]->set_scrolldx(0, 24 * 8);
	tc0180vcu->tilemap[1]->set_scrolldx(0, 24 * 8);
	tc0180vcu->tilemap[2]->set_scrolldx(0, 24 * 8);

	tc0180vcu->ram = auto_alloc_array_clear(device->machine(), UINT16, TC0180VCU_RAM_SIZE / 2);
	tc0180vcu->scrollram = auto_alloc_array_clear(device->machine(), UINT16, TC0180VCU_SCROLLRAM_SIZE / 2);

	device->save_pointer(NAME(tc0180vcu->ram), TC0180VCU_RAM_SIZE / 2);
	device->save_pointer(NAME(tc0180vcu->scrollram), TC0180VCU_SCROLLRAM_SIZE / 2);

	device->save_item(NAME(tc0180vcu->bg_rambank));
	device->save_item(NAME(tc0180vcu->fg_rambank));
	device->save_item(NAME(tc0180vcu->tx_rambank));

	device->save_item(NAME(tc0180vcu->framebuffer_page));

	device->save_item(NAME(tc0180vcu->video_control));
	device->save_item(NAME(tc0180vcu->ctrl));
}

static DEVICE_RESET( tc0180vcu )
{
	tc0180vcu_state *tc0180vcu = tc0180vcu_get_safe_token(device);
	int i;

	for (i = 0; i < 0x10; i++)
		tc0180vcu->ctrl[i] = 0;

	tc0180vcu->bg_rambank[0] = 0;
	tc0180vcu->bg_rambank[1] = 0;
	tc0180vcu->fg_rambank[0] = 0;
	tc0180vcu->fg_rambank[1] = 0;
	tc0180vcu->tx_rambank = 0;

	tc0180vcu->framebuffer_page = 0;
	tc0180vcu->video_control = 0;
}


/***************************************************************************/
/*                                                                         */
/*                         DEVICE_GET_INFOs                                */
/*                                                                         */
/***************************************************************************/

DEVICE_GET_INFO( pc080sn )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(pc080sn_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(pc080sn);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito PC080SN");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( pc090oj )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(pc090oj_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(pc090oj);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(pc090oj);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito PC090OJ");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0080vco )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0080vco_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0080vco);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0080VCO");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0110pcr )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0110pcr_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0110pcr);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0110pcr);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0110PCR");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0100scn )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0100scn_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0100scn);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0100scn);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0100SCN");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0280grd )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0280grd_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0280grd);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0280grd);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0280GRD & TC0430GRW");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0360pri )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0360pri_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0360pri);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0360pri);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0360PRI");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0480scp )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0480scp_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0480scp);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0480scp);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0480SCP");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0150rod )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0150rod_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0150rod);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					/* Nothing */									break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0150ROD");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}

DEVICE_GET_INFO( tc0180vcu )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case DEVINFO_INT_TOKEN_BYTES:			info->i = sizeof(tc0180vcu_state);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case DEVINFO_FCT_START:					info->start = DEVICE_START_NAME(tc0180vcu);		break;
		case DEVINFO_FCT_STOP:					/* Nothing */									break;
		case DEVINFO_FCT_RESET:					info->reset = DEVICE_RESET_NAME(tc0180vcu);		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:					strcpy(info->s, "Taito TC0180VCU");				break;
		case DEVINFO_STR_FAMILY:				strcpy(info->s, "Taito Video IC");					break;
		case DEVINFO_STR_VERSION:				strcpy(info->s, "1.0");							break;
		case DEVINFO_STR_SOURCE_FILE:			strcpy(info->s, __FILE__);						break;
		case DEVINFO_STR_CREDITS:				strcpy(info->s, "Copyright MAME Team");			break;
	}
}


DEFINE_LEGACY_DEVICE(PC080SN, pc080sn);
DEFINE_LEGACY_DEVICE(PC090OJ, pc090oj);
DEFINE_LEGACY_DEVICE(TC0080VCO, tc0080vco);
DEFINE_LEGACY_DEVICE(TC0100SCN, tc0100scn);
DEFINE_LEGACY_DEVICE(TC0280GRD, tc0280grd);
DEFINE_LEGACY_DEVICE(TC0360PRI, tc0360pri);
DEFINE_LEGACY_DEVICE(TC0480SCP, tc0480scp);
DEFINE_LEGACY_DEVICE(TC0150ROD, tc0150rod);
DEFINE_LEGACY_DEVICE(TC0110PCR, tc0110pcr);
DEFINE_LEGACY_DEVICE(TC0180VCU, tc0180vcu);
