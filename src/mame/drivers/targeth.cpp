// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
/***************************************************************************

Target Hits (c) 1994 Gaelco (Designed & Developed by Zigurat. Produced by Gaelco)

Driver by Manuel Abadia <emumanu+mame@gmail.com>

** NOTES: Merge with wrally.cpp???  Address map nearly identical & PCB
          is reworked to add connections for light guns and different RAM

          is the visible area correct? if it's larger than the startup
          screen then scene 5 of desert chariots doesn't cover the entire
          screen either.

          the instructions say to reload the gun after each shot, but
          there is no reload button listed in service mode, and it doesn't
          seem to be required, was it a mechanical feature of the gun or
          is our logic inverted somewhere, the gun test shows 'ON' when
          you're not pressing fire too.

REF.940531
+-------------------------------------------------+
|       C1                                 6116   |
|  VOL  C2                                 6116   |
|          30MHz                                  |
|    M6295                   +----------+         |
|     1MHz                   |TMS       |         |
|      6116                  |TPC1020AFN|         |
|J     6116                  |   -084C  |      I7 |
|A     +------------+        +----------+  H8* I9 |
|M     |DS5002FP Box|        +----------+      I11|
|M     +------------+        |TMS       | H12* I13|
|A             65756         |TPC1020AFN|         |
| JP4          65756         |   -084C  |         |
| JP5                        +----------+         |
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
       UM6116BK-35  2K x 8 SRAM (x6)
  PAL: TI F20L8-25CNT DIP24 (x2)
  VOL: Volume pot
   SW: Two 8 switch dipswitches (SW1 unpopulated)
  JP4: 5 pin header for light gun player 2
  JP5: 5 pin header for light gun player 1

DS5002FP Box contains:
  Dallas DS5002SP @ 12MHz
  KM62256BLG-7L - 32Kx8 Low Power CMOS SRAM
  3.6v Battery
  JP1 - 5 pin port to program SRAM

* NOTE: PCB can use four 27C040 eproms at I7, I9, I11 & I13 or two 8Meg MASK
        roms at H8 & H12.  Same set up as used on the World Rally PCBs

***************************************************************************/

#include "emu.h"
#include "includes/targeth.h"

#include "machine/gaelco_ds5002fp.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"


static const gfx_layout tilelayout =
{
	16,16,                                                          /* 16x16 tiles */
	RGN_FRAC(1,4),                                                  /* number of tiles */
	4,                                                              /* bitplanes */
	{ RGN_FRAC(3,4), RGN_FRAC(2,4), RGN_FRAC(1,4), RGN_FRAC(0,4) }, /* plane offsets */
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static GFXDECODE_START( gfx_targeth )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout, 0, 64 )
GFXDECODE_END

TIMER_CALLBACK_MEMBER(targeth_state::gun1_irq)
{
	/* IRQ 4: Read 1P Gun */
	m_maincpu->set_input_line(4, HOLD_LINE);
	m_gun_irq_timer[0]->adjust( m_screen->time_until_pos(128, 0 ) );
}

TIMER_CALLBACK_MEMBER(targeth_state::gun2_irq)
{
	/* IRQ 6: Read 2P Gun */
	m_maincpu->set_input_line(6, HOLD_LINE);
	m_gun_irq_timer[1]->adjust( m_screen->time_until_pos(160, 0 ) );
}

WRITE8_MEMBER(targeth_state::oki_bankswitch_w)
{
	m_okibank->set_entry(data & 0x0f);
}

WRITE16_MEMBER(targeth_state::output_latch_w)
{
	m_outlatch->write_bit(offset >> 3, BIT(data, 0));
}

WRITE_LINE_MEMBER(targeth_state::coin1_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(targeth_state::coin2_counter_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE8_MEMBER(targeth_state::shareram_w)
{
	// why isn't there an AM_SOMETHING macro for this?
	reinterpret_cast<u8 *>(m_shareram.target())[BYTE_XOR_BE(offset)] = data;
}

READ8_MEMBER(targeth_state::shareram_r)
{
	// why isn't there an AM_SOMETHING macro for this?
	return reinterpret_cast<u8 const *>(m_shareram.target())[BYTE_XOR_BE(offset)];
}


void targeth_state::mcu_hostmem_map(address_map &map)
{
	map(0x8000, 0xffff).rw(FUNC(targeth_state::shareram_r), FUNC(targeth_state::shareram_w)); // confirmed that 0x8000 - 0xffff is a window into 68k shared RAM
}

void targeth_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x103fff).ram().w(FUNC(targeth_state::vram_w)).share("videoram");  /* Video RAM */
	map(0x108000, 0x108001).portr("GUNX1");
	map(0x108002, 0x108003).portr("GUNY1");
	map(0x108004, 0x108005).portr("GUNX2");
	map(0x108006, 0x108007).portr("GUNY2");
	map(0x108000, 0x108007).writeonly().share("vregs"); /* Video Registers */
	map(0x10800c, 0x10800d).nopw();                    /* CLR Video INT */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    /* Palette */
	map(0x440000, 0x440fff).ram().share("spriteram");   /* Sprite RAM */
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700006, 0x700007).portr("SYSTEM");             /* Coins, Start & Fire buttons */
	map(0x700008, 0x700009).portr("SERVICE");            /* Service & Guns Reload? */
	map(0x70000a, 0x70000b).select(0x000070).w(FUNC(targeth_state::output_latch_w));
	map(0x70000d, 0x70000d).w(FUNC(targeth_state::oki_bankswitch_w));    /* OKI6295 bankswitch */
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));  /* OKI6295 status register */
	map(0x700010, 0x700011).nopw();                        /* ??? Guns reload related? */
	map(0xfe0000, 0xfe7fff).ram();                                          /* Work RAM */
	map(0xfe8000, 0xfeffff).ram().share("shareram");                     /* Work RAM (shared with D5002FP) */
}


void targeth_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank");
}

void targeth_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);

	m_gun_irq_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(targeth_state::gun1_irq), this));
	m_gun_irq_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(targeth_state::gun2_irq), this));

	m_gun_irq_timer[0]->adjust(m_screen->time_until_pos(128, 0));
	m_gun_irq_timer[1]->adjust(m_screen->time_until_pos(160, 0));
}

static INPUT_PORTS_START( targeth )
	PORT_START("GUNX1")
	PORT_BIT( 0x01ff, 200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.20, -0.133, 0) PORT_MINMAX( 0, 400 + 4) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNY1")
	PORT_BIT( 0x01ff, 128, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.12, -0.055, 0) PORT_MINMAX(4,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(1)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNX2")
	PORT_BIT( 0x01ff, 200, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.20, -0.133, 0) PORT_MINMAX( 0, 400 + 4) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
	PORT_BIT( 0xfe00, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("GUNY2")
	PORT_BIT( 0x01ff, 128, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.12, -0.055, 0) PORT_MINMAX(4,255) PORT_SENSITIVITY(100) PORT_KEYDELTA(20) PORT_PLAYER(2)
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
	PORT_DIPNAME( 0x40, 0x40, "Gun alarm" ) /* Service mode gets default from here when uninitialized */
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


void targeth_state::targeth(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);          /* 12 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &targeth_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(targeth_state::irq2_line_hold));

	gaelco_ds5002fp_device &ds5002fp(GAELCO_DS5002FP(config, "gaelco_ds5002fp", XTAL(24'000'000) / 2));
	ds5002fp.set_addrmap(0, &targeth_state::mcu_hostmem_map);

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<2>().set(FUNC(targeth_state::coin1_counter_w));
	m_outlatch->q_out_cb<3>().set(FUNC(targeth_state::coin2_counter_w));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	m_screen->set_size(64*16, 16*16);
	m_screen->set_visarea(3*8, 23*16-8-1, 16, 16*16-8-1);
	m_screen->set_screen_update(FUNC(targeth_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_targeth);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // 1MHz resonator - pin 7 not verified
	oki.set_addrmap(0, &targeth_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

ROM_START( targeth )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "th2_b_c_23.c23", 0x000000, 0x040000, CRC(840887d6) SHA1(9a36b346608d531a62a2e0704ea44f12e07f9d91) ) // The "B" was hand written
	ROM_LOAD16_BYTE( "th2_b_c_22.c22", 0x000001, 0x040000, CRC(d2435eb8) SHA1(ce75a115dad8019c8e66a1c3b3e15f54781f65ae) ) // The "B" was hand written

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "targeth_ds5002fp.bin", 0x00000, 0x8000, CRC(abcdfee4) SHA1(c5955d5dbbcecbe1c2ae77d59671ae40eb814d30) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	ROM_LOAD( "targeth_ds5002fp_scratch", 0x00, 0x80, CRC(c927bcb1) SHA1(86b5c7ee6a4a5f0aa538a6742253da1afadb4345) ) // default state so you don't have to manually initialize game
	DS5002FP_SET_MON( 0x49 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Graphics */
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "targeth.c3",     0x080000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

ROM_START( targetha )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "th2_n_c_23.c23", 0x000000, 0x040000, CRC(b99b25dc) SHA1(1bf35b2c05a58f934d06eb6ef93f592d9f16344a) ) // The "N" was hand written
	ROM_LOAD16_BYTE( "th2_n_c_22.c22", 0x000001, 0x040000, CRC(6d34f0cf) SHA1(f44a1231f4fac1f9d443990e8fe2b4aaa3f338be) ) // The "N" was hand written

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "targeth_ds5002fp.bin", 0x00000, 0x8000, CRC(abcdfee4) SHA1(c5955d5dbbcecbe1c2ae77d59671ae40eb814d30) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	ROM_LOAD( "targeth_ds5002fp_scratch", 0x00, 0x80, CRC(c927bcb1) SHA1(86b5c7ee6a4a5f0aa538a6742253da1afadb4345) ) // default state so you don't have to manually initialize game
	DS5002FP_SET_MON( 0x49 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Graphics */
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "targeth.c3",     0x080000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

ROM_START( targeth10 )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "c23.bin", 0x000000, 0x040000, CRC(e38a54e2) SHA1(239bfa6f1c0fc8aa0ad7de9be237bef55b384007) )
	ROM_LOAD16_BYTE( "c22.bin", 0x000001, 0x040000, CRC(24fe3efb) SHA1(8f48f08a6db28966c9263be119883c9179e349ed) )

	ROM_REGION( 0x8000, "gaelco_ds5002fp:sram", 0 ) /* DS5002FP code */
	ROM_LOAD( "targeth_ds5002fp.bin", 0x00000, 0x8000, CRC(abcdfee4) SHA1(c5955d5dbbcecbe1c2ae77d59671ae40eb814d30) )

	ROM_REGION( 0x100, "gaelco_ds5002fp:mcu:internal", ROMREGION_ERASE00 )
	ROM_LOAD( "targeth_ds5002fp_scratch", 0x00, 0x80, CRC(c927bcb1) SHA1(86b5c7ee6a4a5f0aa538a6742253da1afadb4345) ) // default state so you don't have to manually initialize game
	DS5002FP_SET_MON( 0x49 )
	DS5002FP_SET_RPCTL( 0x00 )
	DS5002FP_SET_CRCR( 0x80 )

	ROM_REGION( 0x200000, "gfx1", 0 )   /* Graphics */
	ROM_LOAD( "targeth.i13",    0x000000, 0x080000, CRC(b892be24) SHA1(9cccaaacf20e77c7358f0ceac60b8a1012f1216c) )
	ROM_LOAD( "targeth.i11",    0x080000, 0x080000, CRC(6797faf9) SHA1(112cffe72f91cb46c262e19a47b0cab3237dd60f) )
	ROM_LOAD( "targeth.i9",     0x100000, 0x080000, CRC(0e922c1c) SHA1(6920e345c82e76f7e0af6101f39eb65ac1f112b9) )
	ROM_LOAD( "targeth.i7",     0x180000, 0x080000, CRC(d8b41000) SHA1(cbe91eb91bdc7a60b2333c6bea37d08a57902669) )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "targeth.c1",     0x000000, 0x080000, CRC(d6c9dfbc) SHA1(3ec70dea94fc89df933074012a52de6034571e87) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "targeth.c3",     0x080000, 0x080000, CRC(d4c771df) SHA1(7cc0a86ef6aa3d26ab8f19d198f62112bf012870) )
ROM_END

GAME( 1994, targeth,   0,       targeth, targeth, targeth_state, empty_init, ROT0, "Gaelco", "Target Hits (ver 1.1, Checksum 5152)", 0 )
GAME( 1994, targetha,  targeth, targeth, targeth, targeth_state, empty_init, ROT0, "Gaelco", "Target Hits (ver 1.1, Checksum 86E1)", 0 )
GAME( 1994, targeth10, targeth, targeth, targeth, targeth_state, empty_init, ROT0, "Gaelco", "Target Hits (ver 1.0, Checksum FBCB)", 0 )
