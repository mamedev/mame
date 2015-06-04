// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Haywood
/*
    Diet Go Go

    Driver by Bryan McPhail and David Haywood.

Hold both START buttons on bootup to display version notice.

Diet Go Go (Japan)   DATA EAST

NAME    LOCATION   TYPE
-----------------------
JW-02    14M       27C512
JW-01-2  5H        27C2001
JW-00-2  4H         "
PAL16L8B 7H
PAL16L8B 6H
PAL16R6A 11H

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/h6280/h6280.h"
#include "sound/2151intf.h"
#include "sound/okim6295.h"
#include "includes/decocrpt.h"
#include "includes/dietgo.h"


READ16_MEMBER( dietgo_state::dietgo_protection_region_0_104_r )
{
	int real_address = 0 + (offset *2);
	int deco146_addr = BITSWAP32(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	UINT8 cs = 0;
	UINT16 data = m_deco104->read_data( deco146_addr, mem_mask, cs );
	return data;
}

WRITE16_MEMBER( dietgo_state::dietgo_protection_region_0_104_w )
{
	int real_address = 0 + (offset *2);
	int deco146_addr = BITSWAP32(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	UINT8 cs = 0;
	m_deco104->write_data( space, deco146_addr, data, mem_mask, cs );
}


static ADDRESS_MAP_START( dietgo_map, AS_PROGRAM, 16, dietgo_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
	AM_RANGE(0x200000, 0x20000f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
	AM_RANGE(0x210000, 0x211fff) AM_DEVWRITE("tilegen1", deco16ic_device, pf1_data_w)
	AM_RANGE(0x212000, 0x213fff) AM_DEVWRITE("tilegen1", deco16ic_device, pf2_data_w)
	AM_RANGE(0x220000, 0x2207ff) AM_WRITEONLY AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x222000, 0x2227ff) AM_WRITEONLY AM_SHARE("pf2_rowscroll")
	AM_RANGE(0x280000, 0x2807ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x300000, 0x300bff) AM_RAM_DEVWRITE("deco_common", decocomn_device, nonbuffered_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x340000, 0x343fff) AM_READWRITE(dietgo_protection_region_0_104_r,dietgo_protection_region_0_104_w)AM_SHARE("prot16ram") /* Protection device */
	AM_RANGE(0x380000, 0x38ffff) AM_RAM // mainram
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 16, dietgo_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END


/* Physical memory map (21 bits) */
static ADDRESS_MAP_START( sound_map, AS_PROGRAM, 8, dietgo_state )
	AM_RANGE(0x000000, 0x00ffff) AM_ROM
	AM_RANGE(0x100000, 0x100001) AM_NOP     /* YM2203 - this board doesn't have one */
	AM_RANGE(0x110000, 0x110001) AM_DEVREADWRITE("ymsnd", ym2151_device, read, write)
	AM_RANGE(0x120000, 0x120001) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x130000, 0x130001) AM_NOP     /* This board only has 1 oki chip */
	AM_RANGE(0x140000, 0x140001) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x1f0000, 0x1f1fff) AM_RAMBANK("bank8")
	AM_RANGE(0x1fec00, 0x1fec01) AM_DEVWRITE("audiocpu", h6280_device, timer_w)
	AM_RANGE(0x1ff400, 0x1ff403) AM_DEVWRITE("audiocpu", h6280_device, irq_status_w)
ADDRESS_MAP_END



static INPUT_PORTS_START( dietgo )
	PORT_START("SYSTEM")   /* Verified as 4 bit input port only */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("DSW")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Continue Coin" )
	PORT_DIPSETTING(      0x0080, "1 Start/1 Continue" )
	PORT_DIPSETTING(      0x0000, "2 Start/1 Continue" )

	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0100, "1" )
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0300, "3" )
	PORT_DIPSETTING(      0x0200, "4" )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) ) // Demo_Sounds ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) ) // Players don't move in attract mode if on!?
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

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

static GFXDECODE_START( dietgo )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0, 32 )    /* Tiles (16x16) */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,      512, 16 )    /* Sprites (16x16) */
GFXDECODE_END

DECO16IC_BANK_CB_MEMBER(dietgo_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

void dietgo_state::machine_start()
{
}

static MACHINE_CONFIG_START( dietgo, dietgo_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_28MHz/2) /* DE102 (verified on pcb) */
	MCFG_CPU_PROGRAM_MAP(dietgo_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", dietgo_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", H6280, XTAL_32_22MHz/4/3)  /* Custom chip 45; XIN is 32.220MHZ/4, verified on pcb */
	MCFG_CPU_PROGRAM_MAP(sound_map)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dietgo_state, screen_update_dietgo)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dietgo)

	MCFG_DECOCOMN_ADD("deco_common")
	MCFG_DECOCOMN_PALETTE("palette")

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(dietgo_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(dietgo_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

	MCFG_DECO104_ADD("ioprot104")
	MCFG_DECO146_SET_INTERFACE_SCRAMBLE_INTERLEAVE
	MCFG_DECO146_SET_USE_MAGIC_ADDRESS_XOR

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_32_22MHz/9) /* verified on pcb */
	MCFG_YM2151_IRQ_HANDLER(INPUTLINE("audiocpu", 1)) /* IRQ 2 */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.45)

	MCFG_OKIM6295_ADD("oki", XTAL_32_22MHz/32, OKIM6295_PIN7_HIGH) /* verified on pcb */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.60)
MACHINE_CONFIG_END


/* Diet Go Go */

ROM_START( dietgo )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "jy_00-2.4h", 0x000001, 0x040000, CRC(014dcf62) SHA1(1a28ce4a643ec8b6f062b1200342ed4dc6db38a1) )
	ROM_LOAD16_BYTE( "jy_01-2.5h", 0x000000, 0x040000, CRC(793ebd83) SHA1(b9178f18ce6e9fca848cbbf9dce3f3856672bf94) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jy_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD16_BYTE( "may-02.16a", 0x000001, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( dietgoe ) // weird, still version 1.1 but different (earlier) date
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "jy_00-1.4h", 0x000001, 0x040000, CRC(8bce137d) SHA1(55f5b1c89330803c6147f9656f2cabe8d1de8478) )
	ROM_LOAD16_BYTE( "jy_01-1.5h", 0x000000, 0x040000, CRC(eca50450) SHA1(1a24117e3b1b66d7dbc5484c94cc2c627d34e6a3) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jy_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD16_BYTE( "may-02.16a", 0x000001, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( dietgou )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "jx_00-.4h", 0x000001, 0x040000, CRC(1a9de04f) SHA1(7ce1e7cf4cdce2b02da4df2a6ae9a9e665e24422) )
	ROM_LOAD16_BYTE( "jx_01-.5h", 0x000000, 0x040000, CRC(79c097c8) SHA1(be49055ee324535e1118d243bd49e74ec1d2a2d7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jx_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) ) /* Same as other regions but different label */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD16_BYTE( "may-02.16a", 0x000001, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( dietgoj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_BYTE( "jw_00-2.4h", 0x000001, 0x040000, CRC(e6ba6c49) SHA1(d5eaea81f1353c58c03faae67428f7ee98e766b1) )
	ROM_LOAD16_BYTE( "jw_01-2.5h", 0x000000, 0x040000, CRC(684a3d57) SHA1(bd7a57ba837a1dc8f92b5ebcb46e50db1f98524f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "jw_02.14m", 0x00000, 0x10000, CRC(4e3492a5) SHA1(5f302bdbacbf95ea9f3694c48545a1d6bba4b019) ) /* Same as other regions but different label */

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "may-00.10a", 0x00000, 0x100000, CRC(234d1f8d) SHA1(42d23aad20df20cbd2359cc12bdd47636b2027d3) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "may-01.14a", 0x000000, 0x100000, CRC(2da57d04) SHA1(3898e9fef365ecaa4d86aa11756b527a4fffb494) )
	ROM_LOAD16_BYTE( "may-02.16a", 0x000001, 0x100000, CRC(3a66a713) SHA1(beeb99156332cf4870738f7769b719a02d7b40af) )

	ROM_REGION( 0x80000, "oki", 0 ) /* Oki samples */
	ROM_LOAD( "may-03.11l", 0x00000, 0x80000, CRC(b6e42bae) SHA1(c282cdf7db30fb63340cc609bf00f5ab63a75583) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8b_vd-00.6h",  0x0000, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16l8b_vd-01.7h",  0x0200, 0x0104, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "pal16r6a_vd-02.11h", 0x0400, 0x0104, NO_DUMP ) /* PAL is read protected */
ROM_END


DRIVER_INIT_MEMBER(dietgo_state,dietgo)
{
	deco56_decrypt_gfx(machine(), "gfx1");
	deco102_decrypt_cpu((UINT16 *)memregion("maincpu")->base(), m_decrypted_opcodes, 0x80000, 0xe9ba, 0x01, 0x19);
}

GAME( 1992, dietgo,   0,      dietgo, dietgo, dietgo_state,  dietgo,    ROT0, "Data East Corporation", "Diet Go Go (Euro v1.1 1992.09.26)", GAME_SUPPORTS_SAVE )
GAME( 1992, dietgoe,  dietgo, dietgo, dietgo, dietgo_state,  dietgo,    ROT0, "Data East Corporation", "Diet Go Go (Euro v1.1 1992.08.04)" , GAME_SUPPORTS_SAVE )
GAME( 1992, dietgou,  dietgo, dietgo, dietgo, dietgo_state,  dietgo,    ROT0, "Data East Corporation", "Diet Go Go (USA v1.1 1992.09.26)", GAME_SUPPORTS_SAVE )
GAME( 1992, dietgoj,  dietgo, dietgo, dietgo, dietgo_state,  dietgo,    ROT0, "Data East Corporation", "Diet Go Go (Japan v1.1 1992.09.26)", GAME_SUPPORTS_SAVE )
