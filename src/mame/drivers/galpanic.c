/***************************************************************************

Gals Panic       1990 Kaneko
Fantasia         1994 Comad
Super Model      1994 Comad
New Fantasia     1995 Comad
Fantasy '95      1995 Hi-max Technology Inc. (Running on a Comad PCB)
Miss World '96   1996 Comad
Ms/Mr World '96  1996 Comad
Fantasia II      1997 Comad
Gals Hustler     1997 Ace International

driver by Nicola Salmoria

The Comad games run on hardware similar to Gals Panic, with a different
sprite system. They are not ROM swaps because the addresses of work RAM
and of the OKI chip change from one to the other, however everything else
is pretty much identical.


TODO:
- There is a vector for IRQ4. The function does nothing in galpanic but is
  more complicated in the Comad ones. However I'm not triggering it, and
  they seems to work anyway...
- Four unknown ROMs in fantasia. The game seems to work fine without them.
- There was a ROM in the newfant set, obj2_14.rom, which was identical to
  Terminator 2's t2.107. I can only assume this was a mistake of the dumper.
- lots of unknown reads and writes, also in galpanic but particularly in
  the Comad ones.
- fantasia and newfant have a service mode but they doesn't work well (text
  is missing or replaced by garbage). This might be just how the games are.
- Is there a background enable register? Or a background clear. fantasia and
  newfant certainly look ugly as they are.

Notes about Gals Panic:
-----------------------
The current ROM set is strange because two ROMs overlap two others replacing
the program.

It's definitely a Kaneko boardset, but it could very well be they converted
some other game to run Gals Panic, because there's some ROMs piggybacked
on top of each other and some ROMs on a daughterboard plugged into smaller
sized ROM sockets. It's not a pirate version. The piggybacked ROMs even have
Kaneko stickers. The silkscreen on the board says PAMERA-04.

There is at least another version of the Gals Panic board. It's single board,
so no daughterboard. There are only 4 IC's socketed, the rest is soldered to
the board, and no piggybacked ROMs. Board number is MDK 321 V-0    EXPRO-02
For this version of Gals Panic see the expro02.c driver


Stephh's additional notes :

  - The games might not work correctly if sound is disabled.
  - There seems to exist 3 versions of 'galpanic' (Japan, US and World),
    and we seem to have a World version according to the coinage.
    Version is stored at 0x03ffff.b :
      * 01 : Japan
      * 02 : US
      * 03 : World
    In the version we have, you can only have one type of coinage
    (there is no Dip Switch to change sort of "coin mode").
  - In Comad games, here is a possible explanation of why the "Tilt" button
    may hang the game and/or crash/exit MAME : if you look carefully at the
    code, you'll notice that you have a "rts" instruction WITHOUT restoring
    the registers saved by the "movem.l D0-D7/A0-A6, -(A7)" instruction.
    Then, a "rte" instruction is performed.
  - The "Demo Sounds" Dip Switch is told not to work and not to fit the
    manual, but it appears that 00 seems to be read from in the "trap $d"
    interruption. Is it because the addresses (0x53e830-0x53e84f) are also
    used for 'galpanic_bgvideoram' ?
    In the Comad games, the interruption is the same, but the addresses
    which are checked are in full RAM. So the Dip Switch could be checked.

  - I added the 'galpanica' romset which is in fact the same as 'galpanic',
    but with the PRG ROMS which aren't overwritten and simulated the CALC1
    MCU functions
    Here are a few notes about what I found :
      * This version is also a World version (0x03ffff.b = 03).
      * In this version, there is a "Coin Mode" Dip Switch, but no
        "Character Test" Dip Switch.
      * Area 0xe00000-0xe00014 is a "calculator" area. I've tried to
        simulate it (see 'galpanib_calc*' handlers) by comparing the code
        with the other set. I don't know if there are some other unmapped
        reads, but the game seems to run fine with what I've done.
      * When you press the "Tilt" button, the game enters in an endless
        loop, but this isn't a bug ! Check code beginning at 0x000e02 and
        ending at 0x000976 for more infos.
          -Expects watchdog to reset it- pjp
      * Sound hasn't been tested.
      * The Comad games are definitively based on this version, the main
        differences being that read/writes to 0xe00000 have been replaced.

 - On Gals Hustler there is an extra test mode if you hold down player 2
   button 1, I have no idea if its complete or not


-- Zip Zap notes ---

Bg for select screens seems to be corrupt

-- General Notes --

Fantasia etc. games are locking up when the girl 'changes' due to not liking
the way we handle OKI status reads.. however these reads are correct according to
tests done with a real chip so there must be something odd going on on this hardware

From Miss World 96 manual/dipswitch sheet:

A/B/C Three Versions depending on nude grade
 A-Version is the extreme hottest nude models
 B-Version is the more attractive nude models
 C-Version is very beautiful bikini models

An example of this can be seen in the Fantasia II sets with type A & B
The current set of Super Model is an example of type C

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/kaneko16.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h"
#include "includes/galpanic.h"
#include "includes/galpnipt.h"

static SCREEN_VBLANK( galpanic )
{
	// rising edge
	if (vblank_on)
	{
		device_t *pandora = screen.machine().device("pandora");
		pandora_eof(pandora);
	}
}

static TIMER_DEVICE_CALLBACK( galpanic_scanline )
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		cputag_set_input_line(timer.machine(), "maincpu", 3, HOLD_LINE);

	/* Pandora "sprite end dma" irq? */
	if(scanline == 32)
		cputag_set_input_line(timer.machine(), "maincpu", 5, HOLD_LINE);
}


static TIMER_DEVICE_CALLBACK( galhustl_scanline )
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		cputag_set_input_line(timer.machine(), "maincpu", 3, HOLD_LINE);

	/* Pandora "sprite end dma" irq? */
	if(scanline == 32)
		cputag_set_input_line(timer.machine(), "maincpu", 4, HOLD_LINE);

	if(scanline == 0) // timer irq?
		cputag_set_input_line(timer.machine(), "maincpu", 5, HOLD_LINE);
}


static WRITE16_HANDLER( galpanic_6295_bankswitch_w )
{
	device_t *pandora = space->machine().device("pandora");

	if (ACCESSING_BITS_8_15)
	{
		UINT8 *rom = space->machine().region("oki")->base();

		memcpy(&rom[0x30000],&rom[0x40000 + ((data >> 8) & 0x0f) * 0x10000],0x10000);

		// used before title screen
		pandora_set_clear_bitmap(pandora, (data & 0x8000)>>15);
	}
}

static WRITE16_HANDLER( galpanica_6295_bankswitch_w )
{
	if (ACCESSING_BITS_8_15)
	{
		UINT8 *rom = space->machine().region("oki")->base();

		memcpy(&rom[0x30000],&rom[0x40000 + ((data >> 8) & 0x0f) * 0x10000],0x10000);
	}
}

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( galpanica_misc_w )
{
	device_t *pandora = machine.device("pandora");

	if (ACCESSING_BITS_0_7)
	{
		pandora_set_clear_bitmap(pandora, data & 0x0004);
	}

	// other bits unknown !
}
#endif

static WRITE16_HANDLER( galpanic_coin_w )
{
	if (ACCESSING_BITS_8_15)
	{
		coin_counter_w(space->machine(), 0, data & 0x100);
		coin_counter_w(space->machine(), 1, data & 0x200);

		coin_lockout_w(space->machine(), 0, ~data & 0x400);
		coin_lockout_w(space->machine(), 1, ~data & 0x800);
	}
}

static WRITE16_HANDLER( galpanic_bgvideoram_mirror_w )
{
	int i;
	for(i = 0; i < 8; i++)
	{
		// or offset + i * 0x2000 ?
		galpanic_bgvideoram_w(space, offset * 8 + i, data, mem_mask);
	}
}

static ADDRESS_MAP_START( galpanic_map, AS_PROGRAM, 16, galpanic_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	AM_RANGE(0x400000, 0x400001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_BASE(m_fgvideoram) AM_SIZE(m_fgvideoram_size)
	AM_RANGE(0x520000, 0x53ffff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_w) AM_BASE(m_bgvideoram)	/* + work RAM */
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_WRITE_LEGACY(galpanic_paletteram_w) AM_SHARE("paletteram")	/* 1024 colors, but only 512 seem to be used */
	AM_RANGE(0x700000, 0x701fff) AM_DEVREADWRITE_LEGACY("pandora", pandora_spriteram_LSB_r, pandora_spriteram_LSB_w)
	AM_RANGE(0x702000, 0x704fff) AM_RAM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x900000, 0x900001) AM_WRITE_LEGACY(galpanic_6295_bankswitch_w)
	AM_RANGE(0xa00000, 0xa00001) AM_WRITE_LEGACY(galpanic_coin_w)	/* coin counters */
	AM_RANGE(0xb00000, 0xb00001) AM_WRITENOP	/* ??? */
	AM_RANGE(0xc00000, 0xc00001) AM_WRITENOP	/* ??? */
	AM_RANGE(0xd00000, 0xd00001) AM_WRITENOP	/* ??? */
	AM_RANGE(0xe00000, 0xe00015) AM_READWRITE(galpanib_calc_r,galpanib_calc_w) /* CALC1 MCU interaction (simulated) */
ADDRESS_MAP_END

static READ16_HANDLER( comad_timer_r )
{
	return (space->machine().primary_screen->vpos() & 0x07) << 8;
}

/* a kludge! */
static READ8_DEVICE_HANDLER( comad_okim6295_r )
{
	UINT16 retvalue;
//  okim6295_device *oki = downcast<okim6295_device *>(device);

//  retvalue = oki->read_status(); // doesn't work, causes lockups when girls change..
	retvalue = device->machine().rand();

	return retvalue;
}

static ADDRESS_MAP_START( comad_map, AS_PROGRAM, 16, galpanic_state )
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_BASE(m_fgvideoram) AM_SIZE(m_fgvideoram_size)
	AM_RANGE(0x520000, 0x53ffff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_w) AM_BASE(m_bgvideoram)	/* + work RAM */
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_WRITE_LEGACY(galpanic_paletteram_w) AM_SHARE("paletteram")	/* 1024 colors, but only 512 seem to be used */
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
//  AM_RANGE(0x800006, 0x800007)    ??
	AM_RANGE(0x80000a, 0x80000b) AM_READ_LEGACY(comad_timer_r)	/* bits 8-a = timer? palette update code waits for them to be 111 */
	AM_RANGE(0x80000c, 0x80000d) AM_READ_LEGACY(comad_timer_r)	/* missw96 bits 8-a = timer? palette update code waits for them to be 111 */
	AM_RANGE(0x900000, 0x900001) AM_WRITE_LEGACY(galpanica_6295_bankswitch_w)	/* not sure */
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM				/* missw96 */
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM				/* fantasia, newfant */
	AM_RANGE(0xf00000, 0xf00001) AM_DEVREAD8_LEGACY("oki", comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)	/* fantasia, missw96 */
	AM_RANGE(0xf80000, 0xf80001) AM_DEVREAD8_LEGACY("oki", comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)	/* newfant */
ADDRESS_MAP_END

static ADDRESS_MAP_START( fantsia2_map, AS_PROGRAM, 16, galpanic_state )
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_BASE(m_fgvideoram) AM_SIZE(m_fgvideoram_size)
	AM_RANGE(0x520000, 0x53ffff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_w) AM_BASE(m_bgvideoram)	/* + work RAM */
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_WRITE_LEGACY(galpanic_paletteram_w) AM_SHARE("paletteram")	/* 1024 colors, but only 512 seem to be used */
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
//  AM_RANGE(0x800006, 0x800007)    ??
	AM_RANGE(0x800008, 0x800009) AM_READ_LEGACY(comad_timer_r)	/* bits 8-a = timer? palette update code waits for them to be 111 */
	AM_RANGE(0x900000, 0x900001) AM_WRITE_LEGACY(galpanica_6295_bankswitch_w)	/* not sure */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP	/* coin counters, + ? */
	AM_RANGE(0xc80000, 0xc80001) AM_DEVREAD8_LEGACY("oki", comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)
	AM_RANGE(0xf80000, 0xf8ffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( galhustl_map, AS_PROGRAM, 16, galpanic_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
    AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_BASE(m_fgvideoram) AM_SIZE(m_fgvideoram_size)
	AM_RANGE(0x520000, 0x53ffff) AM_WRITE_LEGACY(galpanic_bgvideoram_w) AM_BASE(m_bgvideoram)
	AM_RANGE(0x580000, 0x583fff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_mirror_w)
	AM_RANGE(0x600000, 0x6007ff) AM_RAM_WRITE_LEGACY(galpanic_paletteram_w) AM_SHARE("paletteram")	/* 1024 colors, but only 512 seem to be used */
	AM_RANGE(0x600800, 0x600fff) AM_RAM	// writes only 1?
	AM_RANGE(0x680000, 0x68001f) AM_RAM	// regs?
	AM_RANGE(0x700000, 0x700fff) AM_RAM	AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x780000, 0x78001f) AM_RAM	// regs?
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x900000, 0x900001) AM_WRITE_LEGACY(galpanica_6295_bankswitch_w)
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP // ?
	AM_RANGE(0xd00000, 0xd00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)
	AM_RANGE(0xe80000, 0xe8ffff) AM_RAM
ADDRESS_MAP_END

#ifdef UNUSED_FUNCTION
READ16_HANDLER( zipzap_random_read )
{
    return space->machine().rand();
}
#endif

static ADDRESS_MAP_START( zipzap_map, AS_PROGRAM, 16, galpanic_state )
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_BASE(m_fgvideoram) AM_SIZE(m_fgvideoram_size)
	AM_RANGE(0x520000, 0x53ffff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_w) AM_BASE(m_bgvideoram)
	AM_RANGE(0x580000, 0x583fff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_mirror_w)
	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE_LEGACY(galpanic_paletteram_w) AM_SHARE("paletteram")	/* 1024 colors, but only 512 seem to be used */
	AM_RANGE(0x680000, 0x68001f) AM_RAM
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x701000, 0x71ffff) AM_RAM
	AM_RANGE(0x780000, 0x78001f) AM_RAM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x900000, 0x900001) AM_WRITE_LEGACY(galpanica_6295_bankswitch_w)

	AM_RANGE(0xc00000, 0xc00001) AM_DEVREAD8_LEGACY("oki", comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)	/* fantasia, missw96 */

	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM		// main ram
ADDRESS_MAP_END

static ADDRESS_MAP_START( supmodel_map, AS_PROGRAM, 16, galpanic_state )
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_BASE(m_fgvideoram) AM_SIZE(m_fgvideoram_size)
	AM_RANGE(0x520000, 0x53ffff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_w) AM_BASE(m_bgvideoram)
//  AM_RANGE(0x580000, 0x583fff) AM_RAM_WRITE_LEGACY(galpanic_bgvideoram_mirror_w) // can't be right, causes half the display to vanish at times!
	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE_LEGACY(galpanic_paletteram_w) AM_SHARE("paletteram")	/* 1024 colors, but only 512 seem to be used */
	AM_RANGE(0x680000, 0x68001f) AM_RAM
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)
	AM_RANGE(0x780000, 0x78001f) AM_RAM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x800006, 0x800007) AM_READ_LEGACY(comad_timer_r)
	AM_RANGE(0x800008, 0x800009) AM_READ_LEGACY(comad_timer_r)
	AM_RANGE(0x900000, 0x900001) AM_WRITE_LEGACY(galpanica_6295_bankswitch_w)	/* not sure */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM
	AM_RANGE(0xd80000, 0xd80001) AM_WRITENOP
	AM_RANGE(0xe00012, 0xe00013) AM_WRITENOP
	AM_RANGE(0xe80000, 0xe80001) AM_WRITENOP
	AM_RANGE(0xf80000, 0xf80001) AM_DEVREAD8_LEGACY("oki", comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)	/* fantasia, missw96 */
ADDRESS_MAP_END


static INPUT_PORTS_START( galpanic )
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:2") /* flip screen? - code at 0x000522 */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	COINAGE_WORLD
	GALS_PANIC_JOYSTICK_4WAY(1)			/* "Shot2" is shown in "test mode" but not used by the game */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW2:7") /* demo sounds? - see notes */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Character Test" )	PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	GALS_PANIC_JOYSTICK_4WAY(2)			/* "Shot2" is shown in "test mode" but not used by the game */

	SYSTEM_SERVICE
INPUT_PORTS_END

static INPUT_PORTS_START( galpanica )
	PORT_START("DSW1")
	COINAGE_TEST_LOC		/* Unknown DSW switch 2 is flip screen? - code at 0x00060a */
	GALS_PANIC_JOYSTICK_4WAY(1)

	PORT_START("DSW2")
	DIFFICULTY_DEMO_SOUNDS
	GALS_PANIC_JOYSTICK_4WAY(2)

	SYSTEM_SERVICE
INPUT_PORTS_END

static INPUT_PORTS_START( fantasia )
	PORT_START("DSW1")
	DIFFICULTY_DEMO_SOUNDS		/* Unknown dip might be freeze/vblank? - code at 0x000734 ('fantasia') or 0x00075a ('newfant') - not called ? */
	GALS_PANIC_JOYSTICK_4WAY(1)	/* "Shot2" is shown in "test mode" but not used by the game */

	PORT_START("DSW2")
	COINAGE_TEST_LOC		/* Unknown DSW switch 2 is flip screen? - code at 0x00021c */
	GALS_PANIC_JOYSTICK_4WAY(2)	/* "Shot2" is shown in "test mode" but not used by the game */

	SYSTEM_NO_SERVICE		/* MAME may crash when TILT is pressed (see notes), "Service" is shown in "test mode" */
INPUT_PORTS_END

/* Same as 'fantasia', but no "Service Mode" Dip Switch (and thus no "hidden" buttons) */
static INPUT_PORTS_START( missw96 )
	PORT_START("DSW1")
	DIFFICULTY_DEMO_SOUNDS		/* Unknown dip might be freeze/vblank? - code at 0x00074e - not called ? */
	GALS_PANIC_JOYSTICK_4WAY(1)

	PORT_START("DSW2")
	COINAGE_NO_TEST_LOC		/* Unknown DSW switch 2 is flip screen? - code at 0x00021c */
	GALS_PANIC_JOYSTICK_4WAY(2)

	SYSTEM_NO_SERVICE		/* MAME may crash when TILT is pressed (see notes) */
INPUT_PORTS_END

static INPUT_PORTS_START( galhustl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )	PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0003, "8" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
	GALS_PANIC_JOYSTICK_8WAY(1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )	PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )			/* 5000 - 7000 */
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )		/* 4000 - 6000 */
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )			/* 6000 - 8000 */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )		/* 7000 - 9000 */
	PORT_DIPNAME( 0x0060, 0x0060, "Play Time" )		PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, "120 Sec" )
	PORT_DIPSETTING(      0x0060, "100 Sec" )
	PORT_DIPSETTING(      0x0020, "80 Sec" )
	PORT_DIPSETTING(      0x0000, "70 Sec" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	GALS_PANIC_JOYSTICK_8WAY(2)

	SYSTEM_NO_TILT
INPUT_PORTS_END

static INPUT_PORTS_START( zipzap )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Additional Obsticals" ) /* Adds 4 Blocker/Bumpers to playing field */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	GALS_PANIC_JOYSTICK_8WAY(1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Select Player Mode" ) /* Amateur, Normal & Exelent Modes */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	GALS_PANIC_JOYSTICK_8WAY(2)

	SYSTEM_NO_TILT
INPUT_PORTS_END


static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			64*4, 65*4, 66*4, 67*4, 68*4, 69*4, 70*4, 71*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	128*8
};

static GFXDECODE_START( galpanic )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,  256, 16 )
GFXDECODE_END


static const kaneko_pandora_interface galpanic_pandora_config =
{
	"screen",	/* screen tag */
	0,	/* gfx_region */
	0, -16	/* x_offs, y_offs */
};


static MACHINE_CONFIG_START( galpanic, galpanic_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz) /* verified on pcb */
	MCFG_CPU_PROGRAM_MAP(galpanic_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", galpanic_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)	/* frames per second, vblank duration */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_STATIC(galpanic)
	MCFG_SCREEN_VBLANK_STATIC( galpanic )

	MCFG_GFXDECODE(galpanic)
	MCFG_PALETTE_LENGTH(1024 + 32768)

	MCFG_KANEKO_PANDORA_ADD("pandora", galpanic_pandora_config)

	MCFG_PALETTE_INIT(galpanic)
	MCFG_VIDEO_START(galpanic)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", XTAL_12MHz/6, OKIM6295_PIN7_LOW) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( galpanica, galpanic )

	/* basic machine hardware */

	/* arm watchdog */
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))	/* a guess, and certainly wrong */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( comad, galpanic )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(10000000)
	MCFG_CPU_PROGRAM_MAP(comad_map)

	MCFG_DEVICE_REMOVE("pandora")

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(comad)
	MCFG_SCREEN_VBLANK_NONE()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supmodel, comad )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)	/* ? */
	MCFG_CPU_PROGRAM_MAP(supmodel_map)
//  MCFG_TIMER_ADD_SCANLINE("scantimer", galpanic_scanline, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(comad)
	MCFG_SCREEN_VBLANK_NONE()

	/* sound hardware */
	MCFG_OKIM6295_REPLACE("oki", 1584000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( fantsia2, comad )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)	/* ? */
	MCFG_CPU_PROGRAM_MAP(fantsia2_map)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(comad)
	MCFG_SCREEN_VBLANK_NONE()
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( galhustl, comad )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)	/* ? */
	MCFG_CPU_PROGRAM_MAP(galhustl_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_CALLBACK(galhustl_scanline)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(comad)
	MCFG_SCREEN_VBLANK_NONE()

	/* sound hardware */
	MCFG_OKIM6295_REPLACE("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( zipzap, comad )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(12000000)	/* ? */
	MCFG_CPU_PROGRAM_MAP(zipzap_map)
	MCFG_TIMER_MODIFY("scantimer")
	MCFG_TIMER_CALLBACK(galhustl_scanline)

	/* video hardware */
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_STATIC(comad)

	/* sound hardware */
	MCFG_OKIM6295_REPLACE("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galpanic ) /* PAMERA-04 PCB with the PAMERA-SUB daughter card and unpopulated CALC1 MCU socket */
	ROM_REGION( 0x400000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "pm110.4m2",    0x000000, 0x80000, CRC(ae6b17a8) SHA1(f3a625eef45cc85cdf9760f77ea7ce93387911f9) )
	ROM_LOAD16_BYTE( "pm109.4m1",    0x000001, 0x80000, CRC(b85d792d) SHA1(0ed78e15f6e58285ce6944200b023ada1e673b0e) )
	ROM_LOAD16_BYTE( "pm112.subic6", 0x000000, 0x20000, CRC(7b972b58) SHA1(a7f619fca665b15f4f004ae739f5776ee2d4d432) ) /* Located on the PAMERA-SUB daughter card */
	ROM_LOAD16_BYTE( "pm111.subic5", 0x000001, 0x20000, CRC(4eb7298d) SHA1(8858a40ffefbe4ecea7d5b70311c3775b7d987eb) ) /* Located on the PAMERA-SUB daughter card */
	ROM_LOAD16_BYTE( "pm004e.8",     0x100001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.7",     0x100000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.15",    0x200001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.14",    0x200000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.17",    0x300001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.16",    0x300000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "pm006e.67",    0x000000, 0x100000, CRC(57aec037) SHA1(e6ba095b6892d4dcd76ba3343a97dd98ae29dc24) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.l",     0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "pm007e.u",     0xc0000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galpanica ) /* PAMERA-04 PCB with the CALC1 MCU used */
	ROM_REGION( 0x400000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "pm110.4m2",    0x000000, 0x80000, CRC(ae6b17a8) SHA1(f3a625eef45cc85cdf9760f77ea7ce93387911f9) )
	ROM_LOAD16_BYTE( "pm109.4m1",    0x000001, 0x80000, CRC(b85d792d) SHA1(0ed78e15f6e58285ce6944200b023ada1e673b0e) )
	ROM_LOAD16_BYTE( "pm004e.8",     0x100001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.7",     0x100000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.15",    0x200001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.14",    0x200000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.17",    0x300001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.16",    0x300000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "pm006e.67",    0x000000, 0x100000, CRC(57aec037) SHA1(e6ba095b6892d4dcd76ba3343a97dd98ae29dc24) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.l",     0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "pm007e.u",     0xc0000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END


/*
Fantasia II
Comad, 1997

Game is a copy/clone of Qix etc, with the usual Comad theme.....
The hardware looks much nicer/cleaner and more professionally made than previous
Comad boards I've seen also.


CPU   : MC68000P12
Sound : AD-65 (OKI M6295)
Osc.  : 12.000MHz, 16.000MHz (both near 68000 & PLCC84)
DIP Sw: 8 position (x2)
RAM   : 62256 (x12), 6116 (x4)
PALs  : plenty .....
OTHER : ACTEL A1020B (84 Pin PLCC)

ROMs: (all type 27C040)

music* - oki samples / music
prog*  - main program
obj*   - objects
scr*   - gfx
*/



// fantasy 95 - derived from new fantasia?
ROM_START( fantsy95 )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "prog2.12",  0x000000, 0x80000, CRC(1e684da7) SHA1(2104a6fb5f019011009f4faa769afcada90cff97) )
	ROM_LOAD16_BYTE( "prog1.7",   0x000001, 0x80000, CRC(dc4e4f6b) SHA1(9934121692a6d32164bef03c72c25dc727438e54) )
	ROM_LOAD16_BYTE( "i-scr2.10", 0x100000, 0x80000, CRC(ab8756ff) SHA1(0a7aa977151962e67b15a7e0f819b1412ff8dbdc) )
	ROM_LOAD16_BYTE( "i-scr1.5",  0x100001, 0x80000, CRC(d8e2ef77) SHA1(ec2c1dcc13e281288b5df43fa7a0b3cdf7357459) )
	ROM_LOAD16_BYTE( "i-scr4.9",  0x200000, 0x80000, CRC(4e52eb23) SHA1(be61c0dc68c49ded2dc6e8852fd92acac4986700) )
	ROM_LOAD16_BYTE( "i-scr3.4",  0x200001, 0x80000, CRC(797731f8) SHA1(571f939a7f85bd5b75a0660621961b531f44f736) )
	ROM_LOAD16_BYTE( "i-scr6.8",  0x300000, 0x80000, CRC(6f8e5239) SHA1(a1c2ec79e80906ca18cf3532ce38a1495ab37e44) )
	ROM_LOAD16_BYTE( "i-scr5.3",  0x300001, 0x80000, CRC(85420e3f) SHA1(d29e81cb1a33dca6232e14a0df2e21c8de45ba71) )
	ROM_LOAD16_BYTE( "i-scr8.11", 0x400000, 0x80000, CRC(33db8177) SHA1(9e9aa890dfa20e5aa6f1caec7d018d992217c2fe) )
	ROM_LOAD16_BYTE( "i-scr7.6",  0x400001, 0x80000, CRC(8662dd01) SHA1(a349c1cd965d3d51c20178fcce2f61ae76f4006a) )

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "obj1.13",  0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1.1", 0x00000, 0x80000, CRC(3117e2ef) SHA1(6581a7104556d44f814c537bbd74998922927034) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2.2", 0xc0000, 0x80000, CRC(0c1109f9) SHA1(0e4ea534a32b1649e2e9bb8af7254b917ec03a90) )
ROM_END

ROM_START( newfant )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "prog2_12.rom", 0x000000, 0x80000, CRC(de43a457) SHA1(91db13f63b46146131c58e775119ea3b073ca409) )
	ROM_LOAD16_BYTE( "prog1_07.rom", 0x000001, 0x80000, CRC(370b45be) SHA1(775873df9d3af803dbd1a392a45cad5f37b1b1c7) )
	ROM_LOAD16_BYTE( "iscr2_10.rom", 0x100000, 0x80000, CRC(4f2da2eb) SHA1(4f0b72327d1bdfad24d822953f45218bfae29cff) )
	ROM_LOAD16_BYTE( "iscr1_05.rom", 0x100001, 0x80000, CRC(63c6894f) SHA1(213544da570a167f3411357308c576805f6882f3) )
	ROM_LOAD16_BYTE( "iscr4_09.rom", 0x200000, 0x80000, CRC(725741ec) SHA1(3455cf0aed9653c66b8b2f905ad683687d517419) )
	ROM_LOAD16_BYTE( "iscr3_04.rom", 0x200001, 0x80000, CRC(51d6b362) SHA1(bcd57643ac9d79c150714ec6b6a2bb8a24acf7a5) )
	ROM_LOAD16_BYTE( "iscr6_08.rom", 0x300000, 0x80000, CRC(178b2ef3) SHA1(d3c092a282278968a319e06731481570f217d404) )
	ROM_LOAD16_BYTE( "iscr5_03.rom", 0x300001, 0x80000, CRC(d2b5c5fa) SHA1(80fde69bc5f4e958b5d57a5179b6e601192780f4) )
	ROM_LOAD16_BYTE( "iscr8_11.rom", 0x400000, 0x80000, CRC(f4148528) SHA1(4e27fff0b7ead068a159b3ed80c5793a6166fc4e) )
	ROM_LOAD16_BYTE( "iscr7_06.rom", 0x400001, 0x80000, CRC(2dee0c31) SHA1(1097006e6e5d16b24fb71615b6c0754fe0ecbe33) )

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "obj1_13.rom",  0x00000, 0x80000, CRC(e6d1bc71) SHA1(df0b6c1742c01991196659bab2691230323e7b8d) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "musc1_01.rom", 0x00000, 0x80000, CRC(10347fce) SHA1(f5fbe8ef363fe18b7104be5d2fa92943d1a5d7a2) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "musc2_02.rom", 0xc0000, 0x80000, CRC(b9646a8c) SHA1(e9432261ac86e4251a2c97301c6d014c05110a9c) )
ROM_END

ROM_START( missw96 )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "mw96_10.bin",  0x000000, 0x80000, CRC(b1309bb1) SHA1(3cc7a903cb007d8fc0f836a33780c1c9231d1629) )
	ROM_LOAD16_BYTE( "mw96_06.bin",  0x000001, 0x80000, CRC(a5892bb3) SHA1(99130eb0af307fe66c9668414475e003f9c7d969) )
	ROM_LOAD16_BYTE( "mw96_09.bin",  0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin",  0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin",  0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin",  0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin",  0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin",  0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "mw96_11.bin",  0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "mw96_01.bin",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "mw96_02.bin",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missmw96 )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "mmw96_10.bin",  0x000000, 0x80000, CRC(45ed1cd9) SHA1(a75b1b6cddde065e6d7f7355a746819c8268c24f) )
	ROM_LOAD16_BYTE( "mmw96_06.bin",  0x000001, 0x80000, CRC(52ec9e5d) SHA1(20b7cc923e9d55e391b09d96248837bb8f28a176) )
	ROM_LOAD16_BYTE( "mmw96_09.bin",  0x100000, 0x80000, CRC(6c458b05) SHA1(249490c45cdecd6496338286a9ab6a6137cefcd0) )
	ROM_LOAD16_BYTE( "mmw96_05.bin",  0x100001, 0x80000, CRC(48159555) SHA1(a7c736f9e41915d06b7242e427282c421c4a8283) )
	ROM_LOAD16_BYTE( "mmw96_08.bin",  0x200000, 0x80000, CRC(1dc72b07) SHA1(fdbdf8298fe98d74ed2a76abf60f60af1c27a65d) )
	ROM_LOAD16_BYTE( "mmw96_04.bin",  0x200001, 0x80000, CRC(fc3e18fa) SHA1(b3ad254aab982dc75a10c2cf2b3815c2fdbba914) )
	ROM_LOAD16_BYTE( "mmw96_07.bin",  0x300000, 0x80000, CRC(001572bf) SHA1(cdf59c624baaeaea70985ee6f2f2fed08a8dfa61) )
	ROM_LOAD16_BYTE( "mmw96_03.bin",  0x300001, 0x80000, CRC(22204025) SHA1(442e7f754c65c598983d6f897a60870d7759c823) )

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "mmw96_11.bin",  0x00000, 0x80000, CRC(7d491f8c) SHA1(63f580bd65579cac70b90eaa0e7f2413ef1597b8) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "mw96_01.bin",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "mw96_02.bin",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( fantsia2 )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "prog2.g17",    0x000000, 0x80000, CRC(57c59972) SHA1(4b1da928b537cf340a67026d07bc3dfc078b0d0f) )
	ROM_LOAD16_BYTE( "prog1.f17",    0x000001, 0x80000, CRC(bf2d9a26) SHA1(92f0c1bd32f1e5e0ede3ba847242a212dfae4986) )
	ROM_LOAD16_BYTE( "scr2.g16",     0x100000, 0x80000, CRC(887b1bc5) SHA1(b6fcdc8a56ea25758f363224d256e9b6c8e30244) )
	ROM_LOAD16_BYTE( "scr1.f16",     0x100001, 0x80000, CRC(cbba3182) SHA1(a484819940fa1ef18ce679465c31075798748bac) )
	ROM_LOAD16_BYTE( "scr4.g15",     0x200000, 0x80000, CRC(ce97e411) SHA1(be0ed41362db03f384229c708f2ba4146e5cb501) )
	ROM_LOAD16_BYTE( "scr3.f15",     0x200001, 0x80000, CRC(480cc2e8) SHA1(38fe57ba1e34537f8be65fcc023ccd43369a5d94) )
	ROM_LOAD16_BYTE( "scr6.g14",     0x300000, 0x80000, CRC(b29d49de) SHA1(854b76755acf58fb8a4648a0ce72ea6bdf26c555) )
	ROM_LOAD16_BYTE( "scr5.f14",     0x300001, 0x80000, CRC(d5f88b83) SHA1(518a1f6732149f2851bbedca61f7313c39beb91b) )
	ROM_LOAD16_BYTE( "scr8.g20",     0x400000, 0x80000, CRC(694ae2b3) SHA1(82b7a565290fce07c8393af4718fd1e6136928e9) )
	ROM_LOAD16_BYTE( "scr7.f20",     0x400001, 0x80000, CRC(6068712c) SHA1(80a136d76dca566772e34d832ac11b8c7d6ce9ab) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "obj1.1i",      0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "obj2.2i",      0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music2.1b",    0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music1.1a",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( fantsia2a )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "fnt2-22.bin",    0x000000, 0x80000, CRC(a3a92c4b) SHA1(6affdcb57e1e0a77c7cc33135dafe86843e9e3d8) )
	ROM_LOAD16_BYTE( "fnt2-17.bin",    0x000001, 0x80000, CRC(d0ce4493) SHA1(9cec088e6630555b6d584df23236c279909820cf) )
	ROM_LOAD16_BYTE( "fnt2-21.bin",    0x100000, 0x80000, CRC(e989c2e7) SHA1(c9eea2a89843cdd9db4a4a0539d0315c125e3e02) )
	ROM_LOAD16_BYTE( "fnt2-16.bin",    0x100001, 0x80000, CRC(8c06d372) SHA1(14fe2c8450f0f2e11e204dd524bfe32a72ddc144) )
	ROM_LOAD16_BYTE( "fnt2-20.bin",    0x200000, 0x80000, CRC(6e9f1e65) SHA1(b6f1eb1a52de18ed5b17de3ef365e5c041d15314) )
	ROM_LOAD16_BYTE( "fnt2-15.bin",    0x200001, 0x80000, CRC(85cbeb2b) SHA1(a213b461019ddb3b319b9815a76c6fb2ecfbe937) )
	ROM_LOAD16_BYTE( "fnt2-19.bin",    0x300000, 0x80000, CRC(7953226a) SHA1(955c779eae496688be2ed416d879d6e83c888368) )
	ROM_LOAD16_BYTE( "fnt2-14.bin",    0x300001, 0x80000, CRC(10d8ccff) SHA1(bf4c49d85556edf49289631ee6178d3fb7dea2cc) )
	ROM_LOAD16_BYTE( "fnt2-18.bin",    0x400000, 0x80000, CRC(4cdaeda3) SHA1(f5b478e49b59496865982409517654f48296565d) )
	ROM_LOAD16_BYTE( "fnt2-13.bin",    0x400001, 0x80000, CRC(68c7f042) SHA1(ed3c864f3d91377fec78f19897ba0b0d2bcf0d2b) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "obj1.1i",      0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "obj2.2i",      0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music2.1b",    0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music1.1a",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( wownfant)
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "ep-4001 42750001 u81.bin",    0x000000, 0x80000, CRC(9942d200) SHA1(d2f69c0949881ef4aef202b564eac069c030a497) )
	ROM_LOAD16_BYTE( "ep-4001 42750001 u80.bin",    0x000001, 0x80000, CRC(17359eeb) SHA1(90bb9da6bdf56fa9eb0ad03691750518a2a3f879) )
	ROM_LOAD16_WORD_SWAP( "ep-061 43750002 - 1.bin",    0x100000, 0x200000, CRC(c318e841) SHA1(ba7af736d3b0accca474b0de1c8299eb3c449ef9) )
	ROM_LOAD16_WORD_SWAP( "ep-061 43750002 - 2.bin",    0x300000, 0x200000, CRC(8871dc3a) SHA1(8e028f1430474df19bb9a912ee9e407fe4582558) )

	ROM_REGION( 0x100000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "ep-4001 42750001 u113.bin",      0x00000, 0x80000, CRC(3e77ca1f) SHA1(f946e65a29bc02b89c02b2a869578d38cfe7e2d0) )
	ROM_LOAD( "ep-4001 42750001 u112.bin",      0x80000, 0x80000, CRC(51f4b604) SHA1(52e8ce0a2c1b9b00f04e0c775789bc550bad8ae0) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "ep-4001 42750001 u4.bin",    0x00000, 0x80000, CRC(06dc889e) SHA1(726561ff01bbde43669293a6ff7ee22b048b4118) ) // almost the same as fantasia2, just some changes to the sample references in the header
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "ep-4001 42750001 u1.bin",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( galhustl )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ue17.3", 0x00000, 0x80000, CRC(b2583dbb) SHA1(536f4aa2246ec816c4f270f9d42acc090718ee8b) )
	ROM_LOAD16_BYTE( "ud17.4", 0x00001, 0x80000, CRC(470a3668) SHA1(ad86e96ab8f1f5da23fb1feaabfb9c757965418e) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "galhstl1.ub6", 0x00000, 0x80000,  CRC(23848790) SHA1(2e77fbe04f46e258daecb4c5917e383c7c06a306) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "galhstl2.uc6", 0xc0000, 0x80000,  CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "galhstl5.u5", 0x00000, 0x80000, CRC(44a18f15) SHA1(1217cf7fbbb442358b15016099efeface5dcbd22) )
ROM_END

/*

Zip & Zap

Zip Zap (pcb marked Barko Corp 950509)

1x 68k
1x Oki m6295
1x osc 12mhz
1x osc 16mhz
1x fpga
2x dipswitch banks

*/

ROM_START( zipzap )
	ROM_REGION( 0x500000, "maincpu", 0 ) /* 68000 Code */
	/* all the roms for this game could do with checking on another board, this one was in pretty bad condition
       and reads weren't always consistent */
	ROM_LOAD16_BYTE( "ud17.bin", 0x000001, 0x40000, BAD_DUMP CRC(2901fae1) SHA1(0d6ca6d48c5586c05f3c02aee51a95da38b3751f) )
	ROM_LOAD16_BYTE( "ue17.bin", 0x000000, 0x40000, BAD_DUMP CRC(da6c3fc8) SHA1(4bc01bc6f62553f6ac4f7252f7d9bf0d639f6935) )
	/* gfx bitmaps */
	ROM_LOAD16_BYTE( "938.bin",  0x400000, 0x80000, CRC(61c06b60) SHA1(b3abae020009a48b99862766e0981e1118159a47) ) // good title backgruond
	ROM_LOAD16_BYTE( "942.bin",  0x400001, 0x80000, CRC(282413b8) SHA1(e2ecaaa3c5b2355eadc016b73d7d658f25e1e0db) ) // (and corrupt gfx on select mode screen)

	ROM_LOAD16_BYTE( "934.bin",  0x300000, 0x80000, CRC(1e65988a) SHA1(64d6f8cbdb28755515d9bbf52f589ce1176fed58) ) // good, girls
	ROM_LOAD16_BYTE( "939.bin",  0x300001, 0x80000, CRC(8790a6a3) SHA1(94f39e48b75144cab191e2de4284c28d18b8f1c7))

	ROM_LOAD16_BYTE( "936.bin",  0x200000, 0x80000, CRC(596543cc) SHA1(10a0eab4ca4a8749f1703ff6fcc80d731d07d087) ) // good, girls
	ROM_LOAD16_BYTE( "940.bin",  0x200001, 0x80000, CRC(0c9dfb53) SHA1(541bd8c79408b7415713b517eacdd565d0ac5cb8) )

	ROM_LOAD16_BYTE( "937.bin",  0x100000, 0x80000, CRC(61dd653f) SHA1(68b5ae3423363cc64d933836bf6881431dad021a) ) // good, girls
	ROM_LOAD16_BYTE( "941.bin",  0x100001, 0x80000, CRC(320321ed) SHA1(00b52cd34cd86c105ff6dbd0248ff239de31c851) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // sprites
	ROM_LOAD( "u5.bin",  0x000000, 0x80000,  CRC(c274d8b5) SHA1(2c45961aaf8311f027a734df7e33fe085dfdd099) )

	ROM_REGION( 0x140000, "oki", 0 ) /* Samples */
	ROM_LOAD( "snd.bin", 0x00000, 0x80000,  CRC(bc20423e) SHA1(1f4bd52ec4f9b3b3e6b10ac2b3afaadf76a2c7c9) )
	ROM_RELOAD(          0x40000, 0x80000 )
	ROM_RELOAD(          0xc0000, 0x80000 )
ROM_END

ROM_START( supmodel )
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "prog2.12",  0x000000, 0x80000, CRC(714b7e74) SHA1(a4f7754a4b04729084ccb1359f9bdfbad6150222) )
	ROM_LOAD16_BYTE( "prog1.7",   0x000001, 0x80000, CRC(0bb858de) SHA1(bd2039fa46fce89289e99a790400bd567f90105e) )
	ROM_LOAD16_BYTE( "i-scr2.10", 0x100000, 0x80000, CRC(d07ec0ce) SHA1(88997254ea2bffa83ab4a77087905cf646ee3c12) )
	ROM_LOAD16_BYTE( "i-scr1.5",  0x100001, 0x80000, CRC(a96a8bde) SHA1(e93de2df1391a8e94d655e1c9e148196e692e661) )
	ROM_LOAD16_BYTE( "i-scr4.9",  0x200000, 0x80000, CRC(e959cab5) SHA1(13d744aa71d9485a4530418536c38a542a269e27) )
	ROM_LOAD16_BYTE( "i-scr3.4",  0x200001, 0x80000, CRC(4bf5e082) SHA1(14ab9ebe0c7a2154275b0aeb76f99d73552d862f) )
	ROM_LOAD16_BYTE( "i-scr6.8",  0x300000, 0x80000, CRC(e71337c2) SHA1(be1b532e66e70f7d30b657a88c1f9b154187636e) )
	ROM_LOAD16_BYTE( "i-scr5.3",  0x300001, 0x80000, CRC(641ccdfb) SHA1(f48dc0461bc49cfe4adcf769e9abfe83efa077a1) )
	ROM_LOAD16_BYTE( "i-scr8.11", 0x400000, 0x80000, CRC(7c1813c8) SHA1(80fe97ac640847360529edfb728955e1067b0c14) )
	ROM_LOAD16_BYTE( "i-scr7.6",  0x400001, 0x80000, CRC(19c73268) SHA1(aa6dc8c817a2e9707ea74e219ab34cf826223741) )

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "obj1.13",  0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1.1", 0x00000, 0x80000, CRC(2b1f6655) SHA1(e7b52cf4bd16590c598c375d5a97b724bc9ef631) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2.2", 0xc0000, 0x80000, CRC(cccae65a) SHA1(5e4e2e51884eaf191f103aa189ff33371fc91d6d) )
ROM_END

GAME( 1990, galpanic, 0,        galpanic, galpanic, 0, ROT90, "Kaneko", "Gals Panic (Unprotected)", GAME_NO_COCKTAIL )
GAME( 1990, galpanica,galpanic, galpanica,galpanica,0, ROT90, "Kaneko", "Gals Panic (MCU Protected)", GAME_NO_COCKTAIL )
GAME( 1994, supmodel, 0,        supmodel, fantasia, 0, ROT90, "Comad & New Japan System", "Super Model",GAME_NO_COCKTAIL )
GAME( 1995, newfant,  0,        comad,    fantasia, 0, ROT90, "Comad & New Japan System", "New Fantasia", GAME_NO_COCKTAIL )
GAME( 1995, fantsy95, 0,        comad,    fantasia, 0, ROT90, "Hi-max Technology Inc.", "Fantasy '95", GAME_NO_COCKTAIL )
GAME( 1996, missw96,  0,        comad,    missw96,  0, ROT0,  "Comad", "Miss World '96 (Nude)", GAME_NO_COCKTAIL )
GAME( 1996, missmw96, missw96,  comad,    missw96,  0, ROT0,  "Comad", "Miss Mister World '96 (Nude)", GAME_NO_COCKTAIL )
GAME( 1997, fantsia2, 0,        fantsia2, missw96,  0, ROT0,  "Comad", "Fantasia II (Explicit)", GAME_NO_COCKTAIL )
GAME( 1997, fantsia2a,fantsia2, fantsia2, missw96,  0, ROT0,  "Comad", "Fantasia II (Less Explicit)", GAME_NO_COCKTAIL )
GAME( 2002, wownfant, 0,        fantsia2, missw96,  0, ROT0,  "Comad", "WOW New Fantasia", GAME_NO_COCKTAIL )
GAME( 1997, galhustl, 0,        galhustl, galhustl, 0, ROT0,  "ACE International", "Gals Hustler", 0 )
GAME( 1995, zipzap,   0,        zipzap,   zipzap,   0, ROT90, "Barko Corp", "Zip & Zap", GAME_IMPERFECT_GRAPHICS|GAME_IMPERFECT_SOUND )
