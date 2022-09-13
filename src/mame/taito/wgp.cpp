// license:BSD-3-Clause
// copyright-holders:David Graves
// thanks-to:Richard Bush
/***************************************************************************

World Grand Prix    (c) Taito Corporation 1989
================

David Graves

(Thanks to Richard Bush and the Raine team for their
preliminary driver.)

It seems likely there are a LOT of undumped versions of this game...
If you have access to a board, please check the rom numbers to see if
any are different from the ones listed below.

                *****

World Grand Prix runs on hardware which is pretty different from the
system Taito commonly used for their pseudo-3d racing games of the
time, the Z system. Different screen and sprite hardware is used.
There's also a LAN hookup (for multiple machines).

As well as a TC0100SCN tilemap generator (two 64x64 layers of 8x8
tiles and a layer of 8x8 tiles with graphics data taken from RAM)
there's a "piv" tilemap generator, which creates three scrollable
row-scrollable zoomable 64x64 tilemaps composed of 16x16 tiles.

As well as these six tilemap layers, there is a sprite layer with
individually zoomable / rotatable sprites. Big sprites are created
from 16x16 gfx chunks via a sprite mapping area in RAM.

The piv and sprite layers are rotatable (but not individually, only
together).

World Grand Prix has twin 68K CPUs which communicate via $4000 bytes
of shared ram.

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
    TC0330CHL - ? (perhaps lan related)
This game has LAN interface board, it uses uPD72105C.

Video Map
---------

400000 area is used for sprites. The game does tile mapping
    in ram to create big sprites from the 16x16 tiles of gfx0.
    See notes in \video\ for details.

500000 area is for the "piv" tilemaps.
    See notes in \video\ for details.


TODO
====

Offer fake-dip selectable analogue steer

Is piv/sprite layers rotation control at 0x600000 ?

Verify y-zoom is correct on the stages that use it (including Wgp2
default course). Row zoom may be hard to verify, but Wgp2 course
selection screen is probably a good test.

Implement proper positioning/zoom/rotation for sprites.

(The current sprite coord calculations are kludgy and flawed and
ignore four control words. The sprites also seem jerky
and have [int?] timing glitches.)

DIP coinage


Wgp
---

Analogue brake pedal works but won't register in service mode.

$28336 is code that shows brake value in service mode.
$ac3e sub (called off int4) at $ac78 does three calcs to the six
    AD values: results are stored at ($d22 / $d24 / $d26,A5)
    It is the third one, ($d26,A5) which gets displayed
    in service mode as brake.


Wgp2
----

Piv y zoom may be imperfect. Check the up/down hill part of the
default course. The road looks a little odd.

Sprite colors seem ok except smoke after you crash. (And one sign on
first bend of default course doesn't go yellow for a few frames.)

[Used to die with common ram error. When CPUA enables CPUB, CPUB
writes to $140000/2 - unfortunately while CPUA is in the middle of
testing that ram. We hack prog for CPUB to disable the writes.]


                *****

[Wgp stopped with LAN error. (Looks like CPUB tells CPUA what is wrong
with LAN in shared ram $142048. Examined at $e57c which prints out
relevant lan error message). Ended up at $e57c from $b14e-xx code
section. CPUA does PEA of $e57c which is the fallback if CPUB doesn't
respond in timely fashion to command 0x1 poked by code at $b1a6.

CPUB $830c-8332 loops waiting for command 1-8 in $140002 from CPUA.
it executes this then clears $140002 to indicate command completed.
It wasn't clearing this fast enough for CPUA, because $142048 wasn't
clear:- and that affects the amount of code command 0x1 runs.

CPUB code at $104d8 had already detected error on LAN and set $142048
to 4. We now return correct lan status read from $380000, so
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

$21f4 copies sprite tilemapping data from data rom to tilemap
area then flows into:

$223c code that fills a sprite entry in spriteram

[$12770 - $133cd seems to be an irregular lookup table used to
calculate extra zoom word poked into each sprite entry.]

$23a8 picks data out of data rom and stores it to sprite tile-
mapping area by heading through to $21f4. (May also enter sprite
details into spriteram.) It uses $a0000 area of data rom.

(Alternate entry point at $23c2 uses $90000 area of data rom.)

$25ee routine stores data from rom into sprite tilemapping area
including the bad parts that produce junk sprites.

It calls interesting sub at $25be which has a table of the number
of sequential words to be pulled out of the data rom, depending
on the first word value in the data rom for that entry ("code").
Each code will pull out the following multiple of 16 words:

    Code  Words   Tiles    Actual data rom entry
     0      1      4x4      [same]
     1      2      8x4      [same]
     2      4      8x8      [same]
     3      3      12x4     [same, see $98186]
     4      6      12x8
     5      9      12x12    [same]
     6      2      4x8      [WRONG! says 8x12 in data rom, see $982bf]
     7      6      8x12     [WRONG! says 4x8 in data rom]
     8      1      (2x2)*4  [2x2 in data rom]  (copies 12 unwanted
                                      words - causing junk sprites)

$4083c0-47f in sprite mapping area has junk words - due to code 7
making it read 6*16 words. 0x60 words are copied from the data rom
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


Data rom entry
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
the ROMS (still version 1.06), but you can't perform lan tests in the
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
#include "wgp.h"
#include "taitoipt.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "sound/ymopn.h"
#include "screen.h"
#include "speaker.h"

#include <algorithm>

void wgp_state::parse_control()
{
	/* bit 0 enables cpu B */
	/* however this fails when recovering from a save state
	   if cpu B is disabled !! */
	m_subcpu->set_input_line(INPUT_LINE_RESET, (m_cpua_ctrl & 0x1) ? CLEAR_LINE : ASSERT_LINE);

	/* bit 1 is "vibration" acc. to test mode */
}

void wgp_state::cpua_ctrl_w(u16 data)/* assumes Z80 sandwiched between 68Ks */
{
	if ((data &0xff00) && ((data &0xff) == 0))
		data = data >> 8;   /* for Wgp */
	m_cpua_ctrl = data;

	parse_control();

	logerror("CPU #0 PC %06x: write %04x to cpu control\n",m_maincpu->pc(),data);
}


/***********************************************************
                        INTERRUPTS
***********************************************************/

TIMER_CALLBACK_MEMBER(wgp_state::trigger_cpu_b_int6)
{
	/* 68000 B */
	m_subcpu->set_input_line(6, HOLD_LINE); /* assumes Z80 sandwiched between the 68Ks */
}


/***** Routines for particular games *****/

/* FWIW offset of 10000,10500 on ints can get CPUB obeying the
   first CPUA command the same frame; probably not necessary */

INTERRUPT_GEN_MEMBER(wgp_state::cpub_interrupt)
{
	m_cpub_int6_timer->adjust(m_subcpu->cycles_to_attotime(200000-500));
	device.execute().set_input_line(4, HOLD_LINE);
}


/**********************************************************
                         GAME INPUTS
**********************************************************/

u16 wgp_state::lan_status_r()
{
	logerror("CPU #2 PC %06x: warning - read lan status\n",m_subcpu->pc());

	return  (0x4 << 8); /* CPUB expects this in code at $104d0 (Wgp) */
}

void wgp_state::rotate_port_w(offs_t offset, u16 data)
{
	/* This port may be for piv/sprite layer rotation.

	Wgp2 pokes a single set of values (see 2 routines from
	$4e4a), so if this is rotation then Wgp2 *doesn't* use
	it.

	Wgp pokes a wide variety of values here, which appear
	to move up and down as rotation control words might.
	See $ae06-d8 which pokes piv ctrl words, then pokes
	values to this port.

	There is a lookup area in the data rom from $d0000-$da400
	which contains sets of 4 words (used for ports 0-3).
	NB: port 6 is not written.
	*/

	switch (offset)
	{
		case 0x00:
		{
//logerror("CPU #0 PC %06x: warning - port %04x write %04x\n",m_maincpu->pc(),port_sel,data);

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
	int fake = m_fake.read_safe(0);

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
	map(0x100000, 0x10ffff).ram();     /* main CPUA ram */
	map(0x140000, 0x143fff).ram().share("sharedram");
	map(0x180000, 0x18000f).rw(m_tc0220ioc, FUNC(tc0220ioc_device::read), FUNC(tc0220ioc_device::write)).umask16(0xff00);
	map(0x1c0000, 0x1c0001).w(FUNC(wgp_state::cpua_ctrl_w));
	map(0x200000, 0x20000f).rw("adc", FUNC(adc0809_device::data_r), FUNC(adc0809_device::address_offset_start_w)).umask16(0x00ff);
	map(0x300000, 0x30ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));            /* tilemaps */
	map(0x320000, 0x32000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0x400000, 0x40bfff).ram().share("spritemap");   /* sprite tilemaps */
	map(0x40c000, 0x40dfff).ram().share("spriteram");   /* sprite ram */
	map(0x40fff0, 0x40fff1).nopw();    /* ?? (writes 0x8000 and 0 alternately - Wgp2 just 0) */
	map(0x500000, 0x501fff).ram();                 /* unknown/unused */
	map(0x502000, 0x517fff).ram().w(FUNC(wgp_state::pivram_word_w)).share("pivram"); /* piv tilemaps */
	map(0x520000, 0x52001f).ram().w(FUNC(wgp_state::piv_ctrl_word_w)).share("piv_ctrlram");
	map(0x600000, 0x600003).w(FUNC(wgp_state::rotate_port_w));    /* rotation control ? */
	map(0x700000, 0x701fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
}

void wgp_state::cpu2_map(address_map &map)
{ /* LAN areas not mapped... */
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x103fff).ram();
	map(0x140000, 0x143fff).ram().share("sharedram");
	map(0x200001, 0x200001).w(m_tc0140syt, FUNC(tc0140syt_device::master_port_w));
	map(0x200003, 0x200003).rw(m_tc0140syt, FUNC(tc0140syt_device::master_comm_r), FUNC(tc0140syt_device::master_comm_w));
//  map(0x380000, 0x383fff).ram();    // LAN RAM
	map(0x380000, 0x380001).r(FUNC(wgp_state::lan_status_r));  // ??
	// a lan input area is read somewhere above the status
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
	map(0xe400, 0xe403).nopw(); /* pan */
	map(0xea00, 0xea00).nopr();
	map(0xee00, 0xee00).nopw(); /* ? */
	map(0xf000, 0xf000).nopw(); /* ? */
	map(0xf200, 0xf200).w(FUNC(wgp_state::sound_bankswitch_w));
}


/***********************************************************
                      INPUT PORTS, DIPs
***********************************************************/

static INPUT_PORTS_START( wgp_joy_generic )
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "Gear Shift" )                     /* see notes */
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, "Normal / Racing" )
	TAITO_DSWA_BITS_1_TO_3
	/* The 4 following Dip Switches will be filled by TAITO_COINAGE_* macros */
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
	PORT_DIPUNUSED( 0x10, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPUNUSED( 0x20, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_LOW )                        /* see notes */
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_LOW )                        /* see notes */

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_PLAYER(1)     /* shift up - see notes */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )    PORT_8WAY PORT_PLAYER(1)     /* shift down - see notes */
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)                      /* brake */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)                      /* accel */
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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)                      /* shift up - see notes */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)                      /* shift down - see notes */
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

	/* fake inputs, allowing digital steer etc. */
	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_4WAY PORT_PLAYER(1)
	PORT_CONFNAME( 0x10, 0x10, "Steering type" )
	PORT_CONFSETTING(    0x10, "Digital" )
	PORT_CONFSETTING(    0x00, "Analogue" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)                     /* accel */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)                     /* brake */
INPUT_PORTS_END


static INPUT_PORTS_START( wgpu )
	PORT_INCLUDE(wgp_no_joy_generic)

	/* 0x180000 -> 0x10bf16 and 0x140010 (shared RAM) */
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_US

	/* 0x180002 -> 0x10bf18 and 0x140012 (shared RAM) : DSWB */

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(1)                     /* "start lump" (lamp?) - test mode only */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1)                     /* "brake lump" (lamp?) - test mode only */
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

	/* 0x180000 -> 0x10bf1a and 0x140010 (shared RAM) */
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW

	/* 0x180002 -> 0x10bf1c and 0x140012 (shared RAM) : DSWB*/
INPUT_PORTS_END

static INPUT_PORTS_START( wgp2 )
	PORT_INCLUDE(wgp_no_joy_generic)

	/* 0x180000 -> 0x107d3a.b (-$2c6,A5) and 0x140018 (shared RAM) */
	PORT_MODIFY("DSWA")
	TAITO_COINAGE_JAPAN_NEW

	/* 0x180002 -> 0x107d3b.b (-$2c5,A5) and 0x140012 (shared RAM) : DSWB */
INPUT_PORTS_END


/***********************************************************
                        GFX DECODING
***********************************************************/

static GFXDECODE_START( gfx_wgp )
	GFXDECODE_ENTRY( "sprites", 0x0, gfx_16x16x4_packed_lsb, 0, 256 )    /* sprites */
	GFXDECODE_ENTRY( "piv",     0x0, gfx_16x16x4_packed_lsb, 0, 256 )    /* piv */
GFXDECODE_END


/***********************************************************
                      MACHINE DRIVERS

Wgp has high interleaving to prevent "common ram error".
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
	/* basic machine hardware */
	M68000(config, m_maincpu, 12000000);    /* 12 MHz ??? */
	m_maincpu->set_addrmap(AS_PROGRAM, &wgp_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(wgp_state::irq4_line_hold));

	Z80(config, m_audiocpu, 16000000/4);    /* 4 MHz ??? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &wgp_state::z80_sound_map);

	M68000(config, m_subcpu, 12000000);     /* 12 MHz ??? */
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

	/* video hardware */
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

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym2610_device &ymsnd(YM2610(config, "ymsnd", 16000000/2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0); // assumes Z80 sandwiched between 68Ks
	ymsnd.add_route(0, "lspeaker", 0.25);
	ymsnd.add_route(0, "rspeaker", 0.25);
	ymsnd.add_route(1, "lspeaker", 1.0);
	ymsnd.add_route(2, "rspeaker", 1.0);

	TC0140SYT(config, m_tc0140syt, 0);
	m_tc0140syt->set_master_tag(m_subcpu);
	m_tc0140syt->set_slave_tag(m_audiocpu);
}

void wgp_state::wgp2(machine_config &config)
{
	wgp(config);

	config.set_maximum_quantum(attotime::from_hz(12000));

	/* video hardware */
	MCFG_VIDEO_START_OVERRIDE(wgp_state, wgp2)

	m_tc0100scn->set_offsets(4, 2);
}


/***************************************************************************
                                   DRIVERS
***************************************************************************/

ROM_START( wgp ) // labels actually have the character * instead of +
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c32-25++.12",    0x00000, 0x20000, CRC(2c7a863b) SHA1(f1e0d6829e74c6f48c4c5a3230e4bea14dbc3c01) )
	ROM_LOAD16_BYTE( "c32-31++.13",    0x00001, 0x20000, CRC(42ff25c0) SHA1(42bdbbd24389384b8772edb38cca3a42312bb59c) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c32-28+.64", 0x00000, 0x20000, CRC(38f3c7bf) SHA1(bfcaa036e5ff23f2bbf74d738498eda7d6ccd554) )
	ROM_LOAD16_BYTE( "c32-27+.63", 0x00001, 0x20000, CRC(be2397fb) SHA1(605a02d56ae6007b36299a2eceb7ca180cbf6df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c32-24.34",   0x00000, 0x10000, CRC(e9adb447) SHA1(8b7044b6ea864e4cfd60b87abd28c38caecb147d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) /* SCR */

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) /* PIV */
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) /* OBJ */
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c32-11.8",  0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c32-12.7",  0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  Pals (Guru dump)
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

//  Pals on lan interface board
//  ROM_LOAD( "c32-34", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-35", 0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( wgpu )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c32-25.12",      0x00000, 0x20000, CRC(0cc81e77) SHA1(435190bc24423e1e34134dff3cd4b79e120852d1) )
	ROM_LOAD16_BYTE( "c32-29.13",      0x00001, 0x20000, CRC(fab47cf0) SHA1(c0129c0290b48f24c25e4dd7c6c937675e31842a) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c32-28.64", 0x00000, 0x20000, CRC(38f3c7bf) SHA1(bfcaa036e5ff23f2bbf74d738498eda7d6ccd554) )
	ROM_LOAD16_BYTE( "c32-27.63", 0x00001, 0x20000, CRC(be2397fb) SHA1(605a02d56ae6007b36299a2eceb7ca180cbf6df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c32-24.34",   0x00000, 0x10000, CRC(e9adb447) SHA1(8b7044b6ea864e4cfd60b87abd28c38caecb147d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) /* SCR */

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) /* PIV */
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) /* OBJ */
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c32-11.8",  0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c32-12.7",  0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  Pals (Guru dump)
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

//  Pals on lan interface board
//  ROM_LOAD( "c32-34", 0x00000, 0x00???, NO_DUMP )
//  ROM_LOAD( "c32-35", 0x00000, 0x00???, NO_DUMP )
ROM_END

ROM_START( wgpj )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c32-48.12",      0x00000, 0x20000, CRC(819cc134) SHA1(501bb1038979117586f6d8202ca6e1e44191f421) )
	ROM_LOAD16_BYTE( "c32-49.13",      0x00001, 0x20000, CRC(4a515f02) SHA1(d0be52bbb5cc8151b23363092ac04e27b2d20a50) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c32-28.64", 0x00000, 0x20000, CRC(38f3c7bf) SHA1(bfcaa036e5ff23f2bbf74d738498eda7d6ccd554) )
	ROM_LOAD16_BYTE( "c32-27.63", 0x00001, 0x20000, CRC(be2397fb) SHA1(605a02d56ae6007b36299a2eceb7ca180cbf6df9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c32-24.34",   0x00000, 0x10000, CRC(e9adb447) SHA1(8b7044b6ea864e4cfd60b87abd28c38caecb147d) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) /* SCR */

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) /* PIV */
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) /* OBJ */
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* Delta-T samples */
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )
ROM_END

ROM_START( wgpjoy )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c32-57.12",      0x00000, 0x20000, CRC(13a78911) SHA1(d3ace25dddce56cc35e93992f4fae01e87693d36) )
	ROM_LOAD16_BYTE( "c32-58.13",      0x00001, 0x20000, CRC(326d367b) SHA1(cbfb15841f61fa856876d4321fbce190f89a5020) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c32-60.64", 0x00000, 0x20000, CRC(7a980312) SHA1(c85beff4c8201061b99d87f8db67e2b85dff00e3) )
	ROM_LOAD16_BYTE( "c32-59.63", 0x00001, 0x20000, CRC(ed75b333) SHA1(fa47ea38f7ba1cb3463065357db9a9b0f0eeab77) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c32-61.34",   0x00000, 0x10000, CRC(2fcad5a3) SHA1(f0f658490655b521af631af763c07e37834dc5a0) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) /* SCR */

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) /* PIV */
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) /* OBJ */
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* delta-t samples */
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )
ROM_END

ROM_START( wgpjoya )    /* Older joystick version ??? */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c32-57.12",      0x00000, 0x20000, CRC(13a78911) SHA1(d3ace25dddce56cc35e93992f4fae01e87693d36) )
	ROM_LOAD16_BYTE( "c32-58.13",      0x00001, 0x20000, CRC(326d367b) SHA1(cbfb15841f61fa856876d4321fbce190f89a5020) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c32-46.64", 0x00000, 0x20000, CRC(64191891) SHA1(91d1d51478f1c2785470de0ac2a048e367f7ea48) )  // older rev?
	ROM_LOAD16_BYTE( "c32-45.63", 0x00001, 0x20000, CRC(759b39d5) SHA1(ed4ccd295c5595bdcac965b59293efb3c21ce48a) )  // older rev?

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c32-61.34",   0x00000, 0x10000, CRC(2fcad5a3) SHA1(f0f658490655b521af631af763c07e37834dc5a0) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) /* SCR */

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) /* PIV */
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) /* OBJ */
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* delta-t samples */
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )
ROM_END

ROM_START( wgp2 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 256K for 68000 code (CPU A) */
	ROM_LOAD16_BYTE( "c73-01.12",      0x00000, 0x20000, CRC(c6434834) SHA1(75b2937a9bf18d268fa7bbfb3e822fba510ec2f1) )
	ROM_LOAD16_BYTE( "c73-02.13",      0x00001, 0x20000, CRC(c67f1ed1) SHA1(c30dc3fd46f103a75aa71f87c1fd6c0e7fed9214) )
	ROM_LOAD16_WORD_SWAP( "c32-10.9",  0x80000, 0x80000, CRC(a44c66e9) SHA1(b5fa978e43303003969033b8096fd68885cfc202) ) /* data rom */

	ROM_REGION( 0x40000, "sub", 0 ) /* 256K for 68000 code (CPU B) */
	ROM_LOAD16_BYTE( "c73-04.64", 0x00000, 0x20000, CRC(383aa776) SHA1(bad18f0506e99a07d53e50abe7a548ff3d745e09) )
	ROM_LOAD16_BYTE( "c73-03.63", 0x00001, 0x20000, CRC(eb5067ef) SHA1(08d9d921c7a74877d7bb7641ae30c82d4d0653e3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Z80 sound cpu */
	ROM_LOAD( "c73-05.34",   0x00000, 0x10000, CRC(7e00a299) SHA1(93696a229f17a15a92a8d9ef3b34d340de5dec44) )

	ROM_REGION( 0x80000, "tc0100scn", 0 )
	ROM_LOAD16_WORD_SWAP( "c32-09.16", 0x00000, 0x80000, CRC(96495f35) SHA1(ce99b4d8aeb98304e8ae3aa4966289c76ae4ff69) ) /* SCR */

	ROM_REGION( 0x200000, "piv", 0 )
	ROM_LOAD64_WORD( "c32-04.9",  0x000006, 0x80000, CRC(473a19c9) SHA1(4c632f4d5b725790a1be9d1143318d2f682fe9be) ) /* PIV */
	ROM_LOAD64_WORD( "c32-03.10", 0x000004, 0x80000, CRC(9ec3e134) SHA1(e82a50927e10e551124a3b81399b052974cfba12) )
	ROM_LOAD64_WORD( "c32-02.11", 0x000002, 0x80000, CRC(c5721f3a) SHA1(4a8e9412de23001b09eb3425ba6006b4c09a907b) )
	ROM_LOAD64_WORD( "c32-01.12", 0x000000, 0x80000, CRC(d27d7d93) SHA1(82ae5856bbdb49cb8c2ca20eef86f6b617ea2c45) )

	ROM_REGION( 0x200000, "sprites", 0 )
	ROM_LOAD32_WORD( "c32-05.71", 0x000000, 0x80000, CRC(3698d47a) SHA1(71978f9e1f58fa1259e67d8a7ea68e3ec1314c6b) ) /* OBJ */
	ROM_LOAD32_WORD( "c32-06.70", 0x000002, 0x80000, CRC(f0267203) SHA1(7fd7b8d7a9efa405fc647c16fb99ffcb1fe985c5) )
	ROM_LOAD32_WORD( "c32-07.69", 0x100000, 0x80000, CRC(743d46bd) SHA1(6b655b3fbfad8b52e38d7388aab564f5fa3e778c) )
	ROM_LOAD32_WORD( "c32-08.68", 0x100002, 0x80000, CRC(faab63b0) SHA1(6e1aaf2642bee7d7bc9e21a7bf7f81d9ff766c50) )

	ROM_REGION( 0x80000, "ymsnd:adpcma", 0 )   /* ADPCM samples */
	ROM_LOAD( "c32-11.8", 0x00000, 0x80000, CRC(2b326ff0) SHA1(3c442e3c97234e4514a7bed31644212586869bd0) )

	ROM_REGION( 0x80000, "ymsnd:adpcmb", 0 )    /* delta-t samples */
	ROM_LOAD( "c32-12.7", 0x00000, 0x80000, CRC(df48a37b) SHA1(c0c191f4b8a5f55c0f1e52dac9cd3f7d15adace6) )

//  WGP2 security board (has TC0190FMC)
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
	/* Code patches to prevent failure in memory checks */
	u16 *ROM = (u16 *)memregion("sub")->base();
	ROM[0x8008 / 2] = 0x0;
	ROM[0x8010 / 2] = 0x0;
}

/* Working Games with some graphics problems - e.g. missing rotation */

GAME( 1989, wgp,        0,    wgp,    wgp,    wgp_state, init_wgp,  ROT0, "Taito Corporation Japan",   "World Grand Prix (World)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpu,     wgp,    wgp,    wgpu,   wgp_state, init_wgp,  ROT0, "Taito America Corporation", "World Grand Prix (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpj,     wgp,    wgp,    wgpj,   wgp_state, init_wgp,  ROT0, "Taito Corporation",         "World Grand Prix (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpjoy,   wgp,    wgp,    wgpjoy, wgp_state, init_wgp,  ROT0, "Taito Corporation",         "World Grand Prix (joystick version) (Japan, set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1989, wgpjoya,  wgp,    wgp,    wgpjoy, wgp_state, init_wgp,  ROT0, "Taito Corporation",         "World Grand Prix (joystick version) (Japan, set 2)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
GAME( 1990, wgp2,     wgp,    wgp2,   wgp2,   wgp_state, init_wgp2, ROT0, "Taito Corporation",         "World Grand Prix 2 (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NODEVICE_LAN | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
