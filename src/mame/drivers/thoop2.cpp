// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Peter Ferrie
/***************************************************************************

Thunder Hoop II: Strikes Back (c) 1994 Gaelco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

updated by Peter Ferrie <peter.ferrie@gmail.com>

Very similar to maniacsq and biomtoy but protected :_(
The DS5002FP has up to 128 KB undumped gameplay code
pf: its presence might be a distraction, since the game runs at least partially without it
pf: but some gameplay bugs - sprite positioning is incorrect, no enemies, jump animation never completes

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "includes/thoop2.h"

static const gfx_layout thoop2_tilelayout =
{
	8,8,                                    /* 8x8 tiles */
	0x400000/16,                            /* number of tiles */
	4,                                      /* 4 bpp */
	{ 0*0x400000*8+8, 0*0x400000*8, 1*0x400000*8+8, 1*0x400000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout thoop2_tilelayout_16 =
{
	16,16,                                  /* 16x16 tiles */
	0x400000/64,                            /* number of tiles */
	4,                                      /* 4 bpp */
	{ 0*0x400000*8+8, 0*0x400000*8, 1*0x400000*8+8, 1*0x400000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+4, 16*16+5, 16*16+6, 16*16+7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
		8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};


static GFXDECODE_START( thoop2 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, thoop2_tilelayout, 0,        64 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, thoop2_tilelayout_16, 0, 64 )
GFXDECODE_END


WRITE16_MEMBER(thoop2_state::OKIM6295_bankswitch_w)
{
	UINT8 *RAM = memregion("oki")->base();

	if (ACCESSING_BITS_0_7){
		memcpy(&RAM[0x30000], &RAM[0x40000 + (data & 0x0f)*0x10000], 0x10000);
	}
}

WRITE16_MEMBER(thoop2_state::thoop2_coin_w)
{
	if (ACCESSING_BITS_0_7){
		switch ((offset >> 3)){
			case 0x00:  /* Coin Lockouts */
			case 0x01:
				coin_lockout_w(machine(), (offset >> 3) & 0x01, ~data & 0x01);
				break;
			case 0x02:  /* Coin Counters */
			case 0x03:
				coin_counter_w(machine(), (offset >> 3) & 0x01, data & 0x01);
				break;
		}
	}

	/* 04b unknown. Sound related? */
	/* 05b unknown */
}

/* pretend that it's there */

READ16_MEMBER(thoop2_state::DS5002FP_R)
{
	return 0x55aa;
}

static ADDRESS_MAP_START( thoop2_map, AS_PROGRAM, 16, thoop2_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                 /* ROM */
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(thoop2_vram_w) AM_SHARE("videoram")   /* Video RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs")                 /* Video Registers */
	AM_RANGE(0x10800c, 0x10800d) AM_WRITE(watchdog_reset16_w)                           /* INT 6 ACK/Watchdog timer */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")/* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                       /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW2")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW1")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)                        /* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)                  /* OKI6295 data register */
	AM_RANGE(0x70000a, 0x70005b) AM_WRITE(thoop2_coin_w)                                /* Coin Counters + Coin Lockout */
	AM_RANGE(0xfeff00, 0xfeff01) AM_READ(DS5002FP_R)
	AM_RANGE(0xfeff02, 0xfeff03) AM_WRITENOP  /* pf: 0xfeff02 and 0xfeff03 need to remain zero always */
	AM_RANGE(0xfe0000, 0xfeffff) AM_RAM                                                 /* Work RAM (partially shared with DS5002FP) */
ADDRESS_MAP_END


static INPUT_PORTS_START( thoop2 )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_A ) )
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
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x18, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )   /* test button */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static MACHINE_CONFIG_START( thoop2, thoop2_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,24000000/2)          /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(thoop2_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", thoop2_state,  irq6_line_hold)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(thoop2_state, screen_update_thoop2)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", thoop2)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( thoop2 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "th2c23.040",   0x000000, 0x080000, CRC(3e465753) SHA1(1ea1173b9fe5d652e7b5fafb822e2535cecbc198) )
	ROM_LOAD16_BYTE(    "th2c22.040",   0x000001, 0x080000, CRC(837205b7) SHA1(f78b90c2be0b4dddaba26f074ea00eff863cfdb2) )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "th2-h8.32m",     0x000000, 0x400000, CRC(60328a11) SHA1(fcdb374d2fc7ef5351a4181c471d192199dc2081) )
	ROM_LOAD( "th2-h12.32m",    0x400000, 0x400000, CRC(b25c2d3e) SHA1(d70f3e4e2432d80c2ac87cd81208ada303bac04a) )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "th2-c1.080",     0x000000, 0x100000, CRC(8fac8c30) SHA1(8e49bb596144761eae95f3e1266e57fb386664f2) )
	ROM_RELOAD(                 0x040000, 0x100000 )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched */
ROM_END

GAME( 1994, thoop2,  0, thoop2, thoop2, driver_device,  0, ROT0, "Gaelco", "TH Strikes Back", MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING )
