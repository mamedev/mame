// license:BSD-3-Clause
// copyright-holders:David Haywood, Bryan McPhail
/*

Pocket Gal Deluxe
Nihon System Inc., 1993

This game runs on Data East hardware.

PCB Layout
----------

DEC-22V0  DE-0378-2
|-------------------------------------|
|  M6295(2)  KG01.14F     DE102  62256|
|  M6295(1)  MAZ-03.13F          62256|
|                                     |
|   32.220MHz              KG00-2.12A |
|                                     |
|J           DE56              DE71   |
|A                                    |
|M     DE153            28MHz         |
|M                    6264     DE52   |
|A                    6264            |
|                                     |
|                                     |
|                                     |
|DSW1                   MAZ-01.3B     |
|     DE104  MAZ-02.2H                |
|DSW2                   MAZ-00.1B     |
|-------------------------------------|

Notes:
      - CPU is DE102. The clock input is 14.000MHz on pin 6
        It's a custom-made encrypted 68000.
        The package is a Quad Flat Pack, has 128 pins and is symmetrically square (1 1/4" square from tip to tip).
      - M6295(1) clock: 2.01375MHz (32.220 / 16)
        M6295(2) clock: 1.006875MHz (32.220 / 32)
      - VSync: 58Hz


    Driver by David Haywood and Bryan McPhail

*/

/*
original todo:
    verify sound.

bootleg todo:

    Fix GFX glitches in background of girls after each level.
    Tidy up code.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "includes/decocrpt.h"
#include "sound/okim6295.h"
#include "includes/pktgaldx.h"

/**********************************************************************************/

WRITE16_MEMBER(pktgaldx_state::pktgaldx_oki_bank_w)
{
	m_oki2->set_bank_base((data & 3) * 0x40000);
}

/**********************************************************************************/

READ16_MEMBER( pktgaldx_state::pktgaldx_protection_region_f_104_r )
{
	int real_address = 0 + (offset *2);
	UINT8 cs = 0;
	UINT16 data = m_deco104->read_data( real_address&0x7fff, mem_mask, cs );
	return data;
}

WRITE16_MEMBER( pktgaldx_state::pktgaldx_protection_region_f_104_w )
{
	int real_address = 0 + (offset *2);
	UINT8 cs = 0;
	m_deco104->write_data( space, real_address&0x7fff, data, mem_mask, cs );
}


static ADDRESS_MAP_START( pktgaldx_map, AS_PROGRAM, 16, pktgaldx_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM

	AM_RANGE(0x100000, 0x100fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x102000, 0x102fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x110000, 0x1107ff) AM_RAM AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x112000, 0x1127ff) AM_RAM AM_SHARE("pf2_rowscroll")

	AM_RANGE(0x120000, 0x1207ff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x130000, 0x130fff) AM_RAM_DEVWRITE("deco_common", decocomn_device, nonbuffered_palette_w) AM_SHARE("paletteram")

	AM_RANGE(0x140000, 0x14000f) AM_DEVWRITE8("oki1", okim6295_device, write, 0x00ff)
	AM_RANGE(0x140006, 0x140007) AM_DEVREAD8("oki1", okim6295_device, read, 0x00ff)
	AM_RANGE(0x150000, 0x15000f) AM_DEVWRITE8("oki2", okim6295_device, write, 0x00ff)
	AM_RANGE(0x150006, 0x150007) AM_DEVREAD8("oki2", okim6295_device, read, 0x00ff)

	AM_RANGE(0x161800, 0x16180f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
	AM_RANGE(0x164800, 0x164801) AM_WRITE(pktgaldx_oki_bank_w)

	AM_RANGE(0x167800, 0x167fff) AM_READWRITE(pktgaldx_protection_region_f_104_r,pktgaldx_protection_region_f_104_w) AM_SHARE("prot16ram") /* Protection device */

	AM_RANGE(0x170000, 0x17ffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 16, pktgaldx_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END


/* Pocket Gal Deluxe (bootleg!) */

READ16_MEMBER(pktgaldx_state::pckgaldx_unknown_r)
{
	return 0xffff;
}

READ16_MEMBER(pktgaldx_state::pckgaldx_protection_r)
{
	logerror("pckgaldx_protection_r address %06x\n",space.device().safe_pc());
	return -1;
}

/*
cpu #0 (PC=0000A0DE): unmapped program memory word read from 00167DB2 & FFFF
cpu #0 (PC=00009DFA): unmapped program memory word read from 00167C4C & FFFF
cpu #0 (PC=00009E58): unmapped program memory word read from 00167842 & 00FF
cpu #0 (PC=00009E80): unmapped program memory word read from 00167842 & FF00
cpu #0 (PC=00009AFE): unmapped program memory word read from 00167842 & 00FF
cpu #0 (PC=00009B12): unmapped program memory word read from 00167842 & FF00
cpu #0 (PC=0000923C): unmapped program memory word read from 00167DB2 & 00FF
*/

/* do the 300000 addresses somehow interact with the protection addresses on this bootleg? */
/* or maybe protection writes go to sound ... */

static ADDRESS_MAP_START( pktgaldb_map, AS_PROGRAM, 16, pktgaldx_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x100fff) AM_RAM AM_SHARE("pktgaldb_fgram") // fgram on original?
	AM_RANGE(0x102000, 0x102fff) AM_RAM // bgram on original?
	AM_RANGE(0x120000, 0x123fff) AM_RAM AM_SHARE("pktgaldb_spr")

	AM_RANGE(0x130000, 0x130fff) AM_RAM // palette on original?

	AM_RANGE(0x140000, 0x14000f) AM_DEVWRITE8("oki1", okim6295_device, write, 0x00ff)
	AM_RANGE(0x140006, 0x140007) AM_DEVREAD8("oki1", okim6295_device, read, 0x00ff)
	AM_RANGE(0x150000, 0x15000f) AM_DEVWRITE8("oki2", okim6295_device, write, 0x00ff)
	AM_RANGE(0x150006, 0x150007) AM_DEVREAD8("oki2", okim6295_device, read, 0x00ff)

//  AM_RANGE(0x160000, 0x167fff) AM_RAM
	AM_RANGE(0x164800, 0x164801) AM_WRITE(pktgaldx_oki_bank_w)
	AM_RANGE(0x160000, 0x167fff) AM_WRITENOP
	AM_RANGE(0x16500a, 0x16500b) AM_READ(pckgaldx_unknown_r)

	/* should we really be using these to read the i/o in the BOOTLEG?
	  these look like i/o through protection ... */
	AM_RANGE(0x167842, 0x167843) AM_READ_PORT("INPUTS")
	AM_RANGE(0x167c4c, 0x167c4d) AM_READ_PORT("DSW")
	AM_RANGE(0x167db2, 0x167db3) AM_READ_PORT("SYSTEM")

	AM_RANGE(0x167d10, 0x167d11) AM_READ(pckgaldx_protection_r) // check code at 6ea
	AM_RANGE(0x167d1a, 0x167d1b) AM_READ(pckgaldx_protection_r) // check code at 7C4

	AM_RANGE(0x170000, 0x17ffff) AM_RAM

	AM_RANGE(0x300000, 0x30000f) AM_RAM // ??

	AM_RANGE(0x330000, 0x330bff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette") // extra colours?
ADDRESS_MAP_END


/**********************************************************************************/

static INPUT_PORTS_START( pktgaldx )
	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SW1:4,5,6")
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0028, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "2 Coins to Start, 1 to Continue" )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Lives ) )            PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "2" )
	PORT_DIPSETTING(      0x0100, "3" )
	PORT_DIPSETTING(      0x0300, "4" )
	PORT_DIPSETTING(      0x0200, "5" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Time"  )                 PORT_DIPLOCATION("SW2:3,4") /* Listed as "Difficulty" */
	PORT_DIPSETTING(      0x0000, "60" )
	PORT_DIPSETTING(      0x0400, "80" )
	PORT_DIPSETTING(      0x0c00, "100" )
	PORT_DIPSETTING(      0x0800, "120" )
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:5" )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Free_Play ) )            PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x4000, "SW2:7" )
	PORT_DIPNAME( 0x8000, 0x0000, DEF_STR( Demo_Sounds ) )          PORT_DIPLOCATION("SW2:8")
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

static GFXDECODE_START( pktgaldx )
	GFXDECODE_ENTRY( "gfx1", 0, tile_8x8_layout,     0, 32 )    /* Tiles (8x8) */
	GFXDECODE_ENTRY( "gfx1", 0, tile_16x16_layout,   0, 32 )    /* Tiles (16x16) */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,      512, 32 )    /* Sprites (16x16) */
GFXDECODE_END

static const gfx_layout bootleg_spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ 48,52,56,60,32,36,40,44,16,20,24,28,0,4,8,12 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64},
	16*64
};

static GFXDECODE_START( bootleg )
	GFXDECODE_ENTRY( "gfx1", 0, bootleg_spritelayout,     0, 64 )
GFXDECODE_END


DECO16IC_BANK_CB_MEMBER(pktgaldx_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}


void pktgaldx_state::machine_start()
{
}

static MACHINE_CONFIG_START( pktgaldx, pktgaldx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 14000000)
	MCFG_CPU_PROGRAM_MAP(pktgaldx_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pktgaldx_state,  irq6_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pktgaldx_state, screen_update_pktgaldx)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_FORMAT(XBGR)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pktgaldx)

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
	MCFG_DECO16IC_BANK1_CB(pktgaldx_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(pktgaldx_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

	MCFG_DECO104_ADD("ioprot104")
	MCFG_DECO146_SET_INTERFACE_SCRAMBLE(8,9,  4,5,6,7    ,1,0,3,2) // hopefully this is correct, nothing else uses this arrangement!

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)

	MCFG_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( pktgaldb, pktgaldx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 16000000)
	MCFG_CPU_PROGRAM_MAP(pktgaldb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pktgaldx_state,  irq6_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(pktgaldx_state, screen_update_pktgaldb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 4096)
	MCFG_PALETTE_FORMAT(XBGR)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bootleg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", 32220000/32, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.75)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.75)

	MCFG_OKIM6295_ADD("oki2", 32220000/16, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.60)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.60)
MACHINE_CONFIG_END


ROM_START( pktgaldx )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_WORD_SWAP( "ke00-2.12a",    0x00000, 0x80000, CRC(b04baf3a) SHA1(680d1b4ab4b6edef36cd96a60539fb7c2dac9637) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "maz-02.2h",    0x00000, 0x100000, CRC(c9d35a59) SHA1(07b44c7d7d76b668b4d6ca5672bd1c2910228e68) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "maz-00.1b",    0x000000, 0x080000, CRC(fa3071f4) SHA1(72e7d920e9ca94f8cb166007a9e9e5426a201af8) )
	ROM_LOAD16_BYTE( "maz-01.3b",    0x000001, 0x080000, CRC(4934fe21) SHA1(b852249f59906d69d32160ebaf9b4781193227e4) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "ke01.14f",    0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) )

	ROM_REGION( 0x100000, "oki2", 0 ) /* Oki samples (banked?) */
	ROM_LOAD( "maz-03.13f",    0x00000, 0x100000, CRC(a313c964) SHA1(4a3664c4e2c44a017a0ab6a6d4361799cbda57b5) )
ROM_END

ROM_START( pktgaldxj )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* DE102 code (encrypted) */
	ROM_LOAD16_WORD_SWAP( "kg00-2.12a",    0x00000, 0x80000, CRC(62dc4137) SHA1(23887dc3f6e7c4cdcb1bf4f4c87fe3cbe8cdbe69) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "maz-02.2h",    0x00000, 0x100000, CRC(c9d35a59) SHA1(07b44c7d7d76b668b4d6ca5672bd1c2910228e68) )

	ROM_REGION( 0x100000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "maz-00.1b",    0x000000, 0x080000, CRC(fa3071f4) SHA1(72e7d920e9ca94f8cb166007a9e9e5426a201af8) )
	ROM_LOAD16_BYTE( "maz-01.3b",    0x000001, 0x080000, CRC(4934fe21) SHA1(b852249f59906d69d32160ebaf9b4781193227e4) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "ke01.14f",    0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) )

	ROM_REGION( 0x100000, "oki2", 0 ) /* Oki samples (banked?) */
	ROM_LOAD( "maz-03.13f",    0x00000, 0x100000, CRC(a313c964) SHA1(4a3664c4e2c44a017a0ab6a6d4361799cbda57b5) )
ROM_END

ROM_START( pktgaldxb )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE ("4.bin", 0x00000, 0x80000, CRC(67ce30aa) SHA1(c5228ed19eebbfb6d5f7cbfcb99734d9fcd5aba3) )
	ROM_LOAD16_BYTE( "5.bin", 0x00001, 0x80000, CRC(64cb4c33) SHA1(02f988f558113dd9a77079dee59e23583394fa98) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD16_BYTE( "11.bin",    0x000000, 0x80000, CRC(a8c8f1fd) SHA1(9fd5fa500967a1bd692abdbeef89ce195c8aecd4) )
	ROM_LOAD16_BYTE( "6.bin",     0x000001, 0x80000, CRC(0e3335a1) SHA1(2d6899336302d222e8404dde159e64911a8f94e6) )
	ROM_LOAD16_BYTE( "10.bin",    0x100000, 0x80000, CRC(9dd743a9) SHA1(dbc3e2bd044dbf21b04c174bd860969ee53b4050) )
	ROM_LOAD16_BYTE( "7.bin",     0x100001, 0x80000, CRC(0ebf12b5) SHA1(17b6c2ce21de3671d75d89a41317efddf5b49339) )
	ROM_LOAD16_BYTE( "9.bin",     0x200001, 0x80000, CRC(078f371c) SHA1(5b510a0f7f50c55cce1ffcc8f2e9c3432b23e352) )
	ROM_LOAD16_BYTE( "8.bin",     0x200000, 0x80000, CRC(40f5a032) SHA1(c2ad585ddbc3ef40c6214cb30b4d78a2cd0a9446) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "kg01.14f",    0x00000, 0x20000, CRC(8a106263) SHA1(229ab17403c2b8f4e89a90a8cda2f3c3a4b55d9e) ) // 1.bin on the bootleg

	ROM_REGION( 0x100000, "oki2", 0 ) /* Oki samples (banked?) */
	ROM_LOAD( "3.bin",    0x00000, 0x80000, CRC(4638747b) SHA1(56d79cd8d4d7b41b71f1e942b5a5bf1bafc5c6e7) )
	ROM_LOAD( "2.bin",    0x80000, 0x80000, CRC(f841d995) SHA1(0ef2f8fd9be62b979862c3688e7aad34c7b0404d) )
ROM_END



DRIVER_INIT_MEMBER(pktgaldx_state,pktgaldx)
{
	deco56_decrypt_gfx(machine(), "gfx1");
	deco102_decrypt_cpu((UINT16 *)memregion("maincpu")->base(), m_decrypted_opcodes, 0x80000, 0x42ba, 0x00, 0x00);
}

GAME( 1992, pktgaldx,  0,        pktgaldx, pktgaldx, pktgaldx_state, pktgaldx,  ROT0, "Data East Corporation", "Pocket Gal Deluxe (Euro v3.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, pktgaldxj, pktgaldx, pktgaldx, pktgaldx, pktgaldx_state, pktgaldx,  ROT0, "Data East Corporation (Nihon System license)", "Pocket Gal Deluxe (Japan v3.00)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, pktgaldxb, pktgaldx, pktgaldb, pktgaldx, driver_device, 0,         ROT0, "bootleg",               "Pocket Gal Deluxe (Euro v3.00, bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
