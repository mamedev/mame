// license:BSD-3-Clause
// copyright-holders:Chris Hardy
/**************************************************************************

Based on drivers from Juno First emulator by Chris Hardy (chris@junofirst.freeserve.co.uk)

To enter service mode, keep 1&2 pressed on reset


 Info provided with these alt sets

        MEGA ZONE   CHIP PLACEMENT

USES 69A09EP, Z80 CPU'S & AY-3-8910 SOUND CHIP W/8039 CPU

THERE ARE AT LEAST THREE VERSIONS OF MEGA ZONE, ALL THE ROMS ARE THE
SAME EXCEPT POSITION 6,7,8,9,11H IN SETS 1,2
ALL ROMS ARE 2764 EXCEPT H01 (E01) IS A 2732

CHIP #     POSITION                         VERS 3
-----------------------------------------------------
VER-1                VER-2
-----------------------------------------------------
319-E08    2D        E08      REAR BOARD    8  SAME
319-E09    2E        E09       "            9  SAME
319-E10    3D        E10       "           10  SAME
319-E11    3E        E11       "           11  SAME
319-G12    8C        G12       "           12
319-G13    10C       G13       "           13  SAME
319-E02    6D        E02      CONN BOARD    2  SAME
319-H03    6H        J03       "            3
319-H04    7H        J04       "            4
319-H05    8H        J05       "            5
319-H06    9H        J06       "            6
319-H07    11H       J07       "            7
319-H01    3A        E01       "            1  SAME
Z80        7E                                        IC#
AY-3-8910  8B                               PROM     98     TBP18S030 (82S123)
AO72       12F   KONAMI                     PROM     48       "
K824-501   8D    KONAMI                     PROM     42       "
8039       4B                               PROM     63     TBP24S10  (823126)
                                            PROM     33       "
                                            PAL16L8  63
                                            PAL16L8A 67

VERSION 3 IS ON THE SAME SIZE CONNECTOR BOARD, BUT THE BOTTOM
BOARD IS ABOUT 1 1/4" LONGER AND WIDER

THE CHIPS THAT HAVE THE DESIGNATION SCRATCHED OFF ON THE ORIGINAL
BOARDS ARE      NAME          CHIP TYPE
            ---------------------------
CONN BOARD      IC3             TMP8039P-6
 "              IC6             AY3-8910
 "              IC26            Z-80
 "              IC39            MC68A09EP (CUSTOM ON ORIGINAL)
 "              IC27            N/U       (CUSTOM ON ORIGINAL)
REAR BOARD      1C026           N/U       (CUSTOM ON ORIGINAL)


***************************************************************************/

#include "emu.h"
#include "includes/megazone.h"
#include "includes/konamipt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/konami1.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "screen.h"
#include "speaker.h"


READ8_MEMBER(megazone_state::megazone_port_a_r)
{
	int clock, timer;


	/* main xtal 14.318MHz, divided by 8 to get the AY-3-8910 clock, further */
	/* divided by 1024 to get this timer */
	/* The base clock for the CPU and 8910 is NOT the same, so we have to */
	/* compensate. */
	/* (divide by (1024/2), and not 1024, because the CPU cycle counter is */
	/* incremented every other state change of the clock) */

	clock = m_audiocpu->total_cycles() * 7159/12288;    /* = (14318/8)/(18432/6) */
	timer = (clock / (1024/2)) & 0x0f;

	/* low three bits come from the 8039 */
	return (timer << 4) | m_i8039_status;
}

WRITE8_MEMBER(megazone_state::megazone_port_b_w)
{
	for (int i = 0; i < 3; i++)
	{
		int C = 0;
		if (data & 1)
			C +=  10000;    /*  10000pF = 0.01uF */
		if (data & 2)
			C += 220000;    /* 220000pF = 0.22uF */

		data >>= 2;
		m_filter[i]->filter_rc_set_RC(filter_rc_device::LOWPASS, 1000, 2200, 200, CAP_P(C));
	}
}

WRITE8_MEMBER(megazone_state::megazone_i8039_irq_w)
{
	m_daccpu->set_input_line(0, ASSERT_LINE);
}

WRITE8_MEMBER(megazone_state::i8039_irqen_and_status_w)
{
	if ((data & 0x80) == 0)
		m_daccpu->set_input_line(0, CLEAR_LINE);
	m_i8039_status = (data & 0x70) >> 4;
}

WRITE_LINE_MEMBER(megazone_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(megazone_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE_LINE_MEMBER(megazone_state::irq_mask_w)
{
	m_irq_mask = state;
}


void megazone_state::megazone_map(address_map &map)
{
	map(0x0000, 0x0007).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x0800, 0x0800).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x1000, 0x1000).writeonly().share("scrolly");
	map(0x1800, 0x1800).writeonly().share("scrollx");
	map(0x2000, 0x23ff).ram().share("videoram");
	map(0x2400, 0x27ff).ram().share("videoram2");
	map(0x2800, 0x2bff).ram().share("colorram");
	map(0x2c00, 0x2fff).ram().share("colorram2");
	map(0x3000, 0x33ff).ram().share("spriteram");
	map(0x3800, 0x3fff).ram().share("share1");
	map(0x4000, 0xffff).rom();     /* 4000->5FFF is a debug rom */
}

void megazone_state::megazone_sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x2000).w(FUNC(megazone_state::megazone_i8039_irq_w)); /* START line. Interrupts 8039 */
	map(0x4000, 0x4000).w("soundlatch", FUNC(generic_latch_8_device::write));            /* CODE  line. Command Interrupts 8039 */
	map(0x6000, 0x6000).portr("IN0");            /* IO Coin */
	map(0x6001, 0x6001).portr("IN1");            /* P1 IO */
	map(0x6002, 0x6002).portr("IN2");            /* P2 IO */
	map(0x8000, 0x8000).portr("DSW2");
	map(0x8001, 0x8001).portr("DSW1");
	map(0xa000, 0xa000).nopw();                    /* INTMAIN - Interrupts main CPU (unused) */
	map(0xc000, 0xc000).nopw();                    /* INT (Actually is NMI) enable/disable (unused)*/
	map(0xc001, 0xc001).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0xe000, 0xe7ff).ram().share("share1");
}

void megazone_state::megazone_sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("aysnd", FUNC(ay8910_device::address_w));
	map(0x00, 0x02).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x02).w("aysnd", FUNC(ay8910_device::data_w));
}

void megazone_state::megazone_i8039_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
}

void megazone_state::megazone_i8039_io_map(address_map &map)
{
	map(0x00, 0xff).r("soundlatch", FUNC(generic_latch_8_device::read));
}

static INPUT_PORTS_START( megazone )
	/* 0x6000 -> 0xe320 (CPU1) = 0x3b20 (CPU0) */
	PORT_START("IN0")
	KONAMI8_SYSTEM_UNK

	/* 0x6001 -> 0xe31e (CPU1) = 0x3b1e (CPU0) */
	PORT_START("IN1")
	KONAMI8_MONO_B1_UNK

	/* 0x6002 -> 0xe31e (CPU1) = 0x3b1e (CPU0) or 0xe31f (CPU1) = 0x3b1f (CPU0) in "test mode" */
	PORT_START("IN2")
	KONAMI8_COCKTAIL_B1_UNK

	/* 0x8001 -> 0xe021 (CPU1) = 0x3821 (CPU0) */
	PORT_START("DSW1")
	KONAMI_COINAGE_LOC(DEF_STR( Free_Play ), "No Coin B", SW1)
	/* "No Coin B" = coins produce sound, but no effect on coin counter */

	/* 0x8000 -> 0xe020 (CPU1) = 0x3820 (CPU0) */
	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "20k 70k 70k+" )
	PORT_DIPSETTING(    0x10, "20k 80k 80k+" )
	PORT_DIPSETTING(    0x08, "30k 90k 90k+" )
	PORT_DIPSETTING(    0x00, "30k 100k 100k+" )
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x60, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( megazona )
	PORT_INCLUDE( megazone )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x00, "7" )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	4,      /* 4 bits per pixel */
	{ 0, 1, 2, 3 }, /* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	4,  /* 4 bits per pixel */
	{ 0x4000*8+4, 0x4000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,
		32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 64 consecutive bytes */
};

static GFXDECODE_START( gfx_megazone )
	GFXDECODE_ENTRY( "gfx1", 0, spritelayout,     0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,   16*16, 16 )
GFXDECODE_END

void megazone_state::machine_start()
{
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_i8039_status));
	save_item(NAME(m_irq_mask));
}

void megazone_state::machine_reset()
{
	m_i8039_status = 0;
}

WRITE_LINE_MEMBER(megazone_state::vblank_irq)
{
	if (state && m_irq_mask)
		m_maincpu->set_input_line(0, HOLD_LINE);

	if (state)
		m_audiocpu->set_input_line(0, HOLD_LINE);
}


void megazone_state::megazone(machine_config &config)
{
	/* basic machine hardware */
	KONAMI1(config, m_maincpu, XTAL(18'432'000)/9);        /* 2.048 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &megazone_state::megazone_map);

	Z80(config, m_audiocpu, XTAL(18'432'000)/6);     /* Z80 Clock is derived from the H1 signal */
	m_audiocpu->set_addrmap(AS_PROGRAM, &megazone_state::megazone_sound_map);
	m_audiocpu->set_addrmap(AS_IO, &megazone_state::megazone_sound_io_map);

	I8039(config, m_daccpu, XTAL(14'318'181)/2);    /* 7.15909MHz */
	m_daccpu->set_addrmap(AS_PROGRAM, &megazone_state::megazone_i8039_map);
	m_daccpu->set_addrmap(AS_IO, &megazone_state::megazone_i8039_io_map);
	m_daccpu->p1_out_cb().set("dac", FUNC(dac_byte_interface::data_w));
	m_daccpu->p2_out_cb().set(FUNC(megazone_state::i8039_irqen_and_status_w));

	config.m_minimum_quantum = attotime::from_hz(900);

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 13A
	mainlatch.q_out_cb<0>().set(FUNC(megazone_state::coin_counter_2_w));
	mainlatch.q_out_cb<1>().set(FUNC(megazone_state::coin_counter_1_w));
	mainlatch.q_out_cb<5>().set(FUNC(megazone_state::flipscreen_w));
	mainlatch.q_out_cb<7>().set(FUNC(megazone_state::irq_mask_w));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(36*8, 32*8);
	screen.set_visarea(0*8, 36*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(megazone_state::screen_update_megazone));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(megazone_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_megazone);
	PALETTE(config, m_palette, FUNC(megazone_state::megazone_palette), 16*16+16*16, 32);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, "soundlatch");

	ay8910_device &aysnd(AY8910(config, "aysnd", XTAL(14'318'181)/8));
	aysnd.port_a_read_callback().set(FUNC(megazone_state::megazone_port_a_r));
	aysnd.port_b_write_callback().set(FUNC(megazone_state::megazone_port_b_w));
	aysnd.add_route(0, "filter.0.0", 0.30);
	aysnd.add_route(1, "filter.0.1", 0.30);
	aysnd.add_route(2, "filter.0.2", 0.30);

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // unknown DAC
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);

	FILTER_RC(config, m_filter[0]).add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_RC(config, m_filter[1]).add_route(ALL_OUTPUTS, "speaker", 1.0);
	FILTER_RC(config, m_filter[2]).add_route(ALL_OUTPUTS, "speaker", 1.0);
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( megazone )
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "319_l07.11h",  0x6000, 0x2000, CRC(73b616ca) SHA1(ecdcdd085020f8ffe87a574832f7cedb9bcacef9) )
	ROM_LOAD( "319_l06.9h",   0x8000, 0x2000, CRC(0ced03f9) SHA1(4c8688b7bde5ee1adfe6a0db7178cca046eca7f4) )
	ROM_LOAD( "319_l05.8h",   0xa000, 0x2000, CRC(9dc3b5a1) SHA1(d6373f5be06cd61b6d3ffbe36c160167ba9852f3) )
	ROM_LOAD( "319_l04.7h",   0xc000, 0x2000, CRC(785b983d) SHA1(389e2f5494284089d39249e91293f3998c2b22c0) )
	ROM_LOAD( "319_l03.6h",   0xe000, 0x2000, CRC(a5318686) SHA1(8c8fbb76a36108f4a0b3a50e8a9b1781508490f1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazonej ) // Interlogic + Kosuka license set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319_j07.11h",  0x6000, 0x2000,  CRC(5161a523) SHA1(90b456c30bccaaca96c75c2f421af3a2875b0b6b) )
	ROM_LOAD( "319_j06.9h",   0x8000, 0x2000,  CRC(7344c3de) SHA1(d3867738d4828afa50c8b43116d68cc6074d6cb5) )
	ROM_LOAD( "319_j05.8h",   0xa000, 0x2000,  CRC(affa492b) SHA1(ee6789f293902716d65d08a89ae12dd96c75c885) )
	ROM_LOAD( "319_j04.7h",   0xc000, 0x2000,  CRC(03544ab3) SHA1(efa034cc6976b47915601cf215758df23e308878) )
	ROM_LOAD( "319_j03.6h",   0xe000, 0x2000,  CRC(0d95cc0a) SHA1(9aadadf09a4826da451ee35c89ee0254ec552d80) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazonei )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319_i07.11h",  0x6000, 0x2000, CRC(94b22ea8) SHA1(dc3ed2a0d1a12df51e46561324d78b7d655be313) )
	ROM_LOAD( "319_i06.9h",   0x8000, 0x2000, CRC(0468b619) SHA1(a6755728fab37674749f9b77cb53f6f228102f2f) )
	ROM_LOAD( "319_i05.8h",   0xa000, 0x2000, CRC(ac59000c) SHA1(c7568589f6b0e1706e996fdfed9c16755541951e) )
	ROM_LOAD( "319_i04.7h",   0xc000, 0x2000, CRC(1e968603) SHA1(fd818f678a3dc8d48a30f9f7670bfcb42a3009a2) )
	ROM_LOAD( "319_i03.6h",   0xe000, 0x2000, CRC(0888b803) SHA1(37249bfb14c6c3ce40ad68be457ab1f66fd7ea70) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_e12.8c",   0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) )
	ROM_LOAD( "319_e13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) )

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazoneh ) // Kosuka license set
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "319_h07.11h",  0x6000, 0x2000,  CRC(8ca47f64) SHA1(1a20db5ac504b9b004116cfa6992d63a86a04cc5) )
	ROM_LOAD( "319_h06.9h",   0x8000, 0x2000,  CRC(ed35b12e) SHA1(69e88c4801c838a24aba0a867af205a7169ad089) )
	ROM_LOAD( "319_h05.8h",   0xa000, 0x2000,  CRC(c3655ccd) SHA1(b86b58a12c6ced9a7e0a6d0cdb3881a28220a650) )
	ROM_LOAD( "319_h04.7h",   0xc000, 0x2000,  CRC(9e221177) SHA1(0c6fffd657d66090362108578aa78eb36bdcce6b) )
	ROM_LOAD( "319_h03.6h",   0xe000, 0x2000,  CRC(9048955b) SHA1(d8a7b46d984832f566298d3b417b3a34c9fea6c7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319h01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "prom.48",     0x0240, 0x020, CRC(796dea94) SHA1(bab3c2a5466e1c07ec27cccf7b1a21e9de4ed982) ) /* timing (not used) */
ROM_END

ROM_START( megazonea ) // Interlogic + Kosuka license set
	ROM_REGION( 2*0x10000, "maincpu", 0 )
	ROM_LOAD( "ic59_cpu.bin",  0x6000, 0x2000, CRC(f41922a0) SHA1(9f54509da18721a76593921c6e52085e62e6ea6b) ) // need to correct program / region code ID
	ROM_LOAD( "ic58_cpu.bin",  0x8000, 0x2000, CRC(7fd7277b) SHA1(e773247e0c9419cae49e04962ea362a2976c2db2) )
	ROM_LOAD( "ic57_cpu.bin",  0xa000, 0x2000, CRC(a4b33b51) SHA1(12bb4da0319a7fe355e5ea4945759c8709aed5fe) )
	ROM_LOAD( "ic56_cpu.bin",  0xc000, 0x2000, CRC(2aabcfbf) SHA1(f0054af98bd68158eab3328f8cf2a04b35e812c7) )
	ROM_LOAD( "ic55_cpu.bin",  0xe000, 0x2000, CRC(b33a3c37) SHA1(2f1fdf1b9f18fcc9bd97cc9adeecc4ce77dd30c9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319e02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) )

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319e01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_g12.8c",   0x0000, 0x2000, CRC(07b8b24b) SHA1(faadcb20ee8b26b9ab0692df6a81e5423514863e) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END

ROM_START( megazoneb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7.11h",  0x6000, 0x2000, CRC(d42d67bf) SHA1(adac80d183ad26a9b1ec25a2da7ebbb33b441b63) ) // need to correct program / region code ID
	ROM_LOAD( "6.9h",   0x8000, 0x2000, CRC(692398eb) SHA1(518001d738c2fb9417e52edfe9a7b74a074af3b0) )
	ROM_LOAD( "5.8h",   0xa000, 0x2000, CRC(620ffec3) SHA1(e047beb29e0cda72126e8dcdd0b7504a202efba2) )
	ROM_LOAD( "4.7h",   0xc000, 0x2000, CRC(28650971) SHA1(25e405fb9f648b7118e3c7c7b3ba59a7b7c29c42) )
	ROM_LOAD( "3.6h",   0xe000, 0x2000, CRC(f264018f) SHA1(6ca0f7e26311799b0650a6c58567405bc2a7f922) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "319h02.6d",    0x0000, 0x2000, CRC(d5d45edb) SHA1(3808d1b58fe152f8f5b49bf0aa40c53e9c9dd4bd) ) // same as e02

	ROM_REGION( 0x1000, "daccpu", 0 )     /* 4k for the 8039 DAC CPU */
	ROM_LOAD( "319h01.3a",    0x0000, 0x1000, CRC(ed5725a0) SHA1(64f54621487291fbfe827fb4cecca299fd0db781) ) // same as e01

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "319e11.3e",    0x0000, 0x2000, CRC(965a7ff6) SHA1(210aae91a3838e5f7c78747d9b7419d266538ffc) )
	ROM_LOAD( "319e09.2e",    0x2000, 0x2000, CRC(5eaa7f3e) SHA1(4c038e80d575988407252897a1f1bc6b76af597c) )
	ROM_LOAD( "319e10.3d",    0x4000, 0x2000, CRC(7bb1aeee) SHA1(be2dd46cd0121cedad6dab90a22643798a3176ab) )
	ROM_LOAD( "319e08.2d",    0x6000, 0x2000, CRC(6add71b1) SHA1(fc8c0ecd3b7f03d63b6c3143143986883345fa38) )

	ROM_REGION( 0x04000, "gfx2", 0 )
	ROM_LOAD( "319_e12.8c",   0x0000, 0x2000, CRC(e0fb7835) SHA1(44ccaaf92bdb83323f45e08dbe118697720e9105) )
	ROM_LOAD( "319_g13.10c",  0x2000, 0x2000, CRC(3d8f3743) SHA1(1f6fbf804dacfa44cd11b4cf41d0bedb7f2ff6b6) ) // same as e13

	ROM_REGION( 0x0260, "proms", 0 )
	ROM_LOAD( "319b18.a16",  0x0000, 0x020, CRC(23cb02af) SHA1(aba459826a75ec07bc6d97580e934f58aa22f4f4) ) /* palette */
	ROM_LOAD( "319b16.c6",   0x0020, 0x100, CRC(5748e933) SHA1(c1478c31533a11421cd4579ccfdbb430e193d17b) ) /* sprite lookup table */
	ROM_LOAD( "319b17.a11",  0x0120, 0x100, CRC(1fbfce73) SHA1(1c58eb91982d5f10511d54a83070e859ac57d2f1) ) /* character lookup table */
	ROM_LOAD( "319b14.e7",   0x0220, 0x020, CRC(55044268) SHA1(29cf4158314ed897daf045a7f07be865dd977663) ) /* timing (not used) */
	ROM_LOAD( "319b15.e8",   0x0240, 0x020, CRC(31fd7ab9) SHA1(04d6e51b4930619c8ee12fb0d7b5f981e4d6d8d3) ) /* timing (not used) */
ROM_END



GAME( 1983, megazone, 0,         megazone, megazone, megazone_state, empty_init, ROT90, "Konami",                               "Mega Zone (program code L)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazonej, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami (Interlogic / Kosuka license)", "Mega Zone (program code J)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazonei, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami",                               "Mega Zone (program code I)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazoneh, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami (Kosuka license)",              "Mega Zone (program code H)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazonea, megazone, megazone, megazone, megazone_state, empty_init, ROT90, "Konami (Interlogic / Kosuka license)", "Mega Zone (unknown program code 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, megazoneb, megazone, megazone, megazona, megazone_state, empty_init, ROT90, "Konami",                               "Mega Zone (unknown program code 2)", MACHINE_SUPPORTS_SAVE )
