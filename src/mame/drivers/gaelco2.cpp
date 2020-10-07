// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco CG-1V/GAE1 based games

    Driver by Manuel Abadia <emumanu+mame@gmail.com>

    Known games that run on this hardware:
    ======================================
    Game                   | Year | Chip        | Ref      |Protected
    -----------------------+------+-------------+----------+--------------------------------------------------
    Alligator Hunt         | 1994 | GAE1 449    | 940411   | DS5002FP (unprotected version available)
    Alligator Hunt (proto) | 1994 | GAE1 CS438  |          | DS5002FP
    World Rally 2          | 1995 | GAE1 449    | 950510   | DS5002FP
    World Rally 2          | 1995 | GAE1 506    | 950510-1 | DS5002FP
    Touch & Go             | 1995 | GAE1 501    | 950906   | DS5002FP (unprotected version available)
    Touch & Go             | 1995 | GAE1 501    | 950510-1 | DS5002FP
    Maniac Square          | 1996 | GAE1 501    | 940411   | DS5002FP (unprotected version available)
    Maniac Square          | 1996 | CG-1V 427   | 960419/1 | Lattice IspLSI 1016-80LJ (not used, unprotected)
    Snow Board             | 1996 | CG-1V 366   | 960419/1 | Lattice IspLSI 1016-80LJ
    Cardioline Cycle       | 1997 | GAE1 501    | 970410   | IO board ST62T15C6 MCU (not really protection)
    Cardioline Stepper     | 1997 | CG-1V 288   | 970410   | IO board ST62T15B6 MCU (not really protection)
    Bang!                  | 1998 | CG-1V 388   | 980921/1 | No
    Super Roller           | 1998 | CG-1V-218   |          | DS5002FP (by Nova Desitec)
    Play 2000              | 1999 | CG-1V-149   | 990315   | DS5002FP (by Nova Desitec)
    -----------------------+------+-------------+----------+--------------------------------------------------

    Notes:
    touchgo:
    sounds cut out sometimes, others are often missing (sound status reads as busy,
    so no attempt made to play new sound) probably bug in devices\sound\gaelco.cpp ??

***************************************************************************/

#include "emu.h"
#include "includes/gaelco2.h"

#include "machine/gaelco_ds5002fp.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/eepromser.h"
#include "sound/gaelco.h"

#include "rendlay.h"
#include "screen.h"
#include "speaker.h"


static const gfx_layout tilelayout16 =
{
	16,16,                                          /* 16x16 tiles */
	RGN_FRAC(1,5),                                  /* number of tiles */
	5,                                              /* 5 bpp */
	{ RGN_FRAC(4,5), RGN_FRAC(3,5), RGN_FRAC(2,5), RGN_FRAC(1,5), 0 },
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static GFXDECODE_START( gfx_gaelco2 )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout16, 0, 128 )
GFXDECODE_END



/*============================================================================
                            COMMON
  ============================================================================*/

void gaelco2_state::mcu_hostmem_map(address_map &map)
{
	map(0x8000, 0xffff).rw(FUNC(gaelco2_state::shareram_r), FUNC(gaelco2_state::shareram_w)); // confirmed that 0x8000 - 0xffff is a window into 68k shared RAM
}


/*============================================================================
                            MANIAC SQUARE (FINAL)
  ============================================================================*/

void gaelco2_state::maniacsq_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                                                                         /* ROM */
	map(0x200000, 0x20ffff).ram().w(FUNC(gaelco2_state::vram_w)).share("spriteram");                                       /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_gae1_device::gaelcosnd_r), FUNC(gaelco_gae1_device::gaelcosnd_w));    /* Sound Registers */
	map(0x210000, 0x211fff).ram().w(FUNC(gaelco2_state::palette_w)).share("paletteram");                                   /* Palette */
	map(0x218004, 0x218009).ram().w(FUNC(gaelco2_state::vregs_w)).share("vregs");                                          /* Video Registers */
	map(0x300000, 0x300001).portr("IN0");                                                                                  /* DSW #1 + Input 1P */
	map(0x300002, 0x300003).portr("IN1");                                                                                  /* DSW #2 + Input 2P */
	map(0x30004a, 0x30004b).nopw();                                                                                        /* Sound muting? */
	map(0x320000, 0x320001).portr("COIN");                                                                                 /* COINSW + SERVICESW */
	map(0x500000, 0x500001).w(FUNC(gaelco2_state::alighunt_coin_w));                                                       /* Coin lockout + counters */
	map(0xfe0000, 0xfe7fff).ram();                                                                                         /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                                                                       /* Work RAM */
}


static INPUT_PORTS_START( maniacsq )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0700, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x7000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin A too)" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, "1P Continue" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_IMPULSE(1) /* go to service mode NOW */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void gaelco2_state::maniacsq(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);     /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::maniacsq_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(30'000'000) / 30));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0080000, 1 * 0x0080000, 0, 0);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}

void gaelco2_state::maniacsq_d5002fp(machine_config &config)
{
	maniacsq(config);
	GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2).set_addrmap(0, &gaelco2_state::mcu_hostmem_map); // clock unknown
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");
}


ROM_START( maniacsq ) // REF 940411
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tms27c010a.msu45",   0x000000, 0x020000, CRC(fa44c907) SHA1(4d9b3a6cf044395cc4e04f6dd8d1109e8ee4d52d) )
	ROM_LOAD16_BYTE( "tms27c010a.msu44",   0x000001, 0x020000, CRC(42e20121) SHA1(6662fa8ec5756bf5c4ebaaa9aa2e0e241cf582a4) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "maniacsq_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(afe9703d) SHA1(e737bf154bcb268b8f0764879b513489b163e462) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0280000, "gfx1", 0 ) /* GFX + Sound */
	// all 4 roms on a sub-board, no IC positions marked
	ROM_LOAD( "ms1",   0x0000000, 0x0080000, CRC(d8551b2f) SHA1(78b5b07112bd89fed18055180e7cc64f8e0bd0b1) )    /* GFX + Sound */
	ROM_LOAD( "ms2",   0x0080000, 0x0080000, CRC(b269c427) SHA1(b7f9501529fbb7ee82700cff82740ba5770cf3c5) )    /* GFX + Sound */
	ROM_LOAD( "ms3",   0x0100000, 0x0020000, CRC(af4ea5e7) SHA1(ffaf09dc2588e32c124e7dd2f86ba009f1b8b176) )    /* GFX only */
	ROM_FILL(          0x0120000, 0x0060000, 0x00 )         /* Empty */
	ROM_LOAD( "ms4",   0x0180000, 0x0020000, CRC(578c3588) SHA1(c2e1fba29f21d6822677886fb2d26e050b336c14) )    /* GFX only */
	ROM_FILL(          0x01a0000, 0x0060000, 0x00 )         /* Empty */
	ROM_FILL(          0x0200000, 0x0080000, 0x00 )         /* to decode GFX as 5bpp */
ROM_END

/*
Maniac Square
PCB Layout:

REF: 940411
------------------------------------------------------------------------------
|                POT1               KM424C257Z-6 (x3)                        |
|                                                                            |
|                POT2                                                        |
|---                                                                         |
   |                                                               U47       |
   |                   30.000MHz          |----------|                       |
|---                                      |          |             U48       |
|                                         | GAE1 449 |                       |
| J                            6264       | (QFP208) |             U49       |
|                              6264       |          |                       |
| A                                       |----------|             U50       |
|                                                                            |
| M                                                                          |
|                         |-------------------------|                        |
| M                       |                         |  24.000MHz     62256   |
|                         |  62256  DS5002  BATT_3V |                62256   |
| A                       |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                    62256                                |
   |                                    62256                                |
   |                                                                         |
|---                                                                         |
|   DSW1                         MC68000P12        U45                       |
|                                                  U44                       |
|   DSW2                                                                     |
|                                                                            |
-----------------------------------------------------------------------------|
*/
ROM_START( maniacsqa ) // REF 940411
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "ms_u_45.u45",   0x000000, 0x020000, CRC(98f4fdc0) SHA1(1e4d5b0a8a432de885c96319c21280d304b38db0) )
	ROM_LOAD16_BYTE( "ms_u_44.u44",   0x000001, 0x020000, CRC(1785dd41) SHA1(5c6a65c00248971ce54c8185858393f2c52cc583) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "maniacsq_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(afe9703d) SHA1(e737bf154bcb268b8f0764879b513489b163e462) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0280000, "gfx1", 0 ) /* GFX + Sound */
	// all 4 roms on a sub-board, no IC positions marked
	ROM_LOAD( "ms1",   0x0000000, 0x0080000, CRC(d8551b2f) SHA1(78b5b07112bd89fed18055180e7cc64f8e0bd0b1) )    /* GFX + Sound */
	ROM_LOAD( "ms2",   0x0080000, 0x0080000, CRC(b269c427) SHA1(b7f9501529fbb7ee82700cff82740ba5770cf3c5) )    /* GFX + Sound */
	ROM_LOAD( "ms3",   0x0100000, 0x0020000, CRC(af4ea5e7) SHA1(ffaf09dc2588e32c124e7dd2f86ba009f1b8b176) )    /* GFX only */
	ROM_FILL(          0x0120000, 0x0060000, 0x00 )         /* Empty */
	ROM_LOAD( "ms4",   0x0180000, 0x0020000, CRC(578c3588) SHA1(c2e1fba29f21d6822677886fb2d26e050b336c14) )    /* GFX only */
	ROM_FILL(          0x01a0000, 0x0060000, 0x00 )         /* Empty */
	ROM_FILL(          0x0200000, 0x0080000, 0x00 )         /* to decode GFX as 5bpp */
ROM_END

ROM_START( maniacsqu ) // REF 940411
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "d8-d15.1m",   0x000000, 0x020000, CRC(9121d1b6) SHA1(ad8f0d996b6d42fc0c6645466608e82ca96e0b66) )
	ROM_LOAD16_BYTE( "d0-d7.1m",    0x000001, 0x020000, CRC(a95cfd2a) SHA1(b5bad76f12d2a1f6bf6b35482f2f933ceb00e552) )

	ROM_REGION( 0x0280000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "d0-d7.4m",   0x0000000, 0x0080000, CRC(d8551b2f) SHA1(78b5b07112bd89fed18055180e7cc64f8e0bd0b1) )    /* GFX + Sound */
	ROM_LOAD( "d8-d15.4m",  0x0080000, 0x0080000, CRC(b269c427) SHA1(b7f9501529fbb7ee82700cff82740ba5770cf3c5) )    /* GFX + Sound */
	ROM_LOAD( "d16-d23.1m", 0x0100000, 0x0020000, CRC(af4ea5e7) SHA1(ffaf09dc2588e32c124e7dd2f86ba009f1b8b176) )    /* GFX only */
	ROM_FILL(               0x0120000, 0x0060000, 0x00 )         /* Empty */
	ROM_LOAD( "d24-d31.1m", 0x0180000, 0x0020000, CRC(578c3588) SHA1(c2e1fba29f21d6822677886fb2d26e050b336c14) )    /* GFX only */
	ROM_FILL(               0x01a0000, 0x0060000, 0x00 )         /* Empty */
	ROM_FILL(               0x0200000, 0x0080000, 0x00 )         /* to decode GFX as 5bpp */
ROM_END

/*
Maniac Square

PCB Layout:
REF: 960419/1
Part No.: E193
------------------------------------------------------------------------------
|                                                                            |
|                                               KM428C256J-6                 |
|                POT1                                                        |
|---                                            KM428C256J-6                 |
   |         SW2                                                   IC43*     |
   |                   30.000MHz          |----------|                       |
|---        93C66                         |          |             IC44      |
|                                         |  CG-1V   |                       |
| J                                       |   427    |             IC45*     |
|                                         |          |                       |
| A                            6264       |----------|             IC46*     |
|                              6264                                          |
| M                                                                IC47      |
|                                                                            |
| M                                                                  62256   |
|                                                                    62256   |
| A                                          |----------|            62256   |
|                                24.000MHz   | Lattice  |                    |
|---                                         | IspLSI   |                    |
   |                                         |   1016   |          MS1.IC53  |
   |                                         |----------|                    |
|---                                        |------------|           62256   |
|                                           |            |                   |
|                                           |  MC68HC000 |         MS2.IC55  |
|                                           |    FN16    |                   |
|                                           |------------|                   |
-----------------------------------------------------------------------------|

Daughterboard plugs in through IC47 and IC44 sockets

PCB Layout:
MUN-M4M/1
Part No.: E192
+--------+
|   F0   |
|        |
|   F1   |
|   F2   |
|        |
|   F3   |
+--------+

* Denotes unpopulated sockets

Although this version of Maniac Square use the same PCB as Snow Board Championship, there are some minor
component changes:

 Slower OSC clocks
   30.000MHz down from 34.000MHz
   24.000MHz down from 30.000MHz

The CG-1V 366 has been upgraded to a CG-1V 427

Game configuration is store in 93C66 EEPROM as this PCB doesn't have dipswitches

This PCB has a large number of unpopulated ICs and connectors at the bottom left, apparently for extra
digital and analog inputs not required by Maniac Square or Snow Board Championship:

  1x 74LS14 (IC20)
  3x 74LS245 (IC26, IC27, IC28)
  4x TLC549 (IC9, IC10, IC21, IC22)
  4x 6-pin headers (JP2, JP3, JP5, JP6)
  2x 15-pin headers (JP4, JP7)

*/

ROM_START( maniacsqs ) // REF 960419/1
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "ms1.ic53", 0x000000, 0x020000, CRC(911fb089) SHA1(62bebf5072331421d4beedf0bde0cffc362b0514) )
	ROM_LOAD16_BYTE( "ms2.ic55", 0x000001, 0x020000, CRC(e77a5537) SHA1(e7e1c7b794515238c4b5e5b8ef050eb945c96a3f) )

	ROM_REGION( 0x0280000, "gfx1", 0 ) /* GFX + Sound - same data as other sets */
	ROM_LOAD( "f0.bin",  0x0000000, 0x0080000, CRC(d8551b2f) SHA1(78b5b07112bd89fed18055180e7cc64f8e0bd0b1) )    /* GFX + Sound */
	ROM_LOAD( "f1.bin",  0x0080000, 0x0080000, CRC(b269c427) SHA1(b7f9501529fbb7ee82700cff82740ba5770cf3c5) )    /* GFX + Sound */
	ROM_LOAD( "f2.bin",  0x0100000, 0x0020000, CRC(af4ea5e7) SHA1(ffaf09dc2588e32c124e7dd2f86ba009f1b8b176) )    /* GFX only */
	ROM_FILL(            0x0120000, 0x0060000, 0x00 )         /* Empty */
	ROM_LOAD( "f3.bin",  0x0180000, 0x0020000, CRC(578c3588) SHA1(c2e1fba29f21d6822677886fb2d26e050b336c14) )    /* GFX only */
	ROM_FILL(            0x01a0000, 0x0060000, 0x00 )         /* Empty */
	ROM_FILL(            0x0200000, 0x0080000, 0x00 )         /* to decode GFX as 5bpp */
ROM_END

/*============================================================================
                            Salter Cardioline Series
  ============================================================================*/

static INPUT_PORTS_START( saltcrdi ) // dipswitches are on the REVERSE side of the PCB (!)
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) // pedal
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) // green
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) // red
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_SERVICE_DIPLOC(0x01, IP_ACTIVE_LOW, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x20, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( German ) )
	PORT_DIPSETTING(    0x60, "Catalan" ) // ?
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) ) // double?
	PORT_DIPSETTING(    0xa0, "Portuguese" ) // ?
	PORT_DIPSETTING(    0xc0, DEF_STR( French ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( Spanish ) ) // triple?
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

// just a copy of maniac square for now
void gaelco2_state::saltcrdi_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x200000, 0x20ffff).ram().w(FUNC(gaelco2_state::vram_w)).share("spriteram");
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_gae1_device::gaelcosnd_r), FUNC(gaelco_gae1_device::gaelcosnd_w));
	map(0x210000, 0x211fff).ram().w(FUNC(gaelco2_state::palette_w)).share("paletteram");
	map(0x218004, 0x218009).ram().w(FUNC(gaelco2_state::vregs_w)).share("vregs");
	map(0x300000, 0x300001).portr("IN0");
	map(0x310000, 0x310001).portr("DSW");
	map(0x320000, 0x320001).portr("COIN");
	map(0xfe0000, 0xfe7fff).ram();
	map(0xfe8000, 0xfeffff).ram().share("shareram");
}

// 34'000'000 XTAL for the video?
void gaelco2_state::saltcrdi(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2); /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::saltcrdi_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 384-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	/* unused? ROMs contain no sound data */
	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(24'000'000) / 24)); // TODO : Correct OSC?
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0080000, 1 * 0x0080000, 0, 0);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}

/*============================================================================
                  Salter Cardioline Pro Cycle / Pro Reclimber
  ============================================================================
  _________________________________________________________________________________
 | GAELCO REF. 970410                           U24                                |
 |                                                       MC74HCT373AN ____________ |
 |__                               SN74LS08N  KM428C256J-6            | BI 37     ||
    |                                                    MC74HCT373AN |___________||
  __|                                  XTAL2  KM428C256J-6            ____________ |
 |__                                                     MC74HCT373AN | BI 38     ||
 |__     74LS259BN                                                    |___________||
 |__                   CD74HCT273E          71256      _________      ____________ |
 |__     TD62064AP                                     |        |     | BI 39     ||
 |__                   CD74HCT273E          71256      | GAE1   |     |___________||
 |__                                                   |  501   |     ____________ |
 |__                   XTAL1           SN74LS374N      |________|     | BI 40     ||
 |__     CNY74-4                                                      |___________||
 |__                   MC74F74N        SN74LS374N                     ____________ |
 |__     CNY74-4                                                      | BI 41     ||
 |__                   SN74F04N          SN74F32N      71256          |___________||
 |__     SN74LS245N                                                   ____________ |
 |__     ________      TIBPAL16L8           71256      71256          | BI 42     ||
 |__     |__SW1__|                    ____________                    |___________||
 |__                   __________     |  BI N21   |   MC74HCT373AN    ____________ |
 |__     SN74LS245N    | U13     |    |___________|                   | BI 43     ||
    |                  |         |          71256     MC74HCT373AN    |___________||
  __|    SN74LS132N    | 02AB    |    ____________                    ____________ |
 |                     |_________|    |  BI N23   |   SN74LS245N      | BI 44     ||
 |  JP1  SN74LS32N                    |___________|                   |___________||
 |                                                    SN74LS245N                   |
 |_________________________________________________________________________________|

  XTAL1 = 24.0000 MHz
  XTAL2 = 34.0000 MHz
  02AB = MC68HC000FN12
  SW1 = 8 dipswitches (soldered on the back side of the PCB, all off by default)
  U24 = Unpopulated socket for 424C257

  There is also a Salter I/O PCB with a MCU (undumped) labeled as "2".
  The I/O PCB layout is slightly different than the one found on the Pro Stepper machine.
*/
ROM_START( sltpcycl ) // REF 970410
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "bi-n-21-27c512.u21",   0x000000, 0x010000, CRC(1d2e9a68) SHA1(b9bb4eeefe90850b648dc45689a08f3c28147856) )
	ROM_LOAD16_BYTE( "bi-n-23-27c512.u23",   0x000001, 0x010000, CRC(5865351d) SHA1(a62b8ec88ef41d96b65a03ccaeadbec21803df34) )

	ROM_REGION( 0x0280000, "gfx1", ROMREGION_ERASEFF ) /* GFX, no sound, machine has none? */
	ROM_LOAD( "bi-40-bank0-27c1001.u40",   0x0000000, 0x0020000, CRC(56822524) SHA1(aae133e9fb85ba8995c095cc540aa35b65c27777) )
	ROM_LOAD( "bi-39-bank0-27c1001.u39",   0x0080000, 0x0020000, CRC(30dfcde1) SHA1(caf4429d0e1185c157eca436e9bb3a8513781a97) )
	ROM_LOAD( "bi-38-bank0-27c1001.u38",   0x0100000, 0x0020000, CRC(84ec4b34) SHA1(01376f2534c4bc51d0a357d80db28b24c3fd71f6) )
	ROM_LOAD( "bi-37-bank0-27c1001.u37",   0x0180000, 0x0020000, CRC(779fca47) SHA1(fce95893a5bcf0c6f26c223491c95154f072c92b) )
	ROM_FILL(                              0x0200000, 0x0020000, 0x00 )         /* to decode GFX as 5bpp */
	ROM_LOAD( "bi-44-bank1-27c1001.u44",   0x0020000, 0x0020000, CRC(171d2f88) SHA1(e2b406dad78e3ab5bebb673ed03db5d27879283e) )
	ROM_LOAD( "bi-43-bank1-27c1001.u43",   0x00a0000, 0x0020000, CRC(69b35d81) SHA1(d9657e3d592079071df019cd75d676fa4b1bcba9) )
	ROM_LOAD( "bi-42-bank1-27c1001.u42",   0x0120000, 0x0020000, CRC(eaef0565) SHA1(4214b05f1df3062eaeea91505b61816725556ed5) )
	ROM_LOAD( "bi-41-bank1-27c1001.u41",   0x01a0000, 0x0020000, CRC(c4d24254) SHA1(e6ff7624e628dc6ace11a50b6ff89812844b52c5) )
	ROM_FILL(                              0x0220000, 0x0020000, 0x00 )         /* to decode GFX as 5bpp */

	ROM_REGION( 0x0800, "iomcu", 0 ) // on IO board
	ROM_LOAD( "2-st62t15c6", 0x0000, 0x0800, NO_DUMP ) // 2KBytes internal ROM

	ROM_REGION( 0x0104, "pals", 0 )
	ROM_LOAD( "6.pal16l8.u12", 0x0000, 0x0104, NO_DUMP )
ROM_END

/*============================================================================
                           Salter Cardioline Pro Steper
  ============================================================================
  _________________________________________________________________________________
 | GAELCO REF. 970410                           U24                                |
 |                                                       MC74HCT373AN ____________ |
 |__                               SN74LS08N  KM428C256J-6            | ST U 37   ||
    |                                                    MC74HCT373AN |___________||
  __|                                  XTAL2  KM428C256J-6            ____________ |
 |__                                                     MC74HCT373AN | ST U 38   ||
 |__     74LS259BN                                                    |___________||
 |__                   CD74HCT273E          71256      _________      ____________ |
 |__     TD62064AP                                     |        |     | ST U 39   ||
 |__                   CD74HCT273E          71256      | CG-1V  |     |___________||
 |__                                                   |  288   |     ____________ |
 |__                   XTAL1           SN74LS374N      |________|     | ST U 40   ||
 |__     CNY74-4                                                      |___________||
 |__                   MC74F74N        SN74LS374N                     ____________ |
 |__     CNY74-4                                                      | EMPTY     ||
 |__                   SN74F04N          SN74F32N      71256          |___________||
 |__     SN74LS245N                                                   ____________ |
 |__     ________      TIBPAL16L8           71256      71256          | EMPTY     ||
 |__     |__SW1__|                    ____________                    |___________||
 |__                   __________     |  ST U 21  |   MC74HCT373AN    ____________ |
 |__     SN74LS245N    | U13     |    |___________|                   | EMPTY     ||
    |                  |         |          71256     MC74HCT373AN    |___________||
  __|    SN74LS132N    | E208    |    ____________                    ____________ |
 |                     |_________|    |  ST U 23  |   SN74LS245N      | EMPTY     ||
 |  JP1  SN74LS32N                    |___________|                   |___________||
 |                                                    SN74LS245N                   |
 |_________________________________________________________________________________|

  XTAL1 = 24.0000 MHz
  XTAL2 = 34.0000 MHz
  E208 = MC68HC000FN16
  SW1 = 8 dipswitches (soldered on the back side of the PCB, all off by default)
  U24 = Unpopulated socket for 424C257

  There is also a Salter I/O PCB with a MCU (undumped):

  Salter PCB "CPU 6022" manufactured by "APEL Electronica"
    ______________________________________________
    |       ___________                __________ |
    |       | JW1FSN  |  Cap 10000uF   |__FUSE__| |
    |                                             --
    | ________    ___            Power LED -> o   -- VAC (3 pin)
    | |AD7424JN  |LM356P       VDC Out LED -> o   --
    | __________________                          -- +/- OUT (2 pin)
    | |_____PC817______|          M338K           --
    |  o o o o o o o o  <- LEDS                   --
    |  __________  _____                          -- CONTROL (9 pin)
    |  |TD62083AP  |SW1|                          --
    |     ______________   ________    __________ --
    | XT1 |   MCU-1    |   |74HC14AP   |__FUSE__| -- +/- TURNS (3 pin)
    |     |____________|                |J6|      --
    |                                   | J5 |    |
    |_____________________________________________|

  XT1 = 8.000MHz
  SW1 = 4 dipswitches (default all open)
  J6 = 12V out for fan
  J5 = 6 pin connector (unused)
  MCU-1 = ST62T15B6-HWD labeled as "1"
*/
ROM_START( sltpstep )
	ROM_REGION( 0x040000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "st_u_21.u21",   0x000000, 0x010000, CRC(2274f931) SHA1(c7b32bbb46e349769376bfaffe663170873bd083) ) // 27C512
	ROM_LOAD16_BYTE( "st_u_23.u23",   0x000001, 0x010000, CRC(07c69f55) SHA1(886bef76b2aff046fd1c9a4837f359cb59095125) ) // 27C512

	ROM_REGION( 0x0280000, "gfx1", ROMREGION_ERASEFF ) /* GFX, no sound, machine has none? */
	ROM_LOAD( "st_u_40.u40",   0x0000000, 0x0080000, CRC(813270de) SHA1(9a6ce7679bd5c6ecf0c3328d6ff9dc2240a95328) ) // 27C4000DC
	ROM_LOAD( "st_u_39.u39",   0x0080000, 0x0080000, CRC(1d42e124) SHA1(add866310511f4f406b80ed0d3983b79b80c701c) ) // 27C4000DC
	ROM_LOAD( "st_u_38.u38",   0x0100000, 0x0080000, CRC(9d0d795c) SHA1(ef7cf61f8c687ecc68678a634f65386cc25d8a8f) ) // 27C4000DC
	ROM_LOAD( "st_u_37.u37",   0x0180000, 0x0080000, CRC(5543d4d1) SHA1(1f9f358dfb252412468ddd68331bda5acbe99329) ) // 27C4000DC
	ROM_FILL(                              0x0200000, 0x0080000, 0x00 )         /* to decode GFX as 5bpp */

	ROM_REGION( 0x0800, "iomcu", 0 ) // on IO board
	ROM_LOAD( "cpu_6022-1-st62t15b6.ic4", 0x0000, 0x0800, NO_DUMP ) // 2KBytes internal ROM

	ROM_REGION( 0x0104, "pals", 0 )
	ROM_LOAD( "6.pal16l8.u12", 0x0000, 0x0104, NO_DUMP )
ROM_END


/*============================================================================
                            PLAY 2000
  ============================================================================*/


/*
CPU 1x MC68HC000FN12 (main)(u18)
1x CG-1V-149 (u42)(sound/gfx? maybe the same as Gaelco)
1x DALLAS DS5002FP (not dumped)(u44)
1x TDA1543 (sound)(u20)
1x LM358N (sound)(u7)
1x TDA2003 (sound)(u1)
1x oscillator 34.000 MHz (xtal1)
1x oscillator 11.0592 MHz (xtal2)
ROMs    2x AM27C010 (u39,u40)
4x NM27C040Q (u50,u52,u53,u54)
1x M27C801 (u51)

6x RAM CY7C199 (u22,u23,u26,u27,u55,u56)
2x RAM KM428C256TR (u37,u41)
1x RAM GM76C256CLLFW70 (u47)(close to Dallas)

1x PALCE16V8H (u28)(read protected -> extracted with CmD's PALdumper - it's registered)
1x PALCE16V8H (u29)(read protected -> extracted with CmD's PALdumper)
Note    1x 28x2 edge connector
1x 2 legs connector (jp1)
1x 4 legs connector (jp2)
1x 12 legs connector (jp3)
1x 5 legs connector (jp4)
1x RS232 connector
1x trimmer (volume)
1x battery 3V (bt1)

*/

void gaelco2_state::play2000_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();                                                                                         /* ROM */
	map(0x100000, 0x100001).portr("IN0");                                                                                  /* Coins + other buttons? */
	map(0x110000, 0x110001).portr("IN1");
	map(0x200000, 0x20ffff).ram().w(FUNC(gaelco2_state::vram_w)).share("spriteram");                                       /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_gae1_device::gaelcosnd_r), FUNC(gaelco_gae1_device::gaelcosnd_w));    /* Sound Registers */
	map(0x214000, 0x214fff).ram().w(FUNC(gaelco2_state::palette_w)).share("paletteram");                                   /* Palette */
	map(0x215000, 0x217fff).ram();                                                                                         /* Written to, but unused? */
	map(0x218000, 0x218003).ram();                                                                                         /* Written to, but unused? */
	map(0x218004, 0x218009).ram().w(FUNC(gaelco2_state::vregs_w)).share("vregs");                                          /* Video Registers */
	map(0x21800a, 0x218fff).ram();                                                                                         /* Written to, but unused? */
	// map(0x843100, 0x84315e)  ?
	map(0xfe0000, 0xfe7fff).ram();                                                                                         /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                                                                       /* Work RAM */
}

u16 gaelco2_state::srollnd_share_sim_r(offs_t offset, u16 mem_mask)
{
	uint16_t ret = m_shareram[offset];

	if (m_maincpu->pc() == 0x0083d0)
		ret = 0x0000;

	if (m_maincpu->pc() == 0x0085B0)
		ret = 0x0000;

	if (m_maincpu->pc() == 0x00839e)
		ret = 0x0000;

	if (m_maincpu->pc() == 0x0035a6)
		ret = 0x0000;

	if (m_maincpu->pc() == 0x00857e) // after restoring default values (write back to nvram)
		ret = 0x0000;


	// reads a bunch of data (game specific? backup ram? default backup ram?) from device (0x180 words - copied to start of RAM)
	if (m_maincpu->pc() == 0x83da)
	{
		ret = 0x0000;

		if (offset == 0x274 / 2)
		{
			//  ret = 0x3112; // checked after copy, otherwise you get password? prompt

			// the 'password' for bootup (reset to default values) is stored at 13454 in ROM
			// sequence value: 0800 0800 1000 4000 2000
			// default key:    x    x    c    b    v

			// the 'password' in service mode checks the following (stored after above) (anything related to countability or where changing it might clear things)
			// sequence value: 0800 1000 0400 0800 4000
			// default key:    x    c    z    x    b

			// 0400 0800 1000 2000 4000  (just a default unused sequence?)
			// z    x    c    v    b

			// 0400 0400 1000 0800 4000  for advanced internal options in service mode
			// z    z    c    x   b
		}
	}


	logerror("%s: srollnd_share_sim_r %04x: %04x (%04x)\n", machine().describe_context(), offset, ret, mem_mask);

	return ret;
}

void gaelco2_state::srollnd_share_sim_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (m_maincpu->pc() != 0x552)
		logerror("%s: srollnd_share_sim_w %04x: %04x (%04x)\n", machine().describe_context(), offset, data, mem_mask);
	COMBINE_DATA(&m_shareram[offset]);
}

void gaelco2_state::srollnd_map(address_map& map)
{
	play2000_map(map);

	map(0xfe8000, 0xfeffff).ram().rw(FUNC(gaelco2_state::srollnd_share_sim_r), FUNC(gaelco2_state::srollnd_share_sim_w)).share("shareram");
}


static INPUT_PORTS_START( play2000 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 ) // NoteA
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) // cycles through games in attract?
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) // shows odds if coins are present?
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN3 ) // NoteB
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN4 ) // NoteC
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN5 ) // NoteD
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Incassa") PORT_CODE(KEYCODE_H) // what standard button would this be?
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) // "Play"
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_NO_TOGGLE( 0x0040, IP_ACTIVE_LOW )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Collect") PORT_CODE(KEYCODE_G) // what standard button would this be?
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void gaelco2_state::init_play2000()
{
	m_global_spritexoff = 1; // aligns flashing sprites with backgrounds
}

ROM_START( play2000 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tms27c010a.2",    0x000000, 0x020000, CRC(598102f9) SHA1(66fb6d321c886099b99d048d1f9f13cf016b9c43) )
	ROM_LOAD16_BYTE( "tms27c010a.1",    0x000001, 0x020000, CRC(e2b5d79a) SHA1(f004352ddb9bc92aab126627689b45b2ef8583b1) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code - had 7.0 sticker on too */
	// This has only been dumped from a single MCU, so there is potential for bad bits as there is risk in the dumping process.  The majority of the ROM is data and thus difficult to verify.
	ROM_LOAD( "ds5002fp_70i.mcu", 0x00000, 0x8000, BAD_DUMP CRC(b51ad3da) SHA1(4a730238b4b875c74dd2e4df6e7880d03659b7d5) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "6.u51", 0x0000000, 0x0100000, CRC(6dafc11c) SHA1(2aa3d6318418578433b3060bda6e27adf794dea4) ) /* GFX + Sound*/
	ROM_LOAD( "4.u53", 0x0200000, 0x0080000, CRC(94dc37a7) SHA1(28f9832b61541b292682a6e2d2264abccd138a2e) ) /* GFX only */
	ROM_LOAD( "7.u50", 0x0400000, 0x0080000, CRC(e80c6d39) SHA1(b3ae5d66c48c2ba6665a181e311b0c834384258a) ) /* GFX only */
	ROM_LOAD( "5.u52", 0x0600000, 0x0080000, CRC(19b939f4) SHA1(7281709aa3ab1decb84bf7ab10492fb6ec197c80) ) /* GFX only */
	ROM_LOAD( "3.u54", 0x0800000, 0x0080000, CRC(085008ed) SHA1(06eb4f972d79eab13b1b3b6829ef280e079abdb6) ) /* GFX only */

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u29",  0x0000, 0x0117, BAD_DUMP CRC(4a0a6f39) SHA1(57351e471649391c9abf110828fe2f128fe84eee) )
ROM_END


ROM_START( play2000_40i ) /* there are version 4.0 and version 1.0 strings in this, go with the higher one */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.u39_v4",    0x000000, 0x020000, CRC(fff16141) SHA1(8493c3e58a231c03b152b336f43422a9a2d2618c) )
	ROM_LOAD16_BYTE( "1.u40_v4",    0x000001, 0x020000, CRC(39f9d58e) SHA1(1cbdae2adc570f2a2e10a707075312ef717e2643) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	// this is the same dump as from the play7000 MCU but with valid default data for this set
	ROM_LOAD( "ds5002fp_40i.mcu", 0x00000, 0x8000, BAD_DUMP CRC(7c45cdf2) SHA1(64aee4d77e0715342634e6eadb83dae4a2db9dfd) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "6.u51", 0x0000000, 0x0100000, CRC(6dafc11c) SHA1(2aa3d6318418578433b3060bda6e27adf794dea4) ) /* GFX + Sound*/
	ROM_LOAD( "4.u53", 0x0200000, 0x0080000, CRC(94dc37a7) SHA1(28f9832b61541b292682a6e2d2264abccd138a2e) ) /* GFX only */
	ROM_LOAD( "7.u50", 0x0400000, 0x0080000, CRC(e80c6d39) SHA1(b3ae5d66c48c2ba6665a181e311b0c834384258a) ) /* GFX only */
	ROM_LOAD( "5.u52", 0x0600000, 0x0080000, CRC(19b939f4) SHA1(7281709aa3ab1decb84bf7ab10492fb6ec197c80) ) /* GFX only */
	ROM_LOAD( "3.u54", 0x0800000, 0x0080000, CRC(085008ed) SHA1(06eb4f972d79eab13b1b3b6829ef280e079abdb6) ) /* GFX only */

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u29",  0x0000, 0x0117, BAD_DUMP CRC(4a0a6f39) SHA1(57351e471649391c9abf110828fe2f128fe84eee) )
ROM_END

/*
Play 2000

PCB Layout:
REF: 990315
------------------------------------------------------------------------------
|                                   KM428C256TR  KM428C256TR                 |
|                                                                            |
|                POT1                                                        |
|---                                                                         |
   |                                                               U50       |
   |                   SRAM_32Kx8         |----------|                       |
|---                   SRAM_32Kx8         |          |             U51       |
|                      34.000MHz          |  CG-1V   |                       |
| J                    SRAM_32Kx8         |   149    |             U52       |
|                      SRAM_32Kx8         |          |                       |
| A                                       |----------|             U53       |
|                                                                            |
| M                                                                U54       |
|                         |---------------------------|                      |
| M                       |                           |         SRAM_32Kx8   |
|                         | BATT_3V DS5002 SRAM_32Kx8 |         SRAM_32Kx8   |
| A                       |                           |                      |
|                         |---------------------------|                      |
|                                                                            |
|---                                         11.0592MHz                      |
   |                                                                         |
   |                                                                         |
|---          |------------|          U39                                    |
|             |            |                                                 |
|             |  MC68HC000 |          U40                                    |
|             |    FN12    |                                                 |
|             |------------|                                                 |
-----------------------------------------------------------------------------|
*/
ROM_START( play2000_50i )
	/*at least 1.u40 is bad, on every 0x40 bytes the first four are always 0xff.*/
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "2.u39",   0x000000, 0x020000, BAD_DUMP CRC(9939299e) SHA1(55303a2adf199f4b5a60f57be7480b0e119f8624) )
	ROM_LOAD16_BYTE( "1.u40",   0x000001, 0x020000, BAD_DUMP CRC(311c2f94) SHA1(963d6b5f479598145146fcb8b7c6ce77fbc92b07) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	// can't create an initialized default for this one as the main program is bad
	ROM_LOAD( "ds5002fp_50i.mcu", 0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x79 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) /* GFX + Sound */
	ROM_LOAD( "6.u51", 0x0000000, 0x0100000, CRC(6dafc11c) SHA1(2aa3d6318418578433b3060bda6e27adf794dea4) ) /* GFX + Sound*/
	ROM_LOAD( "4.u53", 0x0200000, 0x0080000, CRC(94dc37a7) SHA1(28f9832b61541b292682a6e2d2264abccd138a2e) ) /* GFX only */
	ROM_LOAD( "7.u50", 0x0400000, 0x0080000, CRC(e80c6d39) SHA1(b3ae5d66c48c2ba6665a181e311b0c834384258a) ) /* GFX only */
	ROM_LOAD( "5.u52", 0x0600000, 0x0080000, CRC(19b939f4) SHA1(7281709aa3ab1decb84bf7ab10492fb6ec197c80) ) /* GFX only */
	ROM_LOAD( "3.u54", 0x0800000, 0x0080000, CRC(085008ed) SHA1(06eb4f972d79eab13b1b3b6829ef280e079abdb6) ) /* GFX only */

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "palce16v8h.u29",  0x0000, 0x0117, BAD_DUMP CRC(4a0a6f39) SHA1(57351e471649391c9abf110828fe2f128fe84eee) )
ROM_END

/* Super Roller (Nova Desitec on Gaelco hardware)
 ___________________________________________________________________________________________
 |                                     __________                   ___________             |
 |           ____       ___      ___   |SN74LS08N                   |SN74LS373N             |
 |          TDA2003    LM358P  TDA1543   ____________ ____________  ___________ ___________ |
 |                              _____    |M548263-60J||M548263-60J| |SN74LS373N |SN74LS373N |
 |___                           |XTAL|   |___________||___________|       _________________ |
     |                        34.000MHz                                   | U48-6-27C010A  ||
  ___|                         _________             _____________        |________________||
 |___                          |74F32PC_|            |  GC-1V     |       _________________ |
 |___             ___________  _____________         |  218       |       | U49-5-27C4001  ||
 |___             MC74HCT273N  |AS7C164-20PC|        |            |       |________________||
  ___|            ___________  _____________         |            |       _________________ |
 |___             MC74HCT273N  |AS7C164-20PC|        |____________|       | U50-4-27C010A  ||
 |___             ___________                                  ______     |________________||
 |___             MC74HCT274N  ________________________        |XTAL |    _________________ |
 |___  __________ ___________  | Dallas                |      20.000MHz   | U51-3-27C010A  ||
 |___  |TD62083AP MC74HCT273N  | DS5002                |     __________   |________________||
 |___  __________ ___________  |                       |     |MC74F74N|                     |
 |___  |TD62083AP MC74HCT274N  |                       |                                    |
 |___             ___________  |_______________________|    ___________      ______________ |
 |___             MC74HCT273N                               |SN74LS157N      |AS7C256-10PC_||
 |___             ___________  ___________  ______________  ___________      ______________ |
 |___             |SN74LS245N  |MC74F245N_| |AS7C256-10PC_| |SN74LS157N      |AS7C256-10PC_||
 |___             ___________  ___________  ______________  ___________        ____________ |
 |___             |SN74LS245N  |MC74F245N_| |AS7C256-10PC_| |SN74LS157N        |SN74LS373N_||
 |___  _________  ___________  ___________    ___________   ___________        ____________ |
     | |74LS132N| |_74F04PC__| |MC74F245N_|  |_MC74F373N_|  |SN74LS157N        |SN74LS373N_||
  ___| _________  ___________  ___________    ___________     _______________  ____________ |
 |     |74LS245N| PALCE16V8H-25|MC74F245N_|  |_MC74F373N_|    |U44-2-27C512  | |SN74LS245N_||
 |                ___________  ___________________________    |______________| ____________ |
 |: <- JP1        |PAL16L8ACN| |MC68HC000P10              |   _______________  |SN74LS245N_||
 |: <- JP3        ___________  |                          |   |U45-1-27C512  |              |
 |: ..... <- JP3  |_SN74LS20N| |__________________________|   |______________|              |
 |__________________________________________________________________________________________|
*/
ROM_START( srollnd )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "nd2.u44",    0x000001, 0x010000, CRC(ee3ec213) SHA1(80a08839327bf8215abfad1fececac64da6fbcb2) )
	ROM_LOAD16_BYTE( "nd1.u45",    0x000000, 0x010000, CRC(4bf20c7b) SHA1(b483f74fed25139e92359b178f6548b867c999e4) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "srollnd.ds5002fp", 0x00000, 0x8000, NO_DUMP )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0a00000, "gfx1", ROMREGION_ERASE00 ) /* GFX + Sound */
	ROM_LOAD( "nd5.u49", 0x0000000, 0x080000, CRC(5ec78408) SHA1(1a5b3a0bdbd36bf6607e47dedf31f4b9a7b89667) )
	ROM_LOAD( "nd3.u51", 0x0200000, 0x020000, CRC(e19ac5b8) SHA1(980a3b339f6958e5e04ea624f26dabd2e06f0c68) )
	ROM_LOAD( "nd6.u48", 0x0400000, 0x020000, CRC(81cd4097) SHA1(94c7f0d3c21070039dbef9fc43d0f5f2619dad5a) )
	ROM_LOAD( "nd4.u50", 0x0600000, 0x020000, CRC(8c66cd09) SHA1(5cf0a001bfd46c1e955f7952f8a42a001beaf43c) )

	ROM_REGION( 0x21b, "pals", 0 )
	ROM_LOAD( "palce16v8h.u16", 0x000, 0x117, NO_DUMP )
	ROM_LOAD( "pal16l8acn.u17", 0x117, 0x104, NO_DUMP )
ROM_END

void gaelco2_state::play2000(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(11'059'200));     /* or from the 34MHz? (34MHz drives the CG-1V-149 PLD?) */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::play2000_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(32'000'000) / 2).set_addrmap(0, &gaelco2_state::mcu_hostmem_map); /* 16 MHz */
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 384-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(34'000'000) / 34));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x080000, 0 * 0x080000, 0 * 0x080000, 0 * 0x080000);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}

void gaelco2_state::srollnd(machine_config& config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(20'000'000 / 2));
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::srollnd_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	// not dumped
	//GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(32'000'000) / 2).set_addrmap(0, &gaelco2_state::mcu_hostmem_map); /* ? MHz */
	//config.set_perfect_quantum("gaelco_ds5002fp:mcu");

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 384-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(34'000'000) / 34));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x080000, 0 * 0x080000, 0 * 0x080000, 0 * 0x080000);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}


/*============================================================================
                                BANG
  ============================================================================*/

u16 bang_state::p1_gun_x(){return (m_light0_x->read() * 320 / 0x100) + 1;}
u16 bang_state::p1_gun_y(){return (m_light0_y->read() * 240 / 0x100) - 4;}
u16 bang_state::p2_gun_x(){return (m_light1_x->read() * 320 / 0x100) + 1;}
u16 bang_state::p2_gun_y(){return (m_light1_y->read() * 240 / 0x100) - 4;}

void bang_state::bang_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                         /* ROM */
	map(0x200000, 0x20ffff).ram().w(FUNC(bang_state::vram_w)).share("spriteram");                                          /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_cg1v_device::gaelcosnd_r), FUNC(gaelco_cg1v_device::gaelcosnd_w));    /* Sound Registers */
	map(0x210000, 0x211fff).ram().w(FUNC(bang_state::palette_w)).share("paletteram");                                      /* Palette */
	map(0x218004, 0x218007).ram().w(FUNC(bang_state::vregs_w)).share("vregs");                                             /* Video Registers */
	map(0x218008, 0x218009).noprw();                                                                                       /* CLR INT Video */
	map(0x300000, 0x300001).portr("P1");
	map(0x300002, 0x300003).nopr();                                                                                        /* Random number generator? */
	map(0x300000, 0x30000f).w(m_mainlatch, FUNC(ls259_device::write_d0)).umask16(0x00ff);                                  /* Coin Counters & serial EEPROM */
	map(0x300010, 0x300011).portr("P2");
	map(0x300020, 0x300021).portr("COIN");
	map(0x310000, 0x310001).r(FUNC(bang_state::p1_gun_x)).w(FUNC(bang_state::bang_clr_gun_int_w));                         /* Gun 1P X */ /* CLR INT Gun */
	map(0x310002, 0x310003).r(FUNC(bang_state::p2_gun_x));                                                                 /* Gun 2P X */
	map(0x310004, 0x310005).r(FUNC(bang_state::p1_gun_y));                                                                 /* Gun 1P Y */
	map(0x310006, 0x310007).r(FUNC(bang_state::p2_gun_y));                                                                 /* Gun 2P Y */
	map(0xfe0000, 0xfeffff).ram();                                                                                         /* Work RAM */
}


static INPUT_PORTS_START( bang )
	PORT_START("P1")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x0004, IP_ACTIVE_LOW ) /* go to service mode NOW */
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read) /* bit 6 is EEPROM data (DOUT) */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_CUSTOM )  /* bit 7 is EEPROM ready */

	PORT_START("LIGHT0_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_X")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("LIGHT0_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, -6.0 / 240, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("LIGHT1_Y")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, -6.0 / 240, 0) PORT_SENSITIVITY(35) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

void bang_state::bang(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, XTAL(30'000'000) / 2); // 15 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &bang_state::bang_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(bang_state::bang_irq), "screen", 0, 1);

	EEPROM_93C66_16BIT(config, m_eeprom);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(gaelco2_state::coin1_counter_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(gaelco2_state::coin2_counter_w));
	m_mainlatch->q_out_cb<4>().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write));   /* EEPROM data */
	m_mainlatch->q_out_cb<5>().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write));  /* EEPROM serial clock */
	m_mainlatch->q_out_cb<6>().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write));   /* EEPROM chip select */

	// Video hardware
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_cg1v_device &gaelco(GAELCO_CG1V(config, "gaelco", XTAL(30'000'000) / 30));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0200000, 1 * 0x0200000, 2 * 0x0200000, 3 * 0x0200000);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}


/*
PCB Layout:

REF: 980921/1
+--------------------------------------------------------------------+
|                                                                    | Plug-In Daughterboard:
|                      TDA1543          KM428C256TR (x2)             | +--------------------------+
|              POT1                                         IC43*  +-| |                          |
+--+                                                               | | | IC1     IC9     IC16   +-|
   |                                                        IC44*  |J| | IC2     IC10    IC17   | |
   |                               +----------+                    |P| | IC3     IC11    IC18   |J| JP1 - 50 pin (dual in-line) header
+--+                               |          |             IC45*  |1| | IC4*    IC12*   IC19*  |P| * Denotes unpopulated sockets
|                                  |  CG-1V   |                    |0| | IC5     IC13    IC20   |1| All roms are 27C040 type
|                     CY7C199      |   388    |             IC46*  | | | IC6*    IC14    IC21   | |
|                     CY7C199      |          |                    | | |                        +-|
| J                                +----------+             IC47*  +-| | 74LS139N   74LS373N x3   |
| A                                                                  | +--------------------------+
| M                                                                  |
| M                                                                  |
| A                                                                  |
|                         30.000MHz                         CY7C199  |
|        93C66N           30.000MHz       +---------+       CY7C199  |
|                                         | Lattice |       CY7C199  |
+--+                                      | IspLSI  |                |
   |                                      |  1016E  |      IC53      |
   |  J J                                 +---------+                |
+--+  P P                               +-----------+       CY7C199  |
|     2 3                               |           |                |
|                                       | MC68HC000 |      IC55      |
|                                       |   FN16    |                |
|                                       +-----------+                |
+--------------------------------------------------------------------+

   CPU: MC68HC000FN16
Custom: CG-1V 388 (System chip, Graphics & Sound)
   OSC: 30.000MHz (x2)
EEPROM: 93C66N

RAM: CY7C199-15PC (IC12, IC13, IC50, IC51, IC52 & IC54)
     KM428C256TR-7 (IC32 & IC33)

* IC42 through IC47 unpopulated

JP10 - 50 pin (dual in-line) header for Plug-In Daughterboard
JP2  - 4 pin light gun header (player 1)
JP3  - 4 pin light gun header (player 2)

*/

ROM_START( bang )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "bang53.ic53", 0x000000, 0x080000, CRC(014bb939) SHA1(bb245acf7a3bd4a56b3559518bcb8d0ae39dbaf4) )
	ROM_LOAD16_BYTE( "bang55.ic55", 0x000001, 0x080000, CRC(582f8b1e) SHA1(c9b0d4c1dee71cdb2c01d49f20ffde32eddc9583) )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) // GFX + Sound
	ROM_LOAD( "bang16.ic16", 0x0000000, 0x0080000, CRC(6ee4b878) SHA1(f646380d95650a60b5a17973bdfd3b80450a4d3b) )   // GFX only
	ROM_LOAD( "bang17.ic17", 0x0080000, 0x0080000, CRC(0c35aa6f) SHA1(df0474b1b9466d3c199e5aade39b7233f0cb45ee) )   // GFX only
	ROM_LOAD( "bang18.ic18", 0x0100000, 0x0080000, CRC(2056b1ad) SHA1(b796f92eef4bbb0efa12c53580e429b8a0aa394c) )   // Sound only
	ROM_FILL(                0x0180000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang9.ic9",   0x0200000, 0x0080000, CRC(078195dc) SHA1(362ff194e2579346dfc7af88559b0718bc36ec8a) )   // GFX only
	ROM_LOAD( "bang10.ic10", 0x0280000, 0x0080000, CRC(06711eeb) SHA1(3662ffe730fb54ee48925de9765f88be1abd5e4e) )   // GFX only
	ROM_LOAD( "bang11.ic11", 0x0300000, 0x0080000, CRC(2088d15c) SHA1(0c043ab9fd33836fa4b7ad60fd8e7cb96ffb6121) )   // Sound only
	ROM_FILL(                0x0380000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang1.ic1",   0x0400000, 0x0080000, CRC(e7b97b0f) SHA1(b5503687ae3ca0a0faa4b867a267d89dac788d6d) )   // GFX only
	ROM_LOAD( "bang2.ic2",   0x0480000, 0x0080000, CRC(ff297a8f) SHA1(28819a9d7b3cb177e7a7db3fe23a94f5cba33049) )   // GFX only
	ROM_LOAD( "bang3.ic3",   0x0500000, 0x0080000, CRC(d3da5d4f) SHA1(b9bea0b4d20ab0bfda3fac2bb1fab974c007aaf0) )   // Sound only
	ROM_FILL(                0x0580000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang20.ic20", 0x0600000, 0x0080000, CRC(a1145df8) SHA1(305cda041a6f201cb011982f1bf1fc6a4153a669) )   // GFX only
	ROM_LOAD( "bang13.ic13", 0x0680000, 0x0080000, CRC(fe3e8d07) SHA1(7a37561b1cf422b47cddb8751a6b6d57dec8baae) )   // GFX only
	ROM_LOAD( "bang5.ic5",   0x0700000, 0x0080000, CRC(9bee444c) SHA1(aebaa3306e7e5aada99ed469da9bf64507808cff) )   // Sound only
	ROM_FILL(                0x0780000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang21.ic21", 0x0800000, 0x0080000, CRC(fd93d7f2) SHA1(ff9d8eb5ac8d9757132aa6d79d2f7662c14cd650) )   // GFX only
	ROM_LOAD( "bang14.ic14", 0x0880000, 0x0080000, CRC(858fcbf9) SHA1(1e67431c8775666f4839bdc427fabf59ffc708c0) )   // GFX only
	ROM_FILL(                0x0900000, 0x0100000, 0x00 )            // Empty

	ROM_REGION( 0x400, "plds", 0)
	ROM_LOAD ( "bang_gal16v8.ic56", 0x000, 0x117, BAD_DUMP CRC(226923ac) SHA1(b1cac5208673183f401702ba844e1016d5fa4ea0) ) // Bruteforced but verified
ROM_END

ROM_START( bangj )
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "bang-a.ic53", 0x000000, 0x080000, CRC(5ee514e9) SHA1(b78b507d18de41be58049f5c597acd107ec1273f) )
	ROM_LOAD16_BYTE( "bang-a.ic55", 0x000001, 0x080000, CRC(b90223ab) SHA1(7c097754a710169f41c574c3cc1a6346824853c4) )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) // GFX + Sound
	ROM_LOAD( "bang-a.ic16", 0x0000000, 0x0080000, CRC(3b63acfc) SHA1(48f5598cdbc70f342d6b75909166571271920a8f) )   // GFX only
	ROM_LOAD( "bang-a.ic17", 0x0080000, 0x0080000, CRC(72865b80) SHA1(ec7753ea7961015149b9e6386fdeb9bd59aa962a) )   // GFX only
	ROM_LOAD( "bang18.ic18", 0x0100000, 0x0080000, CRC(2056b1ad) SHA1(b796f92eef4bbb0efa12c53580e429b8a0aa394c) )   // Sound only
	ROM_FILL(                0x0180000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang-a.ic9",  0x0200000, 0x0080000, CRC(3cb86360) SHA1(c803b3add253a552a1554714218740bdfca91764) )   // GFX only
	ROM_LOAD( "bang-a.ic10", 0x0280000, 0x0080000, CRC(03fdd777) SHA1(9eec194239f93d961ee9902a585c872dcdc7728f) )   // GFX only
	ROM_LOAD( "bang11.ic11", 0x0300000, 0x0080000, CRC(2088d15c) SHA1(0c043ab9fd33836fa4b7ad60fd8e7cb96ffb6121) )   // Sound only
	ROM_FILL(                0x0380000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang-a.ic1",  0x0400000, 0x0080000, CRC(965d0ad9) SHA1(eff521735129b7dd9366855c6312ed568950233c) )   // GFX only
	ROM_LOAD( "bang-a.ic2",  0x0480000, 0x0080000, CRC(8ea261a7) SHA1(50b59cf058ca03c0b8c888f6ddb40c720a210ece) )   // GFX only
	ROM_LOAD( "bang3.ic3",   0x0500000, 0x0080000, CRC(d3da5d4f) SHA1(b9bea0b4d20ab0bfda3fac2bb1fab974c007aaf0) )   // Sound only
	ROM_FILL(                0x0580000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang-a.ic20", 0x0600000, 0x0080000, CRC(4b828f3c) SHA1(5227a89c05c659a85d33f092c6778ce9d57a0236) )   // GFX only
	ROM_LOAD( "bang-a.ic13", 0x0680000, 0x0080000, CRC(d1146b92) SHA1(2b28d49fbffea6c038160fdab177bc0045195ca8) )   // GFX only
	ROM_LOAD( "bang5.ic5",   0x0700000, 0x0080000, CRC(9bee444c) SHA1(aebaa3306e7e5aada99ed469da9bf64507808cff) )   // Sound only
	ROM_FILL(                0x0780000, 0x0080000, 0x00 )            // Empty
	ROM_LOAD( "bang-a.ic21", 0x0800000, 0x0080000, CRC(531ce3b6) SHA1(196bb720591acc082f815b609a7cf1609510c8c1) )   // GFX only
	ROM_LOAD( "bang-a.ic14", 0x0880000, 0x0080000, CRC(f8e1cf84) SHA1(559c08584094e605635c5ef3a25534ea0bcfa199) )   // GFX only
	ROM_FILL(                0x0900000, 0x0100000, 0x00 )            // Empty

	ROM_REGION( 0x117, "plds", 0)
	ROM_LOAD ( "bang_gal16v8.ic56", 0x000, 0x117, BAD_DUMP CRC(226923ac) SHA1(b1cac5208673183f401702ba844e1016d5fa4ea0) ) // Bruteforced but verified
ROM_END


/*============================================================================
                            ALLIGATOR HUNT
  ============================================================================*/


void gaelco2_state::alighunt_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                         /* ROM */
	map(0x200000, 0x20ffff).ram().w(FUNC(gaelco2_state::vram_w)).share("spriteram");                                       /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_gae1_device::gaelcosnd_r), FUNC(gaelco_gae1_device::gaelcosnd_w));    /* Sound Registers */
	map(0x210000, 0x211fff).ram().w(FUNC(gaelco2_state::palette_w)).share("paletteram");                                   /* Palette */
	map(0x218004, 0x218009).ram().w(FUNC(gaelco2_state::vregs_w)).share("vregs");                                          /* Video Registers */
	map(0x300000, 0x300001).portr("IN0");                                                                                  /* DSW #1 + Input 1P */
	map(0x300002, 0x300003).portr("IN1");                                                                                  /* DSW #2 + Input 2P */
	map(0x320000, 0x320001).portr("COIN");                                                                                 /* COINSW + SERVICESW */
	map(0x500000, 0x500001).w(FUNC(gaelco2_state::alighunt_coin_w));                                                       /* Coin lockout + counters */
	map(0x500006, 0x500007).nopw();                                                                                        /* ??? */
	map(0xfe0000, 0xfe7fff).ram();                                                                                         /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                                                                       /* Work RAM (shared with D5002FP) */
}


static INPUT_PORTS_START( alighunt )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0700, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x7000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin A too)" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c00, 0x0c00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(      0x0800, "1" )
	PORT_DIPSETTING(      0x0c00, "2" )
	PORT_DIPSETTING(      0x0400, "3" )
	PORT_DIPSETTING(      0x0000, "4" )
	PORT_DIPNAME( 0x1000, 0x1000, "Sound Type" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Mono ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Stereo ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Joystick ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, "Analog" )        /* TO-DO */
	PORT_DIPSETTING(      0x4000, DEF_STR( Standard ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("COIN")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_SERVICE2 ) /* go to test mode NOW */
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xffc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

void gaelco2_state::alighunt(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);         /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::alighunt_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(30'000'000) / 30));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0400000, 1 * 0x0400000, 2 * 0x0400000, 3 * 0x0400000);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}

void gaelco2_state::alighunt_d5002fp(machine_config &config)
{
	alighunt(config);
	GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2).set_addrmap(0, &gaelco2_state::mcu_hostmem_map); /* 12 MHz */
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");
}

/*
PCB Layout:

REF: 940411
------------------------------------------------------------------------------
|                POT1               KM428C256J-6 (x3)                        |
|                                                                            |
|                POT2                                                        |
|---                                                                         |
   |                                                               U47       |
   |                   30.000MHz          |----------|                       |
|---                                      |          |             U48       |
|                                         | GAE1 449 |                       |
| J                            6264       | (QFP208) |             U49       |
|                              6264       |          |                       |
| A                                       |----------|             U50       |
|                                                                            |
| M                                                                          |
|                         |-------------------------|                        |
| M                       |                         |  24.000MHz     62256   |
|                         |  62256  DS5002  BATT_3V |                62256   |
| A                       |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                    62256                                |
   |                                    62256                                |
   |                                                                         |
|---                                                                         |
|   DSW1                         MC68000P12        U45                       |
|                                                  U44                       |
|   DSW2                                                                     |
|                                                                            |
-----------------------------------------------------------------------------|
*/


/*
    the byte at 0x1ff in the rom at u44 controls the language / region settings
    and even allows for an alt. title of Lizard Hunt

    Bits        Usage
    Bits        Usage
    ---------------------------------------------------------------------------------
    0000 1000   Title (0x00 = LIZARD HUNT, 0x08 = ALLIGATOR HUNT)
    0000 0100   Language (0x00 = SPANISH, 0x04 = ENGLISH)
    0000 0011   Region warning ( 0x00, 0x02 = USA, 0x01 = NOT USA, 0x03 = NO WARNING)
*/

ROM_START( aligator )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "1.u45",  0x000000, 0x080000, CRC(61c47c56) SHA1(6dd3fc6fdab252e0fb43c0793eef70203c888d7f) )
	ROM_LOAD16_BYTE(    "2.u44",  0x000001, 0x080000, CRC(96bc77c2) SHA1(72975fa188598d8ed595cbba097b60efe14bd190) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "aligator_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(6558f215) SHA1(c961a9c81aa6b746294baf83ea5d1fcf7acab9db) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_FILL(               0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "u48",        0x0000000, 0x0400000, CRC(19e03bf1) SHA1(2b3a4bb438b0aebf4f6a9fd26b071e5c9dd222b8) )    /* GFX only */
	ROM_LOAD( "u47",        0x0400000, 0x0400000, CRC(74a5a29f) SHA1(8ea2aa1f8a80c5b88ca9222c5ecc3c4794e0a160) )    /* GFX + Sound */
	ROM_LOAD( "u50",        0x0800000, 0x0400000, CRC(85daecf9) SHA1(824f6d2491075b1ef96ecd6667c5510409338a2f) )    /* GFX only */
	ROM_LOAD( "u49",        0x0c00000, 0x0400000, CRC(70a4ee0b) SHA1(07b09916f0366d0c6eed94a905ec0b9d6ac9e7e1) )    /* GFX + Sound */
ROM_END

/* PCB without Gaelco logos. Gfx and sound on a subboard with 32 EPROMs connected to the main PCB mask ROMs sockets.
   Checksum = B975CB0B */
ROM_START( aligatorp )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "all_27-10_notext.u45",  0x000000, 0x080000, CRC(da2798df) SHA1(528ef26aca57b8cfaa6f82bbf74e6368741d01ea) )
	ROM_LOAD16_BYTE(    "all_27-10_notext.u44",  0x000001, 0x080000, CRC(b2b6cdeb) SHA1(0ce8982711c16e85da4f7b6756c541d3445a8745) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "aligator_ds5002fp_sram_all_27-10_notext.bin", 0x00000, 0x8000, NO_DUMP ) // doesn't work with release version

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	// data 100% matches final version, just different arrangement
	ROM_LOAD( "a0.bin",        0x0000000, 0x0080000, CRC(f6780a0e) SHA1(3dc850744c2129b5b0fe8ab9eb2afda224cff83a) )
	ROM_LOAD( "a1.bin",        0x0080000, 0x0080000, CRC(a59c32a9) SHA1(c50b9252b1be10ee1e48eff4f72d381e543a62c5) )
	ROM_LOAD( "a2.bin",        0x0100000, 0x0080000, CRC(1470030c) SHA1(927f6d45a6b9c9345de543e2416a9c7e6e401159) )
	ROM_LOAD( "a3.bin",        0x0180000, 0x0080000, CRC(b684705a) SHA1(772a7b763fb8e9cf525af4b7f4f0a16493e9d7f9) )
	ROM_LOAD( "a4.bin",        0x0200000, 0x0080000, CRC(73a317fa) SHA1(97804b1e1a9ea65bce2e4da18ec90ded84e31bba) )
	ROM_LOAD( "a5.bin",        0x0280000, 0x0080000, CRC(3fb37680) SHA1(ea5d877e7626828347f1516142a6e47710e723c0) )
	ROM_LOAD( "a6.bin",        0x0300000, 0x0080000, CRC(8034a5f4) SHA1(d51f47794e9c33d883a77ba603ff89899bb815dd) )
	ROM_LOAD( "a7.bin",        0x0380000, 0x0080000, CRC(e49d3d6d) SHA1(1b8471f8a92f7667822af01dbd017a172f66f4fb) )
	ROM_LOAD( "b0.bin",        0x0400000, 0x0080000, CRC(ccd038c1) SHA1(d9b0a7353627fb2d328d62829300fdde6b51e998) )
	ROM_LOAD( "b1.bin",        0x0480000, 0x0080000, CRC(163b3973) SHA1(18c6c639cbc323d9ca776d78f3c9ed4bc7cf778a) )
	ROM_LOAD( "b2.bin",        0x0500000, 0x0080000, CRC(da2125fb) SHA1(58822e9d7188d7aa436cefaf7fc1585c8efd8c1d) )
	ROM_LOAD( "b3.bin",        0x0580000, 0x0080000, CRC(8b926c7e) SHA1(32e7bf25d2afabb8cff7da9288b8d1ba93d29ef3) )
	ROM_LOAD( "b4.bin",        0x0600000, 0x0080000, CRC(82b807ce) SHA1(60d5b4df5e733b2be9dc5374e2232204ed9d75d1) )
	ROM_LOAD( "b5.bin",        0x0680000, 0x0080000, CRC(58dc1b44) SHA1(cffa7a77c9d944ea1f4f63042a9daceb627518a9) )
	ROM_LOAD( "b6.bin",        0x0700000, 0x0080000, CRC(778e79de) SHA1(158f751975b4bacd3553d592da53cfa504dc6749) )
	ROM_LOAD( "b7.bin",        0x0780000, 0x0080000, CRC(9734fd7e) SHA1(154398b51c97a621d37a41a5133c1d80f5229cc1) )
	ROM_LOAD( "c0.bin",        0x0800000, 0x0080000, CRC(a86d0718) SHA1(39d0ddf5cde5eea6367fa7b1fd895f23a112651e) )
	ROM_LOAD( "c1.bin",        0x0880000, 0x0080000, CRC(ccba9472) SHA1(c7fc8a5340ba560ab51d72a12eccfae78c451cbd) )
	ROM_LOAD( "c2.bin",        0x0900000, 0x0080000, CRC(3ccd59b9) SHA1(b1e72db51f5fe953a4edcace001aa1d5fe83e113) )
	ROM_LOAD( "c3.bin",        0x0980000, 0x0080000, CRC(16ed8ffb) SHA1(18733d6fde5641e317cd9727d556cac929e17170) )
	ROM_LOAD( "c4.bin",        0x0a00000, 0x0080000, CRC(b0106f8d) SHA1(c9a806dc9214ac28f2f88307263d364740b08a66) )
	ROM_LOAD( "c5.bin",        0x0a80000, 0x0080000, CRC(305b798f) SHA1(aac9afe801fdcf0fce1858dadbb5d909ea8ac43b) )
	ROM_LOAD( "c6.bin",        0x0b00000, 0x0080000, CRC(7dd38c7a) SHA1(9564041dbda306f40fee17283a634b6e05c49830) )
	ROM_LOAD( "c7.bin",        0x0b80000, 0x0080000, CRC(5413c9f0) SHA1(633276e82be4e49043869166a67e0db10d205f86) )
	ROM_LOAD( "d0.bin",        0x0c00000, 0x0080000, CRC(5c362787) SHA1(700811da92b1100db7edc33dfb138cc58111f08a) )
	ROM_LOAD( "d1.bin",        0x0c80000, 0x0080000, CRC(131dc831) SHA1(31284be8cc9defe740840b85848fedb8d177eb5f) )
	ROM_LOAD( "d2.bin",        0x0d00000, 0x0080000, CRC(d820af09) SHA1(97244cee2f36493173357e29dad660fd7f2b4e2e) )
	ROM_LOAD( "d3.bin",        0x0d80000, 0x0080000, CRC(39d7ea9e) SHA1(3f1203e5da16360e717404dbbf48a231eaab38f6) )
	ROM_LOAD( "d4.bin",        0x0e00000, 0x0080000, CRC(ccfdc8b4) SHA1(b9bb82e9c150e3fdd839561251bfc1742e6fdbae) )
	ROM_LOAD( "d5.bin",        0x0e80000, 0x0080000, CRC(f4151d83) SHA1(08dafbc2b9e8e89a1bb76778afdae711bf07b431) )
	ROM_LOAD( "d6.bin",        0x0f00000, 0x0080000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) ) // blank ROM (but correct)
	ROM_LOAD( "d7.bin",        0x0f80000, 0x0080000, CRC(67ae054e) SHA1(96210a4ee472abf58b4af9f35db849268e0a5c87) )
	ROM_FILL(                  0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */
ROM_END

ROM_START( aligators )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "u45",  0x000000, 0x080000, CRC(61c47c56) SHA1(6dd3fc6fdab252e0fb43c0793eef70203c888d7f) )
	ROM_LOAD16_BYTE(    "u44",  0x000001, 0x080000, CRC(f0be007a) SHA1(2112b2e5f020028b50c8f2c72c83c9fee7a78224) ) /* differs by 1 byte from above set, see note */

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "aligator_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(6558f215) SHA1(c961a9c81aa6b746294baf83ea5d1fcf7acab9db) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_FILL(               0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "u48",        0x0000000, 0x0400000, CRC(19e03bf1) SHA1(2b3a4bb438b0aebf4f6a9fd26b071e5c9dd222b8) )    /* GFX only */
	ROM_LOAD( "u47",        0x0400000, 0x0400000, CRC(74a5a29f) SHA1(8ea2aa1f8a80c5b88ca9222c5ecc3c4794e0a160) )    /* GFX + Sound */
	ROM_LOAD( "u50",        0x0800000, 0x0400000, CRC(85daecf9) SHA1(824f6d2491075b1ef96ecd6667c5510409338a2f) )    /* GFX only */
	ROM_LOAD( "u49",        0x0c00000, 0x0400000, CRC(70a4ee0b) SHA1(07b09916f0366d0c6eed94a905ec0b9d6ac9e7e1) )    /* GFX + Sound */
ROM_END

ROM_START( aligatorun )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "ahntu45n.040", 0x000000, 0x080000, CRC(fc02cb2d) SHA1(700aa60ec0d2bb705b1335de63daae678dcb8570) )
	ROM_LOAD16_BYTE(    "ahntu44n.040", 0x000001, 0x080000, CRC(7fbea3a3) SHA1(89efa5b7908c2f010a3097954dbccd9cb7adc50c) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_FILL(               0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "u48",        0x0000000, 0x0400000, CRC(19e03bf1) SHA1(2b3a4bb438b0aebf4f6a9fd26b071e5c9dd222b8) )    /* GFX only */
	ROM_LOAD( "u47",        0x0400000, 0x0400000, CRC(74a5a29f) SHA1(8ea2aa1f8a80c5b88ca9222c5ecc3c4794e0a160) )    /* GFX + Sound */
	ROM_LOAD( "u50",        0x0800000, 0x0400000, CRC(85daecf9) SHA1(824f6d2491075b1ef96ecd6667c5510409338a2f) )    /* GFX only */
	ROM_LOAD( "u49",        0x0c00000, 0x0400000, CRC(70a4ee0b) SHA1(07b09916f0366d0c6eed94a905ec0b9d6ac9e7e1) )    /* GFX + Sound */
ROM_END

ROM_START( aligatoruna )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "stm27c4001.45", 0x000000, 0x080000, CRC(a70301b8) SHA1(b6ffb7339a42ec81c3ec7a0681dfea878f11a538) )
	ROM_LOAD16_BYTE(    "am27c040.44",   0x000001, 0x080000, CRC(d45a26ed) SHA1(bb261e7061aba35aa6af6567a8386d9704a9db83) )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_FILL(               0x1000000, 0x0400000, 0x00 )     /* to decode GFX as 5 bpp */

	ROM_REGION( 0x1000000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "u48",        0x0000000, 0x0400000, CRC(19e03bf1) SHA1(2b3a4bb438b0aebf4f6a9fd26b071e5c9dd222b8) )    /* GFX only */
	ROM_LOAD( "u47",        0x0400000, 0x0400000, CRC(74a5a29f) SHA1(8ea2aa1f8a80c5b88ca9222c5ecc3c4794e0a160) )    /* GFX + Sound */
	ROM_LOAD( "u50",        0x0800000, 0x0400000, CRC(85daecf9) SHA1(824f6d2491075b1ef96ecd6667c5510409338a2f) )    /* GFX only */
	ROM_LOAD( "u49",        0x0c00000, 0x0400000, CRC(70a4ee0b) SHA1(07b09916f0366d0c6eed94a905ec0b9d6ac9e7e1) )    /* GFX + Sound */
ROM_END


/*============================================================================
                            TOUCH & GO
  ============================================================================*/


void gaelco2_state::touchgo_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                         /* ROM */
	map(0x200000, 0x20ffff).ram().w(FUNC(gaelco2_state::vram_w)).share("spriteram");                                       /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_gae1_device::gaelcosnd_r), FUNC(gaelco_gae1_device::gaelcosnd_w));    /* Sound Registers */
	map(0x210000, 0x211fff).ram().w(FUNC(gaelco2_state::palette_w)).share("paletteram");                                   /* Palette */
	map(0x218004, 0x218009).ram().w(FUNC(gaelco2_state::vregs_w)).share("vregs");                                          /* Video Registers */
	map(0x300000, 0x300001).portr("IN0");                                                                                  /* DSW #1 + Input 1P */
	map(0x300002, 0x300003).portr("IN1");                                                                                  /* DSW #2 + Input 2P */
	map(0x300004, 0x300005).portr("IN2");                                                                                  /* COINSW + Input 3P */
	map(0x300006, 0x300007).portr("IN3");                                                                                  /* SERVICESW + Input 4P */
	map(0x500000, 0x500001).select(0x0038).w(FUNC(gaelco2_state::wrally2_latch_w));                                        /* Coin counters */
	map(0xfe0000, 0xfe7fff).ram();                                                                                         /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                                                                       /* Work RAM (shared with D5002FP) */
}


static INPUT_PORTS_START( touchgo )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x0300, 0x0300, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0200, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Credit configuration" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0400, "1 Credit Start/1 Credit Continue" )
	PORT_DIPSETTING(      0x0000, "2 Credits Start/1 Credit Continue" )
	PORT_DIPNAME( 0x0800, 0x0800, "Coin Slot" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0800, "Independent" )
	PORT_DIPSETTING(      0x0000, "Common" )
	PORT_DIPNAME( 0x3000, 0x0000, "Monitor Type" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(      0x0000, "Double monitor, 4 players" )
	PORT_DIPSETTING(      0x2000, "Single monitor, 4 players" )
	PORT_DIPSETTING(      0x3000, "Single monitor, 2 players" )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x8000, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x0f00, 0x0f00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin B too)" ) PORT_DIPLOCATION("SW1:1,2,3,4")
	PORT_DIPSETTING(      0x0300, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0900, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0x0f00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0e00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0x0d00, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0b00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0a00, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf000, 0xf000, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(      0x0000, "Disabled or Free Play (if Coin A too)" ) PORT_DIPLOCATION("SW1:5,6,7,8")
	PORT_DIPSETTING(      0x3000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x9000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x5000, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(      0xf000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 4C_5C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x7000, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(      0xd000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0xb000, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_6C ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE3 ) /* go to test mode NOW */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("FAKE")  /* To switch between monitors at run time */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_TOGGLE
INPUT_PORTS_END

void gaelco2_state::touchgo(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(32'000'000) / 2); /* 16 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::touchgo_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(gaelco2_state::irq6_line_hold));

	LS259(config, m_mainlatch); // IC6
	m_mainlatch->q_out_cb<0>().set(FUNC(gaelco2_state::coin1_counter_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(gaelco2_state::coin2_counter_w));
	m_mainlatch->q_out_cb<2>().set(FUNC(gaelco2_state::coin3_counter_w));
	m_mainlatch->q_out_cb<3>().set(FUNC(gaelco2_state::coin4_counter_w));

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */
	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(59.1);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	lscreen.set_size(64*16, 32*16);
	lscreen.set_visarea(0, 480-1, 16, 256-1);
	lscreen.set_screen_update(FUNC(gaelco2_state::screen_update_left));
	lscreen.set_palette(m_palette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(59.1);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	rscreen.set_size(64*16, 32*16);
	rscreen.set_visarea(0, 480-1, 16, 256-1);
	rscreen.set_screen_update(FUNC(gaelco2_state::screen_update_right));
	rscreen.screen_vblank().set("spriteram", FUNC(buffered_spriteram16_device::vblank_copy_rising));
	rscreen.set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2_dual)

	/* sound hardware */
	/* the chip is stereo, but the game sound is mono because the right channel
	   output is for cabinet 1 and the left channel output is for cabinet 2 */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(40'000'000) / 40));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0400000, 1 * 0x0400000, 0, 0);
	gaelco.add_route(0, "rspeaker", 1.0);
	gaelco.add_route(1, "lspeaker", 1.0);
}

void gaelco2_state::touchgo_d5002fp(machine_config &config)
{
	touchgo(config);
	GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(32'000'000) / 2).set_addrmap(0, &gaelco2_state::mcu_hostmem_map); /* 16 MHz */
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");
}

/*
PCB Layout:

REF: 950510-1
------------------------------------------------------------------------------
|                POT1          PAL20L8       KM428C256J-6 (x4)               |
|                POT2                          +--------------------------+  |
|                                              | (Plug-In Daughterboard)  |  |
|                                              |                          |  |
|---                                           |     IC66        IC67     |  |
   |                                           |                          |  |
   |                                           |                          |  |
|---                                           |     IC65        IC69     |  |
|                                              |                          |  |
|                                              +--------------------------+  |
|                                                                            |
|                                            |----------|                    |
| J                                          |          |                    |
|                                            | GAE1 501 |                    |
| A                               6264       | (QFP208) |                    |
|                                 6264       |          |                    |
| M                                          |----------|                    |
|                                                                            |
| M                       |-------------------------|                        |
|                         |                         |  40.000MHz    CY7C199  |
| A                       |  62256  DS5002  BATT_3V |               CY7C199  |
|                         |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                                                         |
   |                                   CY7C199-20PC                          |
   |  DSW1                             CY7C199-20PC                          |
|---  DSW2             PAL16L8                                               |
| J J                                                                        |
| P P                            32.000MHz      MC68000P16        1.IC63     |
| 1 4                                                             2.IC64     |
|      JP2   JP3                                                             |
-----------------------------------------------------------------------------|

REF: 950906
------------------------------------------------------------------------------
|                POT1                        KM428C256J-6 (x4)               |
|                POT2                                                        |
|                                                                            |
|                                             |----------|    TG IC65.IC65 |-|
|---                                          |          |                 | |
   |                                          | GAE1 506 |    TG IC66.IC66 | |
|---                                          | (QFP208) |                 |J|
|                                             |          |    TG IC67.IC67 |P|
|                                      6264   |----------|                 |6|
|                                      6264                         IC68*  | |
| J                                                                        | |
| A                       |-------------------------|         TG IC69.IC69 |-|
| M                       |                         |                        |
| M                       |  62256  DS5002  BATT_3V |                        |
| A                       |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|   IC1* IC7*                           62256                       62256    |
|---                                    62256                       62256    |
   |  DSW1                                            40.000MHz              |
|---  DSW2                         |-----------|                             |
|                                  |           |                             |
| J J                32.000MHz     |MC68000FN16|    TG 57.IC57               |
| P P                              |           |    TG 56.IC56               |
| 1 4  JP2   JP3                   |-----------|                             |
-----------------------------------------------------------------------------|

Notes
-----
IC1, IC7 - 8 pin sockets marked "TLC548" - Not populated
IC68 - 42 pin 32M socket - Not populated
JP6: 50 pin connector for daughter card - Not populated
POT1: Volume adjust for cabinet 1
POT2: Volume adjust for cabinet 2

JP1: 6 pin connector - Video signals for cabinet 2
  1| Video
  2| No Connection
  3| Video GND
  4| Video Blue
  5| Video Green
  6| Video Red
JP4: 4 pin connector - Sound for cabinet 2
  1| Speaker (+)
  2| Speaker (-)
  3| No Connection
  4| Audio GND
JP2: 15 pin connector - pin out unknown - Players 3 & 4 controls for cabinet 2
JP3: 15 pin connector - pin out unknown - Players 3 & 4 controls for cabinet 2

NOTE: It's unknown if Player 1 & Player 2 controls are connected through the JAMMA harness for cabinet 1
      and controls for Player 3 & Player 4 are connected through JP2 & JP3 and what the pin outs are.

Wires run to male JAMMA board with corresponding JP1, JP2, JP3 & JP4 connectors for cabinet 2 JAMMA harness
*/


ROM_START( touchgo ) /* REF: 950906 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tg_56.ic56", 0x000000, 0x080000, CRC(8ab065f3) SHA1(7664abd7e5f66ffca4a2865bba56ac36bd04f4e9) )
	ROM_LOAD16_BYTE( "tg_57.ic57", 0x000001, 0x080000, CRC(0dfd3f65) SHA1(afb2ce8988c84f211ac71b84928ce4c421de7fee) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "touchgo_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(6a238adb) SHA1(4ac5ff8e3d90454f764477146a0b8dc8c8062420) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* touchgo requires some values in scratchram to be initialized or it won't copy the high score table when it boots */
	ROM_LOAD( "touchgo_scratch", 0x00, 0x80, CRC(f9ca54ff) SHA1(416f7bd89442dc1f736efe457b0f9a7f4f9f0bd5) )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "tg_ic69.ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "tg_ic65.ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "tg_ic66.ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(                  0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "tg_ic67.ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END

ROM_START( touchgon ) /* REF 950906, no plug-in daughterboard, Non North America Notice, also found on REF: 950510-1 with daughterboard */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "1.ic63", 0x000000, 0x080000, CRC(fd3b4642) SHA1(3cab42aecad5ee641711763c6047b56784c2bcf3) ) /* IC63 for REF: 950510-1, IC56 for REF: 950906 */
	ROM_LOAD16_BYTE( "2.ic64", 0x000001, 0x080000, CRC(ee891835) SHA1(9f8c60e5e3696b70f756c3521e10313005053cc7) ) /* IC64 for REF: 950510-1, IC57 for REF: 950906 */

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "touchgo_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(6a238adb) SHA1(4ac5ff8e3d90454f764477146a0b8dc8c8062420) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* touchgo requires some values in scratchram to be initialized or it won't copy the high score table when it boots */
	ROM_LOAD( "touchgo_scratch", 0x00, 0x80, CRC(f9ca54ff) SHA1(416f7bd89442dc1f736efe457b0f9a7f4f9f0bd5) )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "tg_ic69.ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "tg_ic65.ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "tg_ic66.ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(                  0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "tg_ic67.ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END

ROM_START( touchgoe ) /* REF: 950510-1 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "tg56.ic63", 0x000000, 0x080000, CRC(6d0f5c65) SHA1(00db7a7da3ec1676169aa78fe4f08a7746c3accf) ) /* IC63 for REF: 950510-1, IC56 for REF: 950906 */
	ROM_LOAD16_BYTE( "tg57.ic64", 0x000001, 0x080000, CRC(845787b5) SHA1(27c9910cd9f38328326ecb5cd093dfeb6d4f6244) ) /* IC64 for REF: 950510-1, IC57 for REF: 950906 */

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "touchgo_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(6a238adb) SHA1(4ac5ff8e3d90454f764477146a0b8dc8c8062420) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	/* touchgo requires some values in scratchram to be initialized or it won't copy the high score table when it boots */
	ROM_LOAD( "touchgo_scratch", 0x00, 0x80, CRC(f9ca54ff) SHA1(416f7bd89442dc1f736efe457b0f9a7f4f9f0bd5) )
	/* these are the default states stored in NVRAM */
	DS5002FP_SET_MON( 0x19 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "tg_ic69.ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "tg_ic65.ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "tg_ic66.ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(                  0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "tg_ic67.ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END

ROM_START( touchgok ) /* REF: 950510-1 - ds5002fp unpopulated, game is unprotected */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "56.ic56", 0x000000, 0x080000, CRC(cbb87505) SHA1(f19832af60fb6273c3263ebdd93bb7705ab61e20) ) /* IC63 for REF: 950510-1, IC56 for REF: 950906 */
	ROM_LOAD16_BYTE( "57.ic57", 0x000001, 0x080000, CRC(36bcc7e7) SHA1(2fff881ba0a99ebcfe3c03fdc61f4bf40e152c7f) ) /* IC64 for REF: 950510-1, IC57 for REF: 950906 */

	ROM_REGION( 0x1400000, "gfx1", 0 ) /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "tg_ic69.ic69",  0x1000000, 0x0200000, CRC(18bb12d4) SHA1(ee6e7a63b86c56d71e62db0ae5892ab3ab94b0a0) ) /* GFX only */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "tg_ic65.ic65",  0x0000000, 0x0400000, CRC(91b89c7c) SHA1(1c24b494b56845b0f21be40ab737f251d7683c7d) ) /* GFX only */
	ROM_LOAD( "tg_ic66.ic66",  0x0400000, 0x0200000, CRC(52682953) SHA1(82cde061bdd827ed4a47a9a4256cd0e887ebc29d) ) /* Sound only */
	ROM_FILL(                  0x0600000, 0x0200000, 0x00 )          /* Empty */
	ROM_LOAD( "tg_ic67.ic67",  0x0800000, 0x0400000, CRC(c0a2ce5b) SHA1(94b024373c7c546c0f4fe9737639f02e9c7ebbdb) ) /* GFX only */
ROM_END

/*============================================================================
                            SNOW BOARD
  ============================================================================*/

void gaelco2_state::snowboar_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                                                /* ROM */
	map(0x200000, 0x20ffff).ram().w(FUNC(gaelco2_state::vram_w)).share("spriteram");                                                              /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_cg1v_device::gaelcosnd_r), FUNC(gaelco_cg1v_device::gaelcosnd_w));                           /* Sound Registers */
	map(0x210000, 0x211fff).ram().w(FUNC(gaelco2_state::palette_w)).share("paletteram");                                                          /* Palette */
	map(0x212000, 0x213fff).ram();                                                                                                                /* Extra RAM */
	map(0x218004, 0x218009).ram().w(FUNC(gaelco2_state::vregs_w)).share("vregs");                                                                 /* Video Registers */
	map(0x300000, 0x300001).portr("P1");
	map(0x300000, 0x30000f).w(m_mainlatch, FUNC(ls259_device::write_d0)).umask16(0x00ff);                                                         /* Coin Counters & serial EEPROM */
	map(0x300010, 0x300011).portr("P2");
	map(0x300020, 0x300021).portr("COIN");
	map(0x310000, 0x31ffff).rw(FUNC(gaelco2_state::snowboar_protection_r), FUNC(gaelco2_state::snowboar_protection_w)).share("snowboar_prot");    /* Protection */
	map(0xfe0000, 0xfeffff).ram();                                                                                                                /* Work RAM */
}


static INPUT_PORTS_START( snowboar )
	PORT_START("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0xff80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   /* go to service mode NOW */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)   /* bit 6 is EEPROM data (DOUT) */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM )
INPUT_PORTS_END

void gaelco2_state::snowboar(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(30'000'000) / 2);         /* 15 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::snowboar_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	EEPROM_93C66_16BIT(config, m_eeprom);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(gaelco2_state::coin1_counter_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(gaelco2_state::coin2_counter_w));
	m_mainlatch->q_out_cb<4>().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write));   /* EEPROM data */
	m_mainlatch->q_out_cb<5>().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write));  /* EEPROM serial clock */
	m_mainlatch->q_out_cb<6>().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write));   /* EEPROM chip select */

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 384-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_cg1v_device &gaelco(GAELCO_CG1V(config, "gaelco", XTAL(34'000'000) / 34));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0400000, 1 * 0x0400000, 0, 0);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}

void gaelco2_state::maniacsqs(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000) / 2);         /* 12 MHz - see PCB layout above with ROM set */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco2_state::snowboar_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco2_state::irq6_line_hold));

	EEPROM_93C66_16BIT(config, m_eeprom);

	LS259(config, m_mainlatch);
	m_mainlatch->q_out_cb<0>().set(FUNC(gaelco2_state::coin1_counter_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(gaelco2_state::coin2_counter_w));
	m_mainlatch->q_out_cb<4>().set("eeprom", FUNC(eeprom_serial_93cxx_device::di_write));   /* EEPROM data */
	m_mainlatch->q_out_cb<5>().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write));  /* EEPROM serial clock */
	m_mainlatch->q_out_cb<6>().set("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write));   /* EEPROM chip select */

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.1);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(64*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco2_state::screen_update));
	screen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(30'000'000) / 30));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0080000, 1 * 0x0080000, 0, 0);
	gaelco.add_route(0, "lspeaker", 1.0);
	gaelco.add_route(1, "rspeaker", 1.0);
}


/*
PCB Layout:

REF: 960419/1
------------------------------------------------------------------------------
|                                                                            |
|                                               KM428C256J-6                 |
|                POT1                                                        |
|---                                            KM428C256J-6                 |
   |         SW2                                                   IC43      |
   |                   34.000MHz          |----------|                       |
|---        93C66                         |          |             IC44      |
|                                         |  CG-1V   |                       |
| J                                       |   366    |             IC45      |
|                                         |          |                       |
| A                            6264       |----------|             IC46      |
|                              6264                                          |
| M                                                                IC47*     |
|                                                                            |
| M                                                                  62256   |
|                                                                    62256   |
| A                                          |----------|            62256   |
|                                30.000MHz   | Lattice  |                    |
|---                                         | IspLSI   |                    |
   |                                         |   1016   |          SB53      |
   |                                         |----------|                    |
|---                                        |------------|           62256   |
|                                           |            |                   |
|                                           |  MC68HC000 |         SB55      |
|                                           |    FN16    |                   |
|                                           |------------|                   |
-----------------------------------------------------------------------------|

* Denotes unpopulated socket

PCB sets with rom board attached to the underside of the main PCB use CG-1V 0797 instead
*/

ROM_START( snowboara ) // REF 960419/1
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "sb_53.ic53", 0x000000, 0x080000, CRC(e4eaefd4) SHA1(c7de2ae3a4a919fbe16d4997e3f9e2303b8c96b1) ) /* Version 2.0 program roms */
	ROM_LOAD16_BYTE( "sb_55.ic55", 0x000001, 0x080000, CRC(e2476994) SHA1(2ad18652a1fc6ac058c8399373fb77e7a81d5bbd) ) /* Version 2.0 program roms */

	ROM_REGION( 0x1400000, "gfx1", 0 )  /* GFX + Sound */
	/* 0x0000000-0x0ffffff filled in in the DRIVER_INIT */
	ROM_LOAD( "sb_ic43.ic43", 0x1000000, 0x0200000, CRC(afce54ed) SHA1(1d2933d64790612918adbaabcd2a82dad79953c9) )    /* GFX only */
	ROM_FILL(                 0x1200000, 0x0200000, 0x00 )         /* Empty */

	ROM_REGION( 0x0c00000, "gfx2", 0 ) /* Temporary storage */
	ROM_LOAD( "sb_ic44.ic44", 0x0000000, 0x0400000, CRC(1bbe88bc) SHA1(15bce9ada2b742ba4d537fa8efc0f29f661bff00) )    /* GFX only */
	ROM_LOAD( "sb_ic45.ic45", 0x0400000, 0x0400000, CRC(373983d9) SHA1(05e35a8b27cab469885f0ec2a5df200a366b50a1) )    /* Sound only */
	ROM_LOAD( "sb_ic46.ic46", 0x0800000, 0x0400000, CRC(22e7c648) SHA1(baddb9bc13accd83bea61533d7286cf61cd89279) )    /* GFX only */
ROM_END

ROM_START( snowboar )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "sb.53",    0x000000, 0x080000, CRC(4742749e) SHA1(933e39893ab74895ae4a99a932f8245a03ea0b5d) ) /* Version 2.1 program roms */
	ROM_LOAD16_BYTE(    "sb.55",    0x000001, 0x080000, CRC(6ddc431f) SHA1(8801c0cf1711bb956447ba1e631db28bd075caea) ) /* Version 2.1 program roms */

	ROM_REGION( 0x1400000, "gfx1", 0 )  /* GFX + Sound */
	ROM_LOAD( "sb.a0",      0x0000000, 0x0080000, CRC(aa476e44) SHA1(2b87689489b9619e9e5ca32c3e3d2aec8ef31c88) )    /* GFX only */
	ROM_LOAD( "sb.a1",      0x0080000, 0x0080000, CRC(6bc99195) SHA1(276e9383fac9cb5141b23ffdf381b0d7e60a6861) )    /* GFX only */
	ROM_LOAD( "sb.a2",      0x0100000, 0x0080000, CRC(fae2ebba) SHA1(653a12846abe4de36f5565c3bf849fce7c2893b6) )    /* GFX only */
	ROM_LOAD( "sb.a3",      0x0180000, 0x0080000, CRC(17ed9cf8) SHA1(c6cab61bbba3b2b1d06b64a68313946299205cc5) )    /* GFX only */
	ROM_LOAD( "sb.a4",      0x0200000, 0x0080000, CRC(2ba3a5c8) SHA1(93de0382cbb41806ae3349ce7cfecdc1404bfb88) )    /* Sound only */
	ROM_LOAD( "sb.a5",      0x0280000, 0x0080000, CRC(ae011eb3) SHA1(17223404640c55637364fa6e51cf07d8e64df085) )    /* Sound only */
	ROM_FILL(               0x0300000, 0x0100000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.b0",      0x0400000, 0x0080000, CRC(96c714cd) SHA1(c6225c43b88531a70436cc8a631b8ba401903e45) )    /* GFX only */
	ROM_LOAD( "sb.b1",      0x0480000, 0x0080000, CRC(39a4c30c) SHA1(4598a68ef41483ba372aa3a40383de8eb70d706e) )    /* GFX only */
	ROM_LOAD( "sb.b2",      0x0500000, 0x0080000, CRC(b58fcdd6) SHA1(21a8c00778be77165f89421fb2e3123244cf02c6) )    /* GFX only */
	ROM_LOAD( "sb.b3",      0x0580000, 0x0080000, CRC(96afdebf) SHA1(880cfb365efa93bbee882aeb483ad6d75d8b7430) )    /* GFX only */
	ROM_LOAD( "sb.b4",      0x0600000, 0x0080000, CRC(e62cf8df) SHA1(8df8df45d99967e52dcec5b589246799f7a39601) )    /* Sound only */
	ROM_LOAD( "sb.b5",      0x0680000, 0x0080000, CRC(caa90856) SHA1(a8f18a878b211366faaf66911c09d0452770cc3f) )    /* Sound only */
	ROM_FILL(               0x0700000, 0x0100000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.c0",      0x0800000, 0x0080000, CRC(c9d57a71) SHA1(4e8b7d821e31afc0750db283470f9c76bceb54da) )    /* GFX only */
	ROM_LOAD( "sb.c1",      0x0880000, 0x0080000, CRC(1d14a3d4) SHA1(eb89cadfe331f77dbc0463151574ba801c248238) )    /* GFX only */
	ROM_LOAD( "sb.c2",      0x0900000, 0x0080000, CRC(55026352) SHA1(7b92f45624dbd122c29e44f82c3c2ffded190efa) )    /* GFX only */
	ROM_LOAD( "sb.c3",      0x0980000, 0x0080000, CRC(d9b62dee) SHA1(409ab4d9a6f9341cf59510c130c705d1ec42d1b3) )    /* GFX only */
	ROM_FILL(               0x0a00000, 0x0200000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.d0",      0x0c00000, 0x0080000, CRC(7434c1ae) SHA1(8e0e6567a461c694a8ba2de5d4cf9ad73e0c83c8) )    /* GFX only */
	ROM_LOAD( "sb.d1",      0x0c80000, 0x0080000, CRC(f00cc6c8) SHA1(b4835e2187e1a985993471d09495cbc1f5cd9417) )    /* GFX only */
	ROM_LOAD( "sb.d2",      0x0d00000, 0x0080000, CRC(019f9aec) SHA1(1a97b84ebbf57e860792ef7a7dc6f51553ae3e26) )    /* GFX only */
	ROM_LOAD( "sb.d3",      0x0d80000, 0x0080000, CRC(d05bd286) SHA1(9eff6f5a4755375b7a16b9d4967a0df933e1b9c4) )    /* GFX only */
	ROM_FILL(               0x0e00000, 0x0200000, 0x00 )         /* Empty */
	ROM_LOAD( "sb.e0",      0x1000000, 0x0080000, CRC(e6195323) SHA1(5ba1cb750dd8cfd0721905174bda6cfbf8c8e694) )    /* GFX only */
	ROM_LOAD( "sb.e1",      0x1080000, 0x0080000, CRC(9f38910b) SHA1(0243c19c7b1bdd3361fc6e177c64528bacafcc33) )    /* GFX only */
	ROM_LOAD( "sb.e2",      0x1100000, 0x0080000, CRC(f5948c6c) SHA1(91bba817ced194b02885ce84b7a8132ef5ca631a) )    /* GFX only */
	ROM_LOAD( "sb.e3",      0x1180000, 0x0080000, CRC(4baa678f) SHA1(a7fbbd687e2d8d7e96207c8ace0799a3cc9c3272) )    /* GFX only */
	ROM_FILL(               0x1200000, 0x0200000, 0x00 )         /* Empty */
ROM_END

/*============================================================================
                            WORLD RALLY 2
  ============================================================================*/

/***************************************************************************

    World Rally 2 analog controls
    - added by Mirko Mattioli <els@fastwebnet.it>
    ---------------------------------------------------------------
    WR2 pcb has two ADC, one for each player. The ADCs have in common
    the clock signal line (adc_clk) and the chip enable signal line
    (adc_cs) and, of course,  two different data out signal lines.
    When "Pot Wheel" option is selected via dip-switch, then the gear
    is enabled (low/high shifter); the gear is disabled in joy mode by
    the CPU program code. No brakes are present in this game.
    Analog controls routines come from modified code wrote by Aaron
    Giles for gaelco3d driver.

***************************************************************************/

template <int N>
READ_LINE_MEMBER(wrally2_state::wrally2_analog_bit_r)
{
	return (m_analog_ports[N] >> 7) & 0x01;
}


WRITE_LINE_MEMBER(wrally2_state::wrally2_adc_clk)
{
	/* a zero/one combo is written here to clock the next analog port bit */
	if (!state)
	{
		m_analog_ports[0] <<= 1;
		m_analog_ports[1] <<= 1;
	}
}


WRITE_LINE_MEMBER(wrally2_state::wrally2_adc_cs)
{
	/* a zero is written here to read the analog ports, and a one is written when finished */
	if (!state)
	{
		m_analog_ports[0] = m_analog0->read();
		m_analog_ports[1] = m_analog1->read();
	}
}

void wrally2_state::wrally2_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                                                         /* ROM */
	map(0x200000, 0x20ffff).ram().w(FUNC(wrally2_state::vram_w)).share("spriteram");                                       /* Video RAM */
	map(0x202890, 0x2028ff).rw("gaelco", FUNC(gaelco_gae1_device::gaelcosnd_r), FUNC(gaelco_gae1_device::gaelcosnd_w));    /* Sound Registers */
	map(0x210000, 0x211fff).ram().w(FUNC(wrally2_state::palette_w)).share("paletteram");                                   /* Palette */
	map(0x212000, 0x213fff).ram();                                                                                         /* Extra RAM */
	map(0x218004, 0x218009).ram().w(FUNC(wrally2_state::vregs_w)).share("vregs");                                          /* Video Registers */
	map(0x300000, 0x300001).portr("IN0");                                                                                  /* DIPSW #2 + Inputs 1P */
	map(0x300002, 0x300003).portr("IN1");                                                                                  /* DIPSW #1 */
	map(0x300004, 0x300005).portr("IN2");                                                                                  /* Inputs 2P + COINSW */
	map(0x300006, 0x300007).portr("IN3");                                                                                  /* SERVICESW */
	map(0x400000, 0x400001).select(0x0038).w(FUNC(wrally2_state::wrally2_latch_w));                                        /* Coin counters, etc. */
	map(0xfe0000, 0xfe7fff).ram();                                                                                         /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                                                                       /* Work RAM (shared with D5002FP) */
}


static INPUT_PORTS_START( wrally2 )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Acc.")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Gear") PORT_TOGGLE
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(wrally2_state, wrally2_analog_bit_r<0>)   /* ADC_1 serial input */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_SERVICE_DIPLOC(  0x0100, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x0200, 0x0000, "Coin mechanism" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(      0x0000, "Common" )
	PORT_DIPSETTING(      0x0200, "Independent" )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, "Cabinet 1 Control Configuration" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(      0x0000, "Pot Wheel" )
	PORT_DIPSETTING(      0x0800, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x1000, 0x1000, "Cabinet 2 Control Configuration" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, "Pot Wheel" )
	PORT_DIPSETTING(      0x1000, DEF_STR( Joystick ) )
	PORT_DIPNAME( 0x2000, 0x2000, "Monitors" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, "One" )
	PORT_DIPSETTING(      0x2000, "Two" )
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )

	PORT_START("IN1")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Credit configuration" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0000, "Start 2C/Continue 1C" )
	PORT_DIPSETTING(      0x0200, "Start 1C/Continue 1C" )
	PORT_DIPNAME( 0x1c00, 0x1c00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:3,4,5")
	PORT_DIPSETTING(      0x1800, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x1c00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0c00, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x1400, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0xe000, 0xe000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(      0xc000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0xe000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x6000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0xa000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_4C ) )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Acc.")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Gear") PORT_TOGGLE
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(wrally2_state, wrally2_analog_bit_r<1>)   /* ADC_2 serial input */
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xfa00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE3 ) /* go to test mode NOW */
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("FAKE")  /* Fake: To switch between monitors at run time */
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_SERVICE4 ) PORT_TOGGLE

	PORT_START("ANALOG0")   /* steering wheel player 1 */
	PORT_BIT( 0xff, 0x8A, IPT_PADDLE ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("P1 Wheel")

	PORT_START("ANALOG1")   /* steering wheel player 2 */
	PORT_BIT( 0xff, 0x8A, IPT_PADDLE_V ) PORT_MINMAX(0x00,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(25) PORT_REVERSE PORT_NAME("P2 Wheel")
INPUT_PORTS_END

void wrally2_state::wrally2(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(26'000'000) / 2); /* 13 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &wrally2_state::wrally2_map);
	m_maincpu->set_vblank_int("lscreen", FUNC(gaelco2_state::irq6_line_hold));

	GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(26'000'000) / 2).set_addrmap(0, &wrally2_state::mcu_hostmem_map); /* 13 MHz */
	config.set_perfect_quantum("gaelco_ds5002fp:mcu");

	LS259(config, m_mainlatch); // IC6
	m_mainlatch->q_out_cb<0>().set(FUNC(gaelco2_state::coin1_counter_w));
	m_mainlatch->q_out_cb<1>().set(FUNC(gaelco2_state::coin2_counter_w));
	m_mainlatch->q_out_cb<5>().set(FUNC(wrally2_state::wrally2_adc_clk));   /* ADCs clock-in line */
	m_mainlatch->q_out_cb<6>().set(FUNC(wrally2_state::wrally2_adc_cs));    /* ADCs chip select line */

	/* video hardware */
	BUFFERED_SPRITERAM16(config, m_spriteram);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco2);
	PALETTE(config, m_palette).set_entries(4096*16 - 16);   /* game's palette is 4096 but we allocate 15 more for shadows & highlights */
	config.set_default_layout(layout_dualhsxs);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_refresh_hz(59.1);
	lscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	lscreen.set_size(384, 32*16);
	lscreen.set_visarea(0, 384-1, 16, 256-1);
	lscreen.set_screen_update(FUNC(gaelco2_state::screen_update_left));
	lscreen.set_palette(m_palette);

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_refresh_hz(59.1);
	rscreen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	rscreen.set_size(384, 32*16);
	rscreen.set_visarea(0, 384-1, 16, 256-1);
	rscreen.set_screen_update(FUNC(gaelco2_state::screen_update_right));
	rscreen.screen_vblank().set(m_spriteram, FUNC(buffered_spriteram16_device::vblank_copy_rising));
	rscreen.set_palette(m_palette);

	MCFG_VIDEO_START_OVERRIDE(gaelco2_state,gaelco2_dual)

	/* sound hardware */
	/* the chip is stereo, but the game sound is mono because the right channel
	   output is for cabinet 1 and the left channel output is for cabinet 2 */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	gaelco_gae1_device &gaelco(GAELCO_GAE1(config, "gaelco", XTAL(34'000'000) / 34));
	gaelco.set_device_rom_tag("gfx1");
	gaelco.set_bank_offsets(0 * 0x0200000, 1 * 0x0200000, 0, 0);
	gaelco.add_route(0, "rspeaker", 1.0);
	gaelco.add_route(1, "lspeaker", 1.0);
}

/*
PCB Layout:

REF: 950510
------------------------------------------------------------------------------
|                POT1          PAL20L8       KM428C256J-6 (x4)               |
|                POT1                        ------------------------------  |
|                                            | (Plug-In Daughterboard)  | |  |
|                                            | WR2.1   WR2.9    WR2.16  | |  |
|                                            | WR2.2   WR2.10   WR2.17  |J|  |
|---                                         |         WR2.11   WR2.18  |P|  |
   |                                         |         WR2.12   WR2.19  |1|  |
   |                                         |         WR2.13   WR2.20  | |  |
|---                                         |         WR2.14   WR2.21  | |  |
|                                            ------------------------------  |
|                                                                            |
|                                            |----------|                    |
| J                                          |          |                    |
|                                            | GAE1 449 |                    |
| A                               6264       | (QFP208) |                    |
|                                 6264       |          |                    |
| M                                          |----------|                    |
|                                                                            |
| M                       |-------------------------|                        |
|                         |                         |  34.000MHz     62256   |
| A                       |  62256  DS5002  BATT_3V |                62256   |
|                         |                         |                        |
|                         |-------------------------|                        |
|                                                                            |
|---                                    62256                                |
   |                                    62256                                |
   |  DSW1                                                                   |
|---  DSW2                                                                   |
|                       PAL16L8                                              |
| J J                                                                        |
| P P                            26.000MHz      MC68000P12      WR2 63.IC63  |
| 1 4                                                           WR2 64.IC64  |
|      JP2   JP3                                                             |
-----------------------------------------------------------------------------|


Notes
-----
JP1 on daughter card connects through the 50 pin connector JP6 on main PCB
All ROMs are type 27C040

POT1: Volume adjust for cabinet 1
POT2: Volume adjust for cabinet 2

JP1: 6 pin connector - Video signals for cabinet 2
  1| Video
  2| No Connection
  3| Video GND
  4| Video Blue
  5| Video Green
  6| Video Red
JP4: 4 pin connector - Sound for cabinet 2
  1| Speaker (+)
  2| Speaker (-)
  3| No Connection
  4| Audio GND
JP2: 15 pin connector - Inputs for Player 2 / Cabinet 2
  1| Coin
  2| Potentiometer VCC
  3| Left
  4| Right
  5| Up
  6| Down
  7| Accelerator
  8| Shift Lever
  9| No Connection
 10| Start
 11| Service
 12| Potentiometer Top
 13| Coin Counter
 14| GND
 15| GND
JP3: 15 pin connector - Not populated (used on Touch and Go PCBs)

Wires run to male JAMMA board with corresponding JP1, JP2, JP3 & JP4 connectors for cabinet 2 JAMMA harness

Controls:
    8-Way Joystick - optional 270 Degree Steering Wheel (Potentiometer 5K)
    Accelerator button
    Shift Lever

            PCB Conector JAMMA for Cabinet 1 and Cabinet 2
                          Main Jamma Connector
            Solder Side          |             Parts Side
------------------------------------------------------------------
             GND             | A | 1 |             GND
             GND             | B | 2 |             GND
             +5              | C | 3 |             +5
             +5              | D | 4 |             +5
                             | E | 5 |
             +12             | F | 6 |             +12
------------ KEY ------------| H | 7 |------------ KEY -----------
                             | J | 8 |      Coin Counter # 1
                             | K | 9 |
        Speaker (-)          | L | 10|        Speaker (+)
                             | M | 11|
        Video Green          | N | 12|        Video Red
        Video Sync           | P | 13|        Video Blue
       Service Switch        | R | 14|        Video GND
                             | S | 15|        Test Switch
                             | T | 16|        Coin Switch
                             | U | 17|        Start
                             | V | 18|        Up
                             | W | 19|        Down
                             | X | 20|        Left
                             | Y | 21|        Right
                             | Z | 22|        Accelerator
                             | a | 23|        Shift Lever
                             | b | 24|
      Potentiometer VCC      | c | 25|        Potentiometer Top
                             | d | 26|
             GND             | e | 27|             GND
             GND             | f | 28|             GND

PCB Layout:


REF: 950510-1
------------------------------------------------------------------------------
|         POT1                 PAL20L8       KM428C256J-6                    |
|         POT2                                                               |
|                                            KM428C256J-6                  |-|
|                                                                          | |
|                                            KM428C256J-6    WR2 IC68.IC68 | |
|---                                                                       |J|
   |                                         KM428C256J-6    WR2 IC69.IC69 |P|
   |                                                                       |6|
|---                                                         WR2 IC70.IC70 | |
|                                                                          | |
|                                                                          |-|
|                                            |----------|                    |
| J                                          |          |                    |
|                                            | GAE1 506 |                    |
| A                              65764       | (QFP160) |                    |
|                                65764       |          |                    |
| M                                          |----------|                    |
|                                                                            |
| M                       |-------------------------|                        |
|                         |                         |  34.000MHz     62256   |
| A                       |  62256  DS5002  BATT_3V |                62256   |
|                         |                         |                        |
|                         |-------------------------|                        |
|    TLC549   TLC549                                                         |
|---                                    62256                                |
   |                                    62256                                |
   |  DSW1                                                                   |
|---  DSW2                                                                   |
|                        PAL16L8                                             |
| J J                                                                        |
| P P                            26.000MHz      MC68000P12      WR2 63.IC63  |
| 1 4                                                           WR2 64.IC64  |
|      JP2   JP3                                                             |
-----------------------------------------------------------------------------|


Notes
-----
Gaelco's mask ROMs:
 WR2 IC70 42pin 16Mbit mask read as 27C160 (Graphics)
 WR2 IC69 42pin 32Mbit mask read as 27C332 (Graphics & Sound)
 WR2 IC68 32pin  8Mbit mask read as 27C801 (Graphics)

JP6: 50 pin connector for daughter card - Not populated

TI F20L8 is a Texas Ins. DIP24 (may be a PAL). Is marked as F 406 XF 21869 F20L8-25CNT
TLC549 (IC2 and IC7) is a 8-bit serial ADC

Also known to come with a GAE1 with various production codes including 449, 501 & 506

*/

ROM_START( wrally2 ) // REF: 950510-1
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "wr2_64.ic64",  0x000000, 0x080000, CRC(4cdf4e1e) SHA1(a3b3ff4a70336b61c7bba5d518527bf4bd901867) )
	ROM_LOAD16_BYTE( "wr2_63.ic63",  0x000001, 0x080000, CRC(94887c9f) SHA1(ad09f1fbeff4c3ba47f72346d261b22fa6a51457) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	/* This SRAM has been dumped from 2 PCBs.  The first had unused space filled as 0xff, the 2nd space was filled as 0x00.
	   In addition, the first had 2 bad bytes, one of which was identified at the time, the other not.  For reference the
	   one that was not is "1938: 18 <-> 9B" (part of a data table)

	   A little less obvious is why the older dump had the following startup code, which appears to have been partially
	   patched out

	    0200: mov   sp,#$70
	    0203: mov   a,pcon
	    0205: anl   a,#$20
	    0207: jnz   $0203
	    0209: nop
	    020A: nop
	    020B: nop
	    020C: mov   dptr,#$FC01

	   while the newer dump has this

	    0200: mov   sp,#$70
	    0203: mov   mcon,#$68
	    0206: mov   i2cfg,#$00
	    0209: mov   crcr,#$80
	    020C: mov   dptr,#$FC01

	   either way the 2nd dump is in much better state, so we're using that.
	*/
	ROM_LOAD( "wrally2_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(4c532e9e) SHA1(d0aad72b204d4abd3b8d7d5bbaf8d2d2f78edaa6) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x69 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0a00000, "gfx1", 0 ) // GFX + Sound
	// 0x0000000-0x06fffff filled in in the DRIVER_INIT
	ROM_LOAD( "wr2_ic68.ic68",  0x0800000, 0x0100000, CRC(4a75ffaa) SHA1(ffae561ad4fa100398ab6b94d8dcb13e9fae4272) ) // GFX only - read as 27C801

	ROM_REGION( 0x0600000, "gfx2", 0 ) // Temporary storage
	ROM_LOAD( "wr2_ic69.ic69",  0x0000000, 0x0400000, CRC(a174d196) SHA1(4a7da1cd288e73518143a027782f3140e6582cf4) ) // GFX & Sound - read as 27C332
	ROM_LOAD( "wr2_ic70.ic70",  0x0400000, 0x0200000, CRC(8d1e43ba) SHA1(79eed51788c6c55a4347be70a3be4eb14a0d1747) ) // GFX only - read as 27C160
ROM_END

ROM_START( wrally2a ) // REF: 950510
	ROM_REGION( 0x100000, "maincpu", 0 )    // 68000 code
	ROM_LOAD16_BYTE( "wr2_64.ic64",  0x000000, 0x080000, CRC(4cdf4e1e) SHA1(a3b3ff4a70336b61c7bba5d518527bf4bd901867) )
	ROM_LOAD16_BYTE( "wr2_63.ic63",  0x000001, 0x080000, CRC(94887c9f) SHA1(ad09f1fbeff4c3ba47f72346d261b22fa6a51457) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) // DS5002FP code
	/* This SRAM has been dumped from 2 PCBs.  The first had unused space filled as 0xff, the 2nd space was filled as 0x00.
	   In addition, the first had 2 bad bytes, one of which was identified at the time, the other not.  For reference the
	   one that was not is "1938: 18 <-> 9B" (part of a data table)

	   A little less obvious is why the older dump had the following startup code, which appears to have been partially
	   patched out

	    0200: mov   sp,#$70
	    0203: mov   a,pcon
	    0205: anl   a,#$20
	    0207: jnz   $0203
	    0209: nop
	    020A: nop
	    020B: nop
	    020C: mov   dptr,#$FC01

	   while the newer dump has this

	    0200: mov   sp,#$70
	    0203: mov   mcon,#$68
	    0206: mov   i2cfg,#$00
	    0209: mov   crcr,#$80
	    020C: mov   dptr,#$FC01

	   either way the 2nd dump is in much better state, so we're using that.
	*/
	ROM_LOAD( "wrally2_ds5002fp_sram.bin", 0x00000, 0x8000, CRC(4c532e9e) SHA1(d0aad72b204d4abd3b8d7d5bbaf8d2d2f78edaa6) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	// These are the default states stored in NVRAM
	DS5002FP_SET_MON( 0x69 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x0a00000, "gfx1", 0 )  // GFX + Sound
	ROM_LOAD( "wr2.16d",    0x0000000, 0x0080000, CRC(ad26086b) SHA1(487ffaaca57c9d030fc486b8cae6735ee40a0ac3) )    // GFX only
	ROM_LOAD( "wr2.17d",    0x0080000, 0x0080000, CRC(c1ec0745) SHA1(a6c3ce9c889e6a53f4155f54d6655825af34a35b) )    // GFX only
	ROM_LOAD( "wr2.18d",    0x0100000, 0x0080000, CRC(e3617814) SHA1(9f9514052bb07d7e243f33b11bae409a444b7d9f) )    // Sound only
	ROM_LOAD( "wr2.19d",    0x0180000, 0x0080000, CRC(2dae988c) SHA1(a585e10b0e1519b828738b0b90698f8600082250) )    // Sound only
	ROM_LOAD( "wr2.09d",    0x0200000, 0x0080000, CRC(372d70c8) SHA1(a6d8419765eab1fa20c6d3ddff9d026adaab5cd9) )    // GFX only
	ROM_LOAD( "wr2.10d",    0x0280000, 0x0080000, CRC(5db67eb3) SHA1(faa58dafa26befb3291e5185ee04c39ce3b45b3f) )    // GFX only
	ROM_LOAD( "wr2.11d",    0x0300000, 0x0080000, CRC(ae66b97c) SHA1(bd0eba0b1c77864e06a9e136cfd834b35f200683) )    // Sound only
	ROM_LOAD( "wr2.12d",    0x0380000, 0x0080000, CRC(6dbdaa95) SHA1(f23df65e3df92d79f7b1e99d611c067a79fc849a) )    // Sound only
	ROM_LOAD( "wr2.01d",    0x0400000, 0x0080000, CRC(753a138d) SHA1(b05348af6d25e95208fc39007eb2082b759384e8) )    // GFX only
	ROM_LOAD( "wr2.02d",    0x0480000, 0x0080000, CRC(9c2a723c) SHA1(5259c8fa1ad73518e89a8df6e76a565b8f8799e3) )    // GFX only
	ROM_FILL(               0x0500000, 0x0100000, 0x00 )         // Empty
	ROM_LOAD( "wr2.20d",    0x0600000, 0x0080000, CRC(4f7ade84) SHA1(c8efcd4bcb1f2ad6ab8104ec0daea8324cefd3fd) )    // GFX only
	ROM_LOAD( "wr2.13d",    0x0680000, 0x0080000, CRC(a4cd32f8) SHA1(bc4cc73b7a58aecd735bf55bb5062baa6dd22f83) )    // GFX only
	ROM_FILL(               0x0700000, 0x0100000, 0x00 )         // Empty
	ROM_LOAD( "wr2.21d",    0x0800000, 0x0080000, CRC(899b0583) SHA1(a313e679980cc4da22bc70f2c7c9685af4f3d6df) )    // GFX only
	ROM_LOAD( "wr2.14d",    0x0880000, 0x0080000, CRC(6eb781d5) SHA1(d5c13db88e6de606b34805391cef9f3fbf09fac4) )    // GFX only
	ROM_FILL(               0x0900000, 0x0100000, 0x00 )         // Empty
ROM_END



GAME( 1994, aligator,    0,         alighunt_d5002fp, alighunt, gaelco2_state, init_alighunt,  ROT0, "Gaelco", "Alligator Hunt (World, protected)", 0 )
GAME( 1994, aligators,   aligator,  alighunt_d5002fp, alighunt, gaelco2_state, init_alighunt,  ROT0, "Gaelco", "Alligator Hunt (Spain, protected)", 0 )
GAME( 1994, aligatorun,  aligator,  alighunt,         alighunt, gaelco2_state, init_alighunt,  ROT0, "Gaelco", "Alligator Hunt (unprotected, set 1)", 0 )
GAME( 1994, aligatoruna, aligator,  alighunt,         alighunt, gaelco2_state, init_alighunt,  ROT0, "Gaelco", "Alligator Hunt (unprotected, set 2)", 0 ) // strange version, starts on space stages, but clearly a recompile not a trivial hack of the above, show version maybe?
GAME( 1994, aligatorp,   aligator,  alighunt_d5002fp, alighunt, gaelco2_state, empty_init,     ROT0, "Gaelco", "Alligator Hunt (protected, prototype?)", MACHINE_NOT_WORKING ) // requires different protection program / data

GAME( 1995, touchgo,     0,         touchgo_d5002fp,  touchgo,  gaelco2_state, init_touchgo,   ROT0, "Gaelco", "Touch & Go (World)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, touchgon,    touchgo,   touchgo_d5002fp,  touchgo,  gaelco2_state, init_touchgo,   ROT0, "Gaelco", "Touch & Go (Non North America)", MACHINE_IMPERFECT_SOUND )
GAME( 1995, touchgoe,    touchgo,   touchgo_d5002fp,  touchgo,  gaelco2_state, init_touchgo,   ROT0, "Gaelco", "Touch & Go (earlier revision)",  MACHINE_IMPERFECT_SOUND )
GAME( 1995, touchgok,    touchgo,   touchgo,          touchgo,  gaelco2_state, init_touchgo,   ROT0, "Gaelco", "Touch & Go (Korea, unprotected)", MACHINE_IMPERFECT_SOUND ) // doesn't say 'Korea' but was sourced there, shows 2 copyright lines like the 'earlier revision'

GAME( 1995, wrally2,     0,         wrally2,          wrally2,  wrally2_state, init_wrally2,   ROT0, "Gaelco", "World Rally 2: Twin Racing (mask ROM version)", 0 )
GAME( 1995, wrally2a,    wrally2,   wrally2,          wrally2,  wrally2_state, empty_init,     ROT0, "Gaelco", "World Rally 2: Twin Racing (EPROM version)", 0 )

// All sets identify as Version 1.0, but are clearly different revisions
GAME( 1996, maniacsq,    0,         maniacsq_d5002fp, maniacsq, gaelco2_state, empty_init,     ROT0, "Gaelco", "Maniac Square (protected, Version 1.0, Checksum DEEE)", 0 )
GAME( 1996, maniacsqa,   maniacsq,  maniacsq_d5002fp, maniacsq, gaelco2_state, empty_init,     ROT0, "Gaelco", "Maniac Square (protected, Version 1.0, Checksum CF2D)", 0 )
GAME( 1996, maniacsqu,   maniacsq,  maniacsq,         maniacsq, gaelco2_state, empty_init,     ROT0, "Gaelco", "Maniac Square (unprotected, Version 1.0, Checksum BB73)", 0 )
GAME( 1996, maniacsqs,   maniacsq,  maniacsqs,        snowboar, gaelco2_state, empty_init,     ROT0, "Gaelco", "Maniac Square (unprotected, Version 1.0, Checksum 66B1, 960419/1 PCB)", 0 ) // Official version on Snow Board Championship PCB, doesn't use the protection

GAME( 1996, snowboar,    0,         snowboar,         snowboar, gaelco2_state, init_snowboar,  ROT0, "Gaelco", "Snow Board Championship (Version 2.1)", 0 )
GAME( 1996, snowboara,   snowboar,  snowboar,         snowboar, gaelco2_state, init_snowboara, ROT0, "Gaelco", "Snow Board Championship (Version 2.0)", 0 )

GAME( 1998, bang,        0,         bang,             bang,     bang_state,    init_bang,      ROT0, "Gaelco", "Bang!", 0 )
GAME( 1998, bangj,       bang,      bang,             bang,     bang_state,    init_bang,      ROT0, "Gaelco", "Gun Gabacho (Japan)", 0 )

// 2-in-1 gambling game, appears to be cloned Gaelco hardware complete with DS5002FP, or possibly manufactured by Gaelco for Nova Desitec but without any Gaelco branding.
// these are Italian versions, English versions also exist
GAME( 1999, play2000,    0,         play2000,         play2000, gaelco2_state, init_play2000,  ROT0, "Nova Desitec", "Play 2000 (Super Slot & Gran Tesoro) (v7.0i) (Italy)",  0 )
GAME( 1999, play2000_50i,play2000,  play2000,         play2000, gaelco2_state, empty_init,     ROT0, "Nova Desitec", "Play 2000 (Super Slot & Gran Tesoro) (v5.0i) (Italy)",  MACHINE_NOT_WORKING ) // bad dump
GAME( 1999, play2000_40i,play2000,  play2000,         play2000, gaelco2_state, init_play2000,  ROT0, "Nova Desitec", "Play 2000 (Super Slot & Gran Tesoro) (v4.0i) (Italy)",  0 )

GAME( 1998, srollnd,     0,         srollnd,          play2000, gaelco2_state, init_play2000,  ROT0, "Nova Desitec", "Super Roller (v7.0)",  MACHINE_NOT_WORKING ) // missing ds5002fp dump

// Gym equipment
GAME( 1997, sltpcycl,   0,          saltcrdi,         saltcrdi, gaelco2_state, init_play2000,  ROT0, "Salter Fitness / Gaelco", "Pro Cycle Tele Cardioline (Salter Fitness Bike V.1.0, Checksum 02AB)", 0 ) // Same board and ROM as Pro Reclimber
GAME( 1997, sltpstep,   0,          saltcrdi,         saltcrdi, gaelco2_state, init_play2000,  ROT0, "Salter Fitness / Gaelco", "Pro Stepper Tele Cardioline (Salter Fitness Stepper V.1.0, Checksum F208)", 0 )
// there are other devices in Cardioline series but they don't use displays and aren't on Gaelco hardware
