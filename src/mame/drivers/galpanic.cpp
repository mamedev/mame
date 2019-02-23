// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

Gals Panic       1990 Kaneko

driver by Nicola Salmoria

The version of Gals Panic in this driver is the one using the PANDORA chip
for sprites, other boardsets use a different sprite chip, see expro02.cpp


Notes about Gals Panic:
-----------------------
The current unprotected ROM set is strange because two ROMs overlap two others
replacing the program.

It's definitely a Kaneko boardset, but it could very well be they converted
some other game to run Gals Panic, because there's some ROMs piggybacked
on top of each other and some ROMs on a daughterboard plugged into smaller
sized ROM sockets. It's not a pirate version. The piggybacked ROMs even have
Kaneko stickers. The silkscreen on the board says PAMERA-04.




Stephh's additional notes :

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

  - I added the 'galpanica' romset which is in fact the same as 'galpanic',
    but with the PRG ROMS which aren't overwritten and simulated the CALC1
    MCU functions
    Here are a few notes about what I found :
      * This version is also a World version (0x03ffff.b = 03).
      * In this version, there is a "Coin Mode" Dip Switch, but no
        "Character Test" Dip Switch.
      * Area 0xe00000-0xe00014 is a "calculator" area. I've tried to
        simulate it (see machine/kaneko_hit.cpp) by comparing the code
        with the other set. I don't know if there are some other unmapped
        reads, but the game seems to run fine with what I've done.
      * When you press the "Tilt" button, the game enters in an endless
        loop, but this isn't a bug ! Check code beginning at 0x000e02 and
        ending at 0x000976 for more infos.
          -Expects watchdog to reset it- pjp
      * Sound hasn't been tested.



***************************************************************************/

#include "emu.h"
#include "includes/galpanic.h"
#include "includes/galpnipt.h"

#include "cpu/m68000/m68000.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "video/kan_pand.h"
#include "machine/kaneko_hit.h"
#include "speaker.h"


void galpanic_state::machine_start()
{
	membank("okibank")->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

WRITE_LINE_MEMBER(galpanic_state::screen_vblank)
{
	// rising edge
	if (state)
	{
		m_pandora->eof();
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(galpanic_state::scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		m_maincpu->set_input_line(3, HOLD_LINE);

	/* Pandora "sprite end dma" irq? */
	if(scanline == 32)
		m_maincpu->set_input_line(5, HOLD_LINE);
}




WRITE16_MEMBER(galpanic_state::m6295_bankswitch_w)
{
	if (ACCESSING_BITS_8_15)
	{
		membank("okibank")->set_entry((data >> 8) & 0x0f);

		// used before title screen
		m_pandora->set_clear_bitmap((data & 0x8000)>>15);
	}
}



WRITE16_MEMBER(galpanic_state::coin_w)
{
	if (ACCESSING_BITS_8_15)
	{
		machine().bookkeeping().coin_counter_w(0, data & 0x100);
		machine().bookkeeping().coin_counter_w(1, data & 0x200);

		machine().bookkeeping().coin_lockout_w(0, ~data & 0x400);
		machine().bookkeeping().coin_lockout_w(1, ~data & 0x800);
	}
}



void galpanic_state::galpanic_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	map(0x400001, 0x400001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x500000, 0x51ffff).ram().share("fgvideoram");
	map(0x520000, 0x53ffff).ram().w(FUNC(galpanic_state::bgvideoram_w)).share("bgvideoram");  /* + work RAM */
	map(0x600000, 0x6007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");  /* 1024 colors, but only 512 seem to be used */
	map(0x700000, 0x701fff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_LSB_r), FUNC(kaneko_pandora_device::spriteram_LSB_w));
	map(0x702000, 0x704fff).ram();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x900000, 0x900001).w(FUNC(galpanic_state::m6295_bankswitch_w));
	map(0xa00000, 0xa00001).w(FUNC(galpanic_state::coin_w));  /* coin counters */
	map(0xb00000, 0xb00001).nopw();    /* ??? */
	map(0xc00000, 0xc00001).nopw();    /* ??? */
	map(0xd00000, 0xd00001).nopw();    /* ??? */
}

void galpanic_state::galpanica_map(address_map &map)
{
	galpanic_map(map);
	map(0xe00000, 0xe00015).rw("calc1_mcu", FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w));
}

void galpanic_state::galpanic_oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank");
}


static INPUT_PORTS_START( galpanic )
	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SW1:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2") /* flip screen? - code at 0x000522 */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0004, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW1:4" )
	COINAGE_WORLD
	GALS_PANIC_JOYSTICK_4WAY(1)         /* "Shot2" is shown in "test mode" but not used by the game */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW2:7") /* demo sounds? - see notes */
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Character Test" )    PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	GALS_PANIC_JOYSTICK_4WAY(2)         /* "Shot2" is shown in "test mode" but not used by the game */

	SYSTEM_SERVICE
INPUT_PORTS_END

static INPUT_PORTS_START( galpanica )
	PORT_START("DSW1")
	COINAGE_TEST_LOC        /* Unknown DSW switch 2 is flip screen? - code at 0x00060a */
	GALS_PANIC_JOYSTICK_4WAY(1)

	PORT_START("DSW2")
	DIFFICULTY_DEMO_SOUNDS
	GALS_PANIC_JOYSTICK_4WAY(2)

	SYSTEM_SERVICE
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

static GFXDECODE_START( gfx_galpanic )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,  256, 16 )
GFXDECODE_END


MACHINE_CONFIG_START(galpanic_state::galpanic)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M68000, XTAL(12'000'000)) /* verified on pcb */
	MCFG_DEVICE_PROGRAM_MAP(galpanic_map)
	TIMER(config, "scantimer").configure_scanline(FUNC(galpanic_state::scanline), "screen", 0, 1);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)  /* frames per second, vblank duration */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 224-1)
	MCFG_SCREEN_UPDATE_DRIVER(galpanic_state, screen_update)
	MCFG_SCREEN_VBLANK_CALLBACK(WRITELINE(*this, galpanic_state, screen_vblank))
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_galpanic);
	// fg palette RAM, bit 0 seems to be a transparency flag for the front bitmap
	PALETTE(config, m_palette, FUNC(galpanic_state::galpanic_palette)).set_format(palette_device::GRBx_555, 1024 + 32768);

	KANEKO_PANDORA(config, m_pandora, 0);
	m_pandora->set_offsets(0, -16);
	m_pandora->set_gfxdecode_tag(m_gfxdecode);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("oki", OKIM6295, XTAL(12'000'000)/6, okim6295_device::PIN7_LOW) /* verified on pcb */
	MCFG_DEVICE_ADDRESS_MAP(0, galpanic_oki_map)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(galpanic_state::galpanica)
	galpanic(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_PROGRAM_MAP(galpanica_map)

	/* basic machine hardware */
	KANEKO_HIT(config, "calc1_mcu").set_type(0);

	/* arm watchdog */
	subdevice<watchdog_timer_device>("watchdog")->set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( galpanic ) /* PAMERA-04 PCB with the PAMERA-SUB daughter card and unpopulated CALC1 MCU socket */
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code */
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

	ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "pm006e.67",    0x000000, 0x100000, CRC(57aec037) SHA1(e6ba095b6892d4dcd76ba3343a97dd98ae29dc24) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.l",     0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_LOAD( "pm007e.u",     0x80000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galpanica ) /* PAMERA-04 PCB with the CALC1 MCU used */
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "pm110.4m2",    0x000000, 0x80000, CRC(ae6b17a8) SHA1(f3a625eef45cc85cdf9760f77ea7ce93387911f9) )
	ROM_LOAD16_BYTE( "pm109.4m1",    0x000001, 0x80000, CRC(b85d792d) SHA1(0ed78e15f6e58285ce6944200b023ada1e673b0e) )
	ROM_LOAD16_BYTE( "pm004e.8",     0x100001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.7",     0x100000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.15",    0x200001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.14",    0x200000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.17",    0x300001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.16",    0x300000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "pm006e.67",    0x000000, 0x100000, CRC(57aec037) SHA1(e6ba095b6892d4dcd76ba3343a97dd98ae29dc24) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.l",     0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_LOAD( "pm007e.u",     0x80000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galpanicb ) /* PAMERA-04 PCB with the CALC1 MCU used */
	ROM_REGION( 0x400000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "pm109p.u88-01.ic6", 0x000000, 0x20000, CRC(a6d60dba) SHA1(2a63642709051c27b9a366c433127426bb579c35) ) // read as 27C010
	ROM_LOAD16_BYTE( "pm110p.u87-01.ic5", 0x000001, 0x20000, CRC(3214fd48) SHA1(d8d77cb6b74caea2545f4e62eb9223aaf770785a) ) // read as 27C010
	ROM_LOAD16_BYTE( "pm004e.8",          0x100001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.7",          0x100000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.15",         0x200001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.14",         0x200000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.17",         0x300001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.16",         0x300000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "pm006e.67",    0x000000, 0x100000, CRC(57aec037) SHA1(e6ba095b6892d4dcd76ba3343a97dd98ae29dc24) )

	ROM_REGION( 0x100000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.l",     0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_LOAD( "pm007e.u",     0x80000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

GAME( 1990, galpanic,  0,        galpanic,  galpanic,  galpanic_state, empty_init, ROT90, "Kaneko", "Gals Panic (Unprotected)",          MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, galpanica, galpanic, galpanica, galpanica, galpanic_state, empty_init, ROT90, "Kaneko", "Gals Panic (MCU Protected, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, galpanicb, galpanic, galpanica, galpanica, galpanic_state, empty_init, ROT90, "Kaneko", "Gals Panic (MCU Protected, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
