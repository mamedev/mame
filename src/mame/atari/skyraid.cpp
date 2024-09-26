// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch

/***************************************************************************

Atari Sky Raider driver

***************************************************************************/

#include "emu.h"

#include "skyraid_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class skyraid_state : public driver_device
{
public:
	skyraid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_pos_ram(*this, "pos_ram"),
		m_alpha_num_ram(*this, "alpha_num_ram"),
		m_obj_ram(*this, "obj_ram"),
		m_terrain_rom(*this, "terrain"),
		m_trapezoid_rom(*this, "trapezoid"),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_discrete(*this, "discrete"),
		m_language(*this, "LANGUAGE"),
		m_stick(*this, "STICK%c", 'X'),
		m_led(*this, "led")
	{ }

	void skyraid(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_pos_ram;
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_shared_ptr<uint8_t> m_obj_ram;

	required_region_ptr<uint8_t> m_terrain_rom;
	required_region_ptr<uint8_t> m_trapezoid_rom;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<discrete_device> m_discrete;

	required_ioport m_language;
	required_ioport_array<2> m_stick;
	output_finder<> m_led;

	uint8_t m_analog_range = 0;
	uint8_t m_analog_offset = 0;

	uint8_t m_scroll = 0;

	bitmap_ind16 m_helper;

	uint8_t port_0_r();
	void range_w(uint8_t data);
	void offset_w(uint8_t data);
	void scroll_w(uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_w(uint8_t data);
	void draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_terrain(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_missiles(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_trapezoid(bitmap_ind16& dst, bitmap_ind16& src);

	void program_map(address_map &map) ATTR_COLD;
};


void skyraid_state::video_start()
{
	m_helper.allocate(128, 240);

	m_screen->register_screen_bitmap(m_helper);
}


void skyraid_state::draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	const uint8_t* p = m_alpha_num_ram;

	for (int i = 0; i < 4; i++)
	{
		int const y = 136 + 16 * (i ^ 1);

		for (int x = 0; x < bitmap.width(); x += 16)
			m_gfxdecode->gfx(0)->transpen(bitmap, cliprect, *p++, 0, 0, 0, x, y, 0);
	}
}


void skyraid_state::draw_terrain(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = 0; y < bitmap.height(); y++)
	{
		int offset = (16 * m_scroll + 16 * ((y + 1) / 2)) & 0x7ff;

		int x = 0;

		while (x < bitmap.width())
		{
			uint8_t val = m_terrain_rom[offset++];

			int const color = val / 32;
			int const count = val % 32;

			rectangle r(x, x + 31 - count, y, y + 1);

			bitmap.fill(color, r);

			x += 32 - count;
		}
	}
}


void skyraid_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 4; i++)
	{
		int const code = m_obj_ram[8 + 2 * i + 0] & 15;
		int const flag = m_obj_ram[8 + 2 * i + 1] & 15;
		int vert = m_pos_ram[8 + 2 * i + 0];
		int const horz = m_pos_ram[8 + 2 * i + 1];

		vert -= 31;

		if (flag & 1)
			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				code ^ 15, code >> 3, 0, 0,
				horz / 2, vert, 2);
	}
}


void skyraid_state::draw_missiles(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// hardware is restricted to one sprite per scanline

	for (int i = 0; i < 4; i++)
	{
		int const code = m_obj_ram[2 * i + 0] & 15;
		int vert = m_pos_ram[2 * i + 0];
		int horz = m_pos_ram[2 * i + 1];

		vert -= 15;
		horz -= 31;

		m_gfxdecode->gfx(2)->transpen(bitmap, cliprect,
			code ^ 15, 0, 0, 0,
			horz / 2, vert, 0);
	}
}


void skyraid_state::draw_trapezoid(bitmap_ind16& dst, bitmap_ind16& src)
{
	for (int y = 0; y < dst.height(); y++)
	{
		uint16_t const *const pSrc = &src.pix(y);
		uint16_t *const pDst = &dst.pix(y);

		int const x1 = 0x000 + m_trapezoid_rom[(y & ~1) + 0];
		int const x2 = 0x100 + m_trapezoid_rom[(y & ~1) + 1];

		for (int x = x1; x < x2; x++)
			pDst[x] = pSrc[128 * (x - x1) / (x2 - x1)];
	}
}


uint32_t skyraid_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	rectangle helper_clip = cliprect;
	helper_clip &= m_helper.cliprect();

	draw_terrain(m_helper, helper_clip);
	draw_sprites(m_helper, helper_clip);
	draw_missiles(m_helper, helper_clip);
	draw_trapezoid(bitmap, m_helper);
	draw_text(bitmap, cliprect);
	return 0;
}


void skyraid_state::machine_start()
{
	m_led.resolve();

	save_item(NAME(m_analog_range));
	save_item(NAME(m_analog_offset));
	save_item(NAME(m_scroll));
}

void skyraid_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));   // terrain
	palette.set_pen_color(1, rgb_t(0x18, 0x18, 0x18));
	palette.set_pen_color(2, rgb_t(0x30, 0x30, 0x30));
	palette.set_pen_color(3, rgb_t(0x48, 0x48, 0x48));
	palette.set_pen_color(4, rgb_t(0x60, 0x60, 0x60));
	palette.set_pen_color(5, rgb_t(0x78, 0x78, 0x78));
	palette.set_pen_color(6, rgb_t(0x90, 0x90, 0x90));
	palette.set_pen_color(7, rgb_t(0xa8, 0xa8, 0xa8));
	palette.set_pen_color(8, rgb_t(0x10, 0x10, 0x10));   // sprites
	palette.set_pen_color(9, rgb_t(0xe0, 0xe0, 0xe0));
	palette.set_pen_color(10, rgb_t(0xa0, 0xa0, 0xa0));
	palette.set_pen_color(11, rgb_t(0x48, 0x48, 0x48));
	palette.set_pen_color(12, rgb_t(0x10, 0x10, 0x10));
	palette.set_pen_color(13, rgb_t(0x48, 0x48, 0x48));
	palette.set_pen_color(14, rgb_t(0xa0, 0xa0, 0xa0));
	palette.set_pen_color(15, rgb_t(0xe0, 0xe0, 0xe0));
	palette.set_pen_color(16, rgb_t(0x00, 0x00, 0x00));   // missiles
	palette.set_pen_color(17, rgb_t(0xff, 0xff, 0xff));
	palette.set_pen_color(18, rgb_t(0x00, 0x00, 0x00));   // text
	palette.set_pen_color(19, rgb_t(0xe0, 0xe0, 0xe0));
}

uint8_t skyraid_state::port_0_r()
{
	uint8_t val = m_language->read();

	if (m_stick[1]->read() > m_analog_range)
		val |= 0x40;
	if (m_stick[0]->read() > m_analog_range)
		val |= 0x80;

	return val;
}


void skyraid_state::range_w(uint8_t data)
{
	m_analog_range = data & 0x3f;
}


void skyraid_state::offset_w(uint8_t data)
{
	m_analog_offset = data & 0x3f;
}


void skyraid_state::scroll_w(uint8_t data)
{
	m_scroll = data;
}


void skyraid_state::sound_w(uint8_t data)
{
	// BIT0 => PLANE SWEEP
	// BIT1 => MISSILE
	// BIT2 => EXPLOSION
	// BIT3 => START LAMP
	// BIT4 => PLANE ON
	// BIT5 => ATTRACT

	m_discrete->write(SKYRAID_PLANE_SWEEP_EN, data & 0x01);
	m_discrete->write(SKYRAID_MISSILE_EN, data & 0x02);
	m_discrete->write(SKYRAID_EXPLOSION_EN, data & 0x04);
	m_led = !BIT(data, 3);
	m_discrete->write(SKYRAID_PLANE_ON_EN, data & 0x10);
	m_discrete->write(SKYRAID_ATTRACT_EN, data & 0x20);
}


void skyraid_state::program_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x300);
	map(0x0400, 0x040f).writeonly().share(m_pos_ram);
	map(0x0800, 0x087f).ram().mirror(0x480).share(m_alpha_num_ram);
	map(0x1000, 0x1000).r(FUNC(skyraid_state::port_0_r));
	map(0x1001, 0x1001).portr("DSW");
	map(0x1400, 0x1400).portr("COIN");
	map(0x1401, 0x1401).portr("SYSTEM");
	map(0x1c00, 0x1c0f).writeonly().share(m_obj_ram);
	map(0x4000, 0x4000).w(FUNC(skyraid_state::scroll_w));
	map(0x4400, 0x4400).w(FUNC(skyraid_state::sound_w));
	map(0x4800, 0x4800).w(FUNC(skyraid_state::range_w));
	map(0x5000, 0x5000).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x5800, 0x5800).w(FUNC(skyraid_state::offset_w));
	map(0x7000, 0x7fff).rom();
	map(0xf000, 0xffff).rom();
}

static INPUT_PORTS_START( skyraid )
	PORT_START("LANGUAGE")
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x10, DEF_STR( French ) )
	PORT_DIPSETTING(    0x20, DEF_STR( German ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Spanish ) )
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_UNUSED) // POT1
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_UNUSED) // POT0

	PORT_START("DSW")
	PORT_DIPNAME( 0x30, 0x10, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x10, "80 Seconds" )
	PORT_DIPSETTING(    0x20, "100 Seconds" )
	PORT_DIPSETTING(    0x30, "120 Seconds" )
	PORT_DIPNAME( 0x40, 0x40, "DIP #5" )    // must be OFF
	PORT_DIPSETTING(    0x40, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x80, 0x00, "Extended Play" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ))
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ))

	// coinage settings are insane, refer to the manual

	PORT_START("COIN")
	PORT_DIPNAME( 0x0F, 0x01, DEF_STR( Coinage )) // dial
	PORT_DIPSETTING(    0x00, "Mode 0" )
	PORT_DIPSETTING(    0x01, "Mode 1" )
	PORT_DIPSETTING(    0x02, "Mode 2" )
	PORT_DIPSETTING(    0x03, "Mode 3" )
	PORT_DIPSETTING(    0x04, "Mode 4" )
	PORT_DIPSETTING(    0x05, "Mode 5" )
	PORT_DIPSETTING(    0x06, "Mode 6" )
	PORT_DIPSETTING(    0x07, "Mode 7" )
	PORT_DIPSETTING(    0x08, "Mode 8" )
	PORT_DIPSETTING(    0x09, "Mode 9" )
	PORT_DIPSETTING(    0x0A, "Mode A" )
	PORT_DIPSETTING(    0x0B, "Mode B" )
	PORT_DIPSETTING(    0x0C, "Mode C" )
	PORT_DIPSETTING(    0x0D, "Mode D" )
	PORT_DIPSETTING(    0x0E, "Mode E" )
	PORT_DIPSETTING(    0x0F, "Mode F" )
	PORT_DIPNAME( 0x10, 0x10, "Score for Extended Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x10, DEF_STR( High ) )
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT (0x40, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT (0x80, IP_ACTIVE_HIGH, IPT_COIN2)

	PORT_START("SYSTEM")
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_TILT)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Hiscore Reset") PORT_CODE(KEYCODE_H)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_START1)
	PORT_SERVICE(0x80, IP_ACTIVE_LOW)

	PORT_START("STICKY")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("STICKX")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10)
INPUT_PORTS_END


static const gfx_layout skyraid_text_layout =
{
	16, 8,  // width, height
	64,     // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0, 0, 1, 1, 2, 2, 3, 3,
		4, 4, 5, 5, 6, 6, 7, 7
	},
	{
		0x38, 0x30, 0x28, 0x20, 0x18, 0x10, 0x08, 0x00
	},
	0x40
};


static const gfx_layout skyraid_sprite_layout =
{
	32, 32, // width, height
	8,      // total
	2,      // planes
			// plane offsets
	{ 0, 1 },
	{
		0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e,
		0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e,
		0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e,
		0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
		0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0
	},
	0x800
};


static const gfx_layout skyraid_missile_layout =
{
	16, 16, // width, height
	8,      // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0
	},
	0x100
};


static GFXDECODE_START( gfx_skyraid )
	GFXDECODE_ENTRY( "chars", 0, skyraid_text_layout, 18, 1 )
	GFXDECODE_ENTRY( "sprites", 0, skyraid_sprite_layout, 8, 2 )
	GFXDECODE_ENTRY( "missiles", 0, skyraid_missile_layout, 16, 1 )
GFXDECODE_END


void skyraid_state::skyraid(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12'096'000 / 12);
	m_maincpu->set_addrmap(AS_PROGRAM, &skyraid_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(skyraid_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog").set_vblank_count(m_screen, 4);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(22 * 1'000'000 / 15'750));
	m_screen->set_size(512, 240);
	m_screen->set_visarea(0, 511, 0, 239);
	m_screen->set_screen_update(FUNC(skyraid_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skyraid);

	PALETTE(config, m_palette, FUNC(skyraid_state::palette), 20);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, skyraid_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( skyraid )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "030595.e1", 0x7000, 0x800, CRC(c6cb3a2b) SHA1(e4cb8d259446d0614c0c8f097f97dcf21869782e) )
	ROM_RELOAD(            0xF000, 0x800 )
	ROM_LOAD( "030594.d1", 0x7800, 0x800, CRC(27979e96) SHA1(55ffe3094c6764e6b99ee148e3dd730ca263fa3a) )
	ROM_RELOAD(            0xF800, 0x800 )

	ROM_REGION( 0x0200, "chars", 0 )
	ROM_LOAD( "030598.h2", 0x0000, 0x200, CRC(2a7c5fa0) SHA1(93a79e5948dfcd9b6c2ff390e85a43f7a8cac327) )

	ROM_REGION( 0x0800, "sprites", 0 )
	ROM_LOAD( "030599.m7", 0x0000, 0x800, CRC(0cd179ea) SHA1(e3c763f76e6103e5909e7b5a979206b262d6e96a) )

	ROM_REGION( 0x0100, "missiles", 0 )
	ROM_LOAD_NIB_LOW ( "030597.n5", 0x0000, 0x100, CRC(319ff49c) SHA1(ff4d8b20436179910bf30c720d98df4678f683a9) )
	ROM_LOAD_NIB_HIGH( "030596.m4", 0x0000, 0x100, CRC(30454ed0) SHA1(4216a54c13d9c4803f88f2de35cdee31290bb15e) )

	ROM_REGION( 0x0800, "terrain", 0 )
	ROM_LOAD_NIB_LOW ( "030584.j5", 0x0000, 0x800, CRC(81f6e8a5) SHA1(ad77b469ed0c9d5dfaa221ecf47d0db4a7f7ac91) )
	ROM_LOAD_NIB_HIGH( "030585.k5", 0x0000, 0x800, CRC(b49bec3f) SHA1(b55d25230ec11c52e7b47d2c10194a49adbeb50a) )

	ROM_REGION( 0x0100, "trapezoid", 0 )
	ROM_LOAD_NIB_LOW ( "030582.a6", 0x0000, 0x100, CRC(0eacd595) SHA1(5469e312a1f522ce0a61054b50895a5b1a3f19ba) )
	ROM_LOAD_NIB_HIGH( "030583.b6", 0x0000, 0x100, CRC(3edd6fbc) SHA1(0418ea78cf51e18c51087b43a41cd9e13aac0a16) )

	ROM_REGION( 0x0100, "sync_prom", 0 )
	ROM_LOAD( "006559.c4", 0x0000, 0x100, CRC(5a8d0e42) SHA1(772220c4c24f18769696ddba26db2bc2e5b0909d) )
ROM_END

} // anonymous namespace


GAME( 1978, skyraid, 0, skyraid, skyraid, skyraid_state, empty_init, ORIENTATION_FLIP_Y, "Atari", "Sky Raider", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
