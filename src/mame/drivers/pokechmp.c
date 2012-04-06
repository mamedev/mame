/* Poke Champ */

/* This is a Korean hack of Data East's Pocket Gal

   It uses RAM for Palette instead of PROMs
   Samples are played by OKIM6295
   Different Banking
   More Tiles, 8bpp
   Sprites 4bpp? instead of 2bpp
   Many code changes

   Todo:

   Fix colours
   Fix sound banking
   Verify frequencies etc.

*/

/* README

-The ROMs are labeled as "Unico".
-The CPUs and some other chips are labeled as "SEA HUNTER".
-The chips with the "SEA HUNTER" label all have their
 surfaces scratched out, so I don't know what they are
 (all 40 -pin chips).

ROMs 1 to 4 = GFX?
ROMs 5 to 8 = Program?
ROM 9 = Sound CPU code?
ROM 10 = Sound samples?
ROM 11 = Main CPU code?

-There's a "copyright 1987 data east corp.all rights reserved"
 string inside ROM 11

-Sound = Yamaha YM2203C + Y3014B

-Also, there are some GALs on the board (not dumped) a
 8-dips bank and two oscilators (4 MHz and 24 MHz, both near
 the sound parts).

ClawGrip, Jul 2006

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/2203intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/pokechmp.h"

WRITE8_MEMBER(pokechmp_state::pokechmp_bank_w)
{
	UINT8 *RAM = machine().region("maincpu")->base();

	if (data == 0x00)
	{
		memory_set_bankptr(machine(), "bank1",&RAM[0x10000]);
		memory_set_bankptr(machine(), "bank2",&RAM[0x12000]);
	}
	if (data == 0x01)
	{
		memory_set_bankptr(machine(), "bank1",&RAM[0x14000]);
		memory_set_bankptr(machine(), "bank2",&RAM[0x16000]);
	}
	if (data == 0x02)
	{
		memory_set_bankptr(machine(), "bank1",&RAM[0x20000]);
		memory_set_bankptr(machine(), "bank2",&RAM[0x22000]);
	}

	if (data == 0x03)
	{
		memory_set_bankptr(machine(), "bank1",&RAM[0x04000]);
		memory_set_bankptr(machine(), "bank2",&RAM[0x06000]);
	}
}

#ifdef UNUSED_FUNCTION
WRITE8_MEMBER(pokechmp_state::pokechmp_sound_bank_w)
{
	memory_set_bank(machine(), "bank3", (data >> 2) & 1);
}
#endif

WRITE8_MEMBER(pokechmp_state::pokechmp_sound_w)
{
	soundlatch_w(space, 0, data);
	cputag_set_input_line(machine(), "audiocpu", INPUT_LINE_NMI, PULSE_LINE);
}


INLINE void pokechmp_set_color(running_machine &machine, pen_t color, int rshift, int gshift, int bshift, UINT16 data)
{
	palette_set_color_rgb(machine, color, pal5bit(data >> rshift), pal5bit(data >> gshift), pal5bit(data >> bshift));
}


WRITE8_MEMBER(pokechmp_state::pokechmp_paletteram_w)
{
	m_generic_paletteram_8[offset] = data;
	pokechmp_set_color(machine(), offset &0x3ff, 0, 5, 10, (m_generic_paletteram_8[offset&0x3ff]<<8) | ( m_generic_paletteram_8[ (offset&0x3ff)+0x400 ] )  );
}


static ADDRESS_MAP_START( pokechmp_map, AS_PROGRAM, 8, pokechmp_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0fff) AM_RAM_WRITE(pokechmp_videoram_w) AM_BASE(m_videoram)
	AM_RANGE(0x1000, 0x11ff) AM_RAM AM_BASE_SIZE(m_spriteram, m_spriteram_size)

	AM_RANGE(0x1800, 0x1800) AM_READ_PORT("P1")
	AM_RANGE(0x1801, 0x1801) AM_WRITE(pokechmp_flipscreen_w)
	/* 1800 - 0x181f are unused BAC-06 registers, see video/dec0.c */
	AM_RANGE(0x1802, 0x181f) AM_WRITENOP

	AM_RANGE(0x1a00, 0x1a00) AM_READ_PORT("P2") AM_WRITE(pokechmp_sound_w)
	AM_RANGE(0x1c00, 0x1c00) AM_READ_PORT("DSW") AM_WRITE(pokechmp_bank_w)

	/* Extra on Poke Champ (not on Pocket Gal) */
	AM_RANGE(0x2000, 0x27ff) AM_RAM_WRITE(pokechmp_paletteram_w) AM_SHARE("paletteram")

	AM_RANGE(0x4000, 0x5fff) AM_ROMBANK("bank1")
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK("bank2")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/***************************************************************************/

static ADDRESS_MAP_START( pokechmp_sound_map, AS_PROGRAM, 8, pokechmp_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x0800, 0x0801) AM_DEVWRITE_LEGACY("ym1", ym2203_w)
	AM_RANGE(0x1000, 0x1001) AM_DEVWRITE_LEGACY("ym2", ym3812_w)
	AM_RANGE(0x1800, 0x1800) AM_WRITENOP	/* MSM5205 chip on Pocket Gal, not connected here? */
//  AM_RANGE(0x2000, 0x2000) AM_WRITE(pokechmp_sound_bank_w)/ * might still be sound bank */
	AM_RANGE(0x2800, 0x2800) AM_DEVREADWRITE("oki", okim6295_device, read, write) // extra
	AM_RANGE(0x3000, 0x3000) AM_READ(soundlatch_r)
//  AM_RANGE(0x3400, 0x3400) AM_READ_LEGACY(pokechmp_adpcm_reset_r)    /* ? not sure */
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank3")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( pokechmp )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("DSW")	/* Dip switch */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )
	PORT_DIPSETTING(	0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(	0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(	0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(	0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(	0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Allow 2 Players Game" )
	PORT_DIPSETTING(	0x00, DEF_STR( No ) )
	PORT_DIPSETTING(	0x08, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(	0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Time" )
	PORT_DIPSETTING(	0x00, "100" )
	PORT_DIPSETTING(	0x20, "120" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(	0x00, "3" )
	PORT_DIPSETTING(	0x40, "4" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(	0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(	0x00, DEF_STR( On ) )
INPUT_PORTS_END

/* this makes no sense at all!  it seems impossible to get the colours to align up in the korean flag ingame and everything else is slightly broken too */
static const gfx_layout pokechmp_charlayout =
{
	8,8,	/* 8*8 characters */
	RGN_FRAC(1,8),
	8,
	{ RGN_FRAC(1,8), RGN_FRAC(0,8),RGN_FRAC(3,8),RGN_FRAC(2,8),RGN_FRAC(5,8),RGN_FRAC(4,8),RGN_FRAC(7,8),RGN_FRAC(6,8) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	 /* every char takes 8 consecutive bytes */
};


/* should be ok.. */
static const gfx_layout pokechmp_spritelayout =
{
16,16,  /* 16*16 sprites */
RGN_FRAC(1,8),   /* 1024 sprites */
4,
{RGN_FRAC(1,8),RGN_FRAC(3,8),RGN_FRAC(5,8),RGN_FRAC(7,8)},
{ 128+7, 128+6, 128+5, 128+4, 128+3, 128+2, 128+1, 128+0, 7, 6, 5, 4, 3, 2, 1, 0 },
{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
32*8    /* every char takes 8 consecutive bytes */
};


static GFXDECODE_START( pokechmp )
	GFXDECODE_ENTRY( "gfx1", 0x00000, pokechmp_charlayout,   0x100, 32 ) /* chars */
	GFXDECODE_ENTRY( "gfx2", 0x00000, pokechmp_spritelayout,   0,  32 ) /* sprites */
GFXDECODE_END


static MACHINE_CONFIG_START( pokechmp, pokechmp_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, 4000000)
	MCFG_CPU_PROGRAM_MAP(pokechmp_map)
	MCFG_CPU_VBLANK_INT("screen", nmi_line_pulse)

	MCFG_CPU_ADD("audiocpu", M6502, 4000000)
	MCFG_CPU_PROGRAM_MAP(pokechmp_sound_map)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_STATIC(pokechmp)

	MCFG_GFXDECODE(pokechmp)
	MCFG_PALETTE_LENGTH(0x400)

	MCFG_VIDEO_START(pokechmp)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ym1", YM2203, 1500000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)

	MCFG_SOUND_ADD("ym2", YM3812, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", 4000000/4, OKIM6295_PIN7_HIGH) // ?? unknown frequency
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)	/* sound fx */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

static DRIVER_INIT( pokechmp )
{
	memory_configure_bank(machine, "bank3", 0, 2, machine.region("audiocpu")->base() + 0x10000, 0x4000);
}


ROM_START( pokechmp )
	ROM_REGION( 0x24000, "maincpu", 0 )	 /* 64k for code + 16k for banks */
	ROM_LOAD( "pokechamp_11_27010.bin",	   0x10000, 0x14000, CRC(9afb6912) SHA1(e45da9524e3bb6f64a68200b70d0f83afe6e4379) )
	ROM_CONTINUE(			   0x04000, 0xc000)

	ROM_REGION( 0x18000, "audiocpu", 0 )	 /* 96k for code + 96k for decrypted opcodes */
	ROM_LOAD( "pokechamp_09_27c512.bin",	   0x10000, 0x8000, CRC(c78f6483) SHA1(a0d063effd8d1850f674edccb6e7a285b2311d21) )
	ROM_CONTINUE(			   0x08000, 0x8000 )

	ROM_REGION( 0x100000, "gfx1", 0)
	/* Seems to be 8bpp */
	ROM_LOAD( "pokechamp_05_27c020.bin",	   0x00000, 0x40000, CRC(554cfa42) SHA1(862d0dd83697da7bd52dc640c34926c62691afea) )
	ROM_LOAD( "pokechamp_06_27c020.bin",	   0x40000, 0x40000, CRC(00bb9536) SHA1(1a5584297ebb425d6ce331955e0c6a4f467cd1e6) )
	ROM_LOAD( "pokechamp_07_27c020.bin",	   0x80000, 0x40000, CRC(4b15ab5e) SHA1(5523134853b9ea1c81fd5aeb58061376d94e9298) )
	ROM_LOAD( "pokechamp_08_27c020.bin",	   0xc0000, 0x40000, CRC(e9db54d6) SHA1(ac3b7c06d0f61847bf9bc6147f2f88d712f2b4b3) )

	ROM_REGION( 0x40000, "gfx2", 0 )
	/* the first half of all these roms is identical.  For rom 3 both halves match.  Correct decode is to ignore the first half */
	ROM_LOAD( "pokechamp_02_27c512.bin",	   0x00000, 0x10000, CRC(1ff44545) SHA1(2eee44484accce7b0ba21babf6e8344b234a4e87) )
	ROM_LOAD( "pokechamp_01_27c512.bin",	   0x10000, 0x10000, CRC(338fc412) SHA1(bb8ae99ee6a399a8c67bedb88d0837fd0a4a426c) )
	ROM_LOAD( "pokechamp_04_27c512.bin",	   0x20000, 0x10000, CRC(ee6991af) SHA1(8eca3cdfd2eb74257253957a87b245b7f85bd038) )
	ROM_LOAD( "pokechamp_03_27c512.bin",	   0x30000, 0x10000, CRC(99f9884a) SHA1(096d6ce70dc51fb9142e80e1ec45d6d7225481f5) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "pokechamp_10_27c040.bin",	   0x00000, 0x80000, CRC(b54806ed) SHA1(c6e1485c263ebd9102ff1e8c09b4c4ca5f63c3da) )
ROM_END

GAME( 1995, pokechmp, 0, pokechmp, pokechmp, pokechmp, ROT0, "D.G.R.M.", "Poke Champ", GAME_WRONG_COLORS | GAME_IMPERFECT_SOUND )
