// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Sprint 8 driver

***************************************************************************/

#include "emu.h"

#include "sprint8_a.h"

#include "cpu/m6800/m6800.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class sprint8_state : public driver_device
{
public:
	sprint8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_video_ram(*this, "video_ram"),
		m_pos_h_ram(*this, "pos_h_ram"),
		m_pos_v_ram(*this, "pos_v_ram"),
		m_pos_d_ram(*this, "pos_d_ram"),
		m_in(*this, "P%u", 1U),
		m_dial(*this, "DIAL%u", 1U)
	{ }

	void sprint8(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<discrete_sound_device> m_discrete;

	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_pos_h_ram;
	required_shared_ptr<uint8_t> m_pos_v_ram;
	required_shared_ptr<uint8_t> m_pos_d_ram;

	required_ioport_array<8> m_in;
	required_ioport_array<8> m_dial;

	uint8_t m_steering[8]{};
	bool m_collision_reset = false;
	uint8_t m_collision_index = 0;
	uint8_t m_prev_dial[8]{};
	bool m_team = false;

	tilemap_t* m_tilemap[2]{};
	bitmap_ind16 m_helper[2];
	emu_timer *m_collision_timer = nullptr;

	uint8_t collision_r();
	uint8_t input_r(offs_t offset);
	void lockout_w(offs_t offset, uint8_t data);
	void int_reset_w(int state);
	void team_w(int state);
	void video_ram_w(offs_t offset, uint8_t data);

	void palette(palette_device &palette) const;

	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	TIMER_CALLBACK_MEMBER(collision_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(input_callback);

	void set_pens();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void program_map(address_map &map) ATTR_COLD;
};


void sprint8_state::palette(palette_device &palette) const
{
	for (int i = 0; i < 0x10; i++)
	{
		palette.set_pen_indirect(2 * i + 0, 0x10);
		palette.set_pen_indirect(2 * i + 1, i);
	}

	palette.set_pen_indirect(0x20, 0x10);
	palette.set_pen_indirect(0x21, 0x10);
	palette.set_pen_indirect(0x22, 0x10);
	palette.set_pen_indirect(0x23, 0x11);
}


void sprint8_state::set_pens()
{
	for (int i = 0; i < 0x10; i += 8)
	{
		if (m_team)
		{
			m_palette->set_indirect_color(i + 0, rgb_t(0xff, 0x00, 0x00)); // red
			m_palette->set_indirect_color(i + 1, rgb_t(0x00, 0x00, 0xff)); // blue
			m_palette->set_indirect_color(i + 2, rgb_t(0xff, 0xff, 0x00)); // yellow
			m_palette->set_indirect_color(i + 3, rgb_t(0x00, 0xff, 0x00)); // green
			m_palette->set_indirect_color(i + 4, rgb_t(0xff, 0x00, 0xff)); // magenta
			m_palette->set_indirect_color(i + 5, rgb_t(0xe0, 0xc0, 0x70)); // puce
			m_palette->set_indirect_color(i + 6, rgb_t(0x00, 0xff, 0xff)); // cyan
			m_palette->set_indirect_color(i + 7, rgb_t(0xff, 0xaa, 0xaa)); // pink
		}
		else
		{
			m_palette->set_indirect_color(i + 0, rgb_t(0xff, 0x00, 0x00)); // red
			m_palette->set_indirect_color(i + 1, rgb_t(0x00, 0x00, 0xff)); // blue
			m_palette->set_indirect_color(i + 2, rgb_t(0xff, 0x00, 0x00)); // red
			m_palette->set_indirect_color(i + 3, rgb_t(0x00, 0x00, 0xff)); // blue
			m_palette->set_indirect_color(i + 4, rgb_t(0xff, 0x00, 0x00)); // red
			m_palette->set_indirect_color(i + 5, rgb_t(0x00, 0x00, 0xff)); // blue
			m_palette->set_indirect_color(i + 6, rgb_t(0xff, 0x00, 0x00)); // red
			m_palette->set_indirect_color(i + 7, rgb_t(0x00, 0x00, 0xff)); // blue
		}
	}

	m_palette->set_indirect_color(0x10, rgb_t(0x00, 0x00, 0x00));
	m_palette->set_indirect_color(0x11, rgb_t(0xff, 0xff, 0xff));
}


TILE_GET_INFO_MEMBER(sprint8_state::get_tile_info1)
{
	uint8_t const code = m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x30) != 0x30) // ?
		color = 17;
	else
	{
		if ((tile_index + 1) & 0x010)
			color |= 1;

		if (code & 0x80)
			color |= 2;

		if (tile_index & 0x200)
			color |= 4;

	}

	tileinfo.set(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


TILE_GET_INFO_MEMBER(sprint8_state::get_tile_info2)
{
	uint8_t const code = m_video_ram[tile_index];

	int color = 0;

	if ((code & 0x38) != 0x28)
		color = 16;
	else
		color = 17;

	tileinfo.set(code >> 7, code, color, (code & 0x40) ? (TILE_FLIPX | TILE_FLIPY) : 0);
}


void sprint8_state::video_ram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_tilemap[0]->mark_tile_dirty(offset);
	m_tilemap[1]->mark_tile_dirty(offset);
}


void sprint8_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper[0]);
	m_screen->register_screen_bitmap(m_helper[1]);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprint8_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sprint8_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 16, 8, 32, 32);

	m_tilemap[0]->set_scrolly(0, +24);
	m_tilemap[1]->set_scrolly(0, +24);

	m_collision_timer = timer_alloc(FUNC(sprint8_state::collision_callback), this);
}


void sprint8_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 16; i++)
	{
		uint8_t const code = m_pos_d_ram[i];

		int x = m_pos_h_ram[i];
		int const y = m_pos_v_ram[i];

		if (code & 0x80)
			x |= 0x100;

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
			code ^ 7,
			i,
			!(code & 0x10), !(code & 0x08),
			496 - x, y - 31, 0);
	}
}


TIMER_CALLBACK_MEMBER(sprint8_state::collision_callback)
{
	if (!m_collision_reset)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);

		m_collision_index = param;
	}
}


uint32_t sprint8_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);
	draw_sprites(bitmap, cliprect);
	return 0;
}


void sprint8_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		const rectangle &visarea = m_screen->visible_area();

		m_tilemap[1]->draw(*m_screen, m_helper[1], visarea, 0, 0);

		m_helper[0].fill(0x20, visarea);

		draw_sprites(m_helper[0], visarea);

		for (int y = visarea.top(); y <= visarea.bottom(); y++)
		{
			uint16_t const *const p1 = &m_helper[0].pix(y);
			uint16_t const *const p2 = &m_helper[1].pix(y);

			for (int x = visarea.left(); x <= visarea.right(); x++)
				if (p1[x] != 0x20 && p2[x] == 0x23)
					m_collision_timer->adjust(m_screen->time_until_pos(y + 24, x), m_palette->pen_indirect(p1[x]));
		}
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(sprint8_state::input_callback)
{
	for (int i = 0; i < 8; i++)
	{
		uint8_t const val = m_dial[i]->read() >> 4;

		int8_t const delta = util::sext((val - m_prev_dial[i]) & 15, 4);

		// steer flag
		if (delta)
			m_steering[i] |= 0x04;
		else
			m_steering[i] &= ~0x04;

		// steer dir
		if (delta < 0)
			m_steering[i] |= 0x02;
		else if (delta > 0)
			m_steering[i] &= ~0x02;

		m_prev_dial[i] = val;
	}
}

void sprint8_state::machine_start()
{
	save_item(NAME(m_steering));
	save_item(NAME(m_collision_reset));
	save_item(NAME(m_collision_index));
	save_item(NAME(m_prev_dial));
	save_item(NAME(m_team));
}

void sprint8_state::machine_reset()
{
	m_collision_reset = false;
	m_collision_index = 0;
}


uint8_t sprint8_state::collision_r()
{
	return m_collision_index;
}


uint8_t sprint8_state::input_r(offs_t offset)
{
	return m_in[offset]->read() | m_steering[offset];
}


void sprint8_state::lockout_w(offs_t offset, uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(offset, !(data & 1));
}


void sprint8_state::int_reset_w(int state)
{
	m_collision_reset = !state;

	if (m_collision_reset)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

void sprint8_state::team_w(int state)
{
	m_team = state;
}


void sprint8_state::program_map(address_map &map)
{
	map(0x0000, 0x00ff).ram();
	map(0x1800, 0x1bff).ram().w(FUNC(sprint8_state::video_ram_w)).share(m_video_ram);
	map(0x1c00, 0x1c00).r(FUNC(sprint8_state::collision_r));
	map(0x1c01, 0x1c08).r(FUNC(sprint8_state::input_r));
	map(0x1c09, 0x1c09).portr("IN0");
	map(0x1c0a, 0x1c0a).portr("IN1");
	map(0x1c0f, 0x1c0f).portr("VBLANK");
	map(0x1c00, 0x1c0f).writeonly().share(m_pos_h_ram);
	map(0x1c10, 0x1c1f).writeonly().share(m_pos_v_ram);
	map(0x1c20, 0x1c2f).writeonly().share(m_pos_d_ram);
	map(0x1c30, 0x1c37).w(FUNC(sprint8_state::lockout_w));
	map(0x1d00, 0x1d07).w("latch", FUNC(f9334_device::write_d0));
	map(0x1e00, 0x1e07).w("motor", FUNC(f9334_device::write_d0));
	map(0x1f00, 0x1f00).nopw(); // probably a watchdog, disabled in service mode
	map(0x2000, 0x3fff).rom();
	map(0xf800, 0xffff).rom();
}

#define SPRINT8_PLAYER_INPUT(player) \
	PORT_START("P" #player) \
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_COIN ## player ) \
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER DIR */ \
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) /* STEER FLAG */ \
	PORT_BIT( 0x08, IP_ACTIVE_LOW,  IPT_BUTTON1 ) PORT_PLAYER(player) PORT_NAME("%p Accelerate") \
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(player) PORT_NAME("%p Gear Shift") PORT_TOGGLE \
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_UNUSED ) \
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED ) \
	PORT_BIT( 0x80, IP_ACTIVE_LOW,  IPT_UNUSED ) \
												 \
	PORT_START("DIAL" #player) \
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(100) PORT_KEYDELTA(10) PORT_PLAYER(player)

static INPUT_PORTS_START( sprint8 )
	SPRINT8_PLAYER_INPUT( 1 )
	SPRINT8_PLAYER_INPUT( 2 )
	SPRINT8_PLAYER_INPUT( 3 )
	SPRINT8_PLAYER_INPUT( 4 )
	SPRINT8_PLAYER_INPUT( 5 )
	SPRINT8_PLAYER_INPUT( 6 )
	SPRINT8_PLAYER_INPUT( 7 )
	SPRINT8_PLAYER_INPUT( 8 )

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

	// this is actually a variable resistor
	PORT_START("R132")
	PORT_ADJUSTER(65, "R132 - Crash & Screech Volume")
INPUT_PORTS_END


static INPUT_PORTS_START( sprint8p )
	PORT_INCLUDE( sprint8 )

	PORT_MODIFY("IN0")
	PORT_DIPNAME( 0x03, 0x03, "Play Time" )
	PORT_DIPSETTING(    0x00, "54 seconds" )
	PORT_DIPSETTING(    0x01, "108 seconds" )
	PORT_DIPSETTING(    0x03, "216 seconds" )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Play Mode" )
	PORT_DIPSETTING(    0x00, "Chase" )
	PORT_DIPSETTING(    0x01, "Tag" )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
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
	GFXDECODE_ENTRY( "tiles", 0, tile_layout_1, 0, 18 )
	GFXDECODE_ENTRY( "tiles", 0, tile_layout_2, 0, 18 )
	GFXDECODE_ENTRY( "cars",  0, car_layout,    0, 16 )
GFXDECODE_END


void sprint8_state::sprint8(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, 11'055'000 / 11); // ?
	m_maincpu->set_addrmap(AS_PROGRAM, &sprint8_state::program_map);

	TIMER(config, "input_timer").configure_periodic(FUNC(sprint8_state::input_callback), attotime::from_hz(60));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(512, 261);
	m_screen->set_visarea(0, 495, 0, 231);
	m_screen->set_screen_update(FUNC(sprint8_state::screen_update));
	m_screen->screen_vblank().set(FUNC(sprint8_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_sprint8);
	PALETTE(config, m_palette, FUNC(sprint8_state::palette), 36, 18);

	// sound hardware
	/* the proper way is to hook up 4 speakers, but they are not really
	 * F/R/L/R speakers.  Though you can pretend the 1-2 mix is the front. */
	SPEAKER(config, "speaker_1_2", 0.0, 0.0, 1.0);      // front
	SPEAKER(config, "speaker_3_7", -0.2, 0.0, 1.0);     // left
	SPEAKER(config, "speaker_5_6",  0.0, 0.0, -0.5);    // back
	SPEAKER(config, "speaker_4_8", 0.2, 0.0, 1.0);      // right

	DISCRETE(config, m_discrete, sprint8_discrete);
	m_discrete->add_route(0, "speaker_1_2", 1.0);
	/* volumes on other channels defaulted to off,
	   user can turn them up if needed.
	   The game does not sound good with all channels mixed to stereo. */
	m_discrete->add_route(1, "speaker_3_7", 0.0);
	m_discrete->add_route(2, "speaker_5_6", 0.0);
	m_discrete->add_route(3, "speaker_4_8", 0.0);

	f9334_device &latch(F9334(config, "latch"));
	latch.q_out_cb<0>().set(FUNC(sprint8_state::int_reset_w));
	latch.q_out_cb<1>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_CRASH_EN>));
	latch.q_out_cb<2>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_SCREECH_EN>));
	latch.q_out_cb<5>().set(FUNC(sprint8_state::team_w));
	latch.q_out_cb<6>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_ATTRACT_EN>));

	f9334_device &motor(F9334(config, "motor"));
	motor.q_out_cb<0>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR1_EN>));
	motor.q_out_cb<1>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR2_EN>));
	motor.q_out_cb<2>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR3_EN>));
	motor.q_out_cb<3>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR4_EN>));
	motor.q_out_cb<4>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR5_EN>));
	motor.q_out_cb<5>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR6_EN>));
	motor.q_out_cb<6>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR7_EN>));
	motor.q_out_cb<7>().set(m_discrete, FUNC(discrete_device::write_line<SPRINT8_MOTOR8_EN>));
}


ROM_START( sprint8 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "7313.j1", 0x2400, 0x0800, CRC(1231f944) SHA1(d16c76da6a74513eb40811d806e0dd009f6dcafb) )
	ROM_LOAD( "7314.h1", 0x2c00, 0x0800, CRC(c77c0d49) SHA1(a57b5d340a41d02edb20fcb66875908110582bc5) )
	ROM_RELOAD(          0xf800, 0x0800 )

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD( "7315-01.n6", 0x0000, 0x0200, CRC(e2f603d0) SHA1(8d82b72d2f4039afa3341774000105a745caf85f) )

	ROM_REGION( 0x0100, "cars", 0 )
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

	ROM_REGION( 0x0200, "tiles", 0 )
	ROM_LOAD( "s8.n6", 0x0000, 0x0200, CRC(92cf9a7e) SHA1(6bd2d396e0a299c2e731425cabd578d569c2061b) )

	ROM_REGION( 0x0100, "cars", 0 )
	ROM_LOAD( "s8.j5", 0x0000, 0x0100, CRC(d37fff36) SHA1(20a7a8caf2fbfe22e307fe8541d31784c8e39d1a) )
ROM_END

} // anonymous namespace


GAME( 1977, sprint8,  0,       sprint8, sprint8,  sprint8_state, empty_init, ROT0, "Atari", "Sprint 8",                    MACHINE_SUPPORTS_SAVE )
GAME( 1977, sprint8a, sprint8, sprint8, sprint8p, sprint8_state, empty_init, ROT0, "Atari", "Sprint 8 (play tag & chase)", MACHINE_SUPPORTS_SAVE )
