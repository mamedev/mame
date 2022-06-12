// license:BSD-3-Clause
// copyright-holders:Mike Balfour
/***************************************************************************

    Atari Avalanche hardware

    driver by Mike Balfour

    Games supported:
        * Avalanche
        * Catch (prototype)

    TODO:
        * Catch discrete sound

****************************************************************************

    Memory Map:
                    0000-1FFF               RAM
                    2000-2FFF       R       INPUTS
                    3000-3FFF       W       WATCHDOG
                    4000-4FFF       W       OUTPUTS
                    5000-5FFF       W       NOISE SOUND LEVEL (not in Catch)
                    6000-7FFF       R       PROGRAM ROM
                    8000-DFFF               UNUSED
                    E000-FFFF               PROGRAM ROM (remapped)

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

***************************************************************************/

#include "emu.h"
#include "includes/avalnche.h"
#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "screen.h"

#include "avalnche.lh"


TIMER_DEVICE_CALLBACK_MEMBER(avalnche_state::nmi_16v)
{
	m_maincpu->pulse_input_line(m6502_device::NMI_LINE, attotime::zero);
}

/*************************************
 *
 *  Video update
 *
 *************************************/

uint32_t avalnche_state::screen_update_avalnche(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t x = offs << 3;
		int const y = offs >> 5;
		uint8_t data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen;

			if (m_avalance_video_inverted)
				pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
			else
				pen = (data & 0x80) ? rgb_t::black() : rgb_t::white();

			bitmap.pix(y, x) = pen;

			data <<= 1;
			x++;
		}
	}

	return 0;
}


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

WRITE_LINE_MEMBER(avalnche_state::video_invert_w)
{
	m_avalance_video_inverted = state;
}

void avalnche_state::catch_coin_counter_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

void avalnche_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).ram().share("videoram");
	map(0x2000, 0x2000).mirror(0x0ffc).portr("IN0");
	map(0x2001, 0x2001).mirror(0x0ffc).portr("IN1");
	map(0x2002, 0x2002).mirror(0x0ffc).portr("PADDLE");
	map(0x2003, 0x2003).mirror(0x0ffc).nopr();
	map(0x3000, 0x3000).mirror(0x0fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x4000, 0x4007).mirror(0x0ff8).w("latch", FUNC(f9334_device::write_d0));
	map(0x5000, 0x5000).mirror(0x0fff).w(FUNC(avalnche_state::avalnche_noise_amplitude_w));
	map(0x6000, 0x7fff).rom();
}

void avalnche_state::catch_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x1fff).ram().share("videoram");
	map(0x2000, 0x2000).mirror(0x0ffc).portr("IN0");
	map(0x2001, 0x2001).mirror(0x0ffc).portr("IN1");
	map(0x2002, 0x2002).mirror(0x0ffc).portr("PADDLE");
	map(0x2003, 0x2003).mirror(0x0ffc).nopr();
	map(0x3000, 0x3000).mirror(0x0fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x4000, 0x4007).mirror(0x0ff8).w("latch", FUNC(f9334_device::write_d0));
	map(0x6000, 0x6000).mirror(0x0fff).w(FUNC(avalnche_state::catch_coin_counter_w));
	map(0x7000, 0x7fff).rom();
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( avalnche )
	PORT_START("IN0")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x01, "SW:6" )         /* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SW:5" )         /* Listed as "Unused" */
	PORT_DIPNAME( 0x0c, 0x04, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Spanish ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_DIPNAME( 0x04, 0x04, "Allow Extended Play" )   PORT_DIPLOCATION("SW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x00, "Lives/Extended Play" )   PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, "3/450 Points" )
	PORT_DIPSETTING(    0x08, "5/750 Points" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* SLAM */
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )       /* Serve */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")           /* VBLANK */

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x40, 0xb7) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
INPUT_PORTS_END

static INPUT_PORTS_START( cascade )
	PORT_INCLUDE( avalnche )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Italian ) )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x08, 0x00, "Extended Play" )         PORT_DIPLOCATION("SW:7")
	PORT_DIPSETTING(    0x00, "1500 Points" )
	PORT_DIPSETTING(    0x08, "2500 Points" )
INPUT_PORTS_END

static INPUT_PORTS_START( catch )
	PORT_INCLUDE( avalnche )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )     PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x30, DEF_STR( German ) )
	PORT_DIPSETTING(    0x20, DEF_STR( French ) )
	PORT_DIPSETTING(    0x10, DEF_STR( English ) )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x0c, 0x00, "Extended Play" )         PORT_DIPLOCATION("SW:8,7")
	PORT_DIPSETTING(    0x00, "600 Points" )
	PORT_DIPSETTING(    0x04, "850 Points" )
	PORT_DIPSETTING(    0x08, "1100 Points" )
	PORT_DIPSETTING(    0x0c, "1350 Points" )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_MODIFY("PADDLE")
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(75) PORT_KEYDELTA(10) PORT_CENTERDELTA(0)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void avalnche_state::machine_start()
{
	save_item(NAME(m_avalance_video_inverted));
}

void avalnche_state::avalnche_base(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 12.096_MHz_XTAL / 16);     /* clock input is the "2H" signal divided by two */
	m_maincpu->set_addrmap(AS_PROGRAM, &avalnche_state::main_map);

	TIMER(config, "16v").configure_scanline(FUNC(avalnche_state::nmi_16v), "screen", 32, 32); // 8 interrupts per frame

	F9334(config, m_latch); // F8
	m_latch->q_out_cb<0>().set_output("led0"); // 1 CREDIT LAMP
	m_latch->q_out_cb<2>().set(FUNC(avalnche_state::video_invert_w));
	m_latch->q_out_cb<3>().set_output("led1"); // 2 CREDIT LAMP
	m_latch->q_out_cb<7>().set_output("led2"); // START LAMP
	// Q1, Q4, Q5, Q6 are configured in audio/avalnche.cpp

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count("screen", 10);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12.096_MHz_XTAL / 2, 384, 0, 256, 262, 16, 256);
	screen.set_screen_update(FUNC(avalnche_state::screen_update_avalnche));
}

void avalnche_state::avalnche(machine_config &config)
{
	avalnche_base(config);
	/* sound hardware */
	avalnche_sound(config);
}

void avalnche_state::acatch(machine_config &config)
{
	avalnche_base(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &avalnche_state::catch_map);

	/* sound hardware... */
	acatch_sound(config);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( avalnche )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH(  "30612-01.d2",     0x6800, 0x0800, CRC(3f975171) SHA1(afe680865da97824f1ebade4c7a2ba5d7ee2cbab) )
	ROM_LOAD_NIB_LOW (  "30615-01.d3",     0x6800, 0x0800, CRC(3e1a86b4) SHA1(3ff4cffea5b7a32231c0996473158f24c3bbe107) )
	ROM_LOAD_NIB_HIGH(  "30613-01.e2",     0x7000, 0x0800, CRC(47a224d3) SHA1(9feb7444a2e5a3d90a4fe78ae5d23c3a5039bfaa) )
	ROM_LOAD_NIB_LOW (  "30616-01.e3",     0x7000, 0x0800, CRC(f620f0f8) SHA1(7802b399b3469fc840796c3145b5f63781090956) )
	ROM_LOAD_NIB_HIGH(  "30611-01.c2",     0x7800, 0x0800, CRC(0ad07f85) SHA1(5a1a873b14e63dbb69ee3686ba53f7ca831fe9d0) )
	ROM_LOAD_NIB_LOW (  "30614-01.c3",     0x7800, 0x0800, CRC(a12d5d64) SHA1(1647d7416bf9266d07f066d3797bda943e004d24) )
ROM_END

ROM_START( cascade )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD(           "10005.1a",     0x6800, 0x0400, CRC(54774594) SHA1(03e013b563675fb8a30bd69836466a353db9bfc7) )
	ROM_LOAD(           "10005.1b",     0x6c00, 0x0400, CRC(fd9575ad) SHA1(ed0a1343d3c0456d458561256d5ee966b6c213f4) )
	ROM_LOAD(           "10005.2a",     0x7000, 0x0400, CRC(12857c75) SHA1(e42fdee70bd19d6f60e88f106a49dbbd68c591cd) )
	ROM_LOAD(           "10005.2b",     0x7400, 0x0400, CRC(26361698) SHA1(587cc6f0dc068a74aac41c2cb3336d70d2dd91e5) )
	ROM_LOAD(           "10005.3a",     0x7800, 0x0400, CRC(d1f422ff) SHA1(65ecbf0a22ba340d6a1768ed029030bac9c19e0f) )
	ROM_LOAD(           "10005.3b",     0x7c00, 0x0400, CRC(bb243d96) SHA1(3a387a8c50cd9b0db37d12b94dc9e260892dbf21) )
ROM_END

ROM_START( catchp )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH(  "catch.e2",     0x7000, 0x0800, CRC(ce1911ef) SHA1(1237b3e2c69342d2cccc8a3a9303f1b39b66e927) ) // n82s185
	ROM_LOAD_NIB_LOW (  "catch.e3",     0x7000, 0x0800, CRC(32bf254a) SHA1(22920d43beb2f21c0862c3675a4ee3e230d889b2) ) // "
	ROM_LOAD_NIB_HIGH(  "catch.d2",     0x7800, 0x0800, CRC(404a9551) SHA1(1e966ebb6e08e86e858146fe6c070e96b80171f8) ) // "
	ROM_LOAD_NIB_LOW (  "catch.d3",     0x7800, 0x0800, CRC(fde4dd73) SHA1(bc6fbfed8099865bef35b4c3061c7f5541bcec23) ) // "

	ROM_REGION( 0x0200, "user1", 0 ) // ?
	ROM_LOAD(           "catch.k3",     0x0000, 0x0200, CRC(a0ffc6fc) SHA1(b5c1a170acbf490786ed6ecc582712c3d2a97f37) ) // n82s129
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAMEL( 1978, avalnche, 0,        avalnche, avalnche, avalnche_state, empty_init, ROT0, "Atari",            "Avalanche", MACHINE_SUPPORTS_SAVE, layout_avalnche )
GAMEL( 1978, cascade,  avalnche, avalnche, cascade,  avalnche_state, empty_init, ROT0, "bootleg? (Sidam)", "Cascade", MACHINE_SUPPORTS_SAVE, layout_avalnche )
GAME(  1977, catchp,   0,        acatch,   catch,    avalnche_state, empty_init, ROT0, "Atari",            "Catch (prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND ) // pre-production board, evolved into Avalanche
