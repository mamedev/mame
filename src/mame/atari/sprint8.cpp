// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 8 driver

***************************************************************************/

#include "emu.h"
#include "sprint8.h"
#include "cpu/m6800/m6800.h"




void sprint8_state::set_collision(int n)
{
	if (m_collision_reset == 0)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);

		m_collision_index = n;
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(sprint8_state::input_callback)
{
	static const char *const dialnames[] = { "DIAL1", "DIAL2", "DIAL3", "DIAL4", "DIAL5", "DIAL6", "DIAL7", "DIAL8" };

	for (int i = 0; i < 8; i++)
	{
		uint8_t val = ioport(dialnames[i])->read() >> 4;

		signed char delta = (val - m_dial[i]) & 15;

		if (delta & 8)
			delta |= 0xf0; /* extend sign to 8 bits */

		m_steer_flag[i] = (delta != 0);

		if (delta > 0)
			m_steer_dir[i] = 0;

		if (delta < 0)
			m_steer_dir[i] = 1;

		m_dial[i] = val;
	}
}

void sprint8_state::machine_start()
{
	save_item(NAME(m_steer_dir));
	save_item(NAME(m_steer_flag));
	save_item(NAME(m_collision_reset));
	save_item(NAME(m_collision_index));
	save_item(NAME(m_dial));
	save_item(NAME(m_team));
}

void sprint8_state::machine_reset()
{
	m_collision_reset = 0;
	m_collision_index = 0;
}


uint8_t sprint8_state::collision_r()
{
	return m_collision_index;
}


uint8_t sprint8_state::input_r(offs_t offset)
{
	static const char *const portnames[] = { "P1", "P2", "P3", "P4", "P5", "P6", "P7", "P8" };
	uint8_t val = ioport(portnames[offset])->read();

	if (m_steer_dir[offset])
	{
		val |= 0x02;
	}
	if (m_steer_flag[offset])
	{
		val |= 0x04;
	}

	return val;
}


void sprint8_state::lockout_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(offset, !(data & 1));
}


WRITE_LINE_MEMBER(sprint8_state::int_reset_w)
{
	m_collision_reset = !state;

	if (m_collision_reset)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE_LINE_MEMBER(sprint8_state::team_w)
{
	m_team = state;
}


void sprint8_state::sprint8_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x1800, 0x1bff).ram().w(FUNC(sprint8_state::video_ram_w)).share("video_ram");
	map(0x1c00, 0x1c00).r(FUNC(sprint8_state::collision_r));
	map(0x1c01, 0x1c08).r(FUNC(sprint8_state::input_r));
	map(0x1c09, 0x1c09).portr("IN0");
	map(0x1c0a, 0x1c0a).portr("IN1");
	map(0x1c0f, 0x1c0f).portr("VBLANK");
	map(0x1c00, 0x1c0f).writeonly().share("pos_h_ram");
	map(0x1c10, 0x1c1f).writeonly().share("pos_v_ram");
	map(0x1c20, 0x1c2f).writeonly().share("pos_d_ram");
	map(0x1c30, 0x1c37).w(FUNC(sprint8_state::lockout_w));
	map(0x1d00, 0x1d07).w("latch", FUNC(f9334_device::write_d0));
	map(0x1e00, 0x1e07).w("motor", FUNC(f9334_device::write_d0));
	map(0x1f00, 0x1f00).nopw(); /* probably a watchdog, disabled in service mode */
	map(0x2000, 0x3fff).rom();
	map(0xf800, 0xffff).rom();
}


static INPUT_PORTS_START( sprint8 )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P1 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P1 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P2 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P2 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P3 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P3 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN4 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P4 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P4 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN5 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P5 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P5 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN6 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P6 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P6 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(6)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(6)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN7 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P7 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P7 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(7)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(7)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN8 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P8 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P8 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("DIAL3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("DIAL4")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)

	PORT_START("DIAL5")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(5)

	PORT_START("DIAL6")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(6)

	PORT_START("DIAL7")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(7)

	PORT_START("DIAL8")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(8)

	PORT_START("IN0")
	PORT_DIPNAME( 0x0f, 0x08, "Play Time" )
	PORT_DIPSETTING(    0x0f, "60 seconds" )
	PORT_DIPSETTING(    0x0e, "69 seconds" )
	PORT_DIPSETTING(    0x0d, "77 seconds" )
	PORT_DIPSETTING(    0x0c, "86 seconds" )
	PORT_DIPSETTING(    0x0b, "95 seconds" )
	PORT_DIPSETTING(    0x0a, "103 seconds" )
	PORT_DIPSETTING(    0x09, "112 seconds" )
	PORT_DIPSETTING(    0x08, "120 seconds" )
	PORT_DIPSETTING(    0x07, "129 seconds" )
	PORT_DIPSETTING(    0x06, "138 seconds" )
	PORT_DIPSETTING(    0x05, "146 seconds" )
	PORT_DIPSETTING(    0x04, "155 seconds" )
	PORT_DIPSETTING(    0x03, "163 seconds" )
	PORT_DIPSETTING(    0x02, "172 seconds" )
	PORT_DIPSETTING(    0x01, "181 seconds" )
	PORT_DIPSETTING(    0x00, "189 seconds" )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Track Select") PORT_CODE(KEYCODE_SPACE)

	PORT_START("VBLANK")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	/* this is actually a variable resistor */
	PORT_START("R132")
	PORT_ADJUSTER(65, "R132 - Crash & Screech Volume")

INPUT_PORTS_END


static INPUT_PORTS_START( sprint8p )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P1 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P1 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P2 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P2 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P3 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P3 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN4 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P4 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P4 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN5 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P5 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P5 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(5)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("P6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN6 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P6 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P6 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(6)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(6)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN7 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P7 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P7 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(7)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(7)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("P8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN8 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR P8 */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG P8 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(8)
	PORT_BIT( 0x10, IP_ACTIVE_LOW,  IPT_BUTTON2 ) PORT_PLAYER(8)
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED )

	PORT_START("DIAL1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("DIAL3")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(3)

	PORT_START("DIAL4")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(4)

	PORT_START("DIAL5")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(5)

	PORT_START("DIAL6")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(6)

	PORT_START("DIAL7")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(7)

	PORT_START("DIAL8")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(8)

	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x03, "Play Time" )
	PORT_DIPSETTING(    0x00, "54 seconds" )
	PORT_DIPSETTING(    0x01, "108 seconds" )
	PORT_DIPSETTING(    0x03, "216 seconds" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Play Mode" )
	PORT_DIPSETTING(    0x00, "Chase" )
	PORT_DIPSETTING(    0x01, "Tag" )

	PORT_START("VBLANK")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	/* this is actually a variable resistor */
	PORT_START("R132")
	PORT_ADJUSTER(65, "R132 - Crash & Screech Volume")

INPUT_PORTS_END


static const gfx_layout tile_layout_1 =
{
	16, 8,
	64,
	1,
	{ 0 },
	{
		7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0, 0
	},
	{
		0x000, 0x200, 0x400, 0x600, 0x800, 0xa00, 0xc00, 0xe00
	},
	8
};


static const gfx_layout tile_layout_2 =
{
	16, 8,
	64,
	1,
	{ 0 },
	{
		0x000, 0x000, 0x200, 0x200, 0x400, 0x400, 0x600, 0x600,
		0x800, 0x800, 0xa00, 0xa00, 0xc00, 0xc00, 0xe00, 0xe00

	},
	{
		0, 1, 2, 3, 4, 5, 6, 7
	},
	8
};


static const gfx_layout car_layout =
{
	16, 8,
	8,
	1,
	{ 0 },
	{
		0x07, 0x06, 0x05, 0x04, 0x0f, 0x0e, 0x0d, 0x0c,
		0x17, 0x16, 0x15, 0x14, 0x1f, 0x1e, 0x1d, 0x1c
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0
	},
	0x100
};


static GFXDECODE_START( gfx_sprint8 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout_1, 0, 18 )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout_2, 0, 18 )
	GFXDECODE_ENTRY( "gfx2", 0, car_layout, 0, 16 )
GFXDECODE_END


void sprint8_state::sprint8(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 11055000 / 11); /* ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &sprint8_state::sprint8_map);

	TIMER(config, "input_timer").configure_periodic(FUNC(sprint8_state::input_callback), attotime::from_hz(60));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(512, 261);
	m_screen->set_visarea(0, 495, 0, 231);
	m_screen->set_screen_update(FUNC(sprint8_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sprint8_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sprint8);
	PALETTE(config, m_palette, FUNC(sprint8_state::sprint8_palette), 36, 18);

	sprint8_audio(config);
}


ROM_START( sprint8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7313.j1", 0x2400, 0x0800, CRC(1231f944) SHA1(d16c76da6a74513eb40811d806e0dd009f6dcafb) )
	ROM_LOAD( "7314.h1", 0x2c00, 0x0800, CRC(c77c0d49) SHA1(a57b5d340a41d02edb20fcb66875908110582bc5) )
	ROM_RELOAD(          0xf800, 0x0800 )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "7315-01.n6", 0x0000, 0x0200, CRC(e2f603d0) SHA1(8d82b72d2f4039afa3341774000105a745caf85f) )

	ROM_REGION( 0x0100, "gfx2", 0 ) /* cars */
	ROM_LOAD( "7316-01.j5", 0x0000, 0x0100, CRC(32c028e3) SHA1(bfa76cf0981640d08e9c7fb15da134afe46afe31) )
ROM_END


ROM_START( sprint8a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_LOW ( "l2800s8", 0x2800, 0x0800, CRC(3926de69) SHA1(ec03d7684e393061d1d48ae73897e9dc38131c14) )
	ROM_LOAD_NIB_HIGH( "m2800s8", 0x2800, 0x0800, CRC(d009d6da) SHA1(3210806b0eb344d88d2cbcc46895f7224771c1f2) )
	ROM_LOAD_NIB_LOW ( "l3000s8", 0x3000, 0x0800, CRC(c78d9888) SHA1(a854b50b2cf0261c1f966ef1bd001084500b3545) )
	ROM_LOAD_NIB_HIGH( "m3000s8", 0x3000, 0x0800, CRC(9ebfe8f8) SHA1(9709f697a7f9cce7ff4edbdccbaf14931328a052) )
	ROM_LOAD_NIB_LOW ( "l3800s8", 0x3800, 0x0800, CRC(74a8f103) SHA1(0cc15006cbd4579feac0f07690f32b2b61f97ae9) )
	ROM_RELOAD(                   0xf800, 0x0800 )
	ROM_LOAD_NIB_HIGH( "m3800s8", 0x3800, 0x0800, CRC(90aadc75) SHA1(34ca21c37573d9a2df92d3a1e73fdc0a9885c0a0) )
	ROM_RELOAD(                   0xf800, 0x0800 )

	ROM_REGION( 0x0200, "gfx1", 0 ) /* tiles */
	ROM_LOAD( "s8.n6", 0x0000, 0x0200, CRC(92cf9a7e) SHA1(6bd2d396e0a299c2e731425cabd578d569c2061b) )

	ROM_REGION( 0x0100, "gfx2", 0 ) /* cars */
	ROM_LOAD( "s8.j5", 0x0000, 0x0100, CRC(d37fff36) SHA1(20a7a8caf2fbfe22e307fe8541d31784c8e39d1a) )
ROM_END


GAME( 1977, sprint8,  0,       sprint8, sprint8,  sprint8_state, empty_init, ROT0, "Atari", "Sprint 8",                    MACHINE_SUPPORTS_SAVE )
GAME( 1977, sprint8a, sprint8, sprint8, sprint8p, sprint8_state, empty_init, ROT0, "Atari", "Sprint 8 (play tag & chase)", MACHINE_SUPPORTS_SAVE )
