/***************************************************************************

Exzisus
-------------------------------------
driver by Yochizo

This driver is heavily dependent on the Raine source.
Very thanks to Richard Bush and the Raine team.


Supported games :
==================
 Exzisus        (C) 1987 Taito


System specs :
===============
   CPU       : Z80(4 MHz) x 4
   Sound     : YM2151 x 1
   Chips     : TC0010VCU + TC0140SYT


TODO:
- There must be a way for cpu a to stop cpu c, otherwise the RAM check in test
  mode cannot work. However, the only way I found to do that is making writes
  to F404 pulse the reset line, which isn't a common way to handle these things.

****************************************************************************/

#include "driver.h"
#include "cpu/z80/z80.h"
#include "taitoipt.h"
#include "audio/taitosnd.h"
#include "sound/2151intf.h"


/***************************************************************************

  Variables

***************************************************************************/

static UINT8 *exzisus_sharedram_ab;
static UINT8 *exzisus_sharedram_ac;

extern UINT8 *exzisus_videoram0;
extern UINT8 *exzisus_videoram1;
extern UINT8 *exzisus_objectram0;
extern UINT8 *exzisus_objectram1;
extern size_t  exzisus_objectram_size0;
extern size_t  exzisus_objectram_size1;

READ8_HANDLER ( exzisus_videoram_0_r );
READ8_HANDLER ( exzisus_videoram_1_r );
READ8_HANDLER ( exzisus_objectram_0_r );
READ8_HANDLER ( exzisus_objectram_1_r );
WRITE8_HANDLER( exzisus_videoram_0_w );
WRITE8_HANDLER( exzisus_videoram_1_w );
WRITE8_HANDLER( exzisus_objectram_0_w );
WRITE8_HANDLER( exzisus_objectram_1_w );

VIDEO_UPDATE( exzisus );


/***************************************************************************

  Memory Handler(s)

***************************************************************************/

static WRITE8_HANDLER( exzisus_cpua_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "maincpu");
	static int exzisus_cpua_bank = 0;

	if ( (data & 0x0f) != exzisus_cpua_bank )
	{
		exzisus_cpua_bank = data & 0x0f;
		if (exzisus_cpua_bank >= 2)
		{
			memory_set_bankptr(space->machine,  2, &RAM[ 0x10000 + ( (exzisus_cpua_bank - 2) * 0x4000 ) ] );
		}
	}

	flip_screen_set(space->machine, data & 0x40);
}

static WRITE8_HANDLER( exzisus_cpub_bankswitch_w )
{
	UINT8 *RAM = memory_region(space->machine, "cpub");
	static int exzisus_cpub_bank = 0;

	if ( (data & 0x0f) != exzisus_cpub_bank )
	{
		exzisus_cpub_bank = data & 0x0f;
		if (exzisus_cpub_bank >= 2)
		{
			memory_set_bankptr(space->machine,  1, &RAM[ 0x10000 + ( (exzisus_cpub_bank - 2) * 0x4000 ) ] );
		}
	}

	flip_screen_set(space->machine, data & 0x40);
}

static WRITE8_HANDLER( exzisus_coincounter_w )
{
	coin_lockout_w(0,~data & 0x01);
	coin_lockout_w(1,~data & 0x02);
	coin_counter_w(0,data & 0x04);
	coin_counter_w(1,data & 0x08);
}

static READ8_HANDLER( exzisus_sharedram_ab_r )
{
	return exzisus_sharedram_ab[offset];
}

static READ8_HANDLER( exzisus_sharedram_ac_r )
{
	return exzisus_sharedram_ac[offset];
}

static WRITE8_HANDLER( exzisus_sharedram_ab_w )
{
	exzisus_sharedram_ab[offset] = data;
}

static WRITE8_HANDLER( exzisus_sharedram_ac_w )
{
	exzisus_sharedram_ac[offset] = data;
}

// is it ok that cpub_reset refers to cpuc?
static WRITE8_HANDLER( exzisus_cpub_reset_w )
{
	cputag_set_input_line(space->machine, "cpuc", INPUT_LINE_RESET, PULSE_LINE);
}

#if 0
// without exzisus_cpub_reset_w, the following patch would be needed for
// the RAM check to work
static DRIVER_INIT( exzisus )
{
	UINT8 *RAM = memory_region(machine, "maincpu");

	/* Fix WORK RAM error */
	RAM[0x67fd] = 0x18;

	/* Fix ROM 1 error */
	RAM[0x6829] = 0x18;
}
#endif


/**************************************************************************

  Memory Map(s)

**************************************************************************/

static ADDRESS_MAP_START( cpua_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(2)
	AM_RANGE(0xc000, 0xc5ff) AM_READWRITE(exzisus_objectram_1_r, exzisus_objectram_1_w) AM_BASE(&exzisus_objectram1) AM_SIZE(&exzisus_objectram_size1)
	AM_RANGE(0xc600, 0xdfff) AM_READWRITE(exzisus_videoram_1_r, exzisus_videoram_1_w) AM_BASE(&exzisus_videoram1)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(exzisus_sharedram_ac_r, exzisus_sharedram_ac_w) AM_BASE(&exzisus_sharedram_ac)
	AM_RANGE(0xf400, 0xf400) AM_WRITE(exzisus_cpua_bankswitch_w)
	AM_RANGE(0xf404, 0xf404) AM_WRITE(exzisus_cpub_reset_w) // ??
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(exzisus_sharedram_ab_r, exzisus_sharedram_ab_w) AM_BASE(&exzisus_sharedram_ab)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpub_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(1)
	AM_RANGE(0xc000, 0xc5ff) AM_READWRITE(exzisus_objectram_0_r, exzisus_objectram_0_w) AM_BASE(&exzisus_objectram0) AM_SIZE(&exzisus_objectram_size0)
	AM_RANGE(0xc600, 0xdfff) AM_READWRITE(exzisus_videoram_0_r, exzisus_videoram_0_w) AM_BASE(&exzisus_videoram0)
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xf000) AM_READNOP AM_WRITE(taitosound_port_w)
	AM_RANGE(0xf001, 0xf001) AM_READWRITE(taitosound_comm_r, taitosound_comm_w)
	AM_RANGE(0xf400, 0xf400) AM_READ_PORT("P1")
	AM_RANGE(0xf400, 0xf400) AM_WRITE(exzisus_cpub_bankswitch_w)
	AM_RANGE(0xf401, 0xf401) AM_READ_PORT("P2")
	AM_RANGE(0xf402, 0xf402) AM_READ_PORT("SYSTEM")
	AM_RANGE(0xf402, 0xf402) AM_WRITE(exzisus_coincounter_w)
	AM_RANGE(0xf404, 0xf404) AM_READ_PORT("DSWA")
	AM_RANGE(0xf404, 0xf404) AM_WRITENOP // ??
	AM_RANGE(0xf405, 0xf405) AM_READ_PORT("DSWB")
	AM_RANGE(0xf800, 0xffff) AM_READWRITE(exzisus_sharedram_ab_r, exzisus_sharedram_ab_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpuc_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x85ff) AM_READWRITE(exzisus_objectram_1_r, exzisus_objectram_1_w)
	AM_RANGE(0x8600, 0x9fff) AM_READWRITE(exzisus_videoram_1_r, exzisus_videoram_1_w)
	AM_RANGE(0xa000, 0xafff) AM_READWRITE(exzisus_sharedram_ac_r, exzisus_sharedram_ac_w)
	AM_RANGE(0xb000, 0xbfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x8fff) AM_RAM
	AM_RANGE(0x9000, 0x9001) AM_DEVREADWRITE("ym", ym2151_r, ym2151_w)
	AM_RANGE(0xa000, 0xa000) AM_READNOP AM_WRITE(taitosound_slave_port_w)
	AM_RANGE(0xa001, 0xa001) AM_READWRITE(taitosound_slave_comm_r, taitosound_slave_comm_w)
ADDRESS_MAP_END


/***************************************************************************

  Input Port(s)

***************************************************************************/

static INPUT_PORTS_START( exzisus )
	PORT_START("P1")
	TAITO_JOY_UDRL_2_BUTTONS( 1 )

	PORT_START("P2")
	TAITO_JOY_UDRL_2_BUTTONS( 2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW,  IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW,  IPT_UNKNOWN )

	PORT_START("DSWA")
	TAITO_MACHINE_COCKTAIL
	TAITO_COINAGE_JAPAN_OLD

	PORT_START("DSWB")
	TAITO_DIFFICULTY
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x08, "100k and every 150k" )
	PORT_DIPSETTING(    0x0c, "150k and every 200k" )
	PORT_DIPSETTING(    0x04, "150k" )
	PORT_DIPSETTING(    0x00, "200k" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Service Mode (buggy)" )      // buggy: all other switches in DSW2 must be on
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

  Machine Driver(s)

***************************************************************************/

static const gfx_layout charlayout =
{
	8, 8,
	8*2048,
	4,
	{ 0x40000*8, 0x40000*8+4, 0, 4 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static GFXDECODE_START( exzisus )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 256 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout, 256, 256 )
GFXDECODE_END



static void irqhandler(const device_config *device, int irq)
{
	cputag_set_input_line(device->machine, "audiocpu", 0, irq ? ASSERT_LINE : CLEAR_LINE);
}

static const ym2151_interface ym2151_config =
{
	irqhandler
};



static MACHINE_DRIVER_START( exzisus )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", Z80, 6000000)			/* 6 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(cpua_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 4000000)			/* 4 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(sound_map)

	MDRV_CPU_ADD("cpub", Z80, 6000000)			/* 6 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(cpub_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_CPU_ADD("cpuc", Z80, 6000000)			/* 6 MHz ??? */
	MDRV_CPU_PROGRAM_MAP(cpuc_map)
	MDRV_CPU_VBLANK_INT("screen", irq0_line_hold)

	MDRV_QUANTUM_TIME(HZ(600))	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */

	/* video hardware */
	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(32*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(exzisus)
	MDRV_PALETTE_LENGTH(1024)

	MDRV_PALETTE_INIT(RRRR_GGGG_BBBB)
	MDRV_VIDEO_UPDATE(exzisus)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ym", YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_config)
	MDRV_SOUND_ROUTE(0, "mono", 0.50)
	MDRV_SOUND_ROUTE(1, "mono", 0.50)
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exzisus )
	ROM_REGION( 0x48000, "maincpu", 0 ) 					/* Z80 CPU A */
	ROM_LOAD( "b23-10.bin", 0x00000, 0x08000, CRC(c80216fc) SHA1(7b952779c420be08573768f09bd65d0a188df024) )
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "b23-12.bin", 0x18000, 0x10000, CRC(13637f54) SHA1(c175bc60120e32eec6ccca822fa497a42dd59823) )

	ROM_REGION( 0x10000, "audiocpu", 0 )     				/* Z80 for Sound */
	ROM_LOAD( "b23-14.bin",  0x00000, 0x08000, CRC(f7ca7df2) SHA1(6048d9341f0303546e447a76439e1927d14cdd57) )

	ROM_REGION( 0x48000, "cpub", 0 )     				/* Z80 CPU B */
	ROM_LOAD( "b23-11.bin", 0x00000, 0x08000, CRC(d6a79cef) SHA1(e2b56aa38c017b24b50f304b9fe49ee14006f9a4) )
	ROM_CONTINUE(           0x10000, 0x08000 )
	ROM_LOAD( "b12-12.bin", 0x18000, 0x10000, CRC(a662be67) SHA1(0643480d56d8ac020288db800a705dd5d0d3ad9f) )
	ROM_LOAD( "b12-13.bin", 0x28000, 0x10000, CRC(04a29633) SHA1(39476365241718f01f9630c12467cb24791a67e1) )

	ROM_REGION( 0x10000, "cpuc", 0 )     				/* Z80 CPU C */
	ROM_LOAD( "b23-13.bin",  0x00000, 0x08000, CRC(51110aa1) SHA1(34c2701625eb1987affad1efd19ff8c9971456ae) )

	ROM_REGION( 0x80000, "gfx1", ROMREGION_INVERT )	/* BG 0 */
	ROM_LOAD( "b12-16.bin",  0x00000, 0x10000, CRC(6fec6acb) SHA1(2289c116d3f6093988a088d011f192dd4a99aa77) )
	ROM_LOAD( "b12-18.bin",  0x10000, 0x10000, CRC(64e358aa) SHA1(cd1a23458b1a2f9c8c8aea8086dc04e0f6cc6908) )
	ROM_LOAD( "b12-20.bin",  0x20000, 0x10000, CRC(87f52e89) SHA1(3f8530aca087fa2a32dc6dfbcfe2f86604ee3ca1) )
	ROM_LOAD( "b12-15.bin",  0x40000, 0x10000, CRC(d81107c8) SHA1(c024c9b7956de493687e1373318d4cd74b3555b2) )
	ROM_LOAD( "b12-17.bin",  0x50000, 0x10000, CRC(db1d5a6c) SHA1(c2e1b8d92c2b3b2ce775ed50ca4a37e84ed35a93) )
	ROM_LOAD( "b12-19.bin",  0x60000, 0x10000, CRC(772b2641) SHA1(35cc6d5a725f1817791e710afde992e64d14104f) )

	ROM_REGION( 0x80000, "gfx2", ROMREGION_INVERT )	/* BG 1 */
	ROM_LOAD( "b23-06.bin",  0x00000, 0x10000, CRC(44f8f661) SHA1(d77160a89e45556cd9ce211d89c398e1086d8d92) )
	ROM_LOAD( "b23-08.bin",  0x10000, 0x10000, CRC(1ce498c1) SHA1(a9ce3de997089bd40c99bd89919b459c9f215fc8) )
	ROM_LOAD( "b23-07.bin",  0x40000, 0x10000, CRC(d7f6ec89) SHA1(e8da207ddaf46ceff870b45ecec0e89c499291b4) )
	ROM_LOAD( "b23-09.bin",  0x50000, 0x10000, CRC(6651617f) SHA1(6351a0b01589cb181b896285ade70e9dfcd799ec) )

	ROM_REGION( 0x00c00, "proms", 0 )					/* PROMS */
	ROM_LOAD( "b23-04.bin",  0x00000, 0x00400, CRC(5042cffa) SHA1(c969748866a12681cf2dbf25a46da2c4e4f92313) )
	ROM_LOAD( "b23-03.bin",  0x00400, 0x00400, CRC(9458fd45) SHA1(7f7cdacf37bb6f15de1109fa73ba3c5fc88893d0) )
	ROM_LOAD( "b23-05.bin",  0x00800, 0x00400, CRC(87f0f69a) SHA1(37df6fd56245fab9beaabfd86fd8f95d7c42c2a5) )
ROM_END


GAME( 1987,  exzisus,      0, exzisus, exzisus, 0,    ROT0, "Taito Corporation", "Exzisus (Japan)", 0 )
