// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

Sauro
-----

driver by Zsolt Vasvari

Main CPU
--------

Memory mapped:

0000-dfff   ROM
e000-e7ff   RAM, battery backed
e800-ebff   Sprite RAM
f000-fbff   Background Video RAM
f400-ffff   Background Color RAM
f800-fbff   Foreground Video RAM
fc00-ffff   Foreground Color RAM

Ports:

00      R   DSW #1
20      R   DSW #2
40      R   Input Ports Player 1
60      R   Input Ports Player 2
80       W  Sound Commnand
c0       W  Flip Screen
c1       W  ???
c2-c4    W  ???
c6-c7    W  ??? (Loads the sound latch?)
c8       W  ???
c9       W  ???
ca-cd    W  ???
ce       W  ???
e0       W  Watchdog


Sound CPU
---------

Memory mapped:

0000-7fff       ROM
8000-87ff       RAM
a000         W  ADPCM trigger
c000-c001    W  YM3812
e000        R   Sound latch
e000-e006    W  ???
e00e-e00f    W  ???


TODO
----

- The readme claims there is a GI-SP0256A-AL ADPCM on the PCB. Needs to be
  emulated. Done (couriersud)

- Verify all clock speeds

- I'm only using colors 0-15. The other 3 banks are mostly the same, but,
  for example, the color that's used to paint the gradients of the sky (color 2)
  is different, so there might be a palette select. I don't see anything
  obviously wrong the way it is right now. It matches the screen shots found
  on the Spanish Dump site.

- What do the rest of the ports in the range c0-ce do?

Tricky Doc
----------

Addition by Reip


Stephh's notes (based on the games Z80 code and some tests) :

1) 'sauro'

  - Press START1 while in "test mode" to cycle through different screens
    (colors, Dip Switches, Inputs)
  - When "Freeze" Dip Switch is ON, press START1 to freeze and START2 to unfreeze.
    This setting (as well as others) must be defined before reseting the games.
  - "Test mode" crashes when trying to display "Difficult" ("Hard") because the full string
    is 15 bytes long while other string are 14, so the 15th "char" is NOT 0x00 :
      * 0xd49f : mask (0x30)
      * 0xd4a0-0xd4a7 : offset of settings to display (4 x 2 bytes, LSB first) :
        0xd58e, 0xd5a5, 0xd5bc, 0xd5d4
  - Player 2 uses player 2 inputs only when "Cabinet" Dip Switch is set to "Cocktail"
    (code at 0x2e40 : start reading inputs).

2) 'trckydoc' and clones

  - Press START1 while in "test mode" to cycle through different screens
    (colors, Dip Switches, Inputs)
  - When "Freeze" Dip Switch is ON, press START1 to freeze and START2 to unfreeze.
    This setting (as well as others) must be defined before reseting the games.

2a) 'trckydoc'

  - Settings are the SAME as in 'sauro', but there is NO bug in the "test mode" :
      * 0xcd19 : mask (0x30)
      * 0xcd1a-0xcd22 : offset of settings to display (4 x 2 bytes, LSB first) :
        0xce0a, 0xce21, 0xce38, 0xce4f
  - Player 2 uses player 2 inputs only when "Cabinet" Dip Switch is set to "Cocktail"
    (extra code at 0xdf10 - code at 0x3f80 : start reading inputs).
  - You can't get any extra life nor extra credit.

2b) 'trckydoca'

  - Coinage B is slightly different : you have 1C_1C instead of 1C_5C (table at 0x02e1).
    Such change isn't notified in the "test mode" though.
  - Player 2 uses player 2 inputs regardless of "Cabinet" Dip Switch
    (NO extra code at 0xdf10 - code at 0x3f80 : start reading inputs).
  - You can get an extra life at 90000 points and an extra credit at 500000 points
    but there is no music/sound to tell that to you (extra code at 0xdf30).
    This is only possible if you continue a game and have already got the life or
    the credit (extra code at 0xdf90 resets the flags if you don't continue).
    This info is written in "attract mode" when you don't have any credits
    instead of displaying the "INSERT COIN" message.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/3812intf.h"
#include "includes/sauro.h"
#include "machine/nvram.h"


WRITE8_MEMBER(sauro_state::sauro_sound_command_w)
{
	data |= 0x80;
	soundlatch_byte_w(space, offset, data);
}

READ8_MEMBER(sauro_state::sauro_sound_command_r)
{
	int ret = soundlatch_byte_r(space, offset);
	soundlatch_clear_byte_w(space, offset, 0);
	return ret;
}

WRITE8_MEMBER(sauro_state::coin1_w)
{
	coin_counter_w(machine(), 0, data);
	coin_counter_w(machine(), 0, 0); // to get the coin counter working in sauro, as it doesn't write 0
}

WRITE8_MEMBER(sauro_state::coin2_w)
{
	coin_counter_w(machine(), 1, data);
	coin_counter_w(machine(), 1, 0); // to get the coin counter working in sauro, as it doesn't write 0
}

WRITE8_MEMBER(sauro_state::flip_screen_w)
{
	flip_screen_set(data);
}

WRITE8_MEMBER(sauro_state::adpcm_w)
{
	m_sp0256->ald_w(space, 0, data);
}

static ADDRESS_MAP_START( sauro_map, AS_PROGRAM, 8, sauro_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe800, 0xebff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0xf000, 0xf3ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xf400, 0xf7ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xf800, 0xfbff) AM_RAM_WRITE(sauro_videoram2_w) AM_SHARE("videoram2")
	AM_RANGE(0xfc00, 0xffff) AM_RAM_WRITE(sauro_colorram2_w) AM_SHARE("colorram2")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sauro_io_map, AS_IO, 8, sauro_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("DSW1")
	AM_RANGE(0x20, 0x20) AM_READ_PORT("DSW2")
	AM_RANGE(0x40, 0x40) AM_READ_PORT("P1")
	AM_RANGE(0x60, 0x60) AM_READ_PORT("P2")
	AM_RANGE(0x80, 0x80) AM_WRITE(sauro_sound_command_w)
	AM_RANGE(0xa0, 0xa0) AM_WRITE(scroll_bg_w)
	AM_RANGE(0xa1, 0xa1) AM_WRITE(sauro_scroll_fg_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(flip_screen_w)
	AM_RANGE(0xc2, 0xc2) AM_WRITENOP        /* coin reset */
	AM_RANGE(0xc3, 0xc3) AM_WRITE(coin1_w)
	AM_RANGE(0xc4, 0xc4) AM_WRITENOP        /* coin reset */
	AM_RANGE(0xc5, 0xc5) AM_WRITE(coin2_w)
	AM_RANGE(0xc6, 0xc7) AM_WRITENOP        /* same as 0x80 - verified with debugger */
	AM_RANGE(0xc8, 0xc8) AM_WRITENOP        /* written every int: 0 written at end   of isr */
	AM_RANGE(0xc9, 0xc9) AM_WRITENOP        /* written every int: 1 written at start of isr */
	AM_RANGE(0xca, 0xcb) AM_WRITE(sauro_palette_bank_w) /* 1 written upon death, cleared 2 vblanks later */
														/* Sequence 3,2,1 written during intro screen */
	AM_RANGE(0xcc, 0xcc) AM_WRITENOP        /* same as 0xca */
	AM_RANGE(0xcd, 0xcd) AM_WRITENOP        /* same as 0xcb */
	AM_RANGE(0xce, 0xce) AM_WRITENOP        /* only written at startup */
	AM_RANGE(0xe0, 0xe0) AM_WRITE(watchdog_reset_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sauro_sound_map, AS_PROGRAM, 8, sauro_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x87ff) AM_RAM
	AM_RANGE(0xc000, 0xc001) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0xa000, 0xa000) AM_WRITE(adpcm_w)
	AM_RANGE(0xe000, 0xe000) AM_READ(sauro_sound_command_r)
	AM_RANGE(0xe000, 0xe006) AM_WRITENOP    /* echo from write to e0000 */
	AM_RANGE(0xe00e, 0xe00f) AM_WRITENOP
ADDRESS_MAP_END


static ADDRESS_MAP_START( trckydoc_map, AS_PROGRAM, 8, sauro_state )
	AM_RANGE(0x0000, 0xdfff) AM_ROM
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xe800, 0xebff) AM_RAM AM_MIRROR(0x400) AM_SHARE("spriteram")
	AM_RANGE(0xf000, 0xf3ff) AM_RAM_WRITE(videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xf400, 0xf7ff) AM_RAM_WRITE(colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("DSW1")
	AM_RANGE(0xf808, 0xf808) AM_READ_PORT("DSW2")
	AM_RANGE(0xf810, 0xf810) AM_READ_PORT("P1")
	AM_RANGE(0xf818, 0xf818) AM_READ_PORT("P2")
	AM_RANGE(0xf820, 0xf821) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0xf828, 0xf828) AM_READ(watchdog_reset_r)
	AM_RANGE(0xf830, 0xf830) AM_WRITE(scroll_bg_w)
	AM_RANGE(0xf838, 0xf838) AM_WRITENOP                /* only written at startup */
	AM_RANGE(0xf839, 0xf839) AM_WRITE(flip_screen_w)
	AM_RANGE(0xf83a, 0xf83a) AM_WRITE(coin1_w)
	AM_RANGE(0xf83b, 0xf83b) AM_WRITE(coin2_w)
	AM_RANGE(0xf83c, 0xf83c) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0xf83f, 0xf83f) AM_WRITENOP                /* only written at startup */
ADDRESS_MAP_END


/* verified from Z80 code */
static INPUT_PORTS_START( tecfri )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY

	PORT_START("P2")                                                  /* see notes */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )  PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )    PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )  PORT_8WAY PORT_COCKTAIL

	PORT_START("DSW1")
	PORT_SERVICE( 0x01, IP_ACTIVE_HIGH )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Very_Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )                       /* This crashes test mode in 'sauro' but not in other games !!! - see notes */
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPUNUSED( 0x40, IP_ACTIVE_HIGH )
	PORT_DIPUNUSED( 0x80, IP_ACTIVE_HIGH )
INPUT_PORTS_END


/* verified from Z80 code */
static INPUT_PORTS_START( trckydoca )
	PORT_INCLUDE(tecfri)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	2048,   /* 2048 characters */
	4,      /* 4 bits per pixel */
	{ 0,1,2,3 },  /* The 4 planes are packed together */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4},
	{ 0*4*8, 1*4*8, 2*4*8, 3*4*8, 4*4*8, 5*4*8, 6*4*8, 7*4*8},
	8*8*4     /* every char takes 32 consecutive bytes */
};

static const gfx_layout trckydoc_spritelayout =
{
	16,16,  /* 16*16 sprites */
	512,    /* 512 sprites */
	4,      /* 4 bits per pixel */
	{ 0,1,2,3 },  /* The 4 planes are packed together */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4, 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4},
	{ RGN_FRAC(3,4)+0*4*16, RGN_FRAC(2,4)+0*4*16, RGN_FRAC(1,4)+0*4*16, RGN_FRAC(0,4)+0*4*16,
		RGN_FRAC(3,4)+1*4*16, RGN_FRAC(2,4)+1*4*16, RGN_FRAC(1,4)+1*4*16, RGN_FRAC(0,4)+1*4*16,
		RGN_FRAC(3,4)+2*4*16, RGN_FRAC(2,4)+2*4*16, RGN_FRAC(1,4)+2*4*16, RGN_FRAC(0,4)+2*4*16,
		RGN_FRAC(3,4)+3*4*16, RGN_FRAC(2,4)+3*4*16, RGN_FRAC(1,4)+3*4*16, RGN_FRAC(0,4)+3*4*16 },
	16*16     /* every sprite takes 32 consecutive bytes */
};

static const gfx_layout sauro_spritelayout =
{
	16,16,  /* 16*16 sprites */
	1024,   /* 1024 sprites */
	4,      /* 4 bits per pixel */
	{ 0,1,2,3 },  /* The 4 planes are packed together */
	{ 1*4, 0*4, 3*4, 2*4, 5*4, 4*4, 7*4, 6*4, 9*4, 8*4, 11*4, 10*4, 13*4, 12*4, 15*4, 14*4},
	{ RGN_FRAC(3,4)+0*4*16, RGN_FRAC(2,4)+0*4*16, RGN_FRAC(1,4)+0*4*16, RGN_FRAC(0,4)+0*4*16,
		RGN_FRAC(3,4)+1*4*16, RGN_FRAC(2,4)+1*4*16, RGN_FRAC(1,4)+1*4*16, RGN_FRAC(0,4)+1*4*16,
		RGN_FRAC(3,4)+2*4*16, RGN_FRAC(2,4)+2*4*16, RGN_FRAC(1,4)+2*4*16, RGN_FRAC(0,4)+2*4*16,
		RGN_FRAC(3,4)+3*4*16, RGN_FRAC(2,4)+3*4*16, RGN_FRAC(1,4)+3*4*16, RGN_FRAC(0,4)+3*4*16 },
	16*16     /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( sauro )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx3", 0, sauro_spritelayout, 0, 64 )
GFXDECODE_END

static GFXDECODE_START( trckydoc )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, trckydoc_spritelayout, 0, 64 )
GFXDECODE_END


static MACHINE_CONFIG_START( tecfri, sauro_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_20MHz/4)       /* verified on pcb */
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sauro_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_1FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(55.72)   /* verified on pcb */
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(5000))  // frames per second, vblank duration (otherwise sprites lag)
	MCFG_SCREEN_SIZE(32 * 8, 32 * 8)
	MCFG_SCREEN_VISIBLE_AREA(1 * 8, 31 * 8 - 1, 2 * 8, 30 * 8 - 1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRGGGGBBBB_PROMS("palette", 1024)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_20MHz/8)       /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( trckydoc, tecfri )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(trckydoc_map)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", trckydoc)

	MCFG_VIDEO_START_OVERRIDE(sauro_state,trckydoc)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(sauro_state, screen_update_trckydoc)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sauro, tecfri )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sauro_map)
	MCFG_CPU_IO_MAP(sauro_io_map)

	MCFG_CPU_ADD("audiocpu", Z80, 4000000)  // 4 MHz?
	MCFG_CPU_PROGRAM_MAP(sauro_sound_map)
	MCFG_CPU_PERIODIC_INT_DRIVER(sauro_state, irq0_line_hold,  8*60) // ?

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sauro)

	MCFG_VIDEO_START_OVERRIDE(sauro_state,sauro)
	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(sauro_state, screen_update_sauro)

	MCFG_SOUND_ADD("speech", SP0256, 3120000)
	MCFG_SP0256_DATA_REQUEST_CB(INPUTLINE("audiocpu", INPUT_LINE_NMI))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( sauro )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sauro-2.bin",     0x00000, 0x8000, CRC(19f8de25) SHA1(52eea7c0416ab0a8dbb3d1664b2f57ab7a405a67) )
	ROM_LOAD( "sauro-1.bin",     0x08000, 0x8000, CRC(0f8b876f) SHA1(6e61a8934a2cc3c80c1f47dd59aa43aaeec12f75) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro-3.bin",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "sauro-5.bin",     0x08000, 0x8000, CRC(a6e2640d) SHA1(346ffcf62e27ce8134f4e5e0dbcf11f110e19e04) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x0000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  /* Red component */
	ROM_LOAD( "82s137-2.bin",    0x0400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  /* Green component */
	ROM_LOAD( "82s137-1.bin",    0x0800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  /* Blue component */

	ROM_REGION( 0x10000, "speech", 0 )
	/* SP0256 mask rom */
	ROM_LOAD( "sp0256-al2.bin",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( saurop )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "s2.3k",     0x00000, 0x8000, CRC(79846222) SHA1(59ccfbaad0251f771c0fd624d00d93a50bca67d8) )
	ROM_LOAD( "s1.3f",     0x08000, 0x8000, CRC(3efd13ed) SHA1(3920d21d5d9c285c5bcf47aa12b4e9a42294f149) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "s3.5x",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "s6.7x",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "s7.7z",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "s4.7h",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "s5.7k",     0x08000, 0x8000, CRC(de5cd249) SHA1(e3752b88b539e1057a35619ffbad01720ab60d7d) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "s8.10l",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "s9.10p",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "s10.10r",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "s11.10t",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x0000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  /* Red component */
	ROM_LOAD( "82s137-2.bin",    0x0400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  /* Green component */
	ROM_LOAD( "82s137-1.bin",    0x0800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  /* Blue component */

	ROM_REGION( 0x10000, "speech", 0 )
	/* SP0256 mask rom */
	ROM_LOAD( "sp0256-al2.bin",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END

ROM_START( saurorr ) // all roms have original Tecfri stickers
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "27256-2.bin",     0x00000, 0x8000, CRC(b0d80eab) SHA1(60cbe16d6c87d4681155814a5034b7e9d10bbd81) )
	ROM_LOAD( "27256-1.bin",     0x08000, 0x8000, CRC(cbb5f06e) SHA1(f93c01006d308e0b6950d720b6fe4409728c79e2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "sauro-3.bin",     0x00000, 0x8000, CRC(0d501e1b) SHA1(20a56ff30d4fa5d2f483a449703b49153839f6bc) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "sauro-6.bin",     0x00000, 0x8000, CRC(4b77cb0f) SHA1(7b9cb2dca561d81390106c1a5c0533dcecaf6f1a) )
	ROM_LOAD( "sauro-7.bin",     0x08000, 0x8000, CRC(187da060) SHA1(1df156e58379bb39acade02aabab6ff1cb7cc288) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "sauro-4.bin",     0x00000, 0x8000, CRC(9b617cda) SHA1(ce26b84ad5ecd6185ae218520e9972645bbf09ad) )
	ROM_LOAD( "27256-5.bin",     0x08000, 0x8000, CRC(9aabdbe5) SHA1(ef008e368024f9377a8d2bc5863b01c63bc8f55b) ) // contains the changed license logo

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "sauro-8.bin",     0x00000, 0x8000, CRC(e08b5d5e) SHA1(eaaeaa08b19c034ab2a2140f887edffca5f441b9) )
	ROM_LOAD( "sauro-9.bin",     0x08000, 0x8000, CRC(7c707195) SHA1(0529f6808b0cec3e12ca51bee189841d21577786) )
	ROM_LOAD( "sauro-10.bin",    0x10000, 0x8000, CRC(c93380d1) SHA1(fc9655cc94c2d2058f83eb341be7e7856a08194f) )
	ROM_LOAD( "sauro-11.bin",    0x18000, 0x8000, CRC(f47982a8) SHA1(cbaeac272c015d9439f151cfb3449082f11a57a1) )

	ROM_REGION( 0x0c00, "proms", 0 )
	ROM_LOAD( "82s137-3.bin",    0x0000, 0x0400, CRC(d52c4cd0) SHA1(27d6126b46616c06b55d8018c97f6c3d7805ae9e) )  /* Red component */
	ROM_LOAD( "82s137-2.bin",    0x0400, 0x0400, CRC(c3e96d5d) SHA1(3f6f21526a4357e4a9a9d56a6f4ef5911af2d120) )  /* Green component */
	ROM_LOAD( "82s137-1.bin",    0x0800, 0x0400, CRC(bdfcf00c) SHA1(9faf4d7f8959b64faa535c9945eec59c774a3760) )  /* Blue component */

	ROM_REGION( 0x10000, "speech", 0 )
	/* SP0256 mask rom */
	ROM_LOAD( "sp0256-al2.bin",   0x1000, 0x0800, CRC(b504ac15) SHA1(e60fcb5fa16ff3f3b69d36c7a6e955744d3feafc) )
ROM_END


ROM_START( trckydoc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trckydoc.d9",  0x0000,  0x8000, CRC(c6242fc3) SHA1(c8a6f6abe8b51061a113ed75fead0479df68ec40) )
	ROM_LOAD( "trckydoc.b9",  0x8000,  0x8000, CRC(8645c840) SHA1(79c2acfc1aeafbe94afd9d230200bd7cdd7bcd1b) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "trckydoc.e6",     0x00000, 0x8000, CRC(ec326392) SHA1(e6954fecc501a821caa21e67597914519fbbe58f) )
	ROM_LOAD( "trckydoc.g6",     0x08000, 0x8000, CRC(6a65c088) SHA1(4a70c104809d86b4eef6cc0df9452966fe7c9859) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "trckydoc.h1",    0x00000, 0x4000, CRC(8b73cbf3) SHA1(d10f79a38c1596c90bac9cf4c64ba38ae6ecd8cb) )
	ROM_LOAD( "trckydoc.e1",    0x04000, 0x4000, CRC(841be98e) SHA1(82da07490b73edcbffc3b9247205aab3a1f7d7ad) )
	ROM_LOAD( "trckydoc.c1",    0x08000, 0x4000, CRC(1d25574b) SHA1(924e4376a7fe6cdfff0fa6045aaa3f7c0633d275) )
	ROM_LOAD( "trckydoc.a1",    0x0c000, 0x4000, CRC(436c59ba) SHA1(2aa9c155c432a3c81420520c53bb944dcc613a94) )

	ROM_REGION( 0x0c00, "proms", 0 ) // colour proms
	ROM_LOAD( "tdclr3.prm",    0x0000, 0x0100, CRC(671d0140) SHA1(7d5fcd9589c46590b0a240cac428f993201bec2a) )
	ROM_LOAD( "tdclr2.prm",    0x0400, 0x0100, CRC(874f9050) SHA1(db40d68f5166657fce0eadcd82143112b0388894) )
	ROM_LOAD( "tdclr1.prm",    0x0800, 0x0100, CRC(57f127b0) SHA1(3d2b18a7a31933579f06d92fa0cc3f0e1fe8b98a) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "tdprm.prm",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )
ROM_END

ROM_START( trckydoca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "trckydca.d9",  0x0000,  0x8000, CRC(99c38aa4) SHA1(298a19439cc17743e10d101c50a26b9a7348299e) )
	ROM_LOAD( "trckydca.b9",  0x8000,  0x8000, CRC(b6048a15) SHA1(d982fafbfa391ef9bab50bfd52607494e2a9eedf) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "trckydoc.e6",     0x00000, 0x8000, CRC(ec326392) SHA1(e6954fecc501a821caa21e67597914519fbbe58f) )
	ROM_LOAD( "trckydoc.g6",     0x08000, 0x8000, CRC(6a65c088) SHA1(4a70c104809d86b4eef6cc0df9452966fe7c9859) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "trckydoc.h1",    0x00000, 0x4000, CRC(8b73cbf3) SHA1(d10f79a38c1596c90bac9cf4c64ba38ae6ecd8cb) )
	ROM_LOAD( "trckydoc.e1",    0x04000, 0x4000, CRC(841be98e) SHA1(82da07490b73edcbffc3b9247205aab3a1f7d7ad) )
	ROM_LOAD( "trckydoc.c1",    0x08000, 0x4000, CRC(1d25574b) SHA1(924e4376a7fe6cdfff0fa6045aaa3f7c0633d275) )
	ROM_LOAD( "trckydoc.a1",    0x0c000, 0x4000, CRC(436c59ba) SHA1(2aa9c155c432a3c81420520c53bb944dcc613a94) )

	ROM_REGION( 0x0c00, "proms", 0 ) // colour proms
	ROM_LOAD( "tdclr3.prm",    0x0000, 0x0100, CRC(671d0140) SHA1(7d5fcd9589c46590b0a240cac428f993201bec2a) )
	ROM_LOAD( "tdclr2.prm",    0x0400, 0x0100, CRC(874f9050) SHA1(db40d68f5166657fce0eadcd82143112b0388894) )
	ROM_LOAD( "tdclr1.prm",    0x0800, 0x0100, CRC(57f127b0) SHA1(3d2b18a7a31933579f06d92fa0cc3f0e1fe8b98a) )

	ROM_REGION( 0x0200, "user1", 0 ) // unknown
	ROM_LOAD( "tdprm.prm",    0x0000, 0x0200,  CRC(5261bc11) SHA1(1cc7a9a7376e65f4587b75ef9382049458656372) )
ROM_END

DRIVER_INIT_MEMBER(sauro_state,tecfri)
{
	/* This game doesn't like all memory to be initialized to zero, it won't
	   initialize the high scores */

	UINT8 *RAM = memregion("maincpu")->base();

	memset(&RAM[0xe000], 0, 0x100);
	RAM[0xe000] = 1;
}

GAME( 1987, sauro,    0,        sauro,    tecfri, sauro_state,    tecfri, ROT0, "Tecfri",                                "Sauro", MACHINE_SUPPORTS_SAVE )
GAME( 1987, saurop,   sauro,    sauro,    tecfri, sauro_state,    tecfri, ROT0, "Tecfri (Philko license)",               "Sauro (Philko license)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, saurorr,  sauro,    sauro,    tecfri, sauro_state,    tecfri, ROT0, "Tecfri (Recreativos Real S.A. license)","Sauro (Recreativos Real S.A. license)", MACHINE_SUPPORTS_SAVE )

GAME( 1987, trckydoc, 0,        trckydoc, tecfri, sauro_state,    tecfri, ROT0, "Tecfri", "Tricky Doc (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, trckydoca,trckydoc, trckydoc, trckydoca, sauro_state, tecfri, ROT0, "Tecfri", "Tricky Doc (set 2)", MACHINE_SUPPORTS_SAVE )
