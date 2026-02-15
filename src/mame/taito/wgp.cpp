// license:BSD-3-Clause
// copyright-holders:David Graves
// thanks-to:Richard Bush

/***************************************************************************

WGP    (c) Taito Corporation 1989
===

David Graves

(Thanks to Richard Bush and the Raine team for their
preliminary driver.)

It seems likely there are a LOT of undumped versions of this game...
If you have access to a board, please check the ROM numbers to see if
any are different from the ones listed below.

                *****

WGP runs on hardware which is pretty different from the system Taito
commonly used for their pseudo-3d racing games of the time, the Z system.
Different screen and sprite hardware is used. There's also a LAN hookup
(for multiple machines).

As well as a TC0100SCN tilemap generator (two 64x64 layers of 8x8
tiles and a layer of 8x8 tiles with graphics data taken from RAM)
there's a "piv" tilemap generator, which creates three scrollable
row-scrollable zoomable 64x64 tilemaps composed of 16x16 tiles.

As well as these six tilemap layers, there is a sprite layer with
individually zoomable / rotatable sprites. Big sprites are created
from 16x16 gfx chunks via a sprite mapping area in RAM.

The piv and sprite layers are rotatable (but not individually, only
together).

WGP has twin 68K CPUs which communicate via $4000 bytes of shared RAM.

There is a Z80 as well, which takes over sound duties. Commands are
written to it by the one of the 68000s (the same as Taito F2 games).

Dumper's info (Raine)
-------------

    TC0100SCN - known
    TC0140SYT - known
    TC0170ABT - ?
    TC0220IOC - known
    TC0240PBJ - motion objects ??
    TC0250SCR - piv tilemaps ?? [TC0280GRD was a rotatable tilemap chip in DonDokoD, also 1989]
    TC0260DAR - color related, paired with TC0360PRI in many F2 games
    TC0330CHL - ? (perhaps LAN related) [This game has LAN interface board, it uses uPD72105C.]

    2*MC68000P12F, Z80-A, 16MHz & 26.686MHz XTALs

Video Map
---------

400000 area is used for sprites. The game does tile mapping
    in RAM to create big sprites from the 16x16 tiles of gfx0.
    See notes in \video\ for details.

500000 area is for the "piv" tilemaps.
    See notes in \video\ for details.


TODO
====

Offer fake-dip selectable analogue steer

Is piv/sprite layers rotation control at 0x600000 ?

Verify y-zoom is correct on the stages that use it (including WGP 2
default course). Row zoom may be hard to verify, but WGP 2 course
selection screen is probably a good test.

Implement proper positioning/zoom/rotation for sprites.

(The current sprite coord calculations are kludgy and flawed and
ignore four control words. The sprites also seem jerky
and have [int?] timing glitches.)

DIP coinage


WGP
---

Analogue brake pedal works but won't register in service mode.

$28336 is code that shows brake value in service mode.
$ac3e sub (called off int4) at $ac78 does three calcs to the six
    AD values: results are stored at ($d22 / $d24 / $d26,A5)
    It is the third one, ($d26,A5) which gets displayed
    in service mode as brake.


WGP 2
-----

Piv y zoom may be imperfect. Check the up/down hill part of the
default course. The road looks a little odd.

Sprite colors seem ok except smoke after you crash. (And one sign on
first bend of default course doesn't go yellow for a few frames.)

[Used to die with common RAM error. When CPUA enables CPUB, CPUB
writes to $140000/2 - unfortunately while CPUA is in the middle of
testing that RAM. We hack prog for CPUB to disable the writes.]


                *****

[WGP stopped with LAN error. (Looks like CPUB tells CPUA what is wrong
with LAN in shared RAM $142048. Examined at $e57c which prints out
relevant LAN error message). Ended up at $e57c from $b14e-xx code
section. CPUA does PEA of $e57c which is the fallback if CPUB doesn't
respond in timely fashion to command 0x1 poked by code at $b1a6.

CPUB $830c-8332 loops waiting for command 1-8 in $140002 from CPUA.
it executes this then clears $140002 to indicate command completed.
It wasn't clearing this fast enough for CPUA, because $142048 wasn't
clear:- and that affects the amount of code command 0x1 runs.

CPUB code at $104d8 had already detected error on LAN and set $142048
to 4. We now return correct LAN status read from $380000, so
no LAN problem is detected.]


Code Documentation
------------------

CPUA
----

$1064e main game loop starting with a trap#$5

Calls $37e78 sub off which spriteram eventually gets updated

Strangely main loop does not seem to be synced to any interrupt.
This may be why the sprites are so glitchy and don't seem to
update every frame. Maybe trap#$5 should be getting us to a known
point in the frame ??

$21f4 copies sprite tilemapping data from data ROM to tilemap
area then flows into:

$223c code that fills a sprite entry in spriteram

[$12770 - $133cd seems to be an irregular lookup table used to
calculate extra zoom word poked into each sprite entry.]

$23a8 picks data out of data ROM and stores it to sprite tile-
mapping area by heading through to $21f4. (May also enter sprite
details into spriteram.) It uses $a0000 area of data ROM.

(Alternate entry point at $23c2 uses $90000 area of data ROM.)

$25ee routine stores data from ROM into sprite tilemapping area
including the bad parts that produce junk sprites.

It calls interesting sub at $25be which has a table of the number
of sequential words to be pulled out of the data ROM, depending
on the first word value in the data ROM for that entry ("code").
Each code will pull out the following multiple of 16 words:

    Code  Words   Tiles    Actual data ROM entry
     0      1      4x4      [same]
     1      2      8x4      [same]
     2      4      8x8      [same]
     3      3      12x4     [same, see $98186]
     4      6      12x8
     5      9      12x12    [same]
     6      2      4x8      [WRONG! says 8x12 in data ROM, see $982bf]
     7      6      8x12     [WRONG! says 4x8 in data ROM]
     8      1      (2x2)*4  [2x2 in data ROM]  (copies 12 unwanted
                                      words - causing junk sprites)

$4083c0-47f in sprite mapping area has junk words - due to code 7
making it read 6*16 words. 0x60 words are copied from the data ROM
when 0x20 would be correct. Careless programming: in the lookup
table Taito got codes 6 and 7 back to front. Enable the patch in
init_wgp to correct this... I can't see what changes.

I'm guessing sprites may be variable size, and the junk sprites
mapped +0x9b80-9d80 are 2x2 tiles rather than 4x4.

If we only use the first 4 tilemapping words, then the junk sprites
look fine. But their spacing is bad: they have gaps left between
them. They'll need to be magnified to 2x size - the pixel zoom
value must do this automatically.

This ties in with the lookup table we need to draw the sprites:
it makes sense if our standard 4x4 tile sprite is actually 4 2x2
sprites.

But what tells the sprite hardware if a sprite is 2x2 or 4x4 ??


Data ROM entry
--------------

+0x00  Control word
        (determines how many words copied to tilemapping area)

+0x01  Sprite X size  [tiles]
        (2,4,8 or 12)

+0x02  Sprite Y size  [tiles]
        (2,4,8 or 12)

+0x03  sprite tile words, total (X size * Y size)
 on...

The X size and Y size values don't seem to be used by the game, and
may be a hangover from the gfx development system used.


Data Rom
--------

$80000-$8ffff   piv tilemaps, preceded by lookup table
$90000-$9ffff   sprite tilemap data, preceded by lookup table
$a0000-$affff   sprite tilemap data, preceded by lookup table
$b0000-$cffff   TC0100SCN tilemaps, preceded by lookup table

    Note that only just over half this space is used, rest is
    0xff fill.

$d0000-$dffff is the pivot port data table

    Four separate start points are available, contained in the
    first 4 long words.

    (Data words up to $da410 then 0xff fill)

$e0000-$e7ffe is a logarithmic curve table

    This is used to look up perceived height of an object at
    "distance" (offset/2).

    ffff 8000 5555 4000 3333 2aaa  etc.
    (tapering to value of 4 towards the end)

    The sprite routine shifts this one bit right and subtracts one
    before poking into spriteram. Hence 4 => 1

$e7fff-$e83fe is an unknown word table
$e83ff-$effff empty (0xff fill)
$f0000-$f03ff unknown lookup table
$f0400-$fcbff series of unknown lookup tables

    Seems to be a series of (zoom/rotate ??) lookup
    tables all $400 words long. (Total no. of tables = 25?)

    Each table starts at zero and tends upwards: the first reaches 0xfe7.
    The final one reaches 0x3ff (i.e. each successive word is 1 higher
    than the last). The values in the tables tend to go up smoothly but
    with discontinuities at regular intervals. The intervals change
    between tables.

$fcc00-$fffff empty (0xff fill)


Additional notes :

1) 'wgp' and 'wgpj'

LAN stuff :

LAN RAM seems to be 0x4000 bytes wide (0x380000-0x383fff in CPUB)

Lan tests start at 0x00f86a (CPUB) where a copy of the 256 bytes from
0x00f8d4 is made to 0x383f00. This is text about the version of the LAN
stuff ("1990 VER 1.06") . Note that at least version 1.05 is required.

Dip Switches :

To see the effect of the "Communication" Dip Switch you must add the memory
read/write handlers for 0x380000 to 0x383fff instead of using the 'lan_status_r'
one. Of course, there will be an error message, but this might help in
finding the useful addresses in the LAN RAM.

Note that the first time you run the game with the new handlers (I've put
standard RAM, let me know if you have a better idea), you'll need to reset
the game ! Is it because the tests in CPUB are too late ?

In the "test mode", if "Communication" Dip Switch is ON, you'll see "NG"
("Not Good") under each machine ID (this is logical).

Be aware that the "Machine ID" Dip Switch must be set to 1, or the tests are
NOT performed (I can't explain why for the moment) !

2) 'wgpjoy*'

LAN stuff :

It is very surprising, but you also find the text about the LAN stuff in
the ROMS (still version 1.06), but you can't perform LAN tests in the
"test mode".

As the LAN version is the same, I'll have to look at the code to see where
the differences are.

3) 'wgp2'

LAN stuff :

I haven't tested the LAN stuff for the moment.


Stephh's notes (based on the game M68000 code and some tests) :

1) 'wgp', 'wgpu' and 'wgpj'

  - Region stored at 0x03fffe.w and sub-region stored at 0x03fffc.w
  - Sets :
      * 'wgp'  : region = 0x0001 - sub-region = 0x0002
      * 'wgpj' : region = 0x0000 - sub-region = 0x0000
  - Coinage relies on the region (code at 0x00dd10) :
      * 0x0000 (Japan) use TAITO_COINAGE_JAPAN_NEW
      * 0x0001 (US) use TAITO_COINAGE_US
      * 0x0002 (World), 0x0003 (US, licensed to ROMSTAR) and 0x0004 (licensed to PHOENIX ELECTRONICS CO.)
        use slightly different TAITO_COINAGE_WORLD : 1C_7C instead of 1C_6C for Coin B
  - GP order relies on the sub-region (code at 0x00bc9c) :
      * 0x0000 : 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
      * 0x0001 : 0x01 0x00 0x02 0x03 0x04 0x05 0x06 0x07
      * 0x0002 : 0x02 0x00 0x01 0x02 0x04 0x05 0x06 0x07
      * 0x0003 : 0x03 0x00 0x01 0x03 0x04 0x05 0x06 0x07
      * 0x0004 : 0x04 0x00 0x01 0x02 0x03 0x05 0x06 0x07
      * 0x0005 : 0x05 0x00 0x01 0x02 0x03 0x04 0x06 0x07
      * 0x0006 : 0x06 0x00 0x01 0x02 0x03 0x04 0x05 0x07
      * 0x0007 : 0x07 0x00 0x01 0x02 0x03 0x04 0x05 0x06
    GP country :
      * 0x00 : Japan
      * 0x01 : Australia
      * 0x02 : USA
      * 0x03 : Germany
      * 0x04 : Netherlands
      * 0x05 : Belgium
      * 0x06 : France
      * 0x07 : England
  - Notice screen only if region = 0x0000 or region = 0x0001
  - FBI logo only if region = 0x0001
  - DSWA bit 0 does the following things when set to ON :
      * "MACHINE TEST" message and additional tests on startup (code at 0x00b06e)
      * alternate "SHIFT PATTERN SELECT" screen (code at 0x00ccdc)
      * accel / brake "buttons" have different behaviour (code at 0x029ca0) :
          . same as default when selecting "NORMAL" (4 gears, street machines)
          . swapped "buttons" when selecting "RACING" (6 gears, racing machines)
      * additional value sent to shared memory every frame (code at 0x00afd2)
      * additional "MOTOR TEST" in the "test mode"


2) 'wgpjoy*'

  - Region stored at 0x03fffe.w and sub-region stored at 0x03fffc.w
  - Sets :
      * 'wgpjoy'  : region = 0x0000 - sub-region = 0x0000
      * 'wgpjoya' : region = 0x0000 - sub-region = 0x0000
  - DSWA bit 0 does the following things when set to ON :
      * "MACHINE TEST" message and additional tests on startup (code at 0x00b094)
      * alternate "SHIFT PATTERN SELECT" screen (code at 0x00cd02)
      * accel / brake "buttons" have different behaviour (code at 0x0298c6) :
          . same as default when selecting "NORMAL" (4 gears, street machines)
          . swapped "buttons" when selecting "RACING" (6 gears, racing machines)
      * additional value sent to shared memory every frame (code at 0x00aff8)
  - DSWB bits 4 to 7 ("Communication" and "Machine ID") have no effect
    due to "ori.w   #$f0ff, D0" instruction at 0x003300 !
  - "Test mode" is completely different
  - Same other notes as for 'wgp' (different addresses though)


3) 'wgp2'

  - Region stored at 0x03fffe.w and sub-region stored at 0x03fffc.w
  - Sets :
      * 'wgp2' : region = 0x0000 - sub-region = 0x0000
  - Coinage relies on the region (code at 0x00166e) :
      * 0x0000 (Japan) use TAITO_COINAGE_JAPAN_NEW
      * 0x0001 (US) use TAITO_COINAGE_US
      * 0x0002 (World) use slightly different TAITO_COINAGE_WORLD :
        1C_7C instead of 1C_6C for Coin B, same settings otherwise
  - Notice screen only if region = 0x0000 or region = 0x0001
  - FBI logo only if region = 0x0001
  - Routine at 0x01116c is the same as the one in 'wgp' based on sub-region;
    however, as you can practically select your GP at start, and as I suck
    at such driving game, I wonder if this routine is still called !
  - DSWA bit 0 does the following things when set to ON :
      * unknown effect (code at 0x0126f6)
      * accel / brake "buttons" have different behaviour (code at 0x0142d2) :
          . same as default when selecting "NORMAL" (4 gears, street machines)
          . swapped "buttons" when selecting "RACING" (6 gears, racing machines)
  - Always "MOTOR TEST" in the "test mode"

***************************************************************************/

#include "emu.h"

#include "taitoio.h"
#include "taitoipt.h"
#include "taitosnd.h"
#include "tc0100scn.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/adc0808.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include <algorithm>


namespace {

class wgp_state : public driver_device
{
public:
	wgp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spritemap(*this, "spritemap"),
		m_spriteram(*this, "spriteram"),
		m_pivram(*this, "pivram"),
		m_piv_ctrlram(*this, "piv_ctrlram"),
		m_z80bank(*this, "z80bank"),
		m_steer(*this, "STEER"),
		m_unknown(*this, "UNKNOWN"),
		m_fake(*this, "FAKE")
	{ }

	void wgp2(machine_config &config) ATTR_COLD;
	void wgp(machine_config &config) ATTR_COLD;

	void init_wgp() ATTR_COLD;
	void init_wgp2() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_spritemap;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_pivram;
	required_shared_ptr<u16> m_piv_ctrlram;
	required_memory_bank m_z80bank;

	optional_ioport m_steer;
	optional_ioport m_unknown;
	optional_ioport m_fake;

	// video-related
	tilemap_t *m_piv_tilemap[3]{};
	u16 m_piv_ctrl_reg = 0;
	u16 m_piv_zoom[3]{};
	u16 m_piv_scrollx[3]{};
	u16 m_piv_scrolly[3]{};
	u16 m_rotate_ctrl[8]{};
	[[maybe_unused]] u8 m_dislayer[4]{};

	// misc
	u16 m_cpua_ctrl = 0;
	u16 m_port_sel = 0;
	emu_timer *m_cpub_int6_timer = nullptr;

	void coins_w(u8 data);
	void cpua_ctrl_w(u16 data);
	u16 lan_status_r();
	void rotate_port_w(offs_t offset, u16 data);
	u8 accel_r();
	u8 steer_r();
	u8 steer_offset_r();
	u8 accel_offset_r();
	u8 brake_r();
	u8 unknown_r();
	void sound_bankswitch_w(u8 data);
	void pivram_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void piv_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpub_interrupt);
	TIMER_CALLBACK_MEMBER(trigger_cpu_b_int6);

	void cpu2_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void z80_sound_map(address_map &map) ATTR_COLD;

	template<unsigned Offset> TILE_GET_INFO_MEMBER(get_piv_tile_info);

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs);
	void piv_layer_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u32 priority);
	void parse_control();
};


// reference : https://www.youtube.com/watch?v=Sb3I3eQQvcU
/*******************************************************************/

template<unsigned Offset>
TILE_GET_INFO_MEMBER(wgp_state::get_piv_tile_info)
{
	const u16 tilenum = m_pivram[tile_index + Offset];    // 3 blocks of $2000
	const u16 attr = m_pivram[tile_index + Offset + 0x8000];  // 3 blocks of $2000

	tileinfo.set(1, tilenum & 0x3fff, (attr & 0x3f), /* attr & 0x1 ?? */ TILE_FLIPYX((attr & 0xc0) >> 6));
}


void wgp_state::video_start()
{
	m_piv_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wgp_state::get_piv_tile_info<0x0000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_piv_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wgp_state::get_piv_tile_info<0x1000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_piv_tilemap[2] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(wgp_state::get_piv_tile_info<0x2000>)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);

	m_piv_tilemap[0]->set_transparent_pen(0);
	m_piv_tilemap[1]->set_transparent_pen(0);
	m_piv_tilemap[2]->set_transparent_pen(0);

	// flipscreen n/a
	m_piv_tilemap[0]->set_scrolldx(-32, 0);
	m_piv_tilemap[1]->set_scrolldx(-32, 0);
	m_piv_tilemap[2]->set_scrolldx(-32, 0);
	m_piv_tilemap[0]->set_scrolldy(-16, 0);
	m_piv_tilemap[1]->set_scrolldy(-16, 0);
	m_piv_tilemap[2]->set_scrolldy(-16, 0);

	// We don't need tilemap_set_scroll_rows, as the custom draw routine applies rowscroll manually
	m_tc0100scn->set_colbanks(0x80, 0xc0, 0x40);

	save_item(NAME(m_piv_ctrl_reg));
	save_item(NAME(m_rotate_ctrl));
	save_item(NAME(m_piv_zoom));
	save_item(NAME(m_piv_scrollx));
	save_item(NAME(m_piv_scrolly));
}


/******************************************************************
                 PIV TILEMAP READ AND WRITE HANDLERS

Piv Tilemaps
------------

(The unused gaps look as though Taito considered making their
custom chip capable of four rather than three tilemaps.)

500000 - 501fff : unknown/unused
502000 - 507fff : piv tilemaps 0-2 [tile numbers only]

508000 - 50ffff : this area relates to pixel rows in each piv tilemap.
    Includes rowscroll for the piv tilemaps, 1-2 of which act as a
    simple road. To curve, it has rowscroll applied to each row.

508000 - 5087ff unknown/unused

508800  piv0 row color bank (low byte = row horizontal zoom)
509000  piv1 row color bank (low byte = row horizontal zoom)
509800  piv2 row color bank (low byte = row horizontal zoom)

    Usual low byte is 0x7f, the default row horizontal zoom.

    The high byte is the color offset per pixel row. Controlling
    color bank per scanline is rare in Taito games. Top Speed may
    have a similar system to make its road 'move'.

    In-game the high bytes are set to various values (seen 0 - 0x2b).

50a000  piv0 rowscroll [sky]  (not used, but the code supports this)
50c000  piv1 rowscroll [road] (values 0xfd00 - 0x400)
50e000  piv2 rowscroll [road or scenery] (values 0xfd00 - 0x403)

    [It seems strange that unnecessarily large space allocations were
    made for rowscroll. Perhaps the raster color/zoom effects were an
    afterthought, and 508000-9fff was originally slated as rowscroll
    for 'missing' 4th piv layer. Certainly the layout is illogical.]

510000 - 511fff : unknown/unused
512000 - 517fff : piv tilemaps 0-2 [just tile colors ??]

*******************************************************************/

void wgp_state::pivram_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_pivram[offset]);

	if (offset < 0x3000)
	{
		m_piv_tilemap[(offset / 0x1000)]->mark_tile_dirty((offset % 0x1000));
	}
	else if ((offset >= 0x3400) && (offset < 0x4000))
	{
		// do nothing, custom draw routine takes care of raster effects
	}
	else if ((offset >= 0x8000) && (offset < 0xb000))
	{
		m_piv_tilemap[((offset - 0x8000) / 0x1000)]->mark_tile_dirty((offset % 0x1000));
	}
}

void wgp_state::piv_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask)
{
	u16 a, b;

	COMBINE_DATA(&m_piv_ctrlram[offset]);
	data = m_piv_ctrlram[offset];

	switch (offset)
	{
		case 0x00:
			a = -data;
			b = (a & 0xffe0) >> 1;  // kill bit 4
			m_piv_scrollx[0] = (a & 0xf) | b;
			break;

		case 0x01:
			a = -data;
			b = (a & 0xffe0) >> 1;
			m_piv_scrollx[1] = (a & 0xf) | b;
			break;

		case 0x02:
			a = -data;
			b = (a & 0xffe0) >> 1;
			m_piv_scrollx[2] = (a & 0xf) | b;
			break;

		case 0x03:
			m_piv_scrolly[0] = data;
			break;

		case 0x04:
			m_piv_scrolly[1] = data;
			break;

		case 0x05:
			m_piv_scrolly[2] = data;
			break;

		case 0x06:
			/* Overall control reg (?)
			   0x39  %00111001   normal
			   0x2d  %00101101   piv2 layer goes under piv1
			         seen on Wgp stages 4,5,7 in which piv 2 used
			         for cloud or scenery wandering up screen */

			m_piv_ctrl_reg = data;
			break;

		case 0x08:
			// piv 0 y zoom (0x7f = normal, not seen others)
			m_piv_zoom[0] = data;
			break;

		case 0x09:
			// piv 1 y zoom (0x7f = normal, values 0 & 0xff7f-ffbc in WGP 2)
			m_piv_zoom[1] = data;
			break;

		case 0x0a:
			// piv 2 y zoom (0x7f = normal, values 0 & 0xff7f-ffbc in WGP 2, 0-0x98 in Wgp round 4/5)
			m_piv_zoom[2] = data;
			break;
	}
}




/****************************************************************
                     SPRITE DRAW ROUTINES

TODO
====

Implement rotation/zoom properly.

Sprite/piv priority: sprites always over?

WGP round 1 had some junky brown mud bank sprites in-game.
They are indexed 0xe720-e790. 0x2720*4 => +0x9c80-9e80 in
the spritemap area. They should be 2x2 not 4x4 tiles. We
kludge this. Round 2 +0x9d40-9f40 contains the 2x2 sprites.
What REALLY controls number of tiles in a sprite?

Sprite colors: dust after crash in WGP 2 is odd; some
black/grey barrels on late WGP circuit also look strange -
possibly the same wrong color.


Memory Map
----------

400000 - 40bfff : Sprite tile mapping area

    Tile numbers (0-0x3fff) alternate with word containing tile
    color/unknown bits. I'm _not_ 100% sure that only WGP 2 uses
    the unknown bits.

    xxxxxxxx x.......  unused ??
    ........ .x......  unknown (WGP 2 only: Taito tyre bridge on default course)
    ........ ..x.....  unknown (WGP 2 only)
    ........ ...x....  unknown (WGP 2 only: Direction signs just before hill # 1)
    ........ ....cccc  color (0-15)

    Tile map for each standard big sprite is 64 bytes (16 tiles).
    (standard big sprite comprises 4x4 16x16 tiles)

    Tile map for each small sprite only uses 16 of the 64 bytes.
      The remaining 48 bytes are garbage and should be ignored.
    (small sprite comprises 2x2 16x16 tiles)

40c000 - 40dbff : Sprite Table

    Every 16 bytes contains one sprite entry. First entry is
    ignored [as 0 in active sprites list means no sprite].

    (0x1c0 [no.of entries] * 0x40 [bytes for big sprite] = 0x6fff
    of sprite tile mapping area can be addressed at any one time.)

    Sprite entry     (preliminary)
    ------------

    +0x00  x pos (signed)
    +0x02  y pos (signed)
    +0x04  index to tile mapping area [2 msbs always set]

           (400000 + (index & 0x3fff) << 2) points to relevant part of
           sprite tile mapping area. Index >0x2fff would be invalid.

    +0x06  zoom size (pixels) [typical range 0x1-5f, 0x3f = standard]
           Looked up from a logarithm table in the data ROM indexed
           by the z coordinate. Max size prog allows before it blanks
           the sprite is 0x140.

    +0x08  incxx ?? (usually stuck at 0xf800)
    +0x0a  incyy ?? (usually stuck at 0xf800)

    +0x0c  z coordinate i.e. how far away the sprite is from you
           going into the screen. Max distance prog allows before it
           blanks the sprite is 0x3fff. 0x1400 is about the farthest
           away that the code creates sprites. 0x400 = standard
           distance corresponding to 0x3f zoom.  <0x400 = close to

    +0x0e  non-zero only during rotation.

    NB: +0x0c and +0x0e are paired. Equivalent of incyx and incxy ??

    (No longer used entries typically have 0xfff6 in +0x06 and +0x08.)

    Only 2 rotation examples (i) at 0x40c000 when Taito
    logo displayed (WGP only). (ii) stage 5 (rain).
    Other in-game sprites are simply using +0x06 and +0x0c,

    So the sprite rotation in WGP screenshots must be a *blanket*
    rotate effect, identical to the one applied to piv layers.
    This explains why sprite/piv positions are basically okay
    despite failure to implement rotation.

40dc00 - 40dfff: Active Sprites list

    Each word is a sprite number, 0x0 through 0x1bf. If !=0
    a word makes active the 0x10 bytes of sprite data at
    (40c000 + sprite_num * 0x10). (WGP 2 fills this in reverse).

40fff0: Unknown (sprite control word?)

    WGP alternates 0x8000 and 0. WGP 2 only pokes 0.
    Could this be some frame buffer control that would help to
    reduce the sprite timing glitches in WGP?

****************************************************************/

/* Sprite tilemapping area doesn't have a straightforward
   structure for each big sprite: the hardware is probably
   constructing each 4x4 sprite from 4 2x2 sprites... */

static const u8 xlookup[16] =
	{ 0, 1, 0, 1,
		2, 3, 2, 3,
		0, 1, 0, 1,
		2, 3, 2, 3 };

static const u8 ylookup[16] =
	{ 0, 0, 1, 1,
		0, 0, 1, 1,
		2, 2, 3, 3,
		2, 2, 3, 3 };

void wgp_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs)
{
	int i, j, k;
//  u16 rotate = 0;
	const u32 tile_mask = (m_gfxdecode->gfx(0)->elements()) - 1;
	static const u32 primasks[2] = {0x0, 0xfffc};   // fff0 => under rhs of road only

	for (int offs = 0x1ff; offs >= 0; offs--)
	{
		const int code = (m_spriteram[0xe00 + offs]);

		if (code)   // do we have an active sprite ?
		{
			i = (code << 3) & 0xfff;    // yes, so we look up its sprite entry

			int x = m_spriteram[i];
			int y = m_spriteram[i + 1];
			const int bigsprite = m_spriteram[i + 2] & 0x3fff;

			/* The last five words [i + 3 through 7] must be zoom/rotation
			   control: for time being we kludge zoom using 1 word.
			   Timing problems are causing many glitches. */

			if ((m_spriteram[i + 4] == 0xfff6) && (m_spriteram[i + 5] == 0))
				continue;

//          if (((m_spriteram[i + 4] != 0xf800) && (m_spriteram[i + 4] != 0xfff6))
//              || ((m_spriteram[i + 5] != 0xf800) && (m_spriteram[i + 5] != 0))
//              || m_spriteram[i + 7] != 0)
//              rotate = i << 1;

			/***** Begin zoom kludge ******/

			const int zoomx = (m_spriteram[i + 3] & 0x1ff) + 1;
			const int zoomy = (m_spriteram[i + 3] & 0x1ff) + 1;

			y -= 4;
			// distant sprites were some 16 pixels too far down
			y -= ((0x40 - zoomy)/4);

			/****** end zoom kludge *******/

			// Treat coords as signed
			if (x & 0x8000)  x -= 0x10000;
			if (y & 0x8000)  y -= 0x10000;

			const int map_index = bigsprite << 1; // now we access sprite tilemap

			// don't know what selects 2x2 sprites: we use a nasty kludge which seems to work

			i = m_spritemap[map_index + 0xa];
			j = m_spritemap[map_index + 0xc];
			const bool small_sprite = ((i > 0) & (i <= 8) & (j > 0) & (j <= 8));

			if (small_sprite)
			{
				for (i = 0; i < 4; i++)
				{
					const u32 tile = m_spritemap[(map_index + (i << 1))] & tile_mask;
					const u32 col  = m_spritemap[(map_index + (i << 1) + 1)] & 0xf;

					// not known what controls priority
					const int priority = (m_spritemap[(map_index + (i << 1) + 1)] & 0x70) >> 4;

					int flipx = 0;  // no flip xy?
					int flipy = 0;

					k = xlookup[i]; // assumes no xflip
					j = ylookup[i]; // assumes no yflip

					const int curx = x + ((k * zoomx) / 2);
					const int cury = y + ((j * zoomy) / 2);

					const int zx = x + (((k + 1) * zoomx) / 2) - curx;
					const int zy = y + (((j + 1) * zoomy) / 2) - cury;

					m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap, cliprect,
							tile,
							col,
							flipx, flipy,
							curx, cury,
							zx << 12, zy << 12,
							screen.priority(), primasks[((priority >> 1) & 1)], 0);  // maybe >> 2 or 0...?
				}
			}
			else
			{
				for (i = 0; i < 16; i++)
				{
					const u32 tile = m_spritemap[(map_index + (i << 1))] & tile_mask;
					const u32 col  = m_spritemap[(map_index + (i << 1) + 1)] & 0xf;

					// not known what controls priority
					const int priority = (m_spritemap[(map_index + (i << 1) + 1)] & 0x70) >> 4;

					int flipx = 0;  // no flip xy?
					int flipy = 0;

					k = xlookup[i]; // assumes no xflip
					j = ylookup[i]; // assumes no yflip

					const int curx = x + ((k * zoomx) / 4);
					const int cury = y + ((j * zoomy) / 4);

					const int zx = x + (((k + 1) * zoomx) / 4) - curx;
					const int zy = y + (((j + 1) * zoomy) / 4) - cury;

					m_gfxdecode->gfx(0)->prio_zoom_transpen(bitmap, cliprect,
							tile,
							col,
							flipx, flipy,
							curx, cury,
							zx << 12, zy << 12,
							screen.priority(), primasks[((priority >> 1) & 1)], 0);  // maybe >> 2 or 0...?
				}
			}
		}

	}
#if 0
	if (rotate)
		popmessage("sprite rotate offs %04x ?", rotate);
#endif
}


/*********************************************************
                       CUSTOM DRAW
*********************************************************/

static inline void bryan2_drawscanline(bitmap_ind16 &bitmap, int x, int y, int length,
		const u16 *src, bool transparent, u32 orient, bitmap_ind8 &priority, u8 pri, u8 primask = 0xff)
{
	u16 *dsti = &bitmap.pix(y, x);
	u8 *dstp = &priority.pix(y, x);

	if (transparent)
	{
		while (length--)
		{
			const u32 spixel = *src++;
			if (spixel < 0x7fff)
			{
				*dsti = spixel;
				*dstp = (*dstp & primask) | pri;
			}
			dsti++;
			dstp++;
		}
	}
	else  // Not transparent case
	{
		while (length--)
		{
			*dsti++ = *src++;
			*dstp = (*dstp & primask) | pri;
			dstp++;
		}
	}
}



void wgp_state::piv_layer_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u32 priority)
{
	bitmap_ind16 &srcbitmap = m_piv_tilemap[layer]->pixmap();
	bitmap_ind8 &flagsbitmap = m_piv_tilemap[layer]->flagsmap();

	int y_index;

	/* I have a fairly strong feeling these should be u32's, x_index is
	   falling through from max +ve to max -ve quite a lot in this routine */
	int sx;

	u16 scanline[512];
	int flipscreen = 0; // n/a

	const int screen_width = cliprect.width();
	const int min_y = cliprect.min_y;
	const int max_y = cliprect.max_y;

	const int width_mask = 0x3ff;

	const u32 zoomx = 0x10000;    // No overall X zoom, unlike TC0480SCP

	/* Y-axis zoom offers expansion/compression: 0x7f = no zoom, 0xff = max ???
	   In WGP see: stage 4 (big spectator stand)
	               stage 5 (cloud layer)
	               stage 7 (two bits of background scenery)
	               stage 8 (unknown - surely something should be appearing here...)
	   In WGP 2 see: road at big hill (default course) */

	// This calculation may be wrong, the y_index one too
	const u32 zoomy = 0x10000 - (((m_piv_ctrlram[0x08 + layer] & 0xff) - 0x7f) * 512);

	if (!flipscreen)
	{
		sx = ((m_piv_scrollx[layer]) << 16);
		sx += (32) * zoomx;     // may be imperfect

		y_index = (m_piv_scrolly[layer] << 16);
		y_index += (16 + min_y) * zoomy;        // may be imperfect
	}
	else    // piv tiles flipscreen n/a
	{
		sx = 0;
		y_index = 0;
	}

	for (int y = min_y; y <= max_y; y++)
	{
		int a;

		const int src_y_index = (y_index >> 16) & 0x3ff;
		const int row_index = src_y_index;

		const int row_zoom = m_pivram[row_index + layer * 0x400 + 0x3400] & 0xff;

		u16 row_colbank = m_pivram[row_index + layer * 0x400 + 0x3400] >> 8;
		a = (row_colbank & 0xe0);   // kill bit 4
		row_colbank = (((row_colbank & 0xf) << 1) | a) << 4;

		u16 row_scroll = m_pivram[row_index + layer * 0x1000 + 0x4000];
		a = (row_scroll & 0xffe0) >> 1; // kill bit 4
		row_scroll = ((row_scroll & 0xf) | a) & width_mask;

		int x_index = sx - (row_scroll << 16);

		int x_step = zoomx;
		if (row_zoom > 0x7f)    // zoom in: reduce x_step
		{
			x_step -= (((row_zoom - 0x7f) << 8) & 0xffff);
		}
		else if (row_zoom < 0x7f)   // zoom out: increase x_step
		{
			x_step += (((0x7f - row_zoom) << 8) & 0xffff);
		}

		const u16 *const src16 = &srcbitmap.pix(src_y_index);
		const u8 *const  tsrc  = &flagsbitmap.pix(src_y_index);
		u16 *dst16 = scanline;

		if (flags & TILEMAP_DRAW_OPAQUE)
		{
			for (int i = 0; i < screen_width; i++)
			{
				*dst16++ = src16[(x_index >> 16) & width_mask] + row_colbank;
				x_index += x_step;
			}
		}
		else
		{
			for (int i = 0; i < screen_width; i++)
			{
				if (tsrc[(x_index >> 16) & width_mask])
					*dst16++ = src16[(x_index >> 16) & width_mask] + row_colbank;
				else
					*dst16++ = 0x8000;
				x_index += x_step;
			}
		}

		bryan2_drawscanline(bitmap, 0, y, screen_width, scanline, (flags & TILEMAP_DRAW_OPAQUE) ? false : true, ROT0, screen.priority(), priority);

		y_index += zoomy;
	}
}



/**************************************************************
                        SCREEN REFRESH
**************************************************************/

u32 wgp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u8 layer[3];

#ifdef MAME_DEBUG
	if (machine().input().code_pressed_once (KEYCODE_V))
	{
		m_dislayer[0] ^= 1;
		popmessage("piv0: %01x",m_dislayer[0]);
	}

	if (machine().input().code_pressed_once (KEYCODE_B))
	{
		m_dislayer[1] ^= 1;
		popmessage("piv1: %01x",m_dislayer[1]);
	}

	if (machine().input().code_pressed_once (KEYCODE_N))
	{
		m_dislayer[2] ^= 1;
		popmessage("piv2: %01x",m_dislayer[2]);
	}

	if (machine().input().code_pressed_once (KEYCODE_M))
	{
		m_dislayer[3] ^= 1;
		popmessage("TC0100SCN top bg layer: %01x",m_dislayer[3]);
	}
#endif

	for (int i = 0; i < 3; i++)
	{
		m_piv_tilemap[i]->set_scrollx(0, m_piv_scrollx[i]);
		m_piv_tilemap[i]->set_scrolly(0, m_piv_scrolly[i]);
	}

	m_tc0100scn->tilemap_update();

	screen.priority().fill(0, cliprect);
	bitmap.fill(0, cliprect);

	layer[0] = 0;
	layer[1] = 1;
	layer[2] = 2;

	if (m_piv_ctrl_reg == 0x2d)
	{
		layer[1] = 2;
		layer[2] = 1;
	}

// We should draw the following on a 1024x1024 bitmap...

#ifdef MAME_DEBUG
	if (m_dislayer[layer[0]] == 0)
#endif
	piv_layer_draw(screen, bitmap, cliprect, layer[0], TILEMAP_DRAW_OPAQUE, 1);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[1]] == 0)
#endif
	piv_layer_draw(screen, bitmap, cliprect, layer[1], 0, 2);

#ifdef MAME_DEBUG
	if (m_dislayer[layer[2]] == 0)
#endif
	piv_layer_draw(screen, bitmap, cliprect, layer[2], 0, 4);

	draw_sprites(screen, bitmap, cliprect, 16);

// ... then here we should apply rotation from m_rotate_ctrl[] to the bitmap before we draw the TC0100SCN layers on it
	layer[0] = m_tc0100scn->bottomlayer();
	layer[1] = layer[0] ^ 1;
	layer[2] = 2;

	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[0], 0, 0);

#ifdef MAME_DEBUG
	if (m_dislayer[3] == 0)
#endif
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[1], 0, 0);
	m_tc0100scn->tilemap_draw(screen, bitmap, cliprect, layer[2], 0, 0);

#if 0
	popmessage("piv_ctrl_reg: %04x y zoom: %04x %04x %04x",
			m_piv_ctrl_reg,
			m_piv_zoom[0], m_piv_zoom[1], m_piv_zoom[2]);
#endif

// Enable this to watch the rotation control words
#if 0
	{
		char buf[80];
		int i;

		for (int i = 0; i < 8; i += 1)
		{
			sprintf (buf, "%02x: %04x", i, rotate_ctrl[i]);
			ui_draw_text (buf, 0, i*8);
		}
	}
#endif
	return 0;
}


void wgp_state::parse_control()
{
	// bit 0 enables cpu B
	// however this fails when recovering from a save state if cpu B is disabled !!
	m_subcpu->set_input_line(INPUT_LINE_RESET, (m_cpua_ctrl & 0x1) ? CLEAR_LINE : ASSERT_LINE);

	// bit 1 is "vibration" acc. to test mode
}

void wgp_state::cpua_ctrl_w(u16 data) // assumes Z80 sandwiched between 68Ks
{
	if ((data &0xff00) && ((data &0xff) == 0))
		data = data >> 8;   // for Wgp
	m_cpua_ctrl = data;

	parse_control();

	logerror("CPU #0 PC %06x: write %04x to cpu control\n", m_maincpu->pc(), data);
}


/***********************************************************
                        INTERRUPTS
***********************************************************/

TIMER_CALLBACK_MEMBER(wgp_state::trigger_cpu_b_int6)
{
	// 68000 B
	m_subcpu->set_input_line(6, HOLD_LINE); // assumes Z80 sandwiched between the 68Ks
}


/***** Routines for particular games *****/

/* FWIW offset of 10000,10500 on ints can get CPUB obeying the
   first CPUA command the same frame; probably not necessary */

INTERRUPT_GEN_MEMBER(wgp_state::cpub_interrupt)
{
	m_cpub_int6_timer->adjust(m_subcpu->cycles_to_attotime(200000 - 500));
	device.execute().set_input_line(4, HOLD_LINE);
}


/**********************************************************
                         GAME INPUTS
**********************************************************/

u16 wgp_state::lan_status_r()
{
	logerror("CPU #2 PC %06x: warning - read LAN status\n", m_subcpu->pc());

	return (0x4 << 8); // CPUB expects this in code at $104d0 (WGP)
}

void wgp_state::rotate_port_w(offs_t offset, u16 data)
{
	/* This port may be for piv/sprite layer rotation.

	WGP 2 pokes a single set of values (see 2 routines from
	$4e4a), so if this is rotation then WGP 2 *doesn't* use
	it.

	WGP pokes a wide variety of values here, which appear
	to move up and down as rotation control words might.
	See $ae06-d8 which pokes piv ctrl words, then pokes
	values to this port.

	There is a lookup area in the data ROM from $d0000-$da400
	which contains sets of 4 words (used for ports 0-3).
	NB: port 6 is not written.
	*/

	switch (offset)
	{
		case 0x00:
		{
			// logerror("CPU #0 PC %06x: warning - port %04x write %04x\n", m_maincpu->pc(), m_port_sel, data);

			m_rotate_ctrl[m_port_sel] = data;
			return;
		}

		case 0x01:
		{
			m_port_sel = data & 0x7;
		}
	}
}

u8 wgp_state::accel_r()
{
	if (m_fake.read_safe(0) & 0x40)    // pressing accel
		return 0xff;
	else
		return 0x00;
}

u8 wgp_state::steer_r()
{
	int steer = 0x40;
	int const fake = m_fake.read_safe(0);

	if (!(fake & 0x10)) // Analogue steer (the real control method)
	{
		// Reduce span to 0x80
		steer = (m_steer.read_safe(0) * 0x80) / 0x100;
	}
	else    // Digital steer
	{
		if (fake & 0x08)    // pressing down
			steer = 0x20;

		if (fake & 0x04)    // pressing up
			steer = 0x60;

		if (fake & 0x02)    // pressing right
			steer = 0x00;

		if (fake & 0x01)    // pressing left
			steer = 0x80;
	}

	return steer;
}

u8 wgp_state::steer_offset_r()
{
	return 0xc0;    // steer offset, correct acc. to service mode
}

u8 wgp_state::accel_offset_r()
{
	return 0xbf;    // accel offset, correct acc. to service mode
}

u8 wgp_state::brake_r()
{
	if (m_fake.read_safe(0) & 0x80)    // pressing brake
		return 0xcf;
	else
		return 0xff;
}

u8 wgp_state::unknown_r()
{
	return m_unknown.read_safe(0);   // unknown
}

void wgp_state::coins_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}


/**********************************************************
                          SOUND
**********************************************************/

void wgp_state::sound_bankswitch_w(u8 data)
{
	m_z80bank->set_entry(data & 3);
}

/*****************************************************************
                         MEMORY STRUCTURES
*****************************************************************/

void wgp_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram(); // main CPUA RAM
	map(0x140000, 0x143fff).ram().share("sharedram");
	map(0x180000, 0x18000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x1c0000, 0x1c0001).w(FUNC(wgp_state::cpua_ctrl_w));
	map(0x200000, 0x20000f).rw("adc", FUNC(adc0809_device::data_r), FUNC(adc0809_device::address_offset_start_w)).umask16(0x00ff);
	map(0x300000, 0x30ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w)); // tilemaps
	map(0x320000, 0x32000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x400000, 0x40bfff).ram().share(m_spritemap);
	map(0x40c000, 0x40dfff).ram().share(m_spriteram);
	map(0x40fff0, 0x40fff1).nopw(); // ?? (writes 0x8000 and 0 alternately - WGP 2 just 0)
	map(0x500000, 0x501fff).ram(); // unknown/unused
	map(0x502000, 0x517fff).ram().w(FUNC(wgp_state::pivram_word_w)).share(m_pivram); // piv tilemaps
	map(0x520000, 0x52001f).ram().w(FUNC(wgp_state::piv_ctrl_word_w)).share(m_piv_ctrlram);
	map(0x600000, 0x600003).w(FUNC(wgp_state::rotate_port_w)); // rotation control ?
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void wgp_state::cpu2_map(address_map &map)
{ // LAN areas not mapped...
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x140000, 0x143fff).ram().share("sharedram");
	map(0x200001, 0x200001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x200003, 0x200003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
//  map(0x380000, 0x383fff).ram(); // LAN RAM
	map(0x380000, 0x380001).r(FUNC(wgp_state::lan_status_r)); // ??
	// a LAN input area is read somewhere above the status
	// (make the status return 0 and log)...
}


/***************************************************************************/

void wgp_state::z80_sound_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x7fff).bankr("z80bank");
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe003).rw("ymsnd", FUNC(ym2610_device::read), FUNC(ym2610_device::write));
	map(0xe200, 0xe200).nopr().w(m_tc0140syt, FUNC(tc0140syt_device::slave_port_w));
	map(0xe201, 0xe201).rw(m_tc0140syt, FUNC(tc0140syt_device::slave_comm_r), FUNC(tc0140syt_device::slave_comm_w));
	map(0xe400, 0xe403).nopw(); // pan
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); // ?
	map(0xf000, 0xf000).nopw(); // ?
	map(0xf200, 0xf200).w(FUNC(wgp_state::sound_bankswitch_w));
}


/***********************************************************
                      INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( wgp_joy_generic )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Gear Shift" ) // see notes
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Normal / Racing" )
	TAITO_DSWA_BITS_1_TO_3
	// The 4 following Dip Switches will be filled by TAITO_COINAGE_* macros
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWB")
	TAITO_DIFFICULTY
	PORT_DIPNAME( 0x04, 0x04, "Shift Pattern Select" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPUNUSED( 0x08, IP_ACTIVE_LOW )
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW ) // see notes
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW ) // see notes
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW ) // see notes
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW ) // see notes

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1) // shift up - see notes
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1) // shift down - see notes
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // brake
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // accel
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( wgp_no_joy_generic )
	PORT_INCLUDE(wgp_joy_generic)

	PORT_MODIFY("DSWB")
	PORT_DIPNAME( 0x10, 0x10, "Communication" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Machine ID" )
	PORT_DIPSETTING(    0xe0, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0xa0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x40, "6" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Freeze") PORT_CODE(KEYCODE_F1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // shift up - see notes
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // shift down - see notes
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )

	PORT_MODIFY("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("STEER")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(25) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("UNKNOWN")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(2)

	// fake inputs, allowing digital steer etc.
	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(1)
	PORT_CONFNAME( 0x10, 0x10, "Steering type" )
	PORT_CONFSETTING(    0x10, "Digital" )
	PORT_CONFSETTING(    0x00, "Analogue" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1) // accel
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) // brake
INPUT_PORTS_END


static INPUT_PORTS_START( wgpu )
	PORT_INCLUDE(wgp_no_joy_generic)

	// 0x180000 -> 0x10bf16 and 0x140010 (shared RAM)
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US

	// 0x180002 -> 0x10bf18 and 0x140012 (shared RAM) : DSWB

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1) // "start lump" (lamp?) - test mode only
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) // "brake lump" (lamp?) - test mode only
INPUT_PORTS_END

static INPUT_PORTS_START( wgpj )
	PORT_INCLUDE(wgpu)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW
INPUT_PORTS_END

static INPUT_PORTS_START( wgp )
	PORT_INCLUDE(wgpu)

	PORT_MODIFY("DSWA")
	TAITO_COINAGE_WORLD
INPUT_PORTS_END

static INPUT_PORTS_START( wgpjoy )
	PORT_INCLUDE(wgp_joy_generic)

	// 0x180000 -> 0x10bf1a and 0x140010 (shared RAM)
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW

	// 0x180002 -> 0x10bf1c and 0x140012 (shared RAM) : DSWB
INPUT_PORTS_END

static INPUT_PORTS_START( wgp2 )
	PORT_INCLUDE(wgp_no_joy_generic)

	// 0x180000 -> 0x107d3a.b (-$2c6,A5) and 0x140018 (shared RAM)
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW

	// 0x180002 -> 0x107d3b.b (-$2c5,A5) and 0x140012 (shared RAM) : DSWB
INPUT_PORTS_END


/***********************************************************
                        GFX DECODING
***********************************************************/

static GFXDECODE_START( gfx_wgp )
	GFXDECODE_ENTRY( "sprites", 0x0, gfx_16x16x4_packed_lsb, 0, 256 )
	GFXDECODE_ENTRY( "piv",     0x0, gfx_16x16x4_packed_lsb, 0, 256 )
GFXDECODE_END


/***********************************************************
                      MACHINE DRIVERS

WGP has high interleaving to prevent "common RAM error".
However sync to vblank is lacking, which is causing the
graphics glitches.

***********************************************************/

void wgp_state::device_post_load()
{
	parse_control();
}

void wgp_state::machine_reset()
{
	m_cpua_ctrl = 0xff;
	m_port_sel = 0;
	m_piv_ctrl_reg = 0;

	for (int i = 0; i < 3; i++)
	{
		m_piv_zoom[i] = 0;
		m_piv_scrollx[i] = 0;
		m_piv_scrolly[i] = 0;
	}

	std::fill(std::begin(m_rotate_ctrl), std::end(m_rotate_ctrl), 0);
}

void wgp_state::machine_start()
{
	m_z80bank->configure_entries(0, 4, memregion("audiocpu")->base(), 0x4000);

	m_cpub_int6_timer = timer_alloc(FUNC(wgp_state::trigger_cpu_b_int6), this);

	save_item(NAME(m_cpua_ctrl));
	save_item(NAME(m_port_sel));
}

void wgp_state::wgp(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wgp_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wgp_state::irq4_line_hold));

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &wgp_state::z80_sound_map);

	M68000(config, m_subcpu, 16_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &wgp_state::cpu2_map);
	m_subcpu->set_vblank_int("screen", FUNC(wgp_state::cpub_interrupt));

	config.set_maximum_quantum(attotime::from_hz(30000));

	TC0220IOC(config, m_tc0220ioc, 0);
	m_tc0220ioc->read_0_callback().set_ioport("DSWA");
	m_tc0220ioc->read_1_callback().set_ioport("DSWB");
	m_tc0220ioc->read_2_callback().set_ioport("IN0");
	m_tc0220ioc->read_3_callback().set_ioport("IN1");
	m_tc0220ioc->write_4_callback().set(FUNC(wgp_state::coins_w));
	m_tc0220ioc->read_7_callback().set_ioport("IN2");

	adc0809_device &adc(ADC0809(config, "adc", 16_MHz_XTAL / 32)); // TODO: verify divider
	adc.eoc_ff_callback().set_inputline(m_maincpu, M68K_IRQ_6);
	adc.in_callback<0>().set(FUNC(wgp_state::accel_r));
	adc.in_callback<1>().set(FUNC(wgp_state::steer_r));
	adc.in_callback<2>().set(FUNC(wgp_state::steer_offset_r));
	adc.in_callback<3>().set(FUNC(wgp_state::accel_offset_r));
	adc.in_callback<4>().set(FUNC(wgp_state::brake_r));
	adc.in_callback<5>().set(FUNC(wgp_state::unknown_r));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 2*8, 32*8-1);
	screen.set_screen_update(FUNC(wgp_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_wgp);
	PALETTE(config, m_palette).set_format(palette_device::RGBx_444, 4096);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_palette(m_palette);

	// sound hardware
	SPEAKER(config, "speaker", 2).front();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16_MHz_XTAL / 2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0); // assumes Z80 sandwiched between 68Ks
	ymsnd.add_route(0, "speaker", 0.75, 0);
	ymsnd.add_route(0, "speaker", 0.75, 1);
	ymsnd.add_route(1, "speaker", 1.0, 0);
	ymsnd.add_route(2, "speaker", 1.0, 1);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->nmi_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);
	m_tc0140syt->reset_callback().set_inputline(m_audiocpu, INPUT_LINE_RESET);
}

void wgp_state::wgp2(machine_config &config)
{
	wgp(config);

	config.set_maximum_quantum(attotime::from_hz(12000));

	// video hardware
	m_tc0100scn->set_offsets(4, 2);
}


/***************************************************************************
                                   DRIVERS
***************************************************************************/

ROM_START( wgp ) // labels actually have the character * instead of +
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-25++.12",    0x00000, 0x20000, CRC(2c7a863b) SHA1(f1e0d6829e74c6f48c4c5a3230e4bea14dbc3c01) )
	ROM_LOAD16_BYTE( "c32-31++.13",    0x00001, 0x20000, CRC(42ff25c0) SHA1(42bdbbd24389384b8772edb38cca3a42312bb59c) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) // data ROM

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-28+.64", 0x00000, 0x20000, CRC(38f3c7bf) SHA1(bfcaa036e5ff23f2bbf74d738498eda7d6ccd554) )
	ROM_LOAD16_BYTE( "c32-27+.63", 0x00001, 0x20000, CRC(be2397fb) SHA1(605a02d56ae6007b36299a2eceb7ca180cbf6df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "c32-24.34",   0x00000, 0x10000, CRC(e9adb447) SHA1(8b7044b6ea864e4cfd60b87abd28c38caecb147d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8",  0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7",  0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  PALs (Guru dump)
//  ROM_LOAD( "c32-13.14", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-14.19", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-15.52", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-16.54", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-17.53", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-18.64", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-19.27", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-20.67", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-21.85", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-22.24", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-23.13", 0x00000, 0x00???, NO_DUMP )

//  PALs on LAN interface board
//  ROM_LOAD( "c32-34", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-35", 0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( wgpu )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-25.12",      0x00000, 0x20000, CRC(0cc81e77) SHA1(435190bc24423e1e34134dff3cd4b79e120852d1) )
	ROM_LOAD16_BYTE( "c32-29.13",      0x00001, 0x20000, CRC(fab47cf0) SHA1(c0129c0290b48f24c25e4dd7c6c937675e31842a) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) // data ROM

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-28.64", 0x00000, 0x20000, CRC(38f3c7bf) SHA1(bfcaa036e5ff23f2bbf74d738498eda7d6ccd554) )
	ROM_LOAD16_BYTE( "c32-27.63", 0x00001, 0x20000, CRC(be2397fb) SHA1(605a02d56ae6007b36299a2eceb7ca180cbf6df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "c32-24.34",   0x00000, 0x10000, CRC(e9adb447) SHA1(8b7044b6ea864e4cfd60b87abd28c38caecb147d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8",  0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7",  0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  PALs (Guru dump)
//  ROM_LOAD( "c32-13.14", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-14.19", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-15.52", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-16.54", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-17.53", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-18.64", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-19.27", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-20.67", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-21.85", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-22.24", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-23.13", 0x00000, 0x00???, NO_DUMP )

//  PALs on LAN interface board
//  ROM_LOAD( "c32-34", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-35", 0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( wgpj )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-48.12",      0x00000, 0x20000, CRC(819cc134) SHA1(501bb1038979117586f6d8202ca6e1e44191f421) )
	ROM_LOAD16_BYTE( "c32-49.13",      0x00001, 0x20000, CRC(4a515f02) SHA1(d0be52bbb5cc8151b23363092ac04e27b2d20a50) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) // data ROM

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-28.64", 0x00000, 0x20000, CRC(38f3c7bf) SHA1(bfcaa036e5ff23f2bbf74d738498eda7d6ccd554) )
	ROM_LOAD16_BYTE( "c32-27.63", 0x00001, 0x20000, CRC(be2397fb) SHA1(605a02d56ae6007b36299a2eceb7ca180cbf6df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "c32-24.34",   0x00000, 0x10000, CRC(e9adb447) SHA1(8b7044b6ea864e4cfd60b87abd28c38caecb147d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )
ROM_END

ROM_START( wgpjoy )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-57.12",      0x00000, 0x20000, CRC(13a78911) SHA1(d3ace25dddce56cc35e93992f4fae01e87693d36) )
	ROM_LOAD16_BYTE( "c32-58.13",      0x00001, 0x20000, CRC(326d367b) SHA1(cbfb15841f61fa856876d4321fbce190f89a5020) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) // data ROM

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-60.64", 0x00000, 0x20000, CRC(7a980312) SHA1(c85beff4c8201061b99d87f8db67e2b85dff00e3) )
	ROM_LOAD16_BYTE( "c32-59.63", 0x00001, 0x20000, CRC(ed75b333) SHA1(fa47ea38f7ba1cb3463065357db9a9b0f0eeab77) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "c32-61.34",   0x00000, 0x10000, CRC(2fcad5a3) SHA1(f0f658490655b521af631af763c07e37834dc5a0) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )
ROM_END

ROM_START( wgpjoya ) // Older joystick version ???
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-57.12",      0x00000, 0x20000, CRC(13a78911) SHA1(d3ace25dddce56cc35e93992f4fae01e87693d36) )
	ROM_LOAD16_BYTE( "c32-58.13",      0x00001, 0x20000, CRC(326d367b) SHA1(cbfb15841f61fa856876d4321fbce190f89a5020) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) // data ROM

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "c32-46.64", 0x00000, 0x20000, CRC(64191891) SHA1(91d1d51478f1c2785470de0ac2a048e367f7ea48) )  // older rev?
	ROM_LOAD16_BYTE( "c32-45.63", 0x00001, 0x20000, CRC(759b39d5) SHA1(ed4ccd295c5595bdcac965b59293efb3c21ce48a) )  // older rev?

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "c32-61.34",   0x00000, 0x10000, CRC(2fcad5a3) SHA1(f0f658490655b521af631af763c07e37834dc5a0) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )
ROM_END

ROM_START( wgp2 ) // original Taito PCB with original Taito mask ROMs. EPROMs all have hand-written labels with just the location.
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "12.12",    0x00000, 0x20000, CRC(41d0b614) SHA1(b52e397ff36c89e4e5fb15b36f744da2b840abda) ) // hand-written label
	ROM_LOAD16_BYTE( "13.13",    0x00001, 0x20000, CRC(131018d8) SHA1(417d46829e4443abb9a6fcde386506b0942ba71a) ) // hand-written label
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "64.64", 0x00000, 0x20000, CRC(ad3eafda) SHA1(5d500b4bee4e31f520205d02a4f7becca2ef2d3e) BAD_DUMP ) // hand-written label, this one was a bit flaky so marking as bad as precaution
	ROM_LOAD16_BYTE( "63.63", 0x00001, 0x20000, CRC(73606a1e) SHA1(5a649c8909e999dd9b6bc832422f9011bcbfbe68) ) // hand-written label

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "34.34",   0x00000, 0x10000, CRC(50c0364e) SHA1(3f6cb886dac74810a04469089b81fd8d151359a3) ) // hand-written label

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8",  0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7",  0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  WGP 2 security board (has TC0190FMC)
//  ROM_LOAD( "c73-06", 0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( wgp2j )
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000
	ROM_LOAD16_BYTE( "c73-01.12",      0x00000, 0x20000, CRC(c6434834) SHA1(75b2937a9bf18d268fa7bbfb3e822fba510ec2f1) )
	ROM_LOAD16_BYTE( "c73-02.13",      0x00001, 0x20000, CRC(c67f1ed1) SHA1(c30dc3fd46f103a75aa71f87c1fd6c0e7fed9214) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) // data ROM

	ROM_REGION( 0x40000, "sub", 0 ) // 68000
	ROM_LOAD16_BYTE( "c73-04.64", 0x00000, 0x20000, CRC(383aa776) SHA1(bad18f0506e99a07d53e50abe7a548ff3d745e09) )
	ROM_LOAD16_BYTE( "c73-03.63", 0x00001, 0x20000, CRC(eb5067ef) SHA1(08d9d921c7a74877d7bb7641ae30c82d4d0653e3) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) // Z80
	ROM_LOAD( "c73-05.34",   0x00000, 0x10000, CRC(7e00a299) SHA1(93696a229f17a15a92a8d9ef3b34d340de5dec44) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) // SCR

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) // PIV
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) // OBJ
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 ) // ADPCM samples
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 ) // Delta-T samples
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  WGP 2 security board (has TC0190FMC)
//  ROM_LOAD( "c73-06", 0x00000, 0x00???, NO_DUMP )
ROM_END


void wgp_state::init_wgp()
{
#if 0
	/* Patch for coding error that causes corrupt data in
	   sprite tilemapping area from $4083c0-847f */
	u16 *ROM = (u16 *)memregion("maincpu")->base();
	ROM[0x25dc / 2] = 0x0602;   // faulty value is 0x0206
#endif
}

void wgp_state::init_wgp2()
{
	// Code patches to prevent failure in memory checks
	u16 *ROM = (u16 *)memregion("sub")->base();
	ROM[0x8008 / 2] = 0x0;
	ROM[0x8010 / 2] = 0x0;
}

} // anonymous namespace


GAME( 1989, wgp,     0,    wgp,  wgp,    wgp_state, init_wgp,  ROT0, "Taito Corporation Japan",   "WGP: Real Race Feeling (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpu,    wgp,  wgp,  wgpu,   wgp_state, init_wgp,  ROT0, "Taito America Corporation", "WGP: Real Race Feeling (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpj,    wgp,  wgp,  wgpj,   wgp_state, init_wgp,  ROT0, "Taito Corporation",         "WGP: Real Race Feeling (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpjoy,  wgp,  wgp,  wgpjoy, wgp_state, init_wgp,  ROT0, "Taito Corporation",         "WGP: Real Race Feeling (joystick version) (Japan, set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpjoya, wgp,  wgp,  wgpjoy, wgp_state, init_wgp,  ROT0, "Taito Corporation",         "WGP: Real Race Feeling (joystick version) (Japan, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )

GAME( 1990, wgp2,    0,    wgp2, wgp,    wgp_state, init_wgp2, ROT0, "Taito Corporation Japan",   "WGP 2: Real Race Feeling (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1990, wgp2j,   wgp2, wgp2, wgp2,   wgp_state, init_wgp2, ROT0, "Taito Corporation",         "WGP 2: Real Race Feeling (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
