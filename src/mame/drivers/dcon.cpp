// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/***************************************************************************

    D-Con                                   (c) 1992 Success
    SD Gundam Psycho Salamander no Kyoui    (c) 1991 Banpresto/Bandai

    These games run on Seibu hardware.

    Emulation by Bryan McPhail, mish@tendril.co.uk

    Coin inputs are handled by the sound CPU, so they don't work with sound
    disabled. Use the service switch instead.

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "audio/seibu.h"
#include "includes/dcon.h"
#include "video/seibu_crtc.h"


/***************************************************************************/

static ADDRESS_MAP_START( dcon_map, AS_PROGRAM, 16, dcon_state )
	AM_RANGE(0x00000, 0x7ffff) AM_ROM
	AM_RANGE(0x80000, 0x8bfff) AM_RAM

	AM_RANGE(0x8c000, 0x8c7ff) AM_RAM_WRITE(background_w) AM_SHARE("back_data")
	AM_RANGE(0x8c800, 0x8cfff) AM_RAM_WRITE(foreground_w) AM_SHARE("fore_data")
	AM_RANGE(0x8d000, 0x8d7ff) AM_RAM_WRITE(midground_w) AM_SHARE("mid_data")
	AM_RANGE(0x8d800, 0x8e7ff) AM_RAM_WRITE(text_w) AM_SHARE("textram")
	AM_RANGE(0x8e800, 0x8f7ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x8f800, 0x8ffff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x9d000, 0x9d7ff) AM_WRITE(gfxbank_w)

	AM_RANGE(0xa0000, 0xa000d) AM_DEVREADWRITE("seibu_sound", seibu_sound_device, main_word_r, main_word_w)
	AM_RANGE(0xc0000, 0xc004f) AM_DEVREADWRITE("crtc", seibu_crtc_device, read, write)
	AM_RANGE(0xc0080, 0xc0081) AM_WRITENOP
	AM_RANGE(0xc00c0, 0xc00c1) AM_WRITENOP
	AM_RANGE(0xe0000, 0xe0001) AM_READ_PORT("DSW")
	AM_RANGE(0xe0002, 0xe0003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xe0004, 0xe0005) AM_READ_PORT("SYSTEM")
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( common )
	SEIBU_COIN_INPUTS   /* coin inputs read through sound cpu */

	PORT_START("P1_P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( dcon )
	PORT_INCLUDE( common )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sdgndmps )
	PORT_INCLUDE( common )

	PORT_MODIFY("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x0100, IP_ACTIVE_LOW )

	PORT_START("DSW")
	PORT_DIPNAME( 0x000f, 0x000f, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x000a, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x000f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x000e, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x000b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x000d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0009, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x00f0, 0x00f0, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x00a0, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x00f0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x00e0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0070, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x00b0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x00d0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0050, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0090, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0200, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0100, "4" )
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) // plays a jingle at the game intro (why?)
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Allow_Continue ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


/******************************************************************************/

static const gfx_layout dcon_charlayout =
{
	8,8,        /* 8*8 characters */
	RGN_FRAC(1,2),
	4,          /* 4 bits per pixel */
	{ 0,4,(0x10000*8)+0,0x10000*8+4 },
	{ 3,2,1,0, 11,10,9,8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	128
};

static const gfx_layout dcon_tilelayout =
{
	16,16,  /* 16*16 tiles */
	RGN_FRAC(1,1),
	4,      /* 4 bits per pixel */
	{ 8, 12, 0,4 },
	{
		3,2,1,0,19,18,17,16,
		512+3,512+2,512+1,512+0,
		512+11+8,512+10+8,512+9+8,512+8+8,
	},
	{
		0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32,
	},
	1024
};

static GFXDECODE_START( dcon )
	GFXDECODE_ENTRY( "gfx1", 0, dcon_charlayout,    1024+768, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, dcon_tilelayout,    1024+0,   16 )
	GFXDECODE_ENTRY( "gfx3", 0, dcon_tilelayout,    1024+512, 16 )
	GFXDECODE_ENTRY( "gfx4", 0, dcon_tilelayout,    1024+256, 16 )
	GFXDECODE_ENTRY( "gfx5", 0, dcon_tilelayout,           0, 64 )
GFXDECODE_END

WRITE16_MEMBER( dcon_state::layer_en_w )
{
	m_layer_en = data;
}

WRITE16_MEMBER( dcon_state::layer_scroll_w )
{
	COMBINE_DATA(&m_scroll_ram[offset]);
}

/******************************************************************************/

static MACHINE_CONFIG_START( dcon, dcon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(dcon_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dcon_state,  irq4_line_hold)

	SEIBU_SOUND_SYSTEM_CPU(4000000) /* Perhaps 14318180/4? */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dcon_state, screen_update_dcon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(dcon_state, layer_en_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(dcon_state, layer_scroll_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dcon)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM3812_INTERFACE(4000000,1320000)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sdgndmps, dcon_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)
	MCFG_CPU_PROGRAM_MAP(dcon_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dcon_state,  irq4_line_hold)

	SEIBU2_SOUND_SYSTEM_CPU(14318180/4)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dcon_state, screen_update_sdgndmps)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("crtc", SEIBU_CRTC, 0)
	MCFG_SEIBU_CRTC_LAYER_EN_CB(WRITE16(dcon_state, layer_en_w))
	MCFG_SEIBU_CRTC_LAYER_SCROLL_CB(WRITE16(dcon_state, layer_scroll_w))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dcon)
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	SEIBU_SOUND_SYSTEM_YM2151_INTERFACE(14318180/4,1320000)
MACHINE_CONFIG_END

/***************************************************************************/

ROM_START( dcon )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_BYTE("p0-0",   0x000000, 0x20000, CRC(a767ec15) SHA1(5ceeba97b58c4e24d8c0991303dd6f7a2dfeda48) )
	ROM_LOAD16_BYTE("p0-1",   0x000001, 0x20000, CRC(a7efa091) SHA1(aa0e97d20f3bdc1adc019fe62112a8417bb3ddf1) )
	ROM_LOAD16_BYTE("p1-0",   0x040000, 0x20000, CRC(3ec1ef7d) SHA1(6195f1402dba5b3d3913e97cd78ba1e8865f7692) )
	ROM_LOAD16_BYTE("p1-1",   0x040001, 0x20000, CRC(4b8de320) SHA1(14a3ab347fc468869355951294c3e3a8f9211b6a) )

	ROM_REGION( 0x20000, "audiocpu", 0 )     /* 64k code for sound Z80 */
	ROM_LOAD( "fmsnd",           0x000000, 0x08000, CRC(50450faa) SHA1(d4add7d357951b51d53ed7f143ece7f3bde7f4cb) )
	ROM_CONTINUE(             0x010000, 0x08000 )
	ROM_COPY( "audiocpu", nullptr, 0x018000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "fix0",  0x000000, 0x10000, CRC(ab30061f) SHA1(14dba37fef7bd13c827fd542b24cc593dcdc9f99) ) /* chars */
	ROM_LOAD( "fix1",  0x010000, 0x10000, CRC(a0582115) SHA1(498d6e4f631a5dfe54d5c2813c47d40c466b694d) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "bg1",   0x000000, 0x80000, CRC(eac43283) SHA1(f5d384c98751002416013a9a920e2ab2cea61cb1) ) /* tiles */

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "bg3",   0x000000, 0x80000, CRC(1408a1e0) SHA1(d96fb8a60af02df313ffc9e0284611d7ca50540d) ) /* tiles */

	ROM_REGION( 0x080000, "gfx4", 0 )
	ROM_LOAD( "bg2",   0x000000, 0x80000, CRC(01864eb6) SHA1(78f755d7462a787bd1a378184e8fce8fa889f258) ) /* tiles */

	ROM_REGION( 0x200000, "gfx5", 0 )
	ROM_LOAD( "obj0",  0x000000, 0x80000, CRC(c3af37db) SHA1(7d6ee07b6302aaec8d792faf78a37898a2ac3c4e) ) /* sprites */
	ROM_LOAD( "obj1",  0x080000, 0x80000, CRC(be1f53ba) SHA1(061b80487e6c4040618af6ed9c5315fba44f5d0c) )
	ROM_LOAD( "obj2",  0x100000, 0x80000, CRC(24e0b51c) SHA1(434b4d58f785eefb5380c08a0704c8dea6609268) )
	ROM_LOAD( "obj3",  0x180000, 0x80000, CRC(5274f02d) SHA1(69b94363624177c92e1b3413244ce649c2e5a696) )

	ROM_REGION( 0x40000, "oki", 0 )  /* ADPCM samples */
	ROM_LOAD( "pcm", 0x000000, 0x20000, CRC(d2133b85) SHA1(a2e61c9893da8a95c35c0b47e2c43c315b654de8) )
ROM_END

ROM_START( sdgndmps )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "911-a01.25",   0x00000, 0x20000, CRC(3362915d) SHA1(d98e2d4de402ca549664e148c9a6fe94fccfd5e9) )
	ROM_LOAD16_BYTE( "911-a02.29",   0x00001, 0x20000, CRC(fbc78285) SHA1(85d40b0e7bb923a0daacbd78ce7d5bb9c80b9ffc) )
	ROM_LOAD16_BYTE( "911-a03.27",   0x40000, 0x20000, CRC(6c24b4f2) SHA1(e9fb82884f47694bebcad9254cb57a0b01dcd9c8) )
	ROM_LOAD16_BYTE( "911-a04.28",   0x40001, 0x20000, CRC(6ff9d716) SHA1(303faec19a84afd6cbcf3ca5d4877693c11d406e) )

	ROM_REGION( 0x20000, "audiocpu", 0 )    /* Z80 code, banked data */
	ROM_LOAD( "911-a05.010",   0x00000, 0x08000, CRC(90455406) SHA1(dd2c5b96ac4b51251a3d34d97cc9af360afaa38c) )
	ROM_CONTINUE(              0x10000, 0x08000 )
	ROM_COPY( "audiocpu", nullptr,  0x18000, 0x08000 )

	ROM_REGION( 0x020000, "gfx1", 0 )
	ROM_LOAD( "911-a08.66",   0x000000, 0x10000, CRC(e7e04823) SHA1(d9b1ace5cd8218d5a4767cf5adbc267dce7c0668) ) /* chars */
	ROM_LOAD( "911-a07.73",   0x010000, 0x10000, CRC(6f40d4a9) SHA1(8abadb2dc07ac22081b2970358e9f92b90b174b0) )

	ROM_REGION( 0x080000, "gfx2", 0 )
	ROM_LOAD( "911-a12.63",   0x000000, 0x080000, CRC(8976bbb6) SHA1(6f510d6506e54ddec7119d85dcc0a169d4901983) )

	ROM_REGION( 0x080000, "gfx3", 0 )
	ROM_LOAD( "911-a11.65",   0x000000, 0x080000, CRC(3f3b7810) SHA1(0761c5fb0802fdd2ee7523f1f4e5cfb2c7a6fce6) )

	ROM_REGION( 0x100000, "gfx4", 0 )
	ROM_LOAD( "911-a13.64",   0x000000, 0x100000, CRC(f38a584a) SHA1(16dd8e7086949d14e9185c37313290024d6dafdc) )

	ROM_REGION( 0x200000, "gfx5", 0 )
	ROM_LOAD( "911-a10.73",   0x000000, 0x100000, CRC(80e341fb) SHA1(619e71aefd0b13a01a6a2ed5d8613fe56242d209) )    /* sprites */
	ROM_LOAD( "911-a09.74",   0x100000, 0x100000, CRC(98f34519) SHA1(20319d546df104485ee553ce0e58364f927d1135) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples */
	ROM_LOAD( "911-a06.97",   0x00000, 0x40000, CRC(12c79440) SHA1(9e9987527f64dfd8a51a2ab49afc465e76c5e7ac) )

	ROM_REGION( 512, "proms", 0 )
	ROM_LOAD( "bnd-007.88",   0x00000, 512, CRC(96f7646e) SHA1(400a831b83d6ac4d2a46ef95b97b1ee237099e44) ) /* Priority */
ROM_END

/***************************************************************************/
DRIVER_INIT_MEMBER(dcon_state,sdgndmps)
{
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();
	RAM[0x1356/2] = 0x4e71; /* beq -> nop */
	RAM[0x1358/2] = 0x4e71;

	RAM[0x4de/2]  = 0x4245; /* ROM checksum */
	RAM[0x4e0/2]  = 0x4e71;
	RAM[0x4e2/2]  = 0x4e71;
}


GAME( 1991, sdgndmps, 0, sdgndmps, sdgndmps, dcon_state, sdgndmps, ROT0, "Banpresto / Bandai", "SD Gundam Psycho Salamander no Kyoui", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, dcon,     0, dcon,     dcon, driver_device,     0,        ROT0, "Success",            "D-Con", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
