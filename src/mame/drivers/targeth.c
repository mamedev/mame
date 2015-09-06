// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

Target Hits (c) 1994 Gaelco (Designed & Developed by Zigurat. Produced by Gaelco)

Driver by Manuel Abadia <emumanu+mame@gmail.com>

The DS5002FP has 32KB undumped gameplay code making the game unplayable :_(

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "includes/targeth.h"

static const gfx_layout tilelayout16_0x080000 =
{
	16,16,                                                      /* 16x16 tiles */
	0x080000/32,                                                /* number of tiles */
	4,                                                          /* bitplanes */
	{ 3*0x080000*8, 2*0x080000*8, 1*0x080000*8, 0*0x080000*8 }, /* plane offsets */
	{ 0,1,2,3,4,5,6,7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },
	32*8
};

static GFXDECODE_START( 0x080000 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout16_0x080000, 0, 64 )
GFXDECODE_END


TIMER_DEVICE_CALLBACK_MEMBER(targeth_state::targeth_interrupt)
{
	int scanline = param;

	if(scanline == 240)
	{
		/* IRQ 2: drives the game */
		m_maincpu->set_input_line(2, HOLD_LINE);
	}

	if(scanline == 0)
	{
		/* IRQ 4: Read 1P Gun */
		m_maincpu->set_input_line(4, HOLD_LINE);
		/* IRQ 6: Read 2P Gun */
		m_maincpu->set_input_line(6, HOLD_LINE);
	}
}

WRITE16_MEMBER(targeth_state::OKIM6295_bankswitch_w)
{
	UINT8 *RAM = memregion("oki")->base();

	if (ACCESSING_BITS_0_7){
		memcpy(&RAM[0x30000], &RAM[0x40000 + (data & 0x0f)*0x10000], 0x10000);
	}
}

WRITE16_MEMBER(targeth_state::targeth_coin_counter_w)
{
	coin_counter_w( machine(), (offset >> 3) & 0x01, data & 0x01);
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 16, targeth_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x103fff) AM_RAM_WRITE(targeth_vram_w) AM_SHARE("videoram")  /* Video RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs") /* Video Registers */
	AM_RANGE(0x108000, 0x108001) AM_READ_PORT("GUNX1")
	AM_RANGE(0x108002, 0x108003) AM_READ_PORT("GUNY1")
	AM_RANGE(0x108004, 0x108005) AM_READ_PORT("GUNX2")
	AM_RANGE(0x108006, 0x108007) AM_READ_PORT("GUNY2")
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs") /* Video Registers */
	AM_RANGE(0x10800c, 0x10800d) AM_WRITENOP                    /* CLR Video INT */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    /* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")   /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW2")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("SYSTEM")             /* Coins, Start & Fire buttons */
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("SERVICE")            /* Service & Guns Reload? */
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)    /* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)  /* OKI6295 status register */
	AM_RANGE(0x700010, 0x70001b) AM_WRITENOP                        /* ??? Guns reload related? */
	AM_RANGE(0x70002a, 0x70003b) AM_WRITE(targeth_coin_counter_w)   /* Coin counters */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                             /* Work RAM (partially shared with DS5002FP) */
ADDRESS_MAP_END

static INPUT_PORTS_START( targeth )
	PORT_START("GUNX1")
	PORT_BIT( 0x01ff, 200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.05, -0.028, 0) PORT_MINMAX( 0, 400 + 4) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNY1")
	PORT_BIT( 0x01ff, 128, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.041, -0.011, 0) PORT_MINMAX(4,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNX2")
	PORT_BIT( 0x01ff, 400 + 4, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.05, -0.028, 0) PORT_MINMAX( 0, 400 + 4) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNY2")
	PORT_BIT( 0x01ff, 255, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.041, -0.011, 0) PORT_MINMAX(4,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, "Credit configuration" )
	PORT_DIPSETTING(    0x40, "Start 1C/Continue 1C" )
	PORT_DIPSETTING(    0x00, "Start 2C/Continue 1C" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Gun alarm" ) /* This doesn't work. What's supposed to do? */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* this MUST be low or the game doesn't boot */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) /* Reload 1P? */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) /* Reload 2P? */
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static MACHINE_CONFIG_START( targeth, targeth_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,24000000/2)          /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", targeth_state, targeth_interrupt, "screen", 0, 1)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(64*16, 32*16)              /* 1024x512 */
	MCFG_SCREEN_VISIBLE_AREA(0, 24*16-1, 16, 16*16-1)   /* 400x240 */
	MCFG_SCREEN_UPDATE_DRIVER(targeth_state, screen_update_targeth)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x080000)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

ROM_START( targeth )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "targeth.c23", 0x000000, 0x040000, CRC(840887d6) SHA1(9a36b346608d531a62a2e0704ea44f12e07f9d91) )
	ROM_LOAD16_BYTE( "targeth.c22", 0x000001, 0x040000, CRC(d2435eb8) SHA1(ce75a115dad8019c8e66a1c3b3e15f54781f65ae) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Graphics */
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(                 0x040000, 0x080000 )
	ROM_LOAD( "targeth.c3",     0x0c0000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

ROM_START( targetha )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x040000, CRC(e38a54e2) SHA1(239bfa6f1c0fc8aa0ad7de9be237bef55b384007) )
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x040000, CRC(24fe3efb) SHA1(8f48f08a6db28966c9263be119883c9179e349ed) )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Graphics */
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(                 0x040000, 0x080000 )
	ROM_LOAD( "targeth.c3",     0x0c0000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

GAME( 1994, targeth,  0,       targeth, targeth, driver_device, 0, ROT0, "Gaelco", "Target Hits (ver 1.1)", MACHINE_UNEMULATED_PROTECTION )
GAME( 1994, targetha, targeth, targeth, targeth, driver_device, 0, ROT0, "Gaelco", "Target Hits (ver 1.0)", MACHINE_UNEMULATED_PROTECTION )
