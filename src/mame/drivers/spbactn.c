// license:BSD-3-Clause
// copyright-holders:David Haywood, Stephh
/*******************************************************************************
 Super Pinball Action (c) 1991 Tecmo
********************************************************************************
 driver by David Haywood
 inputs, dipswitches etc by Stephh

-general info-------------------------------------------------------------------

 A Pinball Game from Tecmo, the Hardware seems to be somewhere between that used
 for Tecmo's classic game Ninja Gaiden (see gaiden.c) and that used in Comad's
 Gals Pinball (see galspnbl.c) I imagine Comad took the hardware that this uses
 as a basis for writing their game on, adding a couple of features such as the
 pixel layer.

The manual defines the controls as 4 push buttons:

   Left Push Buttons       Right Push Buttons

|   o    |      o       |    o    |      o       |
|--------+--------------+---------+--------------|
|Flipper | Shot & Shake | Flipper | Shot & Shake |
| Left   |    Left      |  Right  |    Right     |
|--------+--------------+---------+--------------|


-readme file--------------------------------------------------------------------

 Super Pinball Action
 (c)1991 Tecmo

 CPU  : MC68000P12
 Sound: Z80A YM3812 Y3014B M6295
 OSC  : 12.000MHz 22.656MHz 4.00MHz

 ------
 9002-A
 ------
 ROMs:
 a-u68.1 - Main programs (27c101)
 a-u67.2 /

 a-u14.3 - Sound program (27512)

 a-u19 - Samples (27c1001)

 1 custom chip (u94, surface scrached)

 ------
 9002-B
 ------
 ROMs:
 b-u98  - Graphics (Mask, read as 27c2001)
 b-u99  |
 b-u104 |
 b-u105 /


 b-u110 - Graphics (Mask, read as 27c2001)
 b-u111 /

 Custom chips:
 U101, U102, U106, U107: surface scrached
 probably 2 pairs of TECMO-3&4
 U133: surface scrached
 probably TECMO-6
 U112: TECMO-5

 --- Team Japump!!! ---
 Dumped by Noel Miruru
 17/Oct/2000

-working notes------------------------------------------------------------------

 68k interrupts
 lev 1 : 0x64 : ffff ffff - invalid
 lev 2 : 0x68 : ffff ffff - invalid
 lev 3 : 0x6c : 0000 1a0a - vblank?
 lev 4 : 0x70 : ffff ffff - invalid
 lev 5 : 0x74 : ffff ffff - invalid
 lev 6 : 0x78 : 0000 1ab2 - writes to 90031
 lev 7 : 0x7c : ffff ffff - invalid

TODO : (also check the notes from the galspnbl.c driver)

  - coin insertion is not recognized consistently.
  - rewrite video, do single pass sprite render, move sprite code to device, share with gaiden.c etc.
  - convert to tilemaps
  - all the unknown regs


Unmapped writes (P.O.S.T.)

cpu #0 (PC=00001C3A): unmapped memory word write to 00090080 = 0F30 & FFFF
cpu #0 (PC=00001C42): unmapped memory word write to 00090090 = 0E00 & FFFF
cpu #0 (PC=00001C4A): unmapped memory word write to 000900A0 = 0F74 & FFFF
cpu #0 (PC=00001C52): unmapped memory word write to 000900B0 = 0FBA & FFFF
cpu #0 (PC=00001C5A): unmapped memory word write to 000900C0 = 0FDA & FFFF
cpu #0 (PC=00001C62): unmapped memory word write to 000900D0 = 0F20 & FFFF
cpu #0 (PC=00001C6A): unmapped memory word write to 000900E0 = 0FE7 & FFFF
cpu #0 (PC=00001C72): unmapped memory word write to 000900F0 = 0FF1 & FFFF
cpu #0 (PC=00001C7A): unmapped memory word write to 000A0110 = 0001 & FFFF
cpu #0 (PC=00001C80): unmapped memory word write to 000A0010 = 0001 & FFFF
cpu #0 (PC=00001C88): unmapped memory word write to 000A0200 = 001F & FFFF
cpu #0 (PC=00001C90): unmapped memory word write to 000A0202 = 0010 & FFFF
cpu #0 (PC=00001C98): unmapped memory word write to 000A0204 = 00E0 & FFFF
cpu #0 (PC=00001CA0): unmapped memory word write to 000A0206 = 0001 & FFFF
cpu #0 (PC=00002BFA): unmapped memory word write to 00090000 = 0000 & 00FF
cpu #0 (PC=00002C08): unmapped memory word write to 000A0100 = FF85 & FFFF
cpu #0 (PC=00002C10): unmapped memory word write to 000A0000 = FF85 & FFFF
cpu #0 (PC=00002C18): unmapped memory word write to 000A0108 = 0010 & FFFF
cpu #0 (PC=00002C20): unmapped memory word write to 000A0008 = 0010 & FFFF
cpu #0 (PC=00002C28): unmapped memory word write to 000A0104 = 0000 & FFFF
cpu #0 (PC=00002C2E): unmapped memory word write to 000A010C = 0000 & FFFF
cpu #0 (PC=00002C34): unmapped memory word write to 000A0004 = 0000 & FFFF
cpu #0 (PC=00002C3A): unmapped memory word write to 000A000C = 0000 & FFFF
cpu #0 (PC=00002C42): unmapped memory word write to 00090050 = 0004 & 00FF
cpu #0 (PC=00001A14): unmapped memory word write to 00090020 = 00AA & 00FF
cpu #0 (PC=00001A1A): unmapped memory word write to 00090030 = 0055 & 00FF

   Unmapped writes (when Dip Switches are displayed)

cpu #0 (PC=00001A14): unmapped memory word write to 00090020 = 00FF & 00FF
cpu #0 (PC=00001A1A): unmapped memory word write to 00090030 = 00FF & 00FF

   Unmapped writes (when grid is displayed)

cpu #0 (PC=0000326A): unmapped memory word write to 00090010 = 00FF & 00FF (only once)
cpu #0 (PC=00001A14): unmapped memory word write to 00090020 = 00F6 & 00FF
cpu #0 (PC=00001A1A): unmapped memory word write to 00090030 = 00F7 & 00FF

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "includes/spbactn.h"


WRITE16_MEMBER(spbactn_state::soundcommand_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, offset, data & 0xff);
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
	}
}

static ADDRESS_MAP_START( spbactn_map, AS_PROGRAM, 16, spbactn_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
	AM_RANGE(0x40000, 0x43fff) AM_RAM   // main ram
	AM_RANGE(0x50000, 0x50fff) AM_RAM AM_SHARE("spvideoram")
	AM_RANGE(0x60000, 0x67fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x70000, 0x77fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x80000, 0x827ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x90000, 0x90001) AM_READ_PORT("IN0")
	AM_RANGE(0x90010, 0x90011) AM_READ_PORT("IN1")
	AM_RANGE(0x90020, 0x90021) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x90030, 0x90031) AM_READ_PORT("DSW1")
	AM_RANGE(0x90040, 0x90041) AM_READ_PORT("DSW2")

	/* this are an awful lot of unknowns */
	AM_RANGE(0x90000, 0x90001) AM_WRITENOP
	AM_RANGE(0x90010, 0x90011) AM_WRITE(soundcommand_w)
//  AM_RANGE(0x90020, 0x90021) AM_WRITE(soundcommand_w)
	AM_RANGE(0x90030, 0x90031) AM_WRITENOP

	AM_RANGE(0x90080, 0x90081) AM_WRITENOP
	AM_RANGE(0x90090, 0x90091) AM_WRITENOP
	AM_RANGE(0x900a0, 0x900a1) AM_WRITENOP
	AM_RANGE(0x900b0, 0x900b1) AM_WRITENOP
	AM_RANGE(0x900c0, 0x900c1) AM_WRITENOP
	AM_RANGE(0x900d0, 0x900d1) AM_WRITENOP
	AM_RANGE(0x900e0, 0x900e1) AM_WRITENOP
	AM_RANGE(0x900f0, 0x900f1) AM_WRITENOP

	AM_RANGE(0xa0000, 0xa0001) AM_WRITENOP
	AM_RANGE(0xa0004, 0xa0005) AM_WRITENOP
	AM_RANGE(0xa0008, 0xa0009) AM_WRITENOP
	AM_RANGE(0xa000c, 0xa000d) AM_WRITENOP
	AM_RANGE(0xa0010, 0xa0011) AM_WRITENOP

	AM_RANGE(0xa0100, 0xa0101) AM_WRITENOP
	AM_RANGE(0xa0104, 0xa0105) AM_WRITENOP
	AM_RANGE(0xa0108, 0xa0109) AM_WRITENOP
	AM_RANGE(0xa010c, 0xa010d) AM_WRITENOP
	AM_RANGE(0xa0110, 0xa0111) AM_WRITENOP

	AM_RANGE(0xa0200, 0xa0201) AM_WRITENOP
	AM_RANGE(0xa0202, 0xa0203) AM_WRITENOP
	AM_RANGE(0xa0204, 0xa0205) AM_WRITENOP
	AM_RANGE(0xa0206, 0xa0207) AM_WRITENOP
ADDRESS_MAP_END



static ADDRESS_MAP_START( spbactnp_map, AS_PROGRAM, 16, spbactn_state )
	AM_RANGE(0x00000, 0x3ffff) AM_ROM
	AM_RANGE(0x40000, 0x43fff) AM_RAM   // main ram
	AM_RANGE(0x50000, 0x50fff) AM_RAM AM_SHARE("spvideoram")
	AM_RANGE(0x60000, 0x67fff) AM_RAM_WRITE(fg_videoram_w) AM_SHARE("fgvideoram")
	AM_RANGE(0x70000, 0x77fff) AM_RAM_WRITE(bg_videoram_w) AM_SHARE("bgvideoram")
	AM_RANGE(0x80000, 0x827ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")   // yes R and G are swapped vs. the released version

	AM_RANGE(0x90002, 0x90003) AM_WRITE( spbatnp_90002_w )
	AM_RANGE(0x90006, 0x90007) AM_WRITE( spbatnp_90006_w )
	AM_RANGE(0x9000a, 0x9000b) AM_WRITE( spbatnp_9000a_w )
	AM_RANGE(0x9000c, 0x9000d) AM_WRITE( spbatnp_9000c_w )
	AM_RANGE(0x9000e, 0x9000f) AM_WRITE( spbatnp_9000e_w )

	AM_RANGE(0x90124, 0x90125) AM_WRITE( spbatnp_90124_w ) // bg scroll
	AM_RANGE(0x9012c, 0x9012d) AM_WRITE( spbatnp_9012c_w ) // bg scroll



	AM_RANGE(0x90000, 0x900ff) AM_READ(temp_read_handler_r) // temp

ADDRESS_MAP_END

static ADDRESS_MAP_START( spbactn_sound_map, AS_PROGRAM, 8, spbactn_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf800) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xf810, 0xf811) AM_DEVWRITE("ymsnd", ym3812_device, write)

	AM_RANGE(0xfc00, 0xfc00) AM_READNOP AM_WRITENOP /* irq ack ?? */
	AM_RANGE(0xfc20, 0xfc20) AM_READ(soundlatch_byte_r)
ADDRESS_MAP_END



static ADDRESS_MAP_START( spbactnp_extra_map, AS_PROGRAM, 8, spbactn_state )
	AM_RANGE(0x0000, 0xefff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("extraram2")
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xd000, 0xd1ff) AM_RAM_WRITE( extraram_w ) AM_SHARE("extraram")
	AM_RANGE(0xd200, 0xd200) AM_RAM
ADDRESS_MAP_END



static INPUT_PORTS_START( spbactn )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME( "Left Flippers" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME( "Launch Ball / Shake (Left Side)" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME( "Launch Ball / Shake (Right Side)" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME( "Right Flippers" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME( "Start" )  /* needed to avoid confusion with # of players. Press mulitple times for multiple players */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x02, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x05, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x08, "2 Coins/1 Credit 3/2" )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/1 Credit 5/6" )
	PORT_DIPNAME( 0xc0, 0xc0, "Balls" )         PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
	PORT_DIPSETTING(    0x40, "5" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Extra Ball" )        PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, "100k and 500k" )
	PORT_DIPSETTING(    0x0c, "200k and 800k" )
	PORT_DIPSETTING(    0x08, "200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, "Hit Difficulty" )        PORT_DIPLOCATION("SW2:5")   // From .xls file - WHAT does that mean ?
	PORT_DIPSETTING(    0x10, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x20, 0x20, "Display Instructions" )  PORT_DIPLOCATION("SW2:6") /* Listed in manual as "Change Software", but seems to have no effect? */
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7") /* As listed in manual, but seems to have no effect? */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Match" )         PORT_DIPLOCATION("SW2:8")   // Check code at 0x00bf8c
	PORT_DIPSETTING(    0x80, "1/20" )
	PORT_DIPSETTING(    0x00, "1/40" )
INPUT_PORTS_END

static const gfx_layout fgtilelayout =
{
	16,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, RGN_FRAC(1,2)+0*4, RGN_FRAC(1,2)+1*4, 2*4, 3*4, RGN_FRAC(1,2)+2*4, RGN_FRAC(1,2)+3*4,
			16*8+0*4, 16*8+1*4, 16*8+RGN_FRAC(1,2)+0*4, 16*8+RGN_FRAC(1,2)+1*4, 16*8+2*4, 16*8+3*4, 16*8+RGN_FRAC(1,2)+2*4, 16*8+RGN_FRAC(1,2)+3*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	32*8
};

static const gfx_layout bgtilelayout =
{
	16,8,
	RGN_FRAC(1,2),
	4,
	{ 3, 2, 1, 0 },

	{ RGN_FRAC(1,2)+1*4, RGN_FRAC(1,2)+0*4, 1*4, 0*4,
	RGN_FRAC(1,2)+3*4, RGN_FRAC(1,2)+2*4, 3*4, 2*4,
	16*8+RGN_FRAC(1,2)+1*4,16*8+RGN_FRAC(1,2)+0*4, 16*8+1*4,16*8+0*4,
	16*8+RGN_FRAC(1,2)+3*4, 16*8+RGN_FRAC(1,2)+2*4, 16*8+3*4,16*8+2*4 },

	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	32*8
};

static const gfx_layout spritelayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 0, 4, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+4, 8+0, 8+4, 8+RGN_FRAC(1,2)+0, 8+RGN_FRAC(1,2)+4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( spbactn )
	GFXDECODE_ENTRY( "gfx1", 0, fgtilelayout,   0x0200, 16 + 240 )
	GFXDECODE_ENTRY( "gfx2", 0, bgtilelayout,   0x0300, 16 + 128 )
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,   0x0000, 0x1000 )
GFXDECODE_END


static const gfx_layout proto_fgtilelayout =
{
	16,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,  64*4, 65*4, 66*4, 67*4, 68*4, 69*4, 70*4, 71*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	64*8
};



static const gfx_layout proto_spr_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};


static GFXDECODE_START( spbactnp )
	GFXDECODE_ENTRY( "gfx1", 0, proto_fgtilelayout,   0x0200, 16 + 240 )
	GFXDECODE_ENTRY( "gfx2", 0, proto_fgtilelayout,   0x0300, 16 + 128 ) // wrong
	GFXDECODE_ENTRY( "gfx3", 0, proto_spr_layout,   0x0000, 16 + 384 )

	GFXDECODE_ENTRY( "gfx4", 0, proto_spr_layout,   0x0000, 16 + 384 ) // more sprites maybe?

GFXDECODE_END


static MACHINE_CONFIG_START( spbactn, spbactn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(spbactn_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spbactn_state,  irq3_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(spbactn_sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_VIDEO_START_OVERRIDE(spbactn_state,spbactn)
	MCFG_SCREEN_UPDATE_DRIVER(spbactn_state, screen_update_spbactn)
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", spbactn)
	MCFG_PALETTE_ADD("palette", 0x2800/2)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("spritegen", TECMO_SPRITE, 0)
	MCFG_TECMO_SPRITE_GFX_REGION(2)
	MCFG_DEVICE_ADD("mixer", TECMO_MIXER, 0)
	MCFG_TECMO_MIXER_SHIFTS(8,10,4)
	MCFG_TECMO_MIXER_BLENDCOLS(   0x0000 + 0x300, 0x0000 + 0x200, 0x0000 + 0x100, 0x0000 + 0x000 )
	MCFG_TECMO_MIXER_REGULARCOLS( 0x0800 + 0x300, 0x0800 + 0x200, 0x0800 + 0x100, 0x0800 + 0x000 )
	MCFG_TECMO_MIXER_BLENDSOUCE( 0x1000 + 0x000, 0x1000 + 0x100)
	MCFG_TECMO_MIXER_BGPEN(0x800 + 0x300)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_4MHz) /* Was 3.579545MHz, a common clock, but no way to generate via on PCB OSCs */
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_4MHz/4, OKIM6295_PIN7_HIGH) /* Was 1.056MHz, a common clock, but no way to generate via on PCB OSCs. clock frequency & pin 7 not verified */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( spbactnp, spbactn_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(spbactnp_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spbactn_state,  irq3_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(spbactn_sound_map)

	// yes another cpu..
	MCFG_CPU_ADD("extracpu", Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(spbactnp_extra_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", spbactn_state,  irq0_line_hold)
//  MCFG_CPU_VBLANK_INT_DRIVER("screen", spbactn_state,  nmi_line_pulse)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)
	MCFG_VIDEO_START_OVERRIDE(spbactn_state,spbactnp)
	MCFG_SCREEN_UPDATE_DRIVER(spbactn_state, screen_update_spbactnp)
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", spbactnp)
	MCFG_PALETTE_ADD("palette", 0x2800/2)
	MCFG_PALETTE_FORMAT(xxxxBBBBRRRRGGGG)

	MCFG_DEVICE_ADD("spritegen", TECMO_SPRITE, 0)
	MCFG_TECMO_SPRITE_GFX_REGION(2)
	MCFG_DEVICE_ADD("mixer", TECMO_MIXER, 0)
	MCFG_TECMO_MIXER_SHIFTS(12,14,8)
	MCFG_TECMO_MIXER_BLENDCOLS(   0x0000 + 0x300, 0x0000 + 0x200, 0x0000 + 0x100, 0x0000 + 0x000 )
	MCFG_TECMO_MIXER_REGULARCOLS( 0x0800 + 0x300, 0x0800 + 0x200, 0x0800 + 0x100, 0x0800 + 0x000 )
	MCFG_TECMO_MIXER_BLENDSOUCE( 0x1000 + 0x000, 0x1000 + 0x100)
	MCFG_TECMO_MIXER_BGPEN(0x800 + 0x300)

	/* sound hardware  - different? */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, XTAL_4MHz)
	MCFG_YM3812_IRQ_HANDLER(INPUTLINE("audiocpu", 0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", XTAL_4MHz/4, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( spbactn )
	/* Board 9002-A (CPU Board) */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "rom1.bin", 0x00000, 0x20000, CRC(6741bd3f) SHA1(844eb6465a15d339043fd6d2b6ba20ba216de493) )
	ROM_LOAD16_BYTE( "rom2.bin", 0x00001, 0x20000, CRC(488cc511) SHA1(41b4a01f35e0e93634b4843dbb894ab9840807bf) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a-u14.3", 0x00000, 0x10000, CRC(57f4c503) SHA1(e5ddc63a43ba824bcaa4340eeba25a0d3f26cad9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a-u19",   0x00000, 0x20000,  CRC(87427d7d) SHA1(f76b0dc3f0d87deb0f0c81084aff9756b236e867) )

	/* Board 9002-B (GFX Board) */
	ROM_REGION( 0x080000, "gfx1", 0 ) /* 16x8 FG Tiles */
	ROM_LOAD( "b-u98",   0x00000, 0x40000, CRC(315eab4d) SHA1(6f812c85981dc649caca8b4635e3b8fd3a3c054d) )
	ROM_LOAD( "b-u99",   0x40000, 0x40000, CRC(7b76efd9) SHA1(9f23460aebe12cb5c4193776bf876d6044892979) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* 16x8 BG Tiles */
	ROM_LOAD( "b-u104",  0x00000, 0x40000, CRC(b648a40a) SHA1(1fb756dcd027a5702596e33bbe8a0beeb3ceb22b) )
	ROM_LOAD( "b-u105",  0x40000, 0x40000, CRC(0172d79a) SHA1(7ee1faa65c85860bd81988329df516bc34940ef5) )

	ROM_REGION( 0x080000, "gfx3", 0 ) /* 8x8 Sprite Tiles */
	ROM_LOAD( "b-u110",  0x00000, 0x40000, CRC(862ebacd) SHA1(05732e8524c50256c1db29317625d0edc19b87d2) )
	ROM_LOAD( "b-u111",  0x40000, 0x40000, CRC(1cc1379a) SHA1(44fdab8cb5ab1488688f1ac52f005454e835efee) )
ROM_END

ROM_START( spbactnj )
	/* Board 9002-A (CPU Board) */
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "a-u68.1", 0x00000, 0x20000, CRC(b5b2d824) SHA1(be04ca370a381d7396f39e31fb2680973193daee) )
	ROM_LOAD16_BYTE( "a-u67.2", 0x00001, 0x20000, CRC(9577b48b) SHA1(291d890a9d0e434455f183eb12ae6edf3156688d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "a-u14.3", 0x00000, 0x10000, CRC(57f4c503) SHA1(e5ddc63a43ba824bcaa4340eeba25a0d3f26cad9) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "a-u19",   0x00000, 0x20000,  CRC(87427d7d) SHA1(f76b0dc3f0d87deb0f0c81084aff9756b236e867) )

	/* Board 9002-B (GFX Board) */
	ROM_REGION( 0x080000, "gfx1", 0 ) /* 16x8 FG Tiles */
	ROM_LOAD( "b-u98",   0x00000, 0x40000, CRC(315eab4d) SHA1(6f812c85981dc649caca8b4635e3b8fd3a3c054d) )
	ROM_LOAD( "b-u99",   0x40000, 0x40000, CRC(7b76efd9) SHA1(9f23460aebe12cb5c4193776bf876d6044892979) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* 16x8 BG Tiles */
	ROM_LOAD( "b-u104",  0x00000, 0x40000, CRC(b648a40a) SHA1(1fb756dcd027a5702596e33bbe8a0beeb3ceb22b) )
	ROM_LOAD( "b-u105",  0x40000, 0x40000, CRC(0172d79a) SHA1(7ee1faa65c85860bd81988329df516bc34940ef5) )

	ROM_REGION( 0x080000, "gfx3", 0 ) /* 8x8 Sprite Tiles */
	ROM_LOAD( "b-u110",  0x00000, 0x40000, CRC(862ebacd) SHA1(05732e8524c50256c1db29317625d0edc19b87d2) )
	ROM_LOAD( "b-u111",  0x40000, 0x40000, CRC(1cc1379a) SHA1(44fdab8cb5ab1488688f1ac52f005454e835efee) )
ROM_END


ROM_START( spbactnp )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "spa.18k", 0x00000, 0x10000, CRC(40f6a1e6) SHA1(533e1eb96f54b976f50d5b8927160b46b2740c83) )
	ROM_LOAD16_BYTE( "spa.22k", 0x00001, 0x10000, CRC(ce31871e) SHA1(8670c051d775fee6dcd2aa82cdb6f3fcc4338bd5) )
	ROM_LOAD16_BYTE( "spa.17k", 0x20000, 0x10000, CRC(c9860ae9) SHA1(3c2479be75ee84165470e9ca0a9d3b2ce679703d) )
	ROM_LOAD16_BYTE( "spa.21k", 0x20001, 0x10000, CRC(8226f644) SHA1(2d3e32368fbfec7437bd972096fd92972f52f6b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "spa.17g", 0x00000, 0x10000, CRC(445fc2c5) SHA1(c0e40496cfcaa0a8c90fb05111fadee74582f91a) )


	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "spa_data_2-21-a10.8e",   0x00000, 0x20000,  CRC(87427d7d) SHA1(f76b0dc3f0d87deb0f0c81084aff9756b236e867) ) // same as regular

	ROM_REGION( 0x080000, "gfx1", 0 ) /* 16x8 FG Tiles */
	ROM_LOAD16_BYTE( "spa_back0_split0_5-17-p-1.27b",  0x00000, 0x20000, CRC(37922110) SHA1(8edb6745ab6b6937f1365d35bfcdbe86198de668) )
	ROM_LOAD16_BYTE( "spa_back0_split1_5-17-p-1.27c",  0x00001, 0x20000, CRC(9d6ef9ab) SHA1(338ff1bd9d30a61d782616cccb4108daac6a8612) )

	ROM_REGION( 0x080000, "gfx2", 0 ) /* 16x8 BG Tiles */ // it only ever draws the background from the rocket level, for all levels??
	ROM_LOAD16_BYTE( "spa_back1_split0_3-14-a-11.26b",  0x00000, 0x20000, CRC(6953fd62) SHA1(fb6061f5ad48e0d91d3dad96afbac2d64908f0a7) )
	ROM_LOAD16_BYTE( "spa_back1_split1_3-14-a-11.26c",  0x00001, 0x20000, CRC(b4123511) SHA1(c65b912238bab74bf46b5d5486c1d998813ef511) )

	ROM_REGION( 0x040000, "gfx3", 0 ) /* 8x8 Sprite Tiles */
	ROM_LOAD( "spa_sp0_4-18-p-8.5m",  0x00000, 0x20000, CRC(cd6ba360) SHA1(a01f65a678b6987ae877c381f74515efee4b492e) )
	ROM_LOAD( "spa_sp1_3-14-a-10.4m", 0x20000, 0x20000, CRC(86406336) SHA1(bf091dc13404535e6baee990f5e957d3538841ac) )


	/* does this have an extra (horizontal) screen maybe, with the girls being displayed on that instead of the main one.. */
	ROM_REGION( 0x10000, "extracpu", 0 ) // what? it's another z80 rom... unused for now
	ROM_LOAD( "6204_6-6.29c",   0x00000, 0x10000, CRC(e8250c26) SHA1(9b669878790c8e3c5d80f165b5ffa1d6830f4696) )

	ROM_REGION( 0x080000, "gfx4", 0 ) /* 8x8 BG Tiles */ // more 8x8 tiles, with the girl graphics? unused for now .. for horizontal orientation??
	ROM_LOAD( "spa.25c", 0x00000, 0x20000, CRC(02b69ab9) SHA1(368e774693a6fab756faaeec4ffd42406816e6e2) )

	ROM_REGION( 0x10000, "misc", 0 ) //misc
	ROM_LOAD( "p109.18d", 0x00000, 0x100, CRC(2297a725) SHA1(211ebae11ca55cc67df29291c3e0916836550bfb) ) // mostly empty.. is this correct?
	ROM_LOAD( "pin.b.sub.23g", 0x00000, 0x100, CRC(3a0c70ed) SHA1(9be38c421e9a14f6811752a4464dd5dbf037e385) ) // mostly empty.. is this correct?
	ROM_LOAD( "tcm1.19g.bin", 0x00000, 0x53, CRC(2c54354a) SHA1(11d8b6cdaf052b5a9fbcf6b6fbf99c5f89575cfa) )
ROM_END

GAME( 1991, spbactn, 0,        spbactn, spbactn, driver_device, 0, ROT90, "Tecmo", "Super Pinball Action (US)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1991, spbactnj, spbactn, spbactn, spbactn, driver_device, 0, ROT90, "Tecmo", "Super Pinball Action (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1989, spbactnp, spbactn, spbactnp, spbactn, driver_device, 0, ROT90, "Tecmo", "Super Pinball Action (prototype)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // early proto, (c) date is 2 years earlier!
