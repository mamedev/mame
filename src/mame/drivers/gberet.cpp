// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Green Beret
    Konami

    driver by Nicola Salmoria
    correct ROM naming information by Belgium Dump Team (17/06/2003)

    Games supported:
        * Green Beret
        * Rush'n Attack (US)
        * Green Beret (bootleg)
        * Mr. Goemon (Japan)

    gberetb is a bootleg hacked to run on different hardware.

    TODO
        - Measure IRQ and NMI frequency (of an original board), is IRQ really tied to the same timer that triggers NMI?
        - Correct PROMs decoding for the bootleg

****************************************************************************

    Memory map

    0000-bfff   ROM
    c000-c7ff   Color RAM
    c800-cfff   Video RAM
    d000-d0c0   Sprites (bank 0)
    d100-d1c0   Sprites (bank 1)
    d200-dfff   RAM
    e000-e01f   ZRAM1 line scroll registers
    e020-e03f   ZRAM2 bit 8 of line scroll registers

    read:
    f200      DSW1
                    bit 0-1 lives
                    bit 2   cocktail/upright cabinet (0 = upright)
                    bit 3-4 bonus
                    bit 5-6 difficulty
                    bit 7   demo sounds
    f400      DSW2
                    bit 0 = screen flip
                    bit 1 = single/dual upright controls
    f600      DSW0
                    bit 0-1-2-3 coins per play Coin1
                    bit 4-5-6-7 coins per play Coin2
    f601      IN1 player 2 controls
    f602      IN0 player 1 controls
    f603      IN2
                    bit 0-1-2 coin  bit 3 1 player start  bit 4 2 players start

    write:
    e040      ?
    e041      ?
    e042      ?
    e043      bit 3 = sprite RAM bank select; other bits = ?
    e044      bit 0-2 = interrupt control; bit 3 = flip screen
    f000      ?
    f200      SN76496 command
    f400      SN76496 trigger (write command to f200, then write to this location
                                to cause the chip to read it)
    f600      watchdog reset

****************************************************************************

    Interrupts

    The game uses both IRQ (mode 1) and NMI via a custom interrupt controller.

***************************************************************************/

#include "emu.h"
#include "includes/gberet.h"
#include "includes/konamipt.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "sound/sn76496.h"

#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Interrupt generators
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(gberet_state::interrupt_tick)
{
	uint8_t ticks_mask = ~m_interrupt_ticks & (m_interrupt_ticks + 1); // 0->1
	m_interrupt_ticks++;

	// NMI on d0
	if (ticks_mask & m_interrupt_mask & 1)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	// IRQ on d3 (used by mrgoemon)
	if (ticks_mask & m_interrupt_mask << 2 & 8)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	// IRQ on d4 (used by gberet)
	if (ticks_mask & m_interrupt_mask << 2 & 16)
		m_maincpu->set_input_line(0, ASSERT_LINE);
}


/*************************************
 *
 *  Address maps / Memory handlers
 *
 *************************************/

void gberet_state::coin_counter_w(uint8_t data)
{
	// bits 0/1 = coin counters
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

void mrgoemon_state::coin_counter_w(uint8_t data)
{
	// bits 0/1 = coin counters
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);

	// bits 5-7 = ROM bank select
	m_mainbank->set_entry(((data & 0xe0) >> 5));
}

void gberet_state::flipscreen_w(uint8_t data)
{
	// bits 0/1/2 = interrupt enable
	uint8_t ack_mask = ~data & m_interrupt_mask; // 1->0

	if (ack_mask & 1)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	if (ack_mask & 6)
		m_maincpu->set_input_line(0, CLEAR_LINE);

	m_interrupt_mask = data & 7;

	// bit 3 = flip screen
	flip_screen_set(data & 8);
}

void gberet_state::sound_w(uint8_t data)
{
	m_sn->write(*m_soundlatch);
}

void gberet_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().w(FUNC(gberet_state::colorram_w)).share(m_colorram);
	map(0xc800, 0xcfff).ram().w(FUNC(gberet_state::videoram_w)).share(m_videoram);
	map(0xd000, 0xd0ff).ram().share(m_spriteram2);
	map(0xd100, 0xd1ff).ram().share(m_spriteram);
	map(0xd200, 0xdfff).ram();
	map(0xe000, 0xe03f).ram().w(FUNC(gberet_state::scroll_w)).share(m_scrollram);
	map(0xe040, 0xe042).nopw(); // ???
	map(0xe043, 0xe043).w(FUNC(gberet_state::sprite_bank_w));
	map(0xe044, 0xe044).w(FUNC(gberet_state::flipscreen_w));
	map(0xf000, 0xf000).w(FUNC(gberet_state::coin_counter_w));
	map(0xf200, 0xf200).portr("DSW2").writeonly().share(m_soundlatch);
	map(0xf400, 0xf400).portr("DSW3").w(FUNC(gberet_state::sound_w));
	map(0xf600, 0xf600).portr("DSW1").w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xf601, 0xf601).portr("P2");
	map(0xf602, 0xf602).portr("P1");
	map(0xf603, 0xf603).portr("SYSTEM");
}

void mrgoemon_state::prg_map(address_map &map)
{
	gberet_state::prg_map(map);

	map(0xf000, 0xf000).w(FUNC(mrgoemon_state::coin_counter_w));
	map(0xf800, 0xffff).bankr(m_mainbank);
}


void gberetb_state::flipscreen_w(uint8_t data)
{
	flip_screen_set(data & 8);
}

uint8_t gberetb_state::irq_ack_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

void gberetb_state::nmi_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void gberetb_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram().w(FUNC(gberetb_state::colorram_w)).share(m_colorram);
	map(0xc800, 0xcfff).ram().w(FUNC(gberetb_state::videoram_w)).share(m_videoram);
	map(0xd000, 0xdfff).ram();
	map(0xe000, 0xe03f).ram();
	map(0xe040, 0xe043).nopw(); // ???
	map(0xe044, 0xe044).w(FUNC(gberetb_state::flipscreen_w)); // did hw even support flipscreen?
	map(0xe800, 0xe8ff).ram();
	map(0xe900, 0xe9ff).ram().share(m_spriteram);
	map(0xf000, 0xf000).nopw(); // coin counter not supported
	map(0xf200, 0xf200).portr("DSW2");
	map(0xf400, 0xf400).w(m_sn, FUNC(sn76489a_device::write));
	map(0xf600, 0xf600).portr("P2");
	map(0xf601, 0xf601).portr("DSW1");
	map(0xf602, 0xf602).portr("P1");
	map(0xf603, 0xf603).portr("SYSTEM");
	map(0xf800, 0xf800).rw(FUNC(gberetb_state::irq_ack_r), FUNC(gberetb_state::nmi_ack_w));
	map(0xf900, 0xf901).w(FUNC(gberetb_state::scroll_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( gberet )
	PORT_START("P1")
	KONAMI8_MONO_B12_UNK

	PORT_START("P2")
	KONAMI8_COCKTAIL_B12_UNK

	PORT_START("SYSTEM")
	KONAMI8_SYSTEM_UNK

	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	// "No Coin B" = coins produce sound, but no effect on coin counter

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "30K, 70K, Every 70K" )
	PORT_DIPSETTING(    0x10, "40K, 80K, Every 80K" )   // Japanese default
	PORT_DIPSETTING(    0x08, "50K, 100K, Every 100K" ) // US default
	PORT_DIPSETTING(    0x00, "50K, 200K, Every 200K" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Difficult ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Difficult ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Upright Controls" )      PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Dual ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4" )        // Listed as "Unused"
INPUT_PORTS_END

static INPUT_PORTS_START( gberetb )
	PORT_INCLUDE(gberet)

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_MODIFY("DSW3")
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( mrgoemon )
	PORT_INCLUDE(gberet)

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20K, Every 60K" )
	PORT_DIPSETTING(    0x10, "30K, Every 70K" )
	PORT_DIPSETTING(    0x08, "40K, Every 80K" )
	PORT_DIPSETTING(    0x00, "50K, Every 90K" )

	PORT_MODIFY("DSW3")
	PORT_SERVICE_DIPLOC(   0x04, IP_ACTIVE_LOW, "SW3:3" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    // 8*8 characters
	512,    // 512 characters
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 }, // the four bitplanes are packed in one nibble
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    // every char takes 8 consecutive bytes
};

static const gfx_layout spritelayout =
{
	16,16,  // 16*16 sprites
	512,    // 512 sprites
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 }, // the four bitplanes are packed in one nibble
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
		32*8+0*4, 32*8+1*4, 32*8+2*4, 32*8+3*4, 32*8+4*4, 32*8+5*4, 32*8+6*4, 32*8+7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
		64*8+0*32, 64*8+1*32, 64*8+2*32, 64*8+3*32, 64*8+4*32, 64*8+5*32, 64*8+6*32, 64*8+7*32 },
	128*8   // every sprite takes 128 consecutive bytes
};

static const gfx_layout gberetb_charlayout =
{
	8,8,    // 8*8 characters
	512,    // 512 characters
	4,      // 4 bits per pixel
	{ 0, 1, 2, 3 }, // the four bitplanes are packed in one nibble
	{ 6*4, 7*4, 0*4, 1*4, 2*4, 3*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    // every char takes 8 consecutive bytes
};

static const gfx_layout gberetb_spritelayout =
{
	16,16,  // 16*16 sprites
	512,    // 512 sprites
	4,      // 4 bits per pixel
	{ 0*0x4000*8, 1*0x4000*8, 2*0x4000*8, 3*0x4000*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
		16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8    // every sprite takes 32 consecutive bytes
};

static GFXDECODE_START( gfx_gberet )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 16*16, 16 )
GFXDECODE_END

static GFXDECODE_START( gfx_gberetb )
	GFXDECODE_ENTRY( "gfx1", 0, gberetb_charlayout,       0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, gberetb_spritelayout, 16*16, 16 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/

void gberet_state::machine_start()
{
	save_item(NAME(m_interrupt_mask));
	save_item(NAME(m_interrupt_ticks));
	save_item(NAME(m_spritebank));
}

void mrgoemon_state::machine_start()
{
	gberet_base_state::machine_start();

	uint8_t *ROM = memregion("maincpu")->base();
	m_mainbank->configure_entries(0, 8, &ROM[0xc000], 0x800);
}

void gberet_state::machine_reset()
{
	m_interrupt_mask = 0;
	m_interrupt_ticks = 0;
	m_spritebank = 0;
}

void gberet_state::gberet(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(18'432'000) / 6);      // X1S (generated by a custom IC)
	m_maincpu->set_addrmap(AS_PROGRAM, &gberet_state::prg_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(gberet_state::interrupt_tick), "screen", 0, 16);
	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gberet_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gberet);
	PALETTE(config, m_palette, FUNC(gberet_state::palette), 2*16*16, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sn, XTAL(18'432'000) / 12).add_route(ALL_OUTPUTS, "mono", 1.0); // type verified on real and bootleg PCB
}

void mrgoemon_state::mrgoemon(machine_config &config)
{
	gberet(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mrgoemon_state::prg_map);
}

void gberetb_state::gberetb(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(20'000'000) / 4); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &gberetb_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(gberetb_state::irq0_line_assert));
	m_maincpu->set_periodic_int(FUNC(gberetb_state::nmi_line_assert), attotime::from_hz(XTAL(20'000'000) / 0x8000)); // divider guessed

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(1*8, 31*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(gberetb_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gberetb);
	PALETTE(config, m_palette, FUNC(gberetb_state::palette), 2*16*16, 32);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, m_sn, XTAL(20'000'000) / 12).add_route(ALL_OUTPUTS, "mono", 1.0); // divider guessed
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( gberet )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "577l03.10c",   0x0000, 0x4000, CRC(ae29e4ff) SHA1(5c66de1403c5df5b6647bb37e26070ffd33590e8) )
	ROM_LOAD( "577l02.8c",    0x4000, 0x4000, CRC(240836a5) SHA1(b76f3789f152198bf8a9a366378d664e683c6c9d) )
	ROM_LOAD( "577l01.7c",    0x8000, 0x4000, CRC(41fa3e1f) SHA1(90d1463e16b0f52c01078be044ce3672d4acebff) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "577l07.3f",    0x00000, 0x4000, CRC(4da7bd1b) SHA1(54adba9ae086852902d78ab36039498aae50d7a9) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "577l06.5e",    0x00000, 0x4000, CRC(0f1cb0ca) SHA1(094004e70c05df8cd486d0854c258fa766e2925d) )
	ROM_LOAD( "577l05.4e",    0x04000, 0x4000, CRC(523a8b66) SHA1(5f2bcf2b702fe05f8a022b6284cb2d0a5b5f222f) )
	ROM_LOAD( "577l08.4f",    0x08000, 0x4000, CRC(883933a4) SHA1(b565842edf09feeb2c4ac44ad58331757586b6aa) )
	ROM_LOAD( "577l04.3e",    0x0c000, 0x4000, CRC(ccecda4c) SHA1(cac053cab68cb420edd408ce032143db7abc29f5) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "577h09.2f",    0x0000, 0x0020, CRC(c15e7c80) SHA1(c0e8a01e63ed8cf20b33456b68890313b387ad23) ) // palette
	ROM_LOAD( "577h11.6f",    0x0020, 0x0100, CRC(2a1a992b) SHA1(77cff7c9c8433f999a87776021935864cf9dccb4) ) // characters
	ROM_LOAD( "577h10.5f",    0x0120, 0x0100, CRC(e9de1e53) SHA1(406b8dfe54e6176082005cc5545e79c098672547) ) // sprites
ROM_END

ROM_START( rushatck )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "577h03.10c",   0x0000, 0x4000, CRC(4d276b52) SHA1(ba5d61c89fd2db4b303b81deccc887561156cbe3) )
	ROM_LOAD( "577h02.8c",    0x4000, 0x4000, CRC(b5802806) SHA1(0e4698ecfb9eda916703165ea5d55516fdef5fe4) )
	ROM_LOAD( "577h01.7c",    0x8000, 0x4000, CRC(da7c8f3d) SHA1(eb61eedee169f67db93407ad0fe8a195089b7e3a) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "577h07.3f",    0x00000, 0x4000, CRC(03f9815f) SHA1(209c76fd36d1b5672992c55e24d3cf77d4c5a0aa) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "577l06.5e",    0x00000, 0x4000, CRC(0f1cb0ca) SHA1(094004e70c05df8cd486d0854c258fa766e2925d) )
	ROM_LOAD( "577h05.4e",    0x04000, 0x4000, CRC(9d028e8f) SHA1(4faa47152a6c1da0024bb03fbcf7baf0540e891e) )
	ROM_LOAD( "577l08.4f",    0x08000, 0x4000, CRC(883933a4) SHA1(b565842edf09feeb2c4ac44ad58331757586b6aa) )
	ROM_LOAD( "577l04.3e",    0x0c000, 0x4000, CRC(ccecda4c) SHA1(cac053cab68cb420edd408ce032143db7abc29f5) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "577h09.2f",    0x0000, 0x0020, CRC(c15e7c80) SHA1(c0e8a01e63ed8cf20b33456b68890313b387ad23) ) // palette
	ROM_LOAD( "577h11.6f",    0x0020, 0x0100, CRC(2a1a992b) SHA1(77cff7c9c8433f999a87776021935864cf9dccb4) ) // characters
	ROM_LOAD( "577h10.5f",    0x0120, 0x0100, CRC(e9de1e53) SHA1(406b8dfe54e6176082005cc5545e79c098672547) ) // sprites
ROM_END

ROM_START( gberetb )
	ROM_REGION( 0xc000, "maincpu", 0 )
	ROM_LOAD( "2-ic82.10g",   0x0000, 0x8000, CRC(6d6fb494) SHA1(0d01c86ed7a8962ee3e1056a8d41584ad1406f0f) )
	ROM_LOAD( "3-ic81.10f",   0x8000, 0x4000, CRC(f1520a0a) SHA1(227b2d2e1fc0e81ae02e663a3089e7399612e3cf) )

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "1-ic92.12c",   0x00000, 0x4000, CRC(b0189c87) SHA1(29202978b07bf059b88bf206d8fafc80e0cdb6dc) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "7-1c8.2b",     0x00000, 0x4000, CRC(86334522) SHA1(f2907d136dbfdb92cbd550524b4453755f6244b6) )
	ROM_LOAD( "6-ic9.2c",     0x04000, 0x4000, CRC(bda50d3e) SHA1(c6f5a15270a69464e977926d056b31dcec8b41c3) )
	ROM_LOAD( "5-ic10.2d",    0x08000, 0x4000, CRC(6a7b3881) SHA1(795bfb1fbc11ceac687b15e98574feb650e2f674) )
	ROM_LOAD( "4-ic11.2e",    0x0c000, 0x4000, CRC(3fb186c9) SHA1(40ce0447014af3f5b5b88648ab7e43a955bd1274) )

	ROM_REGION( 0x0420, "proms", 0 )
	ROM_LOAD( "82s123-ic100.13b", 0x0000, 0x0020, CRC(1bef8c7b) SHA1(f8f0ff6f674c8d28b3ac3daaad8e46328659a480) ) // palette
	ROM_LOAD( "82s129-ic26.4g",   0x0020, 0x0100, CRC(2d3ad74b) SHA1(92152fd717901b5532097a7c88916c89c054d135) )
	ROM_LOAD( "82s129-ic34.5g",   0x0120, 0x0100, CRC(a895c3e5) SHA1(e485489ec88bc058ebd8c5645cc951ef08aa778c) )
	ROM_LOAD( "82s129-ic75.9j",   0x0220, 0x0100, CRC(85e757b6) SHA1(6ec7573e14e3418ac1dcaa72326b34e3d3163ea3) )
	ROM_LOAD( "82s129-ic90.12a",  0x0320, 0x0100, CRC(dffe6b15) SHA1(fc83ef5d6b4885194a5fe00521d10aea9213e2c8) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "pal16r6_ic35.5h", 0x0000, 0x0104, CRC(bd76fb53) SHA1(2d0634e8edb3289a103719466465e9777606086e) )
ROM_END

ROM_START( mrgoemon )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "621d01.10c",   0x00000, 0x8000, CRC(b2219c56) SHA1(274160be5dabbbfa61af71d92bddffbb56eadab6) )
	ROM_LOAD( "621d02.12c",   0x08000, 0x8000, CRC(c3337a97) SHA1(6fd5f365b2624a37f252c202cd97877705b4a6c2) ) // 2nd half banked

	ROM_REGION( 0x04000, "gfx1", 0 )
	ROM_LOAD( "621a05.6d",    0x00000, 0x4000, CRC(f0a6dfc5) SHA1(395024ebfff550b0da393096483196fb1152a077) )

	ROM_REGION( 0x10000, "gfx2", 0 )
	ROM_LOAD( "621d03.4d",    0x00000, 0x8000, CRC(66f2b973) SHA1(7e906f258a5f4928f9615c6ea176efbca659b3a7) )
	ROM_LOAD( "621d04.5d",    0x08000, 0x8000, CRC(47df6301) SHA1(e675c070e46993d3453c2ddadc49ec8b84cec854) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "621a06.5f",    0x0000, 0x0020, CRC(7c90de5f) SHA1(8ac5708e72e32f3d79ccde0cbaedefc34f8ac57e) ) // palette
	ROM_LOAD( "621a08.7f",    0x0020, 0x0100, CRC(2fb244dd) SHA1(ceb909ad96c0dabc8684e69b028f4287e227c351) ) // characters
	ROM_LOAD( "621a07.6f",    0x0120, 0x0100, CRC(3980acdc) SHA1(f4e0bd74bccd77b84096c38bc70cf488a42d9562) ) // sprites
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1985, gberet,   0,      gberet,   gberet,   gberet_state,   empty_init, ROT0, "Konami",  "Green Beret", MACHINE_SUPPORTS_SAVE )
GAME( 1985, rushatck, gberet, gberet,   gberet,   gberet_state,   empty_init, ROT0, "Konami",  "Rush'n Attack (US)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, gberetb,  gberet, gberetb,  gberetb,  gberetb_state,  empty_init, ROT0, "bootleg", "Green Beret (bootleg)", MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE ) // needs correct PROM decoding
GAME( 1986, mrgoemon, 0,      mrgoemon, mrgoemon, mrgoemon_state, empty_init, ROT0, "Konami",  "Mr. Goemon (Japan)", MACHINE_SUPPORTS_SAVE )
