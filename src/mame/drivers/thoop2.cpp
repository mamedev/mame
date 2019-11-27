// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, Peter Ferrie, David Haywood
/***************************************************************************

Thunder Hoop II: Strikes Back (c) 1994 Gaelco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

updated by Peter Ferrie <peter.ferrie@gmail.com>

There is a priority bug on the title screen (Gaelco logo is hidden by black
borders)  It seems sprite priority is hacked around on most of the older
Gaelco drivers.


REF.940411
+-------------------------------------------------+
|       C1                                  6116  |
|  VOL  C2*                                 6116  |
|          30MHz                            6116  |
|    M6295                    +----------+  6116  |
|     1MHz                    |TMS       |        |
|       6116                  |TPC1020AFN|        |
|J      6116                  |   -084C  |    H8  |
|A     +------------+         +----------+        |
|M     |DS5002FP Box|         +----------+        |
|M     +------------+         |TMS       |    H12 |
|A             65756          |TPC1020AFN|        |
|              65756          |   -084C  |        |
|                             +----------+        |
|SW1                                   PAL   65764|
|     24MHz    MC68000P12                    65764|
|SW2           C22                    6116        |
|      PAL     C23                    6116        |
+-------------------------------------------------+

  CPU: MC68000P12 & DS5002FP (used for protection)
Sound: OKI M6295
  OSC: 30MHz, 24MHz & 1MHz resonator
  RAM: MHS HM3-65756K-5  32K x 8 SRAM (x2)
       MHS HM3-65764E-5  8K x 8 SRAM (x2)
       UM6116BK-35  2K x 8 SRAM (x8)
  PAL: TI F20L8-25CNT DIP24 (x2)
  VOL: Volume pot
   SW: Two 8 switch dipswitches

DS5002FP Box contains:
  Dallas DS5002SP @ 12MHz
  KM62256BLG-7L - 32Kx8 Low Power CMOS SRAM
  3.6v Battery
  JP1 - 5 pin port to program SRAM

Measurements from actual PCB:
  DS5002FP - 12MHz
  OKI MSM6295 - 1MHz, pin 7 is disconnected (neither pulled LOW or HIGH)
  H-SYNC - 15.151KHz
  V-SYNC - 59.24Hz

***************************************************************************/

#include "emu.h"
#include "includes/thoop2.h"

#include "machine/gaelco_ds5002fp.h"

#include "cpu/m68000/m68000.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"

#include "screen.h"
#include "speaker.h"


void thoop2_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

WRITE8_MEMBER(thoop2_state::oki_bankswitch_w)
{
	m_okibank->set_entry(data & 0x0f);
}

WRITE_LINE_MEMBER(thoop2_state::coin1_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(0, !state);
}

WRITE_LINE_MEMBER(thoop2_state::coin2_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(1, !state);
}

WRITE_LINE_MEMBER(thoop2_state::coin1_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(thoop2_state::coin2_counter_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE8_MEMBER(thoop2_state::shareram_w)
{
	// why isn't there address map functionality for this?
	reinterpret_cast<u8 *>(m_shareram.target())[BYTE_XOR_BE(offset)] = data;
}

READ8_MEMBER(thoop2_state::shareram_r)
{
	// why isn't there address map functionality for this?
	return reinterpret_cast<u8 const *>(m_shareram.target())[BYTE_XOR_BE(offset)];
}


void thoop2_state::mcu_hostmem_map(address_map &map)
{
	map(0x8000, 0xffff).rw(FUNC(thoop2_state::shareram_r), FUNC(thoop2_state::shareram_w)); // confirmed that 0x8000 - 0xffff is a window into 68k shared RAM
}


void thoop2_state::thoop2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                 /* ROM */
	map(0x100000, 0x101fff).ram().w(FUNC(thoop2_state::vram_w)).share("videoram");   /* Video RAM */
	map(0x108000, 0x108007).writeonly().share("vregs");                 /* Video Registers */
	map(0x10800c, 0x10800d).w("watchdog", FUNC(watchdog_timer_device::reset16_w));                           /* INT 6 ACK/Watchdog timer */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");/* Palette */
	map(0x440000, 0x440fff).ram().share("spriteram");                       /* Sprite RAM */
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x700008, 0x700009).portr("SYSTEM");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(thoop2_state::oki_bankswitch_w));               /* OKI6295 bankswitch */
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));                  /* OKI6295 data register */
	map(0xfe0000, 0xfe7fff).ram();                                          /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                     /* Work RAM (shared with D5002FP) */
}


void thoop2_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank");
}


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


static const gfx_layout thoop2_tilelayout =
{
	8,8,                                    /* 8x8 tiles */
	RGN_FRAC(1,2),                            /* number of tiles */
	4,                                      /* 4 bpp */
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*2) },
	16*8
};

static const gfx_layout thoop2_tilelayout_16 =
{
	16,16,                                  /* 16x16 tiles */
	RGN_FRAC(1,2),                            /* number of tiles */
	4,                                      /* 4 bpp */
	{ 8, 0, RGN_FRAC(1,2)+8, RGN_FRAC(1,2)+0 },
	{ STEP8(0,1), STEP8(8*2*16,1) },
	{ STEP16(0,8*2) },
	64*8
};


static GFXDECODE_START( gfx_thoop2 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, thoop2_tilelayout,    0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, thoop2_tilelayout_16, 0, 64 )
GFXDECODE_END


void thoop2_state::thoop2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2); // 12MHz verified
	m_maincpu->set_addrmap(AS_PROGRAM, &thoop2_state::thoop2_map);
	m_maincpu->set_vblank_int("screen", FUNC(thoop2_state::irq6_line_hold));

	gaelco_ds5002fp_device &ds5002fp(GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2)); // 12MHz verified
	ds5002fp.set_addrmap(0, &thoop2_state::mcu_hostmem_map);

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(thoop2_state::coin1_lockout_w));
	m_outlatch->q_out_cb<1>().set(FUNC(thoop2_state::coin2_lockout_w));
	m_outlatch->q_out_cb<2>().set(FUNC(thoop2_state::coin1_counter_w));
	m_outlatch->q_out_cb<3>().set(FUNC(thoop2_state::coin2_counter_w));
	m_outlatch->q_out_cb<4>().set_nop(); // unknown. Sound related?
	m_outlatch->q_out_cb<5>().set_nop(); // unknown

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.24);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(thoop2_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_thoop2);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // 1MHz resonator - pin 7 not connected
	oki.set_addrmap(0, &thoop2_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}



ROM_START( thoop2 ) /* REF.940411 PCB */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "th2c23.c23",   0x000000, 0x080000, CRC(3e465753) SHA1(1ea1173b9fe5d652e7b5fafb822e2535cecbc198) )
	ROM_LOAD16_BYTE(    "th2c22.c22",   0x000001, 0x080000, CRC(837205b7) SHA1(f78b90c2be0b4dddaba26f074ea00eff863cfdb2) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "thoop2_ds5002fp.bin", 0x00000, 0x8000, CRC(6881384d) SHA1(c1eff5558716293e1325b766e2205783286c12f9) ) /* dumped from 3 boards, reconstructed with 2/3 wins rule, all bytes verified by hand as correct */

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "th2-h8.h8",     0x000000, 0x400000, CRC(60328a11) SHA1(fcdb374d2fc7ef5351a4181c471d192199dc2081) )
	ROM_LOAD( "th2-h12.h12",   0x400000, 0x400000, CRC(b25c2d3e) SHA1(d70f3e4e2432d80c2ac87cd81208ada303bac04a) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "th2-c1.c1",     0x000000, 0x100000, CRC(8fac8c30) SHA1(8e49bb596144761eae95f3e1266e57fb386664f2) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched */
ROM_END

ROM_START( thoop2a ) /* REF.940411 PCB */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "3.c23",   0x000000, 0x080000, CRC(6cd4a8dc) SHA1(7d0cdce64b390c3f9769b07d57cf1eee1e6a7bf5) )
	ROM_LOAD16_BYTE(    "2.c22",   0x000001, 0x080000, CRC(59ba9b43) SHA1(6c6690a2e389fc9f1e166c87748da1175e3b58f8) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "thoop2_ds5002fp.bin", 0x00000, 0x8000, CRC(6881384d) SHA1(c1eff5558716293e1325b766e2205783286c12f9) ) /* dumped from 3 boards, reconstructed with 2/3 wins rule, all bytes verified by hand as correct */

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x800000, "gfx1", 0 )
	ROM_LOAD( "th2-h8.h8",     0x000000, 0x400000, CRC(60328a11) SHA1(fcdb374d2fc7ef5351a4181c471d192199dc2081) )
	ROM_LOAD( "th2-h12.h12",   0x400000, 0x400000, CRC(b25c2d3e) SHA1(d70f3e4e2432d80c2ac87cd81208ada303bac04a) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "th2-c1.c1",     0x000000, 0x100000, CRC(8fac8c30) SHA1(8e49bb596144761eae95f3e1266e57fb386664f2) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched */
ROM_END

GAME( 1994, thoop2,       0, thoop2, thoop2, thoop2_state, empty_init, ROT0, "Gaelco", "TH Strikes Back (Non North America, Version 1.0, Checksum 020E0867)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, thoop2a, thoop2, thoop2, thoop2, thoop2_state, empty_init, ROT0, "Gaelco", "TH Strikes Back (Non North America, Version 1.0, Checksum 020EB356)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
