// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
/***************************************************************************

  Bingo Mania.
  HP Automaten.

  Multi game system.
  Can handle up to 3 different games
  which could be selected on startup.

  Skeleton driver by Angelo Salese & Roberto Fresca.


  TODO:

  - Fix ROM loading structure.
  - Check if the H8's need to be decapped.


****************************************************************************

  Hardware specs...

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

  a large amount of jumpers...

  There are 3 games P03, P07 and P14 (2 roms each), plus main code.
  Also 2 sound ROMs.

  All ROMs have copyright "(c) H.Polanz GmbH"


****************************************************************************

  PCB Layouts...


  MAIN BOARD:

       +-------------+      +---------------------------------------+        +-----+
       | | | | | | | |      | | | | | | | | | | | | | | | | | | | | |        |     |
  +----+     2x8     +------+                2x22                   +-----+--+-----+--+-+
  |                                                                       |VGA MONITOR| |
  |                                           +-------+ +-------+         +-+CONNECT+-+ |
  |            +---------+  +---------+       |411GR-1| |411GR-1|           |       |   |
  |            |ULN2803A |  |ULN2803A |       +-------+ +-------+           |       |   |
  |            +---------+  +---------+                                     +-------+   |
  |                                                                                     |
  |                                                                          +------+   |
  |           +-------+                                                      |      |   |
  |           |74HCT14|                                                      |ADV476|   |
  |           +-------+                                                      |KN50E |   |
  |                                                                          |      |   |
  | +-------+ +------------+ +-------+         +-------+ +-------+           |02-32 |   |
  | |411GR-1| | 74HCT541N  | |74HCT14|         |411GR-1| |411GR-1|           |0S    |   |
  | +-------+ +------------+ +-------+         +-------+ +-------+           |      |   |
  |                                                                          |      |   |
  | +-------+ +------------+ +-------+    +-------+ +-------+                |      |   |
  | |74HCT14| | 74HCT541N  | |74HCT14|    |74HCT14| |74HCT14|    +-------+   |      |   |
  | +-------+ +------------+ +-------+    +-------+ +-------+    |74AS00N|   +------+   |
  |                                                              +-------+              |
  |                                        +------+                         +--------+  |
  | +----------------+  +----------------+ |H99470| +------+     +-------+  |  XTAL  |  |
  | |  EMPTY SOCKET  |  |  EMPTY SOCKET  | |045   | |74AS74|     |74AS32N|  |25.00Mhz|  |
  | | CONNECTION FOR |  | CONNECTION FOR | |      | +------+     +-------+  +--------+  |
  | |   TOP-BOARD    |  |   TOP-BOARD    | |M48T08|                                     |
  | +----------------+  +----------------+ |-150PC| +------+     +------+     +------+  |
  | +----------------+  +----------------+ |      | |74AS74|     |74AS74|     |74AS04|  |
  | |SAMSUNG         |  |PS.020          | | TIME | +------+     +------+     +------+  |
  | |     K6T1008C2E |  |                | |KEEPER|                                     |
  | |                |  |         27C4001| | RAM  | +---------+  +---------+  +-------+ |
  | +----------------+  +----------------+ |      | |102U5500 |  |102U5400 |  |74AS161| |
  |                                        +------+ +---------+  +---------+  +-------+ |
  |    +-------+    +-------+    +-------+                                              |
  |    |411GR-1|    |411GR-1|    |411GR-1|                                              |
  |    +-------+    +-------+    +-------+           +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  |                                                                                     |
  | +----------+ +----------+ +----------+           +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  | |74HCT541N | |74HCT541N | |74HCT541N |                                              |
  | +----------+ +----------+ +----------+           +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  |                                                                                     |
  |    +-------+                   +--+  +--+        +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  |    |411GR-1| +-------------+   |10|  |10|                                           |
  |    +-------+ |             |   |2U|  |2U|        +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  |              |   H8/534    |   |30|  |29|                                           |
  | +----------+ |  HD6475348  |   |10|  |20|        +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  | |74HCT541N | |       CP16  |   |  |  |  |                                           |
  | +----------+ |             |   |  |  |  |        +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  |              |             |   |  |  |  |                                           |
  | +----+ +--+  |             |   +--+  +--+        +KM424C2572-6+  +MB81C4256A-70PSZ+ |
  | |XTAL| |74|  |             |                                                        |
  | |    | |HC|  +-------------+     +------+       +-------+  +---------+  +---------+ |
  | |20.0| |T0|                      |  A0  |       |411GR-1|  |74ALS245A|  |74ALS245A| |
  | |0Mhz| |4N|                      +------+       +-------+  +---------+  +---------+ |
  | +----+ +--+                                                                         |
  |                                                                                     |
  |                                                                                     |
  | +---+  +------------+                           +---------+   +-------+ +---------+ |
  | | A5|  |            | +----------------+        |74AS573N |   |411GR-1| |74AS573N | |
  | +---+  |   H8/520   | |  EMPTY SOCKET  |        +---------+   +-------+ +---------+ |
  |        | HD6435208  | | CONNECTION FOR |                                            |
  |        |      CP10  | |   TOP-BOARD    |        +------+ +---------+ +------------+ |
  |        |            | +----------------+        |  A1  | |74AS244N | |  102U1400  | |
  |        |            |                           +------+ +---------+ +------------+ |
  |        |            |                                                               |
  |        +------------+ +------+   +---------+   +---------+  +------+ +------------+ |
  |                       |  A2  |   |74HCT573N|   |74HCT245N|  |74AS08| |            | |
  |                       +------+   +---------+   +---------+  +------+ |  TMS34010  | |
  |                                                                      |    FNL-50  | |
  |                                                                      |            | |
  |  +------+             +---------+   +------+   +---------+  +------+ |   WEU9310  | |
  |  |  A4  |             |74LS682N |   |  A3  |   |102U0520 |  | XTAL | |            | |
  |  +------+             +---------+   +------+   +---------+  |40.000| |            | |
  |                                                             +------+ +------------+ |
  |                   +--+                                        +--+                  |
  |                   |  |                                        |  |                  |
  |                   |TL|                                        |A6|                  |
  |                   |07|                                        |  |      +-------+   |
  |                   |4C|                                        |  |      |       |   |
  |                   |N |                +---------+ +------+    |  |      |       |   |
  |                   +--+                |:::::::::| |::::::|    +--+    +-+       +-+ |
  |                                       +---------+ +------+            |SERIAL PORT| |
  +-----------------------------------------------------------------------+--+-----+--+-+
                                                                             |     |
  A0 = 74HCT174N                                                             +-----+
  A1 = 74HCT32N
  A2 = 74HCT393N
  A3 = 74HCT05N
  A4 = W990K9948
  A5 = Maxim MAX707
  A6 = HIN232CP / H0044CDSR


  TOP-BOARD:

  +-------------------------------------------+
  |                                           |
  | +----------------+                        |
  | |P03 036.015     |                        |
  | |                |                        |
  | |         27C4001|                        |
  | +----------------+                        |
  |                                           |
  | +----------------+ +----------------+     |
  | |P03 TMS.030     | |P14 036.012     |     |
  | |                | |                |     |
  | |         27C4001| |         27C4001|     |
  | +----------------+ +----------------+     |
  |                                           |
  | +----------------+ +----------------+     |
  | |P07 036.010     | |P14 TMS.030     |     |
  | |                | |                |     |
  | |         27C4001| |         27C4001|     |
  | +----------------+ +----------------+     |
  |                                           |
  | +----------------+     +------------+     |
  | |P07 TMS.025     |     |  103U0701  |     |
  | |                |     +------------+     |
  | |         27C4001|                        |
  | +----------------+       +----------+     |
  |                          | 103U0900 |     |
  |                          +----------+     |
  |       +----------+                        |
  |       | 103U0820 |       +----------+     |
  |       +----------+       | 103U1301 |     |
  |                          +----------+     |
  | +----------------+                        |
  | |SND.U10.021     |                        |
  | |                |                        |
  | |         27C4001|                        |
  | +----------------+                        |
  |                                           |
  | +----------------+                        |
  | |SND.U11.021     |                        |
  | |                |                        |
  | |         27C4001|   +---------------+    |
  | +----------------+   |               |    |
  |                      | EMPTY SOCKET  |    |
  |                      |               |    |
  |                      +---------------+    |
  +-------------------------------------------+


  102U0520 = GAL16V8D
  102U1400 = GAL20V8B
  102U2920 = ATF20V8BQL
  102U3010 = GAL20V8B
  102U5400 = GAL16V8D
  102U5500 = GAL16V8D

  103U0701 = ATF20V8BQL
  103U0820 = GAL16V8D
  103U0900 = ATF16V8BQL
  103U1301 = GAL16V8D


****************************************************************************

  Games:

      CODE    |    DATE    |   TITLE
  ------------+------------+-----------------------------
  A03.037.013 | 10-08-2002 | Gold Jackpot
  ------------+------------+-----------------------------
  P03.036.015 | 10-08-2002 | Gold Jackpot
  ------------+------------+-----------------------------
  P07.036.010 | 21-09-1996 | Turbo Game
  ------------+------------+-----------------------------
  P14.036.012 | 18-11-1997 | Rolling Bingo - Joker Bonus
  ------------+------------+-----------------------------


***************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8520.h"
#include "cpu/h8500/h8534.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bingoman_state : public driver_device
{
public:
	bingoman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void bingoman(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<h8520_device> m_maincpu;

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void bingoman_palette(palette_device &palette) const;
	void bingoman_prg_map(address_map &map) ATTR_COLD;
};

void bingoman_state::video_start()
{
}

uint32_t bingoman_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}

void bingoman_state::bingoman_prg_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
}

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

static GFXDECODE_START( gfx_bingoman )
//  GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
GFXDECODE_END


void bingoman_state::machine_start()
{
}

void bingoman_state::machine_reset()
{
}


void bingoman_state::bingoman_palette(palette_device &palette) const
{
}

void bingoman_state::bingoman(machine_config &config)
{
	/* basic machine hardware */
	HD6435208(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &bingoman_state::bingoman_prg_map);

	HD6475348(config, "subcpu", 20_MHz_XTAL).set_disable();

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(bingoman_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea_full();
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_bingoman);
	PALETTE(config, "palette", FUNC(bingoman_state::bingoman_palette), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}


/***************************************************************************

   ROM Load

***************************************************************************/

ROM_START( bingoman )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ps.020.u51",   0x000000, 0x080000, CRC(0f40b10d) SHA1(96a24547a612ba7c2b33c84a0f3afecc9a7cc076) ) // wrong ...

	ROM_REGION( 0x8000, "subcpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd6475348cp16.u33", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x300000, "tms", ROMREGION_ERASE00 )    // banked
	ROM_LOAD( "p03_036.015.u01", 0x000000, 0x080000, CRC(b78b7fca) SHA1(8e4147bb8351db5b17e2bf39bb12ca31cf02f3a6) ) // Game 1 (Gold Jackpot)
	ROM_LOAD( "p03_tms.030.u02", 0x080000, 0x080000, CRC(94f0076e) SHA1(45d0379ad232ae7c5723c87a5fed9f9cc576aea2) ) // Game 1 (Gold Jackpot)
	ROM_LOAD( "p07_036.010.u03", 0x100000, 0x080000, CRC(dda80fab) SHA1(4fb06ca94a8a03e5ee91d4cb4a24ac35863a82a1) ) // Game 2 (Turbo Game)
	ROM_LOAD( "p07_tms.025.u04", 0x180000, 0x080000, CRC(6c4a84f8) SHA1(e683753eaf54fdedd1cdc64c4dd4591e3b48dc75) ) // Game 2 (Turbo Game)
	ROM_LOAD( "p14_036.012.u05", 0x200000, 0x080000, CRC(a9f2c609) SHA1(0669aba100a8263b99373d7ac997ec4f1967bb1b) ) // Game 3 (Rolling Bingo - Joker Bonus)
	ROM_LOAD( "p14_tms.030.u06", 0x280000, 0x080000, CRC(cdf60d47) SHA1(ef7e107f1713466fb18e940e90e7f46c781d4581) ) // Game 3 (Rolling Bingo - Joker Bonus)

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
	ROM_LOAD( "103u0701_atf20v8bq.u07",  0x6000, 0x0a92, CRC(aef4ee3f) SHA1(147423b3ba93e70af8129f31411b489dafbe8db6) )
	ROM_LOAD( "103u0820_gal16v8d.u08",   0x7000, 0x0892, CRC(37c1f1c1) SHA1(c08b590a18a9ef5e06352c0b8429676f02bc5765) )
	ROM_LOAD( "103u0900_atf16v8bql.u09", 0x8000, 0x0892, CRC(63214d15) SHA1(c6f4f68a9bccd954fde13bef94bd28097d6bebfc) )
	ROM_LOAD( "103u1301_gal16v8d.u13",   0x9000, 0x0892, CRC(45bffe9a) SHA1(86a61f23b2da2bc3a4b0f95826638719b925b399) )
ROM_END

ROM_START( bingomana )
	ROM_REGION( 0x80000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD16_WORD_SWAP( "ps.020.u51",   0x000000, 0x080000, CRC(0f40b10d) SHA1(96a24547a612ba7c2b33c84a0f3afecc9a7cc076) ) // wrong ...

	ROM_REGION( 0x8000, "subcpu", ROMREGION_ERASE00 )
	ROM_LOAD( "hd6475348cp16.u33", 0x0000, 0x8000, NO_DUMP )

	ROM_REGION( 0x300000, "tms", ROMREGION_ERASE00 )    // banked
	ROM_LOAD( "a03_037.013.u01", 0x000000, 0x080000, CRC(9c3ed8e9) SHA1(263431ed6db314bee64709bae16fa8c6d5adbd41) ) // Game 1 (Gold Jackpot)
	ROM_LOAD( "a03_tms.010.u02", 0x080000, 0x080000, CRC(f4142b1a) SHA1(1a14865bd567d5e7bf9e0e0765f6443c8165f46b) ) // Game 1 (Gold Jackpot)

	ROM_REGION( 0x100000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd_u10.022.u10", 0x00000, 0x80000, CRC(216a9c4a) SHA1(f1eb5f6517d6b579bf977b8eb4dd3f3544b75796) )   // different revision
	ROM_LOAD( "snd_u11.022.u11", 0x80000, 0x80000, CRC(c0bb0056) SHA1(c8bc3a618eb9be940e089ec00d88c0b92d42025c) )   // different revision

	ROM_REGION( 0x10000, "plds", ROMREGION_ERASE00 )
	ROM_LOAD( "102u0530_gal16v8d.u05",   0x0000, 0x0892, CRC(3f0f57e5) SHA1(65383da38f4ad6c5ecaf84336eabea3a77db1307) )
	ROM_LOAD( "102u1400_palce20v8h.u14", 0x1000, 0x0a92, CRC(c0a0d80d) SHA1(75c3cf2a7a454d40770e31ad1d9096a1b977f786) )
	ROM_LOAD( "102u2920_gal20v8b.u29",   0x2000, 0x0a92, CRC(a0ec643e) SHA1(280ad44a4d484326b015620a8a6307bbaf030c21) )
	ROM_LOAD( "102u3010_gal20v8b.u30",   0x3000, 0x0a92, CRC(ee5a4e08) SHA1(5ae4e853c76444062a60612c8db179c8704e09de) )
	ROM_LOAD( "102u5400_atf16v8b.u54",   0x4000, 0x0892, CRC(aff40999) SHA1(cf12b31d3a28b6380c65c5fd0c1db43d7378f6b2) )
	ROM_LOAD( "102u5500_atf16v8b.u55",   0x5000, 0x0892, CRC(aff40999) SHA1(cf12b31d3a28b6380c65c5fd0c1db43d7378f6b2) )
	ROM_LOAD( "103u0790_gal20v8b.u07",   0x6000, 0x0a92, CRC(1ed19f8d) SHA1(f2bcbe06dc575f2b894208326b9dfae2b6cac2be) )
	ROM_LOAD( "103u0820_atf16v8bql.u08", 0x7000, 0x0892, CRC(908c2e36) SHA1(7d8be56cbcef7eeb2d1e40f528789dab58e86c95) )
	ROM_LOAD( "103u0900_atf16v8bql.u09", 0x8000, 0x0892, CRC(63214d15) SHA1(c6f4f68a9bccd954fde13bef94bd28097d6bebfc) )
	ROM_LOAD( "103u1301_atf16v8bql.u13", 0x9000, 0x0892, CRC(e8b65072) SHA1(b2f7d70ee228c28a19c2594fc3631b8afd4a0f5e) )
ROM_END

} // anonymous namespace


/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME       PARENT    MACHINE    INPUT     STATE           INIT        ROT    COMPANY          FULLNAME                    FLAGS   */
GAME( 1993, bingoman,  0,        bingoman,  bingoman, bingoman_state, empty_init, ROT0, "HP Automaten",  "Bingo Mania (P03-P07-P14)", MACHINE_IS_SKELETON )
GAME( 1993, bingomana, bingoman, bingoman,  bingoman, bingoman_state, empty_init, ROT0, "HP Automaten",  "Bingo Mania (A03)",         MACHINE_IS_SKELETON )
