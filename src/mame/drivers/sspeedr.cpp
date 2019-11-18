// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Taito Super Speed Race driver

***************************************************************************/

#include "emu.h"
#include "includes/sspeedr.h"

#include "cpu/z80/z80.h"
#include "machine/watchdog.h"
#include "screen.h"

#include "sspeedr.lh"



void sspeedr_state::sspeedr_palette(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		int r = BIT(i, 0) ? 0xb0 : 0x20;
		int g = BIT(i, 1) ? 0xb0 : 0x20;
		int b = BIT(i, 2) ? 0xb0 : 0x20;

		if (BIT(i, 3))
		{
			r += 0x4f;
			g += 0x4f;
			b += 0x4f;
		}

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


WRITE8_MEMBER(sspeedr_state::sspeedr_int_ack_w)
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
}


WRITE8_MEMBER(sspeedr_state::sspeedr_lamp_w)
{
	output().set_value("lampGO", BIT(data, 0));
	output().set_value("lampEP", BIT(data, 1));
	machine().bookkeeping().coin_counter_w(0, data & 8);
}


/* uses a 7447A, which is equivalent to an LS47/48 */
constexpr uint8_t ls48_map[16] =
	{ 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7c,0x07,0x7f,0x67,0x58,0x4c,0x62,0x69,0x78,0x00 };

WRITE8_MEMBER(sspeedr_state::sspeedr_time_w)
{
	data = data & 15;
	m_digits[24 + offset] = ls48_map[data];
	m_led_TIME[offset] = data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_score_w)
{
	data = ~data & 15;
	m_digits[offset] = ls48_map[data];
	m_led_SCORE[offset] = data;
}


WRITE8_MEMBER(sspeedr_state::sspeedr_sound_w)
{
	/* not implemented */
}


void sspeedr_state::sspeedr_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x7f00, 0x7f17).w(FUNC(sspeedr_state::sspeedr_score_w));
}


void sspeedr_state::sspeedr_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("IN0");
	map(0x01, 0x01).portr("IN1");
	map(0x00, 0x01).w(FUNC(sspeedr_state::sspeedr_sound_w));
	map(0x02, 0x02).w(FUNC(sspeedr_state::sspeedr_lamp_w));
	map(0x03, 0x03).portr("DSW");
	map(0x04, 0x04).portr("IN2");
	map(0x04, 0x05).w(FUNC(sspeedr_state::sspeedr_time_w));
	map(0x06, 0x06).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x10, 0x10).w(FUNC(sspeedr_state::sspeedr_driver_horz_w));
	map(0x11, 0x11).w(FUNC(sspeedr_state::sspeedr_driver_pic_w));
	map(0x12, 0x12).w(FUNC(sspeedr_state::sspeedr_driver_horz_2_w));
	map(0x13, 0x13).w(FUNC(sspeedr_state::sspeedr_drones_horz_w));
	map(0x14, 0x14).w(FUNC(sspeedr_state::sspeedr_drones_horz_2_w));
	map(0x15, 0x15).w(FUNC(sspeedr_state::sspeedr_drones_mask_w));
	map(0x16, 0x16).w(FUNC(sspeedr_state::sspeedr_driver_vert_w));
	map(0x17, 0x18).w(FUNC(sspeedr_state::sspeedr_track_vert_w));
	map(0x19, 0x19).w(FUNC(sspeedr_state::sspeedr_track_horz_w));
	map(0x1a, 0x1a).w(FUNC(sspeedr_state::sspeedr_track_horz_2_w));
	map(0x1b, 0x1b).w(FUNC(sspeedr_state::sspeedr_track_ice_w));
	map(0x1c, 0x1e).w(FUNC(sspeedr_state::sspeedr_drones_vert_w));
	map(0x1f, 0x1f).w(FUNC(sspeedr_state::sspeedr_int_ack_w));
}


static const ioport_value sspeedr_controller_table[] =
{
	0x3f, 0x3e, 0x3c, 0x3d, 0x39, 0x38, 0x3a, 0x3b,
	0x33, 0x32, 0x30, 0x31, 0x35, 0x34, 0x36, 0x37,
	0x27, 0x26, 0x24, 0x25, 0x21, 0x20, 0x22, 0x23,
	0x2b, 0x2a, 0x28, 0x29, 0x2d, 0x2c, 0x2e, 0x2f,
	0x0f, 0x0e, 0x0c, 0x0d, 0x09, 0x08, 0x0a, 0x0b,
	0x03, 0x02, 0x00, 0x01, 0x05, 0x04, 0x06, 0x07,
	0x17, 0x16, 0x14, 0x15, 0x11, 0x10, 0x12, 0x13,
	0x1b, 0x1a, 0x18, 0x19, 0x1d, 0x1c, 0x1e, 0x1f
};


static INPUT_PORTS_START( sspeedr )

	PORT_START("IN0")
	PORT_BIT( 0x3f, 0x00, IPT_POSITIONAL ) PORT_POSITIONS(64) PORT_REMAP_TABLE(sspeedr_controller_table) PORT_WRAPS PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("IN1")
	/* The gas pedal is adjusted physically so the encoder is at position 2 when the pedal is not pressed. */
	/* It also only uses half of the encoder. */
	PORT_BIT( 0x1f, 0x00, IPT_POSITIONAL_V ) PORT_POSITIONS(30) PORT_REMAP_TABLE(sspeedr_controller_table + 2) PORT_SENSITIVITY(25) PORT_KEYDELTA(20)

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )       PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0c, 0x08, "Play Time" )              PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x00, "60 seconds")
	PORT_DIPSETTING(    0x04, "70 seconds")
	PORT_DIPSETTING(    0x08, "80 seconds")
	PORT_DIPSETTING(    0x0c, "90 seconds")
	PORT_DIPNAME( 0x10, 0x00, "Extended Play" )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "20 seconds" )
	PORT_DIPSETTING(    0x10, "30 seconds" )
	PORT_DIPNAME( 0xe0, 0x20, DEF_STR( Service_Mode ) )  PORT_DIPLOCATION("SW1:6,7,8")
	PORT_DIPSETTING(    0x20, "Play Mode" )
	PORT_DIPSETTING(    0xa0, "RAM/ROM Test" )
	PORT_DIPSETTING(    0xe0, "Accelerator Adjustment" )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Shifter") PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 )
INPUT_PORTS_END


static const gfx_layout car_layout =
{
	32, 16,
	16,
	4,
	{ 0, 1, 2, 3 },
	{
		0x04, 0x04, 0x00, 0x00, 0x0c, 0x0c, 0x08, 0x08,
		0x14, 0x14, 0x10, 0x10, 0x1c, 0x1c, 0x18, 0x18,
		0x24, 0x24, 0x20, 0x20, 0x2c, 0x2c, 0x28, 0x28,
		0x34, 0x34, 0x30, 0x30, 0x3c, 0x3c, 0x38, 0x38
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0
	},
	0x400
};


static GFXDECODE_START( gfx_sspeedr )
	GFXDECODE_ENTRY( "gfx1", 0, car_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, car_layout, 0, 1 )
GFXDECODE_END


void sspeedr_state::sspeedr(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, XTAL(19'968'000)/8);
	m_maincpu->set_addrmap(AS_PROGRAM, &sspeedr_state::sspeedr_map);
	m_maincpu->set_addrmap(AS_IO, &sspeedr_state::sspeedr_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(sspeedr_state::irq0_line_assert));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.39);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(16 * 1000000 / 15680));
	screen.set_size(376, 256);
	screen.set_visarea(0, 375, 0, 247);
	screen.set_screen_update(FUNC(sspeedr_state::screen_update_sspeedr));
	screen.screen_vblank().set(FUNC(sspeedr_state::screen_vblank_sspeedr));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sspeedr);
	PALETTE(config, m_palette, FUNC(sspeedr_state::sspeedr_palette), 16);

	/* sound hardware */
}


ROM_START( sspeedr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ssr0000.pgm", 0x0000, 0x0800, CRC(bfc7069a) SHA1(2f7aa3d3c7cfd804ba4b625c6a8338534a204855) )
	ROM_LOAD( "ssr0800.pgm", 0x0800, 0x0800, CRC(ec46b59a) SHA1(d5727efecb32ad3d034b885e4a57d7373368ca9e) )

	ROM_REGION( 0x0800, "gfx1", 0 ) /* driver */
	ROM_LOAD( "ssrm762a.f3", 0x0000, 0x0800, CRC(de4653a9) SHA1(a6bbffb7eb60581eee43c74d20ca00b50c9a6e07) )

	ROM_REGION( 0x0800, "gfx2", 0 ) /* drone */
	ROM_LOAD( "ssrm762b.j3", 0x0000, 0x0800, CRC(ef6a1cd6) SHA1(77c31f14783e5ba90849bdc930b099c8360aeba7) )

	ROM_REGION( 0x0800, "gfx3", 0 ) /* track */
	ROM_LOAD( "ssrm762c.l3", 0x0000, 0x0800, CRC(ebaad3ee) SHA1(54ac994b505d20c75cf07a4f68da12360ee00153) )
ROM_END


GAMEL( 1979, sspeedr, 0, sspeedr, sspeedr, sspeedr_state, empty_init, ROT270, "Midway", "Super Speed Race", MACHINE_NO_SOUND, layout_sspeedr )
