// license:BSD-3-Clause
// copyright-holders:David Haywood,Bryan McPhail
/***************************************************************************

  Tumblepop (World)     (c) 1991 Data East Corporation
  Tumblepop (Japan)     (c) 1991 Data East Corporation

Stephh's notes (based on the games M68000 code and some tests) :

1) 'tumblep*' and 'jumpkids'

  - I don't understand the interest of the "Remove Monsters" Dip Switch :
    as I haven't found a way to "end" a level, I guess that it was used to
    test the backgrounds and the "platforms".

  - The "Edit Levels" Dip Switch allows you to add/delete monsters and
    change their position.

    Notes (for 'tumblep', 'tumblepj', 'tumblep2') :
      * "worlds" and levels are 0-based (00-09 & 00-09) :

          World      Name
            0      America
            1      Brazil
            2      Asia
            3      Soviet
            4      Europe
            5      Egypt
            6      Australia
            7      Antartica
            8      Stratosphere
            9      Space

      * As levels x-9 and 9-x are only constitued of a "big boss", you can't
        edit them !
      * All data is stored within the range 0x02b8c8-0x02d2c9, but it should be
        extended to 0x02ebeb (and perhaps 0x02ffff). TO BE CONFIRMED !
      * Once your levels are ready, turn the Dip Switch OFF and reset the game.
      * Of course, there is no possibility to save the levels when you exit
        MAME, nor the way to reload the default ones 8(

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "includes/decocrpt.h"
#include "sound/2151intf.h"
#include "sound/3812intf.h"
#include "sound/okim6295.h"
#include "includes/tumblep.h"


#define TUMBLEP_HACK    0

/******************************************************************************/

#ifdef UNUSED_FUNCTION
WRITE16_MEMBER(tumblep_state::tumblep_oki_w)
{
	okim6295_device *oki = downcast<okim6295_device *>(device);
	oki->write(0, data & 0xff);
	/* STUFF IN OTHER BYTE TOO..*/
}

READ16_MEMBER(tumblep_state::tumblep_prot_r)
{
	return ~0;
}
#endif

WRITE16_MEMBER(tumblep_state::tumblep_sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(0, HOLD_LINE);
}

#ifdef UNUSED_FUNCTION
WRITE16_MEMBER(tumblep_state::jumppop_sound_w)
{
	soundlatch_byte_w(space, 0, data & 0xff);
	m_audiocpu->set_input_line(ASSERT_LINE );
}
#endif

/******************************************************************************/

READ16_MEMBER(tumblep_state::tumblepop_controls_r)
{
	switch (offset << 1)
	{
		case 0:
			return ioport("PLAYERS")->read();
		case 2:
			return ioport("DSW")->read();
		case 8:
			return ioport("SYSTEM")->read();
		case 10: /* ? */
		case 12:
			return 0;
	}

	return ~0;
}

/******************************************************************************/

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, tumblep_state )
#if TUMBLEP_HACK
	AM_RANGE(0x000000, 0x07ffff) AM_WRITEONLY   // To write levels modifications
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
#else
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
#endif
	AM_RANGE(0x100000, 0x100001) AM_WRITE(tumblep_sound_w)
	AM_RANGE(0x120000, 0x123fff) AM_RAM
	AM_RANGE(0x140000, 0x1407ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x180000, 0x18000f) AM_READ(tumblepop_controls_r)
	AM_RANGE(0x18000c, 0x18000d) AM_WRITENOP
	AM_RANGE(0x1a0000, 0x1a07ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x300000, 0x30000f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
	AM_RANGE(0x320000, 0x320fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x322000, 0x322fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x340000, 0x3407ff) AM_WRITEONLY AM_SHARE("pf1_rowscroll") // unused
	AM_RANGE(0x342000, 0x3427ff) AM_WRITEONLY AM_SHARE("pf2_rowscroll") // unused
ADDRESS_MAP_END

/******************************************************************************/

/* Physical memory map (21 bits) */
static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, tumblep_state )
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

static INPUT_PORTS_START( tumblep )
	PORT_START("PLAYERS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
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

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x00e0, 0x00e0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00e0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0060, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00a0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00c0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x001c, 0x001c, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x001c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x000c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0014, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0001, 0x0001, "2 Coins to Start, 1 to Continue" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x8000, "1" )
	PORT_DIPSETTING(    0x0000, "2" )
	PORT_DIPSETTING(    0xc000, "3" )
	PORT_DIPSETTING(    0x4000, "4" )
	PORT_DIPNAME( 0x3000, 0x3000, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x3000, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( Hardest ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0800, 0x0800, "Remove Monsters" )
#else
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unused ) )       // See notes
#endif
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
#if TUMBLEP_HACK
	PORT_DIPNAME( 0x0400, 0x0400, "Edit Levels" )
#else
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unused ) )       // See notes
#endif
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0100, 0x0000, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
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

static GFXDECODE_START( tumblep )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0x100, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0x100, 32 )    /* Tiles (16x16) */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,        0, 16 )    /* Sprites (16x16) */
GFXDECODE_END

/***************************************************************************/

void tumblep_state::machine_start()
{
}

static MACHINE_CONFIG_START( tumblep, tumblep_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tumblep_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", H6280, 32220000/8) /* Custom chip 45; Audio section crystal is 32.220 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-2, 1*8, 31*8-1) // hmm
	MCFG_SCREEN_UPDATE_DRIVER(tumblep_state, screen_update_tumblep)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tumblep)
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
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 32220000/9)
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) /* IRQ 2 */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.45)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.45)

	MCFG_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( tumblep )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hl00-1.f12", 0x00000, 0x40000, CRC(fd697c1b) SHA1(1a3dee4c7383f2bc2d73037e80f8f5d8297e7433) )
	ROM_LOAD16_BYTE("hl01-1.f13", 0x00001, 0x40000, CRC(d5a62a3f) SHA1(7249563993fa8e1f19ddae51306d4a576b5cb206) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound cpu */
	ROM_LOAD( "hl02-.f16",    0x00000, 0x10000, CRC(a5cab888) SHA1(622f6adb01e31b8f3adbaed2b9900b54c5922c57) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "map-02.rom",   0x00000, 0x80000, CRC(dfceaa26) SHA1(83e391ff39efda71e5fa368ac68ba7d6134bac21) )  // encrypted

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD16_BYTE( "map-00.rom",   0x00001, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "hl03-.j15",    0x00000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b) )
ROM_END

ROM_START( tumblepj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE("hk00-1.f12", 0x00000, 0x40000, CRC(2d3e4d3d) SHA1(0acc8b93bd49395904dff11c582bdbaccdbd3eef) )
	ROM_LOAD16_BYTE("hk01-1.f13", 0x00001, 0x40000, CRC(56912a00) SHA1(0545f6bff2a0aa2f36adda0f9d73b165387abc3a) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound cpu */
	ROM_LOAD( "hl02-.f16",    0x00000, 0x10000, CRC(a5cab888) SHA1(622f6adb01e31b8f3adbaed2b9900b54c5922c57) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "map-02.rom",   0x00000, 0x80000, CRC(dfceaa26) SHA1(83e391ff39efda71e5fa368ac68ba7d6134bac21) )  // encrypted

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "map-01.rom",   0x00000, 0x80000, CRC(e81ffa09) SHA1(01ada9557ead91eb76cf00db118d6c432104a398) )
	ROM_LOAD16_BYTE( "map-00.rom",   0x00001, 0x80000, CRC(8c879cfe) SHA1(a53ef7811f14a8b105749b1cf29fe8a3a33bab5e) )

	ROM_REGION( 0x40000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "hl03-.j15",    0x00000, 0x20000, CRC(01b81da0) SHA1(914802f3206dc59a720af9d57eb2285bc8ba822b) )
ROM_END

/******************************************************************************/

#if TUMBLEP_HACK
void ::tumblep_patch_code(UINT16 offset)
{
	/* A hack which enables all Dip Switches effects */
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();
	RAM[(offset + 0)/2] = 0x0240;
	RAM[(offset + 2)/2] = 0xffff;   // andi.w  #$f3ff, D0
}
#endif

DRIVER_INIT_MEMBER(tumblep_state,tumblep)
{
	deco56_decrypt_gfx(machine(), "gfx1");

	#if TUMBLEP_HACK
	tumblep_patch_code(0x000132);
	#endif
}

/******************************************************************************/

GAME( 1991, tumblep,  0,       tumblep,   tumblep, tumblep_state,  tumblep,  ROT0, "Data East Corporation", "Tumble Pop (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1991, tumblepj, tumblep, tumblep,   tumblep, tumblep_state,  tumblep,  ROT0, "Data East Corporation", "Tumble Pop (Japan)", MACHINE_SUPPORTS_SAVE )
/* for bootlegs and games on similar hardware see tumbleb.c */
