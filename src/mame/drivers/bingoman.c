/***************************************************************************

  Bingo Mania.
  HP Automaten.

  Skeleton driver by Angelo Salese & Roberto Fresca.


  TODO:

  - Fix ROM loading structure.
  - Check if the H8's need to be decapped.


****************************************************************************

  Hardware specs....

  Etched on PCB:
  "H8 Tiga Polanz grafik board"
  "KRR-A11-0002-28.11.1993"

  1x TMS34010FNL-50 @ u09
  1x H8/520 (HD6435208CP10) @ u18
  1x H8/534 (HD6475348CP16) @ u33

  1x M48T08-150PC1 Timekeeper RAM.@ u50
  1x ADV476KN50E (CMOS Monolithic 256x18 Color Palette RAM-DAC @ 50 MHz) @ u73
  1x MAX707 (MAXIM uP Supervisory Circuits. Watchdog + Reset > 200ms) @ u21

  1x Xtal 20 MHz @ u26
  1x Xtal 25 MHz @ u61
  1x Xtal 40 MHz @ u04  (near TMS34010)

  1x empty DIP8 socket. (Maybe for a serial EPROM) @ u03 
  1x empty DIP32 socket. Near H8/520. @ u17

  1x 2x8 edge connector.
  1x 2x22 edge connector.
  1x 2x5 pins male connector.
  1x 2x7 pins male connector.
  2x VGA type connector (can't see them well).

  1x Trimmer/pot @ r34

  a shitload of jumpers...

  There are 3 games P03, P07 and P14 (2 roms each), plus main code.
  Also 2 sound ROMs.

  All ROMs have copyright "(c) H.Polanz GmbH"


***************************************************************************/

#include "emu.h"
#include "cpu/h83002/h8.h"
//#include "sound/ay8910.h"


class bingoman_state : public driver_device
{
public:
	bingoman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu")
	{ }

	// devices
	required_device<cpu_device> m_maincpu;

	// screen updates
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual void palette_init();
};

void bingoman_state::video_start()
{

}

UINT32 bingoman_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

static ADDRESS_MAP_START( bingoman_prg_map, AS_PROGRAM, 16, bingoman_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( bingoman_io_map, AS_IO, 8, bingoman_state )
//  ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END

static INPUT_PORTS_START( bingoman )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

#if 0
static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ RGN_FRAC(0,1) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
#endif

static GFXDECODE_START( bingoman )
//	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void bingoman_state::machine_start()
{
}

void bingoman_state::machine_reset()
{
}


void bingoman_state::palette_init()
{
}

static MACHINE_CONFIG_START( bingoman, bingoman_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", H8S2394, XTAL_20MHz) /* TODO: correct CPU type */
	MCFG_CPU_PROGRAM_MAP(bingoman_prg_map)
	MCFG_CPU_IO_MAP(bingoman_io_map)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DRIVER(bingoman_state, screen_update)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)

	MCFG_GFXDECODE(bingoman)

	MCFG_PALETTE_LENGTH(8)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, MAIN_CLOCK/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bingoman )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ps.020.u51",   0x000000, 0x080000, CRC(0f40b10d) SHA1(96a24547a612ba7c2b33c84a0f3afecc9a7cc076) ) // wrong ...

	ROM_REGION( 0x300000, "tms", ROMREGION_ERASE00 )	// banked
	ROM_LOAD( "p03_036.015.u1", 0x000000, 0x080000, CRC(b78b7fca) SHA1(8e4147bb8351db5b17e2bf39bb12ca31cf02f3a6) )	// seems to be for game1
	ROM_LOAD( "p03_tms.030.u2", 0x080000, 0x080000, CRC(94f0076e) SHA1(45d0379ad232ae7c5723c87a5fed9f9cc576aea2) )	// seems to be for game1
	ROM_LOAD( "p07_036.010.u3", 0x100000, 0x080000, CRC(dda80fab) SHA1(4fb06ca94a8a03e5ee91d4cb4a24ac35863a82a1) )	// seems to be for game2
	ROM_LOAD( "p07_tms.025.u4", 0x180000, 0x080000, CRC(6c4a84f8) SHA1(e683753eaf54fdedd1cdc64c4dd4591e3b48dc75) )	// seems to be for game2
	ROM_LOAD( "p14_036.012.u5", 0x200000, 0x080000, CRC(a9f2c609) SHA1(0669aba100a8263b99373d7ac997ec4f1967bb1b) )	// seems to be for game3
	ROM_LOAD( "p14_tms.030.u6", 0x280000, 0x080000, CRC(cdf60d47) SHA1(ef7e107f1713466fb18e940e90e7f46c781d4581) )	// seems to be for game3

	ROM_REGION( 0x100000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd.u10.021.u10", 0x00000, 0x80000, CRC(676f7c4f) SHA1(acdec156cb2d7b880cc1668cce50268bb2b4ec72) )
	ROM_LOAD( "snd.u11.021.u11", 0x80000, 0x80000, CRC(d993f3b6) SHA1(ac9f21135e3b3035a007bc9fdf83d04b535e7a85) )

	ROM_REGION( 0x10000, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "102u0520_gal16v8d.u05",   0x0000, 0x0892, CRC(3f0f57e5) SHA1(65383da38f4ad6c5ecaf84336eabea3a77db1307) )
	ROM_LOAD( "102u1400_gal20v8b.u14",   0x1000, 0x0a92, CRC(7a9d0543) SHA1(bed7359c1e1f418e751956fa37499a0afb441bf5) )
	ROM_LOAD( "102u2920_atf20v8bql.u29", 0x2000, 0x0a92, CRC(3efc98b6) SHA1(791834d63d5f30aa726d42e5ce14ec0f46883e6f) )
	ROM_LOAD( "102u3010_gal20v8b.u30",   0x3000, 0x0a92, CRC(ee5a4e08) SHA1(5ae4e853c76444062a60612c8db179c8704e09de) )
	ROM_LOAD( "102u5400_gal16v8d.u54",   0x4000, 0x0892, CRC(cfd94d14) SHA1(730a02c8741be583e03a1a487b0a0d76a99b6e71) )
	ROM_LOAD( "102u5500_gal16v8d.u55",   0x5000, 0x0892, CRC(cfd94d14) SHA1(730a02c8741be583e03a1a487b0a0d76a99b6e71) )
	ROM_LOAD( "103u0701_atf20v8bq.u7",   0x6000, 0x0a92, CRC(aef4ee3f) SHA1(147423b3ba93e70af8129f31411b489dafbe8db6) )
	ROM_LOAD( "103u0820_gal16v8d.u8",    0x7000, 0x0892, CRC(37c1f1c1) SHA1(c08b590a18a9ef5e06352c0b8429676f02bc5765) )
	ROM_LOAD( "103u0900_atf16v8bql.u9",  0x8000, 0x0892, CRC(63214d15) SHA1(c6f4f68a9bccd954fde13bef94bd28097d6bebfc) )
	ROM_LOAD( "103u1301_gal16v8d.u13",   0x9000, 0x0892, CRC(45bffe9a) SHA1(86a61f23b2da2bc3a4b0f95826638719b925b399) )
ROM_END

GAME( 1993, bingoman,  0,   bingoman,  bingoman, driver_device,  0,       ROT0, "HP Automaten",      "Bingo Mania", GAME_IS_SKELETON )
