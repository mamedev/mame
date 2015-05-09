// license:???
// copyright-holders:Kenneth Lin
/***************************************************************************

  jackal.c

  Written by Kenneth Lin (kenneth_lin@ai.vancouver.bc.ca)

Notes:
- This game uses two 005885 gfx chip in parallel. The unique thing about it is
  that the two 4bpp tilemaps from the two chips are merged to form a single
  8bpp tilemap.
- topgunbl is derived from a completely different version, which supports gun
  turret rotation. The copyright year is also different, but this doesn't
  necessarily mean anything.

TODO:
- Coin counters don't work correctly, because the register is overwritten by
  other routines and the coin counter bits rapidly toggle between 0 and 1.
- running the sound CPU at the nominal clock rate, music stops working at the
  beginning of the game. This is kludged by overclocking the sound CPU. This
  looks like a CPU communication timing issue however fiddling with the
  interleave factor has no effect.


Memory Map
----------

MAIN CPU:

Address range 00xxxxxxxxxx---- is handled by the 007343 custom so layout is
inferred by program behaviour. Note that address lines A8 and A9 are ORed
together and go to the single A8.9 input of the 007343.

Address          Dir Data     Description
---------------- --- -------- -----------------------
00000000000000xx R/W xxxxxxxx 005885 registers
0000000000000100 R/W xxxxxxxx 005885 registers
0000000000010000 R   xxxxxxxx DIPSW1
0000000000010001 R   xxxxxxxx P1 inputs + DIPSW3.4
0000000000010010 R   xxxxxxxx P2 inputs
0000000000010011 R   xxxxxxxx Coin inputs + DIPSW3.1-3
00000000000101-0 R   xxxxxxxx P1 extra inputs (only used by the bootleg for the rotary control)
00000000000101-1 R   xxxxxxxx P2 extra inputs (only used by the bootleg for the rotary control)
00000000000110-0 R   xxxxxxxx DIPSW2
00000000000111-0   W -------x Coin Counter 1 (to 005924 OUT1 input)
                   W ------x- Coin Counter 2 (to 005924 OUT2 input)
                   W -----x-- unknown ("END", to connector SVCN4P pin 4)
                   W ----x--- sprite RAM bank (to 007343 OBJB input)
                   W ---x---- 005885 select (to 007343 GATEB input)
                   W --x----- ROM bank
00000000000111-1 R/W -------- Watchdog reset (to 005924 AFR input)
00000000001xxxxx R/W xxxxxxxx scroll RAM (005885)
00000000010xxxxx R/W xxxxxxxx Z RAM (005885)
000xxxxxxxxxxxxx R/W xxxxxxxx RAM (shared with sound CPU--note that addresses 0000-005F are handled above so they are excluded)
0010xxxxxxxxxxxx R/W xxxxxxxx video RAM (005885)
0011xxxxxxxxxxxx R/W xxxxxxxx sprite RAM (005885)
01xxxxxxxxxxxxxx R   xxxxxxxx ROM (banked)
10xxxxxxxxxxxxxx R   xxxxxxxx ROM (banked)
11xxxxxxxxxxxxxx R   xxxxxxxx ROM


SOUND CPU:

Address          Dir Data     Description
---------------- --- -------- -----------------------
000-------------              n.c.
001------------x R/W xxxxxxxx YM2151
010-xxxxxxxxxxxx R/W xxxxxxxx 007327 (palette)
011xxxxxxxxxxxxx R/W xxxxxxxx RAM (shared with main CPU)
1xxxxxxxxxxxxxxx R   xxxxxxxx ROM

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "sound/2151intf.h"
#include "includes/jackal.h"
#include "includes/konamipt.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

READ8_MEMBER(jackal_state::jackalr_rotary_r)
{
	return (1 << ioport(offset ? "DIAL1" : "DIAL0")->read_safe(0x00)) ^ 0xff;
}

WRITE8_MEMBER(jackal_state::jackal_flipscreen_w)
{
	m_irq_enable = data & 0x02;
	flip_screen_set(data & 0x08);
}

READ8_MEMBER(jackal_state::jackal_zram_r)
{
	return m_rambank[0x0020 + offset];
}


READ8_MEMBER(jackal_state::jackal_voram_r)
{
	return m_rambank[0x2000 + offset];
}


READ8_MEMBER(jackal_state::jackal_spriteram_r)
{
	return m_spritebank[0x3000 + offset];
}


WRITE8_MEMBER(jackal_state::jackal_rambank_w)
{
	UINT8 *rgn = memregion("master")->base();

	if (data & 0x04)
		popmessage("jackal_rambank_w %02x", data);

	coin_counter_w(machine(), 0, data & 0x01);
	coin_counter_w(machine(), 1, data & 0x02);

	m_spritebank = &rgn[((data & 0x08) << 13)];
	m_rambank = &rgn[((data & 0x10) << 12)];
	membank("bank1")->set_entry((data & 0x20) ? 1 : 0);
}


WRITE8_MEMBER(jackal_state::jackal_zram_w)
{
	m_rambank[0x0020 + offset] = data;
}


WRITE8_MEMBER(jackal_state::jackal_voram_w)
{
	if ((offset & 0xf800) == 0)
		jackal_mark_tile_dirty(offset & 0x3ff);

	m_rambank[0x2000 + offset] = data;
}


WRITE8_MEMBER(jackal_state::jackal_spriteram_w)
{
	m_spritebank[0x3000 + offset] = data;
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( master_map, AS_PROGRAM, 8, jackal_state )
	AM_RANGE(0x0000, 0x0003) AM_RAM AM_SHARE("videoctrl")   // scroll + other things
	AM_RANGE(0x0004, 0x0004) AM_WRITE(jackal_flipscreen_w)
	AM_RANGE(0x0010, 0x0010) AM_READ_PORT("DSW1")
	AM_RANGE(0x0011, 0x0011) AM_READ_PORT("IN1")
	AM_RANGE(0x0012, 0x0012) AM_READ_PORT("IN2")
	AM_RANGE(0x0013, 0x0013) AM_READ_PORT("IN0")
	AM_RANGE(0x0014, 0x0015) AM_READ(jackalr_rotary_r)
	AM_RANGE(0x0018, 0x0018) AM_READ_PORT("DSW2")
	AM_RANGE(0x0019, 0x0019) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x001c, 0x001c) AM_WRITE(jackal_rambank_w)
	AM_RANGE(0x0020, 0x005f) AM_READWRITE(jackal_zram_r, jackal_zram_w)             // MAIN   Z RAM,SUB    Z RAM
	AM_RANGE(0x0060, 0x1fff) AM_RAM AM_SHARE("share1")                          // M COMMON RAM,S COMMON RAM
	AM_RANGE(0x2000, 0x2fff) AM_READWRITE(jackal_voram_r, jackal_voram_w)           // MAIN V O RAM,SUB  V O RAM
	AM_RANGE(0x3000, 0x3fff) AM_READWRITE(jackal_spriteram_r, jackal_spriteram_w)   // MAIN V O RAM,SUB  V O RAM
	AM_RANGE(0x4000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_map, AS_PROGRAM, 8, jackal_state )
	AM_RANGE(0x2000, 0x2001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x4000, 0x43ff) AM_RAM_DEVWRITE("palette", palette_device, write_indirect) AM_SHARE("palette")  // self test only checks 0x4000-0x423f, 007327 should actually go up to 4fff
	AM_RANGE(0x6000, 0x605f) AM_RAM                     // SOUND RAM (Self test check 0x6000-605f, 0x7c00-0x7fff)
	AM_RANGE(0x6060, 0x7fff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jackal )
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) ) PORT_DIPLOCATION( "SW2:1,2" )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION( "SW2:3" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION( "SW2:4,5" )
	PORT_DIPSETTING(    0x18, "30K 150K" )
	PORT_DIPSETTING(    0x10, "50K 200K" )
	PORT_DIPSETTING(    0x08, "30K" )
	PORT_DIPSETTING(    0x00, "50K" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) ) PORT_DIPLOCATION( "SW2:6,7" )
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION( "SW2:8" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	KONAMI8_SYSTEM_10
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION( "SW3:1" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Sound Adjustment" ) PORT_DIPLOCATION( "SW3:2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, "Sound Mode" ) PORT_DIPLOCATION( "SW3:3" )
	PORT_DIPSETTING(    0x80, DEF_STR( Mono ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )

	PORT_START("IN1")
	KONAMI8_B12(1)
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION( "SW3:4" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	KONAMI8_B12_UNK(2)
INPUT_PORTS_END

static INPUT_PORTS_START( jackalr )
	PORT_INCLUDE(jackal)

	PORT_MODIFY("IN0")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIAL0") // player 1 8-way rotary control - converted in jackalr_rotary_r()
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(8) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_Z) PORT_CODE_INC(KEYCODE_X) PORT_FULL_TURN_COUNT(8)

	PORT_START("DIAL1") // player 2 8-way rotary control - converted in jackalr_rotary_r()
	PORT_BIT( 0xff, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(8) PORT_WRAPS PORT_SENSITIVITY(15) PORT_KEYDELTA(1) PORT_CODE_DEC(KEYCODE_N) PORT_CODE_INC(KEYCODE_M) PORT_PLAYER(2) PORT_FULL_TURN_COUNT(8)
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,4),
	8,  /* 8 bits per pixel (!) */
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout spritelayout =
{
	16, 16,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
			32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			16*32, 17*32, 18*32, 19*32, 20*32, 21*32, 22*32, 23*32 },
	32*32
};

static const gfx_layout spritelayout8 =
{
	8, 8,
	RGN_FRAC(1,4),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static GFXDECODE_START( jackal )
	GFXDECODE_ENTRY( "gfx1", 0x00000, charlayout,        0,  1 )    // colors 256-511 without lookup
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout,  0x100, 16 )    // colors   0- 15 with lookup
	GFXDECODE_ENTRY( "gfx1", 0x20000, spritelayout8, 0x100, 16 )    // to handle 8x8 sprites
	GFXDECODE_ENTRY( "gfx1", 0x60000, spritelayout,  0x200, 16 )    // colors  16- 31 with lookup
	GFXDECODE_ENTRY( "gfx1", 0x60000, spritelayout8, 0x200, 16 )    // to handle 8x8 sprites
GFXDECODE_END

/*************************************
 *
 *  Interrupt generator
 *
 *************************************/

INTERRUPT_GEN_MEMBER(jackal_state::jackal_interrupt)
{
	if (m_irq_enable)
	{
		device.execute().set_input_line(0, HOLD_LINE);
		m_slavecpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jackal_state::machine_start()
{
	UINT8 *ROM = memregion("master")->base();

	membank("bank1")->configure_entry(0, &ROM[0x04000]);
	membank("bank1")->configure_entry(1, &ROM[0x14000]);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_irq_enable));
}

void jackal_state::machine_reset()
{
	UINT8 *rgn = memregion("master")->base();

	// HACK: running at the nominal clock rate, music stops working
	// at the beginning of the game. This fixes it.
	m_slavecpu->set_clock_scale(1.2f);

	m_rambank = rgn;
	m_spritebank = rgn;

	m_irq_enable = 0;
}

static MACHINE_CONFIG_START( jackal, jackal_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("master", M6809, MASTER_CLOCK/12) // verified on pcb
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", jackal_state,  jackal_interrupt)

	MCFG_CPU_ADD("slave", M6809, MASTER_CLOCK/12) // verified on pcb
	MCFG_CPU_PROGRAM_MAP(slave_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(1*8, 31*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(jackal_state, screen_update_jackal)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", jackal)
	MCFG_PALETTE_ADD("palette", 0x300)
	MCFG_PALETTE_INDIRECT_ENTRIES(0x200)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)
	MCFG_PALETTE_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_PALETTE_INIT_OWNER(jackal_state, jackal)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", SOUND_CLOCK) // verified on pcb
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jackal ) /* 8-Way Joystick: You can only shoot in one direction regardless of travel - up the screen */
	ROM_REGION( 0x20000, "master", 0 )  /* Banked 64k for 1st CPU */
	ROM_LOAD( "631_v02.15d", 0x04000, 0x8000, CRC(0b7e0584) SHA1(e4019463345a4c020d5a004c9a400aca4bdae07b) )
	ROM_CONTINUE(            0x14000, 0x8000 )
	ROM_LOAD( "631_v03.16d", 0x0c000, 0x4000, CRC(3e0dfb83) SHA1(5ba7073751eee33180e51143b348256597909516) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631_t01.11d", 0x8000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "631t04.7h",  0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t05.8h",  0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t06.12h", 0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t07.13h", 0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) ) /* Silkscreened MASK1M */

	ROM_REGION( 0x0200, "proms", 0 )    /* color lookup tables */
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
ROM_END

ROM_START( jackalr ) /* Rotary Joystick: Shot direction is controlled via the rotary function of the joystick */
	ROM_REGION( 0x20000, "master", 0 )  /* Banked 64k for 1st CPU */
	ROM_LOAD( "631_q02.15d", 0x04000, 0x8000, CRC(ed2a7d66) SHA1(3d9b31fa8b31e509880d617feb0dd4bd9790d2d5) )
	ROM_CONTINUE(            0x14000, 0x8000 )
	ROM_LOAD( "631_q03.16d", 0x0c000, 0x4000, CRC(b9d34836) SHA1(af23a0c844fb9e60a757511ca898d73eef4c2e51) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631_q01.11d", 0x8000, 0x8000, CRC(54aa2d29) SHA1(ebc6b3a5db5120cc33d62e3213d0e881f658282d) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "631t04.7h",  0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t05.8h",  0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t06.12h", 0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t07.13h", 0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) ) /* Silkscreened MASK1M */
		/* These roms are on a tiny riser board - two smaller roms instead of MASK1M roms */
//  ROM_LOAD16_BYTE( "631_q04.7h",  0x20000, 0x10000, CRC(0) SHA1(0) )
//  ROM_LOAD16_BYTE( "631_q05.7h",  0x00000, 0x10000, CRC(0) SHA1(0) )
//  ROM_LOAD16_BYTE( "631_q06.8h",  0x20001, 0x10000, CRC(0) SHA1(0) )
//  ROM_LOAD16_BYTE( "631_q07.8h",  0x00001, 0x10000, CRC(0) SHA1(0) ) /* 631 Q04 through 631 Q11 need to be redumped and verified. Should be the same data */
//  ROM_LOAD16_BYTE( "631_q08.12h", 0x40000, 0x10000, CRC(0) SHA1(0) ) /* until then we are going to use the standard MASK1M roms - Will fixed when dumped  */
//  ROM_LOAD16_BYTE( "631_q09.12h", 0x60000, 0x10000, CRC(0) SHA1(0) )
//  ROM_LOAD16_BYTE( "631_q10.13h", 0x40001, 0x10000, CRC(0) SHA1(0) )
//  ROM_LOAD16_BYTE( "631_q11.13h", 0x60001, 0x10000, CRC(0) SHA1(0) )

	ROM_REGION( 0x0200, "proms", 0 )    /* color lookup tables */
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
ROM_END

ROM_START( topgunr ) /* 8-Way Joystick:  You can only shoot in one direction regardless of travel - up the screen */
	ROM_REGION( 0x20000, "master", 0 )  /* Banked 64k for 1st CPU */
	ROM_LOAD( "631_u02.15d", 0x04000, 0x8000, CRC(f7e28426) SHA1(db2d5f252a574b8aa4d8406a8e93b423fd2a7fef) )
	ROM_CONTINUE(            0x14000, 0x8000 )
	ROM_LOAD( "631_u03.16d", 0x0c000, 0x4000, CRC(c086844e) SHA1(4d6f27ac3aabb4b2d673aa619e407e417ad89337) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631_t01.11d", 0x8000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "631u04.7h",  0x00000, 0x20000, CRC(50122a12) SHA1(c9e0132a3a40d9d28685c867c70231947d8a9cb7) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631u05.8h",  0x00001, 0x20000, CRC(6943b1a4) SHA1(40de2b434600ea4c8fb42e6b21be2c3705a55d67) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631u06.12h", 0x40000, 0x20000, CRC(37dbbdb0) SHA1(f94db780d69e7dd40231a75629af79469d957378) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631u07.13h", 0x40001, 0x20000, CRC(22effcc8) SHA1(4d174b0ce64def32050f87343c4b1424e0fef6f7) ) /* Silkscreened MASK1M */

	ROM_REGION( 0x0200, "proms", 0 )    /* color lookup tables */
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
ROM_END

ROM_START( jackalj ) /* 8-Way Joystick: You can only shoot in the direction you're traveling */
	ROM_REGION( 0x20000, "master", 0 )  /* Banked 64k for 1st CPU */
	ROM_LOAD( "631_t02.15d", 0x04000, 0x8000, CRC(14db6b1a) SHA1(b469ea50aa94a2bda3bd0442300aa1272e5f30c4) )
	ROM_CONTINUE(            0x14000, 0x8000 )
	ROM_LOAD( "631_t03.16d", 0x0c000, 0x4000, CRC(fd5f9624) SHA1(2520c1ff54410ef498ecbf52877f011900baed4c) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "631_t01.11d", 0x8000, 0x8000, CRC(b189af6a) SHA1(f7df996c394fdd6f2ce128a8df38d7838f7ec6d6) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "631t04.7h",  0x00000, 0x20000, CRC(457f42f0) SHA1(08413a13d128875dddcf4f6ad302363096bf1d41) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t05.8h",  0x00001, 0x20000, CRC(732b3fc1) SHA1(7e89650b9e5e2b7ae82f8c55ac9995740f6fdfe1) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t06.12h", 0x40000, 0x20000, CRC(2d10e56e) SHA1(447b464ea725fb9ef87da067a41bcf463b427cce) ) /* Silkscreened MASK1M */
	ROM_LOAD16_BYTE( "631t07.13h", 0x40001, 0x20000, CRC(4961c397) SHA1(b430df58fc3bb722d6fb23bed7d04afdb7e5d9c1) ) /* Silkscreened MASK1M */

	ROM_REGION( 0x0200, "proms", 0 )    /* color lookup tables */
	ROM_LOAD( "631r08.9h",  0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
	ROM_LOAD( "631r09.14h", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) ) /* MMI 63S141AN or compatible (silkscreened 6301) */
ROM_END

ROM_START( topgunbl ) /* Rotary Joystick: Shot direction is controlled via the Rotary function of the joystick */
	ROM_REGION( 0x20000, "master", 0 )  /* Banked 64k for 1st CPU */
	ROM_LOAD( "t-3.c5", 0x04000, 0x8000, CRC(7826ad38) SHA1(875e87867924905b9b83bc203eb7ffe81cf72233) )
	ROM_LOAD( "t-4.c4", 0x14000, 0x8000, CRC(976c8431) SHA1(c199f57c25380d741aec85b0e0bfb6acf383e6a6) ) /* == 2nd half of 631_q02.15d */
	ROM_LOAD( "t-2.c6", 0x0c000, 0x4000, CRC(d53172e5) SHA1(44b7f180c17f9a121a2f06f2d3471920a8989e21) )

	ROM_REGION( 0x10000, "slave", 0 )     /* 64k for 2nd cpu (Graphics & Sound)*/
	ROM_LOAD( "t-1.c14", 0x8000, 0x8000, CRC(54aa2d29) SHA1(ebc6b3a5db5120cc33d62e3213d0e881f658282d) ) /* == 631_q01.11d */

	ROM_REGION( 0x80000, "gfx1", 0 )
	/* same data, different layout */
	ROM_LOAD16_WORD_SWAP( "t-17.n12", 0x00000, 0x08000, CRC(e8875110) SHA1(73f4c47ab039dce8c285bf222253084c860c95bf) )
	ROM_LOAD16_WORD_SWAP( "t-18.n13", 0x08000, 0x08000, CRC(cf14471d) SHA1(896aa8d7c93f837f6661d30bd0d6e19d16669107) )
	ROM_LOAD16_WORD_SWAP( "t-19.n14", 0x10000, 0x08000, CRC(46ee5dd2) SHA1(1a910984a197af341f13b4683babee857aafb245) )
	ROM_LOAD16_WORD_SWAP( "t-20.n15", 0x18000, 0x08000, CRC(3f472344) SHA1(49b9da8741b8e474d25726a706cf3008096ab2dc) )
	ROM_LOAD16_WORD_SWAP( "t-6.n1",   0x20000, 0x08000, CRC(539cc48c) SHA1(476ff5fe239e5acb61ede4d745d327f6bc3709f3) )
	ROM_LOAD16_WORD_SWAP( "t-5.m1",   0x28000, 0x08000, CRC(dbc26afe) SHA1(faab1feae91a9c22c008555955596c55d77b70c7) )
	ROM_LOAD16_WORD_SWAP( "t-7.n2",   0x30000, 0x08000, CRC(0ecd31b1) SHA1(06d77159ed55c1e288f2a194cdb09d29542e06d6) )
	ROM_LOAD16_WORD_SWAP( "t-8.n3",   0x38000, 0x08000, CRC(f946ada7) SHA1(fd9a0786436cbdb4c844f71342232e4e6645d98f) )
	ROM_LOAD16_WORD_SWAP( "t-13.n8",  0x40000, 0x08000, CRC(5d669abb) SHA1(faba6d7b47caae2ecdf15fb3527824bdb22e3d6b) )
	ROM_LOAD16_WORD_SWAP( "t-14.n9",  0x48000, 0x08000, CRC(f349369b) SHA1(f19238ef5feb1c89ef58c17a2506cc96ed8054e1) )
	ROM_LOAD16_WORD_SWAP( "t-15.n10", 0x50000, 0x08000, CRC(7c5a91dd) SHA1(85a1d76efc385e8e971a65e225de7f5d100bfbc7) )
	ROM_LOAD16_WORD_SWAP( "t-16.n11", 0x58000, 0x08000, CRC(5ec46d8e) SHA1(350e983b56a9f7d95e98429ee9a5fa6d3af36db4) )
	ROM_LOAD16_WORD_SWAP( "t-9.n4",   0x60000, 0x08000, CRC(8269caca) SHA1(8b80b7bad966d5b61a5c22d2ced625b5645f2ce2) )
	ROM_LOAD16_WORD_SWAP( "t-10.n5",  0x68000, 0x08000, CRC(25393e4f) SHA1(f6d7995b51d5bbbc3e325d6949dbc435446b5cf9) )
	ROM_LOAD16_WORD_SWAP( "t-11.n6",  0x70000, 0x08000, CRC(7895c22d) SHA1(c81ae51116fb32ac99d37eb7c2000c990d089b8d) )
	ROM_LOAD16_WORD_SWAP( "t-12.n7",  0x78000, 0x08000, CRC(15606dfc) SHA1(829492da49dbe70f81d15237803c5203aa011957) )

	ROM_REGION( 0x0200, "proms", 0 )    /* color lookup tables */
	ROM_LOAD( "631r08.bpr", 0x0000, 0x0100, CRC(7553a172) SHA1(eadf1b4157f62c3af4602da764268df954aa0018) )
	ROM_LOAD( "631r09.bpr", 0x0100, 0x0100, CRC(a74dd86c) SHA1(571f606f8fc0fd3d98d26761de79ccb4cc9ab044) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, jackal,   0,      jackal, jackal,  driver_device, 0, ROT90, "Konami",  "Jackal (World, 8-way Joystick)", 0 )
GAME( 1986, jackalr,  jackal, jackal, jackalr, driver_device, 0, ROT90, "Konami",  "Jackal (World, Rotary Joystick)", 0 )
GAME( 1986, topgunr,  jackal, jackal, jackal,  driver_device, 0, ROT90, "Konami",  "Top Gunner (US, 8-way Joystick)", 0 )
GAME( 1986, jackalj,  jackal, jackal, jackal,  driver_device, 0, ROT90, "Konami",  "Tokushu Butai Jackal (Japan, 8-way Joystick)", 0 )
GAME( 1986, topgunbl, jackal, jackal, jackalr, driver_device, 0, ROT90, "bootleg", "Top Gunner (bootleg, Rotary Joystick)", 0 )
