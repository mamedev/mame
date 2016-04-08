// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

  Super Burger Time     (c) 1990 Data East Corporation (DE-0343)
  China Town            (c) 1991 Data East Corporation (DE-0343)

  Sound:  Ym2151, Oki adpcm - NOTE!  The sound program writes to the address
of a YM2203 and a 2nd Oki chip but the board does _not_ have them.  The sound
program is simply the 'generic' Data East sound program unmodified for this cut
down hardware (it doesn't write any good sound data btw, mostly zeros).

  Super Burgertime has a few bugs:

  Some sprites clip at the edges of the screen.
  Some burgers (from crushing an enemy) appear with wrong colour.
  Colour cycle on title screen doesn't work first time around.

  These are NOT driver bugs!  They all exist in the original game.

  Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/supbtime.h"

/******************************************************************************/

READ16_MEMBER(supbtime_state::supbtime_controls_r)
{
	switch (offset << 1)
	{
		case 0:
			return ioport("INPUTS")->read();
		case 2:
			return ioport("DSW")->read();
		case 8:
			return ioport("COIN")->read();
		case 10: /* ?  Not used for anything */
		case 12:
			return 0;
	}

	logerror("CPU #0 PC %06x: warning - read unmapped control address %06x\n", space.device().safe_pc(), offset);
	return ~0;
}

WRITE16_MEMBER(supbtime_state::sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

/******************************************************************************/

static ADDRESS_MAP_START( supbtime_map, AS_PROGRAM, 16, supbtime_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x104000, 0x11ffff) AM_WRITENOP /* Nothing there */
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x120800, 0x13ffff) AM_WRITENOP /* Nothing there */
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x180000, 0x18000f) AM_READ(supbtime_controls_r)
	AM_RANGE(0x18000a, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a0001) AM_WRITE(sound_w)
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_r, pf_control_w)
	AM_RANGE(0x320000, 0x321fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x322000, 0x323fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x340000, 0x3407ff) AM_RAM AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x342000, 0x3427ff) AM_RAM AM_SHARE("pf2_rowscroll")
ADDRESS_MAP_END

static ADDRESS_MAP_START( chinatwn_map, AS_PROGRAM, 16, supbtime_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_WRITE(sound_w)
	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x180000, 0x18000f) AM_READ(supbtime_controls_r)
	AM_RANGE(0x18000a, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a3fff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_r, pf_control_w)
	AM_RANGE(0x320000, 0x321fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x322000, 0x323fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x340000, 0x3407ff) AM_RAM AM_SHARE("pf1_rowscroll") // unused
	AM_RANGE(0x342000, 0x3427ff) AM_RAM AM_SHARE("pf2_rowscroll") // unused
ADDRESS_MAP_END

/******************************************************************************/

/* Physical memory map (21 bits) */
static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, supbtime_state )
	ADDRESS_MAP_GLOBAL_MASK(0x1fffff)
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP /* YM2203 - this board doesn't have one */
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_NOP /* This board only has 1 oki chip */
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_DEVWRITE("audiocpu", h6280_device, timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE("audiocpu", h6280_device, irq_status_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( supbtime )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   /* inverted with respect to other Deco games */
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x8000, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0xc000, "3" )
	PORT_DIPSETTING(      0x4000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chinatwn )
	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* Used? */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )   /* button 3 - unused */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)  /* Used? */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )  /* button 3 - unused */
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")   /* inverted with respect to other Deco games */
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0xc000, 0xc000, "Time" )
	PORT_DIPSETTING(      0x0000, "1500" )
	PORT_DIPSETTING(      0x8000, "2000" )
	PORT_DIPSETTING(      0xc000, "2500" )
	PORT_DIPSETTING(      0x4000, "3000" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout tile_8x8_layout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout tile_16x16_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8,RGN_FRAC(1,2)+0,RGN_FRAC(0,2)+8,RGN_FRAC(0,2)+0 },
	{ 256,257,258,259,260,261,262,263,0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	32*16
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 24,8,16,0 },
	{ 512,513,514,515,516,517,518,519, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32,10*32,11*32,12*32,13*32,14*32,15*32},
	32*32
};

static GFXDECODE_START( supbtime )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0x100, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0x100, 32 )    /* Tiles (16x16) */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,        0, 16 )    /* Sprites (16x16) */
GFXDECODE_END

/******************************************************************************/

void supbtime_state::machine_start()
{
}

static MACHINE_CONFIG_START( supbtime, supbtime_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(supbtime_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supbtime_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", H6280, 32220000/8) /* Custom chip 45, audio section crystal is 32.220 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(supbtime_state, screen_update_supbtime)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supbtime)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 32220000/9)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) /* IRQ 2 */
	MCFG_SOUND_ROUTE(0, "mono", 0.45)
	MCFG_SOUND_ROUTE(1, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( chinatwn, supbtime_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(chinatwn_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", supbtime_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", H6280, 32220000/8) /* Custom chip 45, audio section crystal is 32.220 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_BEFORE_VBLANK)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(supbtime_state, screen_update_supbtime)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", supbtime)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", 32220000/9)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) /* IRQ 2 */
	MCFG_SOUND_ROUTE(0, "mono", 0.45)
	MCFG_SOUND_ROUTE(1, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( supbtime )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gk03", 0x00000, 0x20000, CRC(aeaeed61) SHA1(4bceb4475a642a36406395f1e84b16fa137f67a5) )
	ROM_LOAD16_BYTE( "gk04", 0x00001, 0x20000, CRC(2bc5a4eb) SHA1(721ec73c32af8b998babb6d7c9e526ced0c2389b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "gc06.bin",    0x00000, 0x10000, CRC(e0e6c0f4) SHA1(5a8b29752c58ea76d9c7961c5b0d8c94f35037af) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mae02.bin", 0x000000, 0x80000, CRC(a715cca0) SHA1(0539bba39c60324d85599ac69ff78bb215deb511) ) /* chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mae00.bin", 0x000001, 0x80000, CRC(30043094) SHA1(5302cfd9bdaf90c4901fda75407379c4ce1cbdec) ) /* sprites */
	ROM_LOAD16_BYTE( "mae01.bin", 0x000000, 0x80000, CRC(434af3fb) SHA1(1cfd30d14f03554e826576d6d32ce424f0df3748) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "gc05.bin",    0x00000, 0x20000, CRC(2f2246ff) SHA1(3fcceb6f5aa5f33187bcf4c59d88327f396fa80d) )
ROM_END

/* is this actually a good dump?, there are no backgrounds ingame! */
ROM_START( supbtimea )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "3.11f", 0x00000, 0x20000, CRC(98b5f263) SHA1(ee4b0d2fcdc95aba0e78d066bd6c4d553a902848) )
	ROM_LOAD16_BYTE( "4.12f", 0x00001, 0x20000, CRC(937e68b9) SHA1(4779e150518b9014c2154f33d38767c6a7447334) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "gc06.bin",    0x00000, 0x10000, CRC(e0e6c0f4) SHA1(5a8b29752c58ea76d9c7961c5b0d8c94f35037af) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mae02.bin", 0x000000, 0x80000, CRC(a715cca0) SHA1(0539bba39c60324d85599ac69ff78bb215deb511) ) /* chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mae00.bin", 0x000001, 0x80000, CRC(30043094) SHA1(5302cfd9bdaf90c4901fda75407379c4ce1cbdec) ) /* sprites */
	ROM_LOAD16_BYTE( "mae01.bin", 0x000000, 0x80000, CRC(434af3fb) SHA1(1cfd30d14f03554e826576d6d32ce424f0df3748) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "gc05.bin",    0x00000, 0x20000, CRC(2f2246ff) SHA1(3fcceb6f5aa5f33187bcf4c59d88327f396fa80d) )
ROM_END

ROM_START( supbtimej )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gc03.bin", 0x00000, 0x20000, CRC(b5621f6a) SHA1(2dfd361e81dc4805bc248cc293d94131162df2d2) )
	ROM_LOAD16_BYTE( "gc04.bin", 0x00001, 0x20000, CRC(551b2a0c) SHA1(8a6dde2d64029b8e7f7c9b88bd05633b69417dc1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "gc06.bin",    0x00000, 0x10000, CRC(e0e6c0f4) SHA1(5a8b29752c58ea76d9c7961c5b0d8c94f35037af) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mae02.bin", 0x000000, 0x80000, CRC(a715cca0) SHA1(0539bba39c60324d85599ac69ff78bb215deb511) ) /* chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mae00.bin", 0x000001, 0x80000, CRC(30043094) SHA1(5302cfd9bdaf90c4901fda75407379c4ce1cbdec) ) /* sprites */
	ROM_LOAD16_BYTE( "mae01.bin", 0x000000, 0x80000, CRC(434af3fb) SHA1(1cfd30d14f03554e826576d6d32ce424f0df3748) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "gc05.bin",    0x00000, 0x20000, CRC(2f2246ff) SHA1(3fcceb6f5aa5f33187bcf4c59d88327f396fa80d) )
ROM_END

ROM_START( chinatwn )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "gv_00-.f11", 0x00000, 0x20000, CRC(2ea7ea5d) SHA1(3d0eb63f3af00bcf10ba7416dd26b366578006bf) )
	ROM_LOAD16_BYTE( "gv_01-.f13", 0x00001, 0x20000, CRC(bcab03c7) SHA1(cd6c1ad26a0867482565a0544ea1870012cabf34) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "gv_02-.f16",    0x00000, 0x10000, CRC(95151d84) SHA1(9f49e49f966c3fc460773b187a110073eb595880) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mak-02.h2", 0x000000, 0x80000, CRC(745b2c50) SHA1(557ac71da170a04caaab393dc43e46858ef8dd70) ) /* chars */

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mak-00.a2", 0x000001, 0x80000, CRC(18e8cc1b) SHA1(afa79557222a94de7d9fde526ca45796f74fb3b2) ) /* sprites */
	ROM_LOAD16_BYTE( "mak-01.a4", 0x000000, 0x80000, CRC(d88ebda8) SHA1(ec6eab95f3ca8ee946151c46c6570b0b0c508ffc) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "gv_03-.j14",    0x00000, 0x20000, CRC(948faf92) SHA1(2538c7d4fa7fe0bfdd5dccece8ee82e911cee63f) )
ROM_END

/******************************************************************************/

GAME( 1990, supbtime, 0,        supbtime, supbtime, driver_device, 0, ROT0, "Data East Corporation", "Super Burger Time (World, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, supbtimea,supbtime, supbtime, supbtime, driver_device, 0, ROT0, "Data East Corporation", "Super Burger Time (World, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, supbtimej,supbtime, supbtime, supbtime, driver_device, 0, ROT0, "Data East Corporation", "Super Burger Time (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, chinatwn, 0,        chinatwn, chinatwn, driver_device, 0, ROT0, "Data East Corporation", "China Town (Japan)", MACHINE_SUPPORTS_SAVE )
