// license:BSD-3-Clause
// copyright-holders: Phil Stroffolino

/***************************************************************************

    Atari Fire Truck + Super Bug + Monte Carlo driver

***************************************************************************/

#include "emu.h"

#include "firetrk_a.h"

#include "cpu/m6800/m6800.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "superbug.lh"


namespace {

class firetrk_state : public driver_device
{
public:
	firetrk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_watchdog(*this, "watchdog")
		, m_discrete(*this, "discrete")
		, m_alpha_num_ram(*this, "alpha_num_ram")
		, m_playfield_ram(*this, "playfield_ram")
		, m_scroll_y(*this, "scroll_y")
		, m_scroll_x(*this, "scroll_x")
		, m_car_rot(*this, "car_rot")
		, m_blink(*this, "blink")
		, m_drone_x(*this, "drone_x")
		, m_drone_y(*this, "drone_y")
		, m_drone_rot(*this, "drone_rot")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_bit_0(*this, "BIT_0")
		, m_bit_6(*this, "BIT_6")
		, m_bit_7(*this, "BIT_7")
		, m_dips(*this, {"DIP_0", "DIP_1"})
		, m_steer(*this, "STEER_%u", 1U)
		, m_leds(*this, "led%u", 0U)
		, m_p1gear(*this, "P1gear")
	{ }

	void firetrk(machine_config &config);

	template <int P> int steer_dir_r();
	template <int P> int steer_flag_r();
	template <int P> int skid_r();
	template <int P> int crash_r();
	template <int P> int gear_r();
	DECLARE_INPUT_CHANGED_MEMBER(service_mode_switch_changed);
	DECLARE_INPUT_CHANGED_MEMBER(firetrk_horn_changed);
	DECLARE_INPUT_CHANGED_MEMBER(gear_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<discrete_device> m_discrete;
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_scroll_y;
	required_shared_ptr<uint8_t> m_scroll_x;
	required_shared_ptr<uint8_t> m_car_rot;
	optional_shared_ptr<uint8_t> m_blink;
	optional_shared_ptr<uint8_t> m_drone_x;
	optional_shared_ptr<uint8_t> m_drone_y;
	optional_shared_ptr<uint8_t> m_drone_rot;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	optional_ioport m_bit_0;
	optional_ioport m_bit_6;
	optional_ioport m_bit_7;
	required_ioport_array<2> m_dips;
	optional_ioport_array<2> m_steer;
	output_finder<4> m_leds;
	output_finder<> m_p1gear;

	uint8_t m_in_service_mode = 0;
	uint32_t m_dial[2]{};
	uint8_t m_steer_dir[2]{};
	uint8_t m_steer_flag[2]{};
	uint8_t m_gear = 0;

	uint8_t m_flash = 0;
	uint8_t m_crash[2]{};
	uint8_t m_skid[2]{};
	bitmap_ind16 m_helper[2]{};
	uint32_t m_color_mask[2]{};
	tilemap_t *m_tilemap[2]{};

	const rectangle m_playfield_window { rectangle(0x02a, 0x115, 0x000, 0x0ff) };

	uint8_t dip_r(offs_t offset);
	uint8_t input_r(offs_t offset);
	void blink_on_w(uint8_t data);
	void steer_reset_w(uint8_t data);
	void crash_reset_w(uint8_t data);
	void skid_reset_w(uint8_t data);
	void crash_snd_w(uint8_t data);
	void skid_snd_w(uint8_t data);
	void motor_snd_w(uint8_t data);
	void xtndply_w(uint8_t data);
	void check_collision(int which);
	void draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *alpha_ram, int x, int count, int height);

private:
	static constexpr XTAL MASTER_CLOCK = 12.096_MHz_XTAL;

	void output_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
	void draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flash);
	void set_service_mode(int enable);

	void palette(palette_device &palette);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void program_map(address_map &map) ATTR_COLD;
};

class montecar_state : public firetrk_state
{
public:
	using firetrk_state::firetrk_state;

	void montecar(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void output_1_w(uint8_t data);
	void output_2_w(uint8_t data);
	uint8_t dip_r(offs_t offset);
	uint8_t input_r(offs_t offset);
	void car_reset_w(uint8_t data);
	void drone_reset_w(uint8_t data);
	void skid_reset_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int is_collision_detection);

	void prom_to_palette(int number, uint8_t val);
	void palette(palette_device &palette);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void program_map(address_map &map) ATTR_COLD;
};

class superbug_state : public firetrk_state
{
public:
	using firetrk_state::firetrk_state;

	void superbug(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void output_w(offs_t offset, uint8_t data);
	void motor_snd_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int flash);

	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);

	void program_map(address_map &map) ATTR_COLD;
};


void firetrk_state::palette(palette_device &palette)
{
	static constexpr uint8_t colortable_source[] =
	{
		0, 0, 1, 0,
		2, 0, 3, 0,
		3, 3, 2, 3,
		1, 3, 0, 3,
		0, 0, 1, 0,
		2, 0, 0, 3,
		3, 0, 0, 3
	};
	static constexpr rgb_t palette_source[] =
	{
		rgb_t::black(),
		rgb_t(0x5b, 0x5b, 0x5b),
		rgb_t(0xa4, 0xa4, 0xa4),
		rgb_t::white()
	};

	m_color_mask[0] = m_color_mask[1] = 0;

	for (int i = 0; i < std::size(colortable_source); i++)
	{
		uint8_t const color = colortable_source[i];

		if (color == 1)
			m_color_mask[0] |= 1 << i;
		else if (color == 2)
			m_color_mask[1] |= 1 << i;

		palette.set_pen_color(i, palette_source[color]);
	}
}


void montecar_state::prom_to_palette(int number, uint8_t val)
{
	m_palette->set_pen_color(number, rgb_t(pal1bit(val >> 2), pal1bit(val >> 1), pal1bit(val >> 0)));
}


void montecar_state::palette(palette_device &palette)
{
	const uint8_t *color_prom = memregion("proms")->base();

	static constexpr uint8_t colortable_source[] =
	{
		0x00, 0x00, 0x00, 0x01,
		0x00, 0x02, 0x00, 0x03,
		0x03, 0x03, 0x03, 0x02,
		0x03, 0x01, 0x03, 0x00,
		0x00, 0x00, 0x02, 0x00,
		0x02, 0x01, 0x02, 0x02,
		0x00, 0x10, 0x20, 0x30,
		0x00, 0x04, 0x08, 0x0c,
		0x00, 0x44, 0x48, 0x4c,
		0x00, 0x84, 0x88, 0x8c,
		0x00, 0xc4, 0xc8, 0xcc
	};

	/*
	 * The color PROM is addressed as follows:
	 *
	 *   A0 => PLAYFIELD 1
	 *   A1 => PLAYFIELD 2
	 *   A2 => DRONE 1
	 *   A3 => DRONE 2
	 *   A4 => CAR 1
	 *   A5 => CAR 2
	 *   A6 => DRONE COLOR 1
	 *   A7 => DRONE COLOR 2
	 *   A8 => PLAYFIELD WINDOW
	 *
	 * This driver hard-codes some behavior which actually depends
	 * on the PROM, like priorities, clipping and transparency.
	 *
	 */

	m_color_mask[0] = m_color_mask[1] = 0;

	for (int i = 0; i < std::size(colortable_source); i++)
	{
		uint8_t const color = colortable_source[i];

		if (color == 1)
			m_color_mask[0] |= 1 << i;
		else if (color == 2)
			m_color_mask[1] |= 1 << i;

		prom_to_palette(i, color_prom[0x100 + colortable_source[i]]);
	}

	palette.set_pen_color(std::size(colortable_source) + 0, rgb_t::black());
	palette.set_pen_color(std::size(colortable_source) + 1, rgb_t::white());
}


TILE_GET_INFO_MEMBER(firetrk_state::get_tile_info1)
{
	int const code = m_playfield_ram[tile_index] & 0x3f;
	int color = (m_playfield_ram[tile_index] >> 6) & 0x03;

	if (*m_blink && (code >= 0x04) && (code <= 0x0b))
		color = 0;

	if (m_flash)
		color = color | 0x04;

	tileinfo.set(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(superbug_state::get_tile_info1)
{
	int const code = m_playfield_ram[tile_index] & 0x3f;
	int color = (m_playfield_ram[tile_index] >> 6) & 0x03;

	if (*m_blink && (code >= 0x08) && (code <= 0x0f))
		color = 0;

	if (m_flash)
		color = color | 0x04;

	tileinfo.set(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(montecar_state::get_tile_info1)
{
	int const code = m_playfield_ram[tile_index] & 0x3f;
	int color = (m_playfield_ram[tile_index] >> 6) & 0x03;

	if (m_flash)
		color = color | 0x04;

	tileinfo.set(1, code, color, 0);
}


TILE_GET_INFO_MEMBER(firetrk_state::get_tile_info2)
{
	uint8_t const code = m_playfield_ram[tile_index] & 0x3f;
	int color = 0;

	// palette 1 for crash and palette 2 for skid
	if (((code & 0x30) != 0x00) || ((code & 0x0c) == 0x00))
		color = 1;   // palette 0, 1

	if ((code & 0x3c) == 0x0c)
		color = 2;   // palette 0, 2

	tileinfo.set(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(superbug_state::get_tile_info2)
{
	uint8_t const code = m_playfield_ram[tile_index] & 0x3f;
	int color = 0;

	// palette 1 for crash and palette 2 for skid
	if ((code & 0x30) != 0x00)
		color = 1;   // palette 0, 1

	if ((code & 0x38) == 0x00)
		color = 2;   // palette 0, 2

	tileinfo.set(2, code, color, 0);
}


TILE_GET_INFO_MEMBER(montecar_state::get_tile_info2)
{
	uint8_t const code = m_playfield_ram[tile_index];
	int color = 0;

	// palette 1 for crash and palette 2 for skid
	if (((code & 0xc0) == 0x40) || ((code & 0xc0) == 0x80))
		color = 2;   // palette 2, 1

	if ((code & 0xc0) == 0xc0)
		color = 1;   // palette 2, 0

	if ((code & 0xc0) == 0x00)
		color = 3;   // palette 2, 2

	if ((code & 0x30) == 0x30)
		color = 0;   // palette 0, 0

	tileinfo.set(2, code & 0x3f, color, 0);
}


void firetrk_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper[0]);
	m_screen->register_screen_bitmap(m_helper[1]);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(firetrk_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(firetrk_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


void superbug_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper[0]);
	m_screen->register_screen_bitmap(m_helper[1]);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(superbug_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(superbug_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


void montecar_state::video_start()
{
	m_screen->register_screen_bitmap(m_helper[0]);
	m_screen->register_screen_bitmap(m_helper[1]);

	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(montecar_state::get_tile_info1)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(montecar_state::get_tile_info2)), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
}


void firetrk_state::draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flash)
{
	int gfx_bank, code, color, flip_x, flip_y, x, y;

	if (which)
	{
		gfx_bank = 5;
		code = *m_drone_rot & 0x07;
		color = flash ? 1 : 0;
		flip_x = *m_drone_rot & 0x08;
		flip_y = *m_drone_rot & 0x10;
		x = (flip_x ? *m_drone_x - 63 : 192 - *m_drone_x) + 36;
		y =  flip_y ? *m_drone_y - 63 : 192 - *m_drone_y;
	}
	else
	{
		gfx_bank = (*m_car_rot & 0x10) ? 4 : 3;
		code = *m_car_rot & 0x03;
		color = flash ? 1 : 0;
		flip_x = *m_car_rot & 0x04;
		flip_y = *m_car_rot & 0x08;
		x = 144;
		y = 104;
	}

		m_gfxdecode->gfx(gfx_bank)->transpen(bitmap, cliprect, code, color, flip_x, flip_y, x, y, 0);
}


void superbug_state::draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int flash)
{
	int const gfx_bank = (*m_car_rot & 0x10) ? 4 : 3;
	int const code = ~*m_car_rot & 0x03;
	int const color = flash ? 1 : 0;
	int const flip_x = *m_car_rot & 0x04;
	int const flip_y = *m_car_rot & 0x08;

	m_gfxdecode->gfx(gfx_bank)->transpen(bitmap, cliprect, code, color, flip_x, flip_y, 144, 104, 0);
}


void montecar_state::draw_car(bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int is_collision_detection)
{
	int gfx_bank, code, color, flip_x, flip_y, x, y;

	if (which)
	{
		gfx_bank = 4;
		code = *m_drone_rot & 0x07;
		color = is_collision_detection ? 0 : (((*m_car_rot & 0x80) >> 6) | ((*m_drone_rot & 0x80) >> 7));
		flip_x = *m_drone_rot & 0x10;
		flip_y = *m_drone_rot & 0x08;
		x = (flip_x ? *m_drone_x - 31 : 224 - *m_drone_x) + 34;
		y =  flip_y ? *m_drone_y - 31 : 224 - *m_drone_y;
	}
	else
	{
		gfx_bank = 3;
		code = *m_car_rot & 0x07;
		color = 0;
		flip_x = *m_car_rot & 0x10;
		flip_y = *m_car_rot & 0x08;
		x = 144;
		y = 104;
	}

		m_gfxdecode->gfx(gfx_bank)->transpen(bitmap, cliprect, code, color, flip_x, flip_y, x, y, 0);
}


void firetrk_state::draw_text(bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t *alpha_ram, int x, int count, int height)
{
	for (int i = 0; i < count; i++)
			m_gfxdecode->gfx(0)->opaque(bitmap,cliprect, alpha_ram[i], 0, 0, 0, x, i * height);
}


void firetrk_state::check_collision(int which)
{
	for (int y = m_playfield_window.top(); y <= m_playfield_window.bottom(); y++)
		for (int x = m_playfield_window.left(); x <= m_playfield_window.right(); x++)
		{
			pen_t const a = m_helper[0].pix(y, x);
			pen_t const b = m_helper[1].pix(y, x);

			if (b != 0xff && (m_color_mask[0] >> a) & 1)
				m_crash[which] = 1;

			if (b != 0xff && (m_color_mask[1] >> a) & 1)
				m_skid[which] = 1;
		}
}


uint32_t firetrk_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	m_tilemap[0]->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap[1]->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap[0]->set_scrolly(0, *m_scroll_y);
	m_tilemap[1]->set_scrolly(0, *m_scroll_y);

	bitmap.fill(0, cliprect);
	m_tilemap[0]->draw(screen, bitmap, m_playfield_window, 0, 0);
	draw_car(bitmap, m_playfield_window, 0, m_flash);
	draw_car(bitmap, m_playfield_window, 1, m_flash);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x00, 296, 0x10, 0x10);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x10,   8, 0x10, 0x10);

	if (cliprect.bottom() == screen.visible_area().bottom())
	{
		m_tilemap[1]->draw(screen, m_helper[0], m_playfield_window, 0, 0);

		m_helper[1].fill(0xff, m_playfield_window);
		draw_car(m_helper[1], m_playfield_window, 0, false);
		check_collision(0);

		m_helper[1].fill(0xff, m_playfield_window);
		draw_car(m_helper[1], m_playfield_window, 1, false);
		check_collision(1);

		*m_blink = false;
	}

	return 0;
}


uint32_t superbug_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	m_tilemap[0]->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap[1]->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap[0]->set_scrolly(0, *m_scroll_y);
	m_tilemap[1]->set_scrolly(0, *m_scroll_y);

	bitmap.fill(0, cliprect);
	m_tilemap[0]->draw(screen, bitmap, m_playfield_window, 0, 0);
	draw_car(bitmap, m_playfield_window, m_flash);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x00, 296, 0x10, 0x10);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x10,   8, 0x10, 0x10);

	if (cliprect.bottom() == screen.visible_area().bottom())
	{
		m_tilemap[1]->draw(screen, m_helper[0], m_playfield_window, 0, 0);

		m_helper[1].fill(0xff, m_playfield_window);
		draw_car(m_helper[1], m_playfield_window, false);
		check_collision(0);

		*m_blink = false;
	}

	return 0;
}


uint32_t montecar_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	machine().tilemap().mark_all_dirty();
	m_tilemap[0]->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap[1]->set_scrollx(0, *m_scroll_x - 37);
	m_tilemap[0]->set_scrolly(0, *m_scroll_y);
	m_tilemap[1]->set_scrolly(0, *m_scroll_y);

	bitmap.fill(0x2c, cliprect);
	m_tilemap[0]->draw(screen, bitmap, m_playfield_window, 0, 0);
	draw_car(bitmap, m_playfield_window, 0, false);
	draw_car(bitmap, m_playfield_window, 1, false);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x00, 24, 0x20, 0x08);
	draw_text(bitmap, cliprect, m_alpha_num_ram + 0x20, 16, 0x20, 0x08);

	if (cliprect.bottom() == screen.visible_area().bottom())
	{
		m_tilemap[1]->draw(screen, m_helper[0], m_playfield_window, 0, 0);

		m_helper[1].fill(0xff, m_playfield_window);
		draw_car(m_helper[1], m_playfield_window, 0, true);
		check_collision(0);

		m_helper[1].fill(0xff, m_playfield_window);
		draw_car(m_helper[1], m_playfield_window, 1, true);
		check_collision(1);
	}

	return 0;
}


void firetrk_state::skid_reset_w(uint8_t data)
{
	m_skid[0] = 0;
	m_skid[1] = 0;

	// also SUPERBUG_SKID_EN
	m_discrete->write(FIRETRUCK_SKID_EN, 1);
}


void montecar_state::skid_reset_w(uint8_t data)
{
	m_discrete->write(MONTECAR_SKID_EN, 1);
}


void firetrk_state::crash_snd_w(uint8_t data)
{
	// also SUPERBUG_CRASH_DATA and MONTECAR_CRASH_DATA
	m_discrete->write(FIRETRUCK_CRASH_DATA, data >> 4);
}


void firetrk_state::skid_snd_w(uint8_t data)
{
	// also SUPERBUG_SKID_EN and MONTECAR_SKID_EN
	m_discrete->write(FIRETRUCK_SKID_EN, 0);
}


void firetrk_state::motor_snd_w(uint8_t data)
{
	// also MONTECAR_DRONE_MOTOR_DATA
	m_discrete->write(FIRETRUCK_SIREN_DATA, data >> 4);

	// also MONTECAR_MOTOR_DATA
	m_discrete->write(FIRETRUCK_MOTOR_DATA, data & 0x0f);
}


void superbug_state::motor_snd_w(uint8_t data)
{
	m_discrete->write(SUPERBUG_SPEED_DATA, data & 0x0f);
}


void firetrk_state::xtndply_w(uint8_t data)
{
	// also SUPERBUG_ASR_EN (extended play)
	m_discrete->write(FIRETRUCK_XTNDPLY_EN, data);
}


void firetrk_state::set_service_mode(int enable)
{
	m_in_service_mode = enable;

	// watchdog is disabled during service mode
	m_watchdog->watchdog_enable(!enable);

	// change CPU clock speed according to service switch change
	m_maincpu->set_unscaled_clock(enable ? (MASTER_CLOCK / 16) : (MASTER_CLOCK / 12));
}


INPUT_CHANGED_MEMBER(firetrk_state::service_mode_switch_changed)
{
	set_service_mode(newval);
}


INPUT_CHANGED_MEMBER(firetrk_state::firetrk_horn_changed)
{
	m_discrete->write(FIRETRUCK_HORN_EN, newval);
}


INPUT_CHANGED_MEMBER(firetrk_state::gear_changed)
{
	if (newval)
	{
		m_gear = param;
		m_p1gear = m_gear + 1;
	}
}


TIMER_DEVICE_CALLBACK_MEMBER(firetrk_state::scanline)
{
	int const scanline = param;

	// periodic IRQs are generated by inverse 16V signal
	if ((scanline & 0x1f) == 0 && scanline != 0)
		m_maincpu->pulse_input_line(0, attotime::from_hz(MASTER_CLOCK / 2 / 64));

	// vblank interrupt
	// NMIs are disabled during service mode
	if (!m_in_service_mode && scanline == 240)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void firetrk_state::output_w(uint8_t data)
{
	// BIT0 => START1 LAMP
	m_leds[0] = BIT(~data, 0);

	// BIT1 => START2 LAMP
	m_leds[1] = BIT(~data, 1);

	// BIT2 => FLASH
	m_flash = data & 0x04;

	// BIT3 => TRACK LAMP
	m_leds[3] = BIT(~data, 3);

	// BIT4 => ATTRACT
	m_discrete->write(FIRETRUCK_ATTRACT_EN, data & 0x10);
	machine().bookkeeping().coin_lockout_w(0, !(data & 0x10));
	machine().bookkeeping().coin_lockout_w(1, !(data & 0x10));

	// BIT5 => START3 LAMP
	m_leds[2] = BIT(~data, 5);

	// BIT6 => UNUSED

	// BIT7 => BELL OUT
	m_discrete->write(FIRETRUCK_BELL_EN, data & 0x80);
}


void superbug_state::output_w(offs_t offset, uint8_t data)
{
	// BIT0 => START LAMP
	m_leds[0] = BIT(offset, 0);

	// BIT1 => ATTRACT
	m_discrete->write(SUPERBUG_ATTRACT_EN, offset & 0x02);
	machine().bookkeeping().coin_lockout_w(0, !(offset & 0x02));
	machine().bookkeeping().coin_lockout_w(1, !(offset & 0x02));

	// BIT2 => FLASH
	m_flash = offset & 0x04;

	// BIT3 => TRACK LAMP
	m_leds[1] = BIT(offset, 3);
}


void montecar_state::output_1_w(uint8_t data)
{
	// BIT0 => START LAMP
	m_leds[0] = BIT(~data, 0);

	// BIT1 => TRACK LAMP
	m_leds[1] = BIT(~data, 1);

	// BIT2 => ATTRACT
	m_discrete->write(MONTECAR_ATTRACT_INV, data & 0x04);

	// BIT3 => UNUSED
	// BIT4 => UNUSED

	// BIT5 => COIN3 COUNTER
	machine().bookkeeping().coin_counter_w(0, data & 0x80);

	// BIT6 => COIN2 COUNTER
	machine().bookkeeping().coin_counter_w(1, data & 0x40);

	// BIT7 => COIN1 COUNTER
	machine().bookkeeping().coin_counter_w(2, data & 0x20);
}


void montecar_state::output_2_w(uint8_t data)
{
	m_flash = data & 0x80;

	m_discrete->write(MONTECAR_BEEPER_EN, data & 0x10);
	m_discrete->write(MONTECAR_DRONE_LOUD_DATA, data & 0x0f);
}


void firetrk_state::machine_start()
{
	m_leds.resolve();
	m_p1gear.resolve();

	save_item(NAME(m_in_service_mode));
	save_item(NAME(m_dial));
	save_item(NAME(m_steer_dir));
	save_item(NAME(m_steer_flag));
	save_item(NAME(m_gear));
	save_item(NAME(m_crash));
	save_item(NAME(m_skid));
}


void firetrk_state::machine_reset()
{
	set_service_mode(0);
}


uint8_t firetrk_state::dip_r(offs_t offset)
{
	uint8_t val0 = m_dips[0]->read();
	uint8_t const val1 = m_dips[1]->read();

	if (val1 & (1 << (2 * offset + 0))) val0 |= 1;
	if (val1 & (1 << (2 * offset + 1))) val0 |= 2;

	return val0;
}


uint8_t montecar_state::dip_r(offs_t offset)
{
	uint8_t val0 = m_dips[0]->read();
	uint8_t const val1 = m_dips[1]->read();

	if (val1 & (1 << (3 - offset))) val0 |= 1;
	if (val1 & (1 << (7 - offset))) val0 |= 2;

	return val0;
}


template <int P>
int firetrk_state::steer_dir_r()
{
	return m_steer_dir[P];
}


template <int P>
int firetrk_state::steer_flag_r()
{
	return m_steer_flag[P];
}


template <int P>
int firetrk_state::skid_r()
{
	uint32_t ret;

	if (P != 2)
		ret = m_skid[P];
	else
		ret = m_skid[0] | m_skid[1];

	return ret;
}


template <int P>
int firetrk_state::crash_r()
{
	uint32_t ret;

	if (P != 2)
		ret = m_crash[P];
	else
		ret = m_crash[0] | m_crash[1];

	return ret;
}


template <int P>
int firetrk_state::gear_r()
{
	return (m_gear == P) ? 1 : 0;
}


uint8_t firetrk_state::input_r(offs_t offset)
{
	// update steering wheels
	for (int i = 0; i < 2; i++)
	{
		uint32_t const new_dial = m_steer[i].read_safe(0);
		int32_t const delta = new_dial - m_dial[i];

		if (delta != 0)
		{
			m_steer_flag[i] = 0;
			m_steer_dir[i] = (delta < 0) ? 1 : 0;

			m_dial[i] = m_dial[i] + delta;
		}
	}

	return ((m_bit_0.read_safe(0) & (1 << offset)) ? 0x01 : 0) |
			((m_bit_6.read_safe(0) & (1 << offset)) ? 0x40 : 0) |
			((m_bit_7.read_safe(0) & (1 << offset)) ? 0x80 : 0);
}


uint8_t montecar_state::input_r(offs_t offset)
{
	uint8_t ret = firetrk_state::input_r(offset);

	if (m_crash[0])
		ret |= 0x02;

	// can this be right, bit 0 again ????
	if (m_crash[1])
		ret |= 0x01;

	return ret;
}


void firetrk_state::blink_on_w(uint8_t data)
{
	*m_blink = true;
}


void montecar_state::car_reset_w(uint8_t data)
{
	m_crash[0] = 0;
	m_skid[0] = 0;
}


void montecar_state::drone_reset_w(uint8_t data)
{
	m_crash[1] = 0;
	m_skid[1] = 0;
}


void firetrk_state::steer_reset_w(uint8_t data)
{
	m_steer_flag[0] = 1;
	m_steer_flag[1] = 1;
}


void firetrk_state::crash_reset_w(uint8_t data)
{
	m_crash[0] = 0;
	m_crash[1] = 0;
}


void firetrk_state::program_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x00ff).mirror(0x0700).ram().share(m_alpha_num_ram);
	map(0x0800, 0x08ff).mirror(0x0700).ram().share(m_playfield_ram);
	map(0x1000, 0x1000).mirror(0x001f).writeonly().share(m_scroll_y);
	map(0x1020, 0x1020).mirror(0x001f).writeonly().share(m_scroll_x);
	map(0x1040, 0x1040).mirror(0x001f).w(FUNC(firetrk_state::crash_reset_w));
	map(0x1060, 0x1060).mirror(0x001f).w(FUNC(firetrk_state::skid_reset_w));
	map(0x1080, 0x1080).mirror(0x001f).writeonly().share(m_car_rot);
	map(0x10a0, 0x10a0).mirror(0x001f).w(FUNC(firetrk_state::steer_reset_w));
	map(0x10c0, 0x10c0).mirror(0x001f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x10e0, 0x10e0).mirror(0x001f).w(FUNC(firetrk_state::blink_on_w)).share(m_blink);
	map(0x1400, 0x1400).mirror(0x001f).w(FUNC(firetrk_state::motor_snd_w));
	map(0x1420, 0x1420).mirror(0x001f).w(FUNC(firetrk_state::crash_snd_w));
	map(0x1440, 0x1440).mirror(0x001f).w(FUNC(firetrk_state::skid_snd_w));
	map(0x1460, 0x1460).mirror(0x001f).writeonly().share(m_drone_x);
	map(0x1480, 0x1480).mirror(0x001f).writeonly().share(m_drone_y);
	map(0x14a0, 0x14a0).mirror(0x001f).writeonly().share(m_drone_rot);
	map(0x14c0, 0x14c0).mirror(0x001f).w(FUNC(firetrk_state::output_w));
	map(0x14e0, 0x14e0).mirror(0x001f).w(FUNC(firetrk_state::xtndply_w));
	map(0x1800, 0x1807).mirror(0x03f8).r(FUNC(firetrk_state::input_r)).nopw();
	map(0x1c00, 0x1c03).mirror(0x03fc).r(FUNC(firetrk_state::dip_r));
	map(0x2000, 0x3fff).rom();
}


void superbug_state::program_map(address_map &map)
{
	map.global_mask(0x1fff);
	map(0x0000, 0x00ff).ram();
	map(0x0100, 0x0100).mirror(0x001f).writeonly().share(m_scroll_y);
	map(0x0120, 0x0120).mirror(0x001f).writeonly().share(m_scroll_x);
	map(0x0140, 0x0140).mirror(0x001f).w(FUNC(superbug_state::crash_reset_w));
	map(0x0160, 0x0160).mirror(0x001f).w(FUNC(superbug_state::skid_reset_w));
	map(0x0180, 0x0180).mirror(0x001f).writeonly().share(m_car_rot);
	map(0x01a0, 0x01a0).mirror(0x001f).w(FUNC(superbug_state::steer_reset_w));
	map(0x01c0, 0x01c0).mirror(0x001f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x01e0, 0x01e0).mirror(0x001f).w(FUNC(superbug_state::blink_on_w)).share(m_blink);
	map(0x0200, 0x0207).mirror(0x0018).r(FUNC(superbug_state::input_r));
	map(0x0220, 0x0220).mirror(0x001f).w(FUNC(superbug_state::xtndply_w));
	map(0x0240, 0x0243).mirror(0x001c).r(FUNC(superbug_state::dip_r));
	map(0x0260, 0x026f).mirror(0x0010).w(FUNC(superbug_state::output_w));
	map(0x0280, 0x0280).mirror(0x001f).w(FUNC(superbug_state::motor_snd_w));
	map(0x02a0, 0x02a0).mirror(0x001f).w(FUNC(superbug_state::crash_snd_w));
	map(0x02c0, 0x02c0).mirror(0x001f).w(FUNC(superbug_state::skid_snd_w));
	map(0x0400, 0x041f).ram().share(m_alpha_num_ram);
	map(0x0500, 0x05ff).ram().share(m_playfield_ram);
	map(0x0800, 0x1fff).rom();
}


void montecar_state::program_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x00ff).mirror(0x0700).ram().share(m_alpha_num_ram);
	map(0x0800, 0x08ff).mirror(0x0700).ram().share(m_playfield_ram);
	map(0x1000, 0x1000).mirror(0x001f).writeonly().share(m_scroll_y);
	map(0x1020, 0x1020).mirror(0x001f).writeonly().share(m_scroll_x);
	map(0x1040, 0x1040).mirror(0x001f).w(FUNC(montecar_state::drone_reset_w));
	map(0x1060, 0x1060).mirror(0x001f).w(FUNC(montecar_state::car_reset_w));
	map(0x1080, 0x1080).mirror(0x001f).writeonly().share(m_car_rot);
	map(0x10a0, 0x10a0).mirror(0x001f).w(FUNC(montecar_state::steer_reset_w));
	map(0x10c0, 0x10c0).mirror(0x001f).w(m_watchdog, FUNC(watchdog_timer_device::reset_w));
	map(0x10e0, 0x10e0).mirror(0x001f).w(FUNC(montecar_state::skid_reset_w));
	map(0x1400, 0x1400).mirror(0x001f).w(FUNC(montecar_state::motor_snd_w));
	map(0x1420, 0x1420).mirror(0x001f).w(FUNC(montecar_state::crash_snd_w));
	map(0x1440, 0x1440).mirror(0x001f).w(FUNC(montecar_state::skid_snd_w));
	map(0x1460, 0x1460).mirror(0x001f).writeonly().share(m_drone_x);
	map(0x1480, 0x1480).mirror(0x001f).writeonly().share(m_drone_y);
	map(0x14a0, 0x14a0).mirror(0x001f).writeonly().share(m_drone_rot);
	map(0x14c0, 0x14c0).mirror(0x001f).w(FUNC(montecar_state::output_1_w));
	map(0x14e0, 0x14e0).mirror(0x001f).w(FUNC(montecar_state::output_2_w));
	map(0x1800, 0x1807).mirror(0x03f8).r(FUNC(montecar_state::input_r)).nopw();
	map(0x1c00, 0x1c03).mirror(0x03fc).r(FUNC(montecar_state::dip_r));
	map(0x2000, 0x3fff).rom();
}


static INPUT_PORTS_START( firetrk )
	PORT_START("STEER_1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("STEER_2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("DIP_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // other DIPs connect here
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // other DIPs connect here
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("DIP_1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x01, DEF_STR( French ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x03, DEF_STR( German ) )
	PORT_DIPNAME( 0x0c, 0x04, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x04, "90 Seconds" )
	PORT_DIPSETTING(    0x08, "120 Seconds" )
	PORT_DIPSETTING(    0x0c, "150 Seconds" )
	PORT_DIPNAME( 0x30, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x10, "Liberal" )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, "Conservative" )
	PORT_DIPSETTING(    0x00, "Never" )

	PORT_START("BIT_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas") PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_dir_r<0>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_dir_r<1>)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Bell") PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, skid_r<2>)
	PORT_SERVICE( 0x80, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, service_mode_switch_changed, 0)

	PORT_START("BIT_6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 ) PORT_NAME("Front Player Start")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START2 ) PORT_NAME("Back Player Start")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START3 ) PORT_NAME("Both Players Start")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Track Select") PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ))
	PORT_DIPSETTING(    0x00, "Smokey Joe (1 Player)" )
	PORT_DIPSETTING(    0x40, "Fire Truck (2 Players)" )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Diag Hold")

	PORT_START("BIT_7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_flag_r<0>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_flag_r<1>)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, crash_r<2>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Diag Step")

	PORT_START("HORN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Horn") PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, firetrk_horn_changed, 0)

	PORT_START("R27")
	PORT_ADJUSTER( 20, "R27 - Motor Frequency" )
INPUT_PORTS_END


static INPUT_PORTS_START( superbug )
	PORT_START("STEER_1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10)

	PORT_START("DIP_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // other DIPs connect here
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // other DIPs connect here

	PORT_START("DIP_1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0c, 0x04, "Play Time" )
	PORT_DIPSETTING(    0x00, "60 seconds" )
	PORT_DIPSETTING(    0x04, "90 seconds" )
	PORT_DIPSETTING(    0x08, "120 seconds" )
	PORT_DIPSETTING(    0x0c, "150 seconds" )
	PORT_DIPNAME( 0x30, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x10, "Liberal" )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, "Conservative" )
	PORT_DIPSETTING(    0x00, "Never" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Language ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( German ) )

	PORT_START("BIT_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, gear_r<1>)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_dir_r<0>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_MEMORY_RESET ) PORT_NAME("Hiscore Reset")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_SERVICE( 0x20, IP_ACTIVE_HIGH )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, skid_r<0>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )

	PORT_START("BIT_7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, gear_r<2>)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, gear_r<0>)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_flag_r<0>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, crash_r<0>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Track Select")

	PORT_START("GEAR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear 1") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear 2") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear 3") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear 4") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 3)

	PORT_START("R62")
	PORT_ADJUSTER( 20, "R62 - Motor Frequency" )
INPUT_PORTS_END


static INPUT_PORTS_START( montecar )
	PORT_START("STEER_1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("DIP_0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNUSED ) // other DIPs connect here
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED ) // other DIPs connect here
	PORT_DIPNAME( 0x0c, 0x0c, "Coin 3 Multiplier" )
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	PORT_DIPNAME( 0x10, 0x10, "Coin 2 Multiplier" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unused ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_DIPSETTING(    0x00, DEF_STR( On ))

	PORT_START("DIP_1")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ))
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ))
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ))
	PORT_DIPNAME( 0x0c, 0x08, "Extended Play" )
	PORT_DIPSETTING(    0x04, "Liberal" )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x00, "Conservative" )
	PORT_DIPSETTING(    0x0c, "Never" )
	PORT_DIPNAME( 0x30, 0x20, "Play Time" )
	PORT_DIPSETTING(    0x30, "60 Seconds" )
	PORT_DIPSETTING(    0x10, "90 Seconds" )
	PORT_DIPSETTING(    0x20, "120 Seconds" )
	PORT_DIPSETTING(    0x00, "150 Seconds" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Language ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( English ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Spanish ) )
	PORT_DIPSETTING(    0x40, DEF_STR( French ) )
	PORT_DIPSETTING(    0x00, DEF_STR( German ) )

	PORT_START("BIT_6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, gear_r<0>)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, gear_r<1>)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, gear_r<2>)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Track Select")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Gas")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_dir_r<0>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, skid_r<1>)

	PORT_START("BIT_7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH ) PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, service_mode_switch_changed, 0)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_CUSTOM )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, steer_flag_r<0>)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(firetrk_state, skid_r<0>)

	PORT_START("GEAR")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gear 1") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gear 2") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Gear 3") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 2)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Gear 4") PORT_CHANGED_MEMBER(DEVICE_SELF, firetrk_state, gear_changed, 3)

	PORT_START("R89")
	PORT_ADJUSTER( 20, "R89 - Motor Frequency" )

	PORT_START("R88")
	PORT_ADJUSTER( 25, "R88 - Drone Motor Frequency" )
INPUT_PORTS_END


static const gfx_layout firetrk_text_layout =
{
	16, 16, // width, height
	32,     // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x1c, 0x1d, 0x1e, 0x1f, 0x04, 0x05, 0x06, 0x07,
		0x0c, 0x0d, 0x0e, 0x0f, 0x14, 0x15, 0x16, 0x17
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200
};


static const gfx_layout superbug_text_layout =
{
	16, 16, // width, height
	32,     // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x0c, 0x0d, 0x0e, 0x0f, 0x14, 0x15, 0x16, 0x17,
		0x1c, 0x1d, 0x1e, 0x1f, 0x04, 0x05, 0x06, 0x07
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200
};


static const gfx_layout montecar_text_layout =
{
	8, 8,   // width, height
	64,     // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0xc, 0xd, 0xe, 0xf, 0x4, 0x5, 0x6, 0x7
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static const gfx_layout firetrk_tile_layout =
{
	16, 16, // width, height
	64,     // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
		0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0
	},
	0x100
};


static const gfx_layout superbug_tile_layout =
{
	16, 16, // width, height
	64,     // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x07, 0x06, 0x05, 0x04, 0x0f, 0x0e, 0x0d, 0x0c,
		0x17, 0x16, 0x15, 0x14, 0x1f, 0x1e, 0x1d, 0x1c
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200
};


static const gfx_layout firetrk_car_layout1 =
{
	32, 32, // width, height
	4,      // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
		0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0
	},
	{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3b
	},
	0x800
};


static const gfx_layout superbug_car_layout1 =
{
	32, 32, // width, height
	4,      // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
		0x0800, 0x0900, 0x0a00, 0x0b00, 0x0c00, 0x0d00, 0x0e00, 0x0f00,
		0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
		0x1800, 0x1900, 0x1a00, 0x1b00, 0x1c00, 0x1d00, 0x1e00, 0x1f00
	},
	{
		0x04, 0x0c, 0x14, 0x1c, 0x24, 0x2c, 0x34, 0x3c,
		0x44, 0x4c, 0x54, 0x5c, 0x64, 0x6c, 0x74, 0x7c,
		0x84, 0x8c, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbc,
		0xc4, 0xcc, 0xd4, 0xdc, 0xe4, 0xec, 0xf4, 0xfc
	},
	0x001
};


static const gfx_layout montecar_car_layout =
{
	32, 32, // width, height
	8,      // total
	2,      // planes
			// plane offsets
	{ 1, 0 },
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


static const gfx_layout firetrk_car_layout2 =
{
	32, 32, // width, height
	4,      // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3b
	},
	{
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
		0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0
	},
	0x800
};


static const gfx_layout superbug_car_layout2 =
{
	32, 32, // width, height
	4,      // total
	1,      // planes
	{ 0 },  // plane offsets
	{
		0x04, 0x0c, 0x14, 0x1c, 0x24, 0x2c, 0x34, 0x3c,
		0x44, 0x4c, 0x54, 0x5c, 0x64, 0x6c, 0x74, 0x7c,
		0x84, 0x8c, 0x94, 0x9c, 0xa4, 0xac, 0xb4, 0xbc,
		0xc4, 0xcc, 0xd4, 0xdc, 0xe4, 0xec, 0xf4, 0xfc
	},
	{
		0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
		0x0800, 0x0900, 0x0a00, 0x0b00, 0x0c00, 0x0d00, 0x0e00, 0x0f00,
		0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
		0x1800, 0x1900, 0x1a00, 0x1b00, 0x1c00, 0x1d00, 0x1e00, 0x1f00
	},
	0x001
};

static const uint32_t firetrk_trailer_layout_xoffset[64] =
{
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

static const uint32_t firetrk_trailer_layout_yoffset[64] =
{
	0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
	0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0,
	0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0,
	0x600, 0x640, 0x680, 0x6c0, 0x700, 0x740, 0x780, 0x7c0,
	0x800, 0x840, 0x880, 0x8c0, 0x900, 0x940, 0x980, 0x9c0,
	0xa00, 0xa40, 0xa80, 0xac0, 0xb00, 0xb40, 0xb80, 0xbc0,
	0xc00, 0xc40, 0xc80, 0xcc0, 0xd00, 0xd40, 0xd80, 0xdc0,
	0xe00, 0xe40, 0xe80, 0xec0, 0xf00, 0xf40, 0xf80, 0xfc0
};

static const gfx_layout firetrk_trailer_layout =
{
	64, 64, // width, height
	8,      // total
	1,      // planes
	{ 0 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	0x1000,
	firetrk_trailer_layout_xoffset,
	firetrk_trailer_layout_yoffset
};


static GFXDECODE_START( gfx_firetrk )
	GFXDECODE_ENTRY( "chars",   0, firetrk_text_layout, 26, 1 )
	GFXDECODE_ENTRY( "tiles",   0, firetrk_tile_layout, 0, 8 )
	GFXDECODE_ENTRY( "tiles",   0, firetrk_tile_layout, 16, 3 )
	GFXDECODE_ENTRY( "cars",    0, firetrk_car_layout1, 22, 2 )
	GFXDECODE_ENTRY( "cars",    0, firetrk_car_layout2, 22, 2 )
	GFXDECODE_ENTRY( "trailer", 0, firetrk_trailer_layout, 22, 2 )
GFXDECODE_END


static GFXDECODE_START( gfx_superbug )
	GFXDECODE_ENTRY( "chars", 0, superbug_text_layout, 26, 1 )
	GFXDECODE_ENTRY( "tiles", 0, superbug_tile_layout, 0, 8 )
	GFXDECODE_ENTRY( "tiles", 0, superbug_tile_layout, 16, 3 )
	GFXDECODE_ENTRY( "cars",  0, superbug_car_layout1, 22, 2 )
	GFXDECODE_ENTRY( "cars",  0, superbug_car_layout2, 22, 2 )
GFXDECODE_END


static GFXDECODE_START( gfx_montecar )
	GFXDECODE_ENTRY( "chars", 0, montecar_text_layout, 44, 1 )
	GFXDECODE_ENTRY( "tiles", 0, firetrk_tile_layout, 0, 8 )
	GFXDECODE_ENTRY( "tiles", 0, firetrk_tile_layout, 16, 4 )
	GFXDECODE_ENTRY( "cars",  0, montecar_car_layout, 24, 1 )
	GFXDECODE_ENTRY( "drone", 0, montecar_car_layout, 28, 4 )
GFXDECODE_END


void firetrk_state::firetrk(machine_config &config)
{
	// basic machine hardware
	M6800(config, m_maincpu, MASTER_CLOCK / 12); // 750Khz during service mode
	m_maincpu->set_addrmap(AS_PROGRAM, &firetrk_state::program_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(firetrk_state::scanline), "screen", 0, 1);

	WATCHDOG_TIMER(config, m_watchdog).set_vblank_count("screen", 5);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE);
	m_screen->set_raw(MASTER_CLOCK / 2, 384, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(firetrk_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(firetrk_state::palette), 28);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_firetrk);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, firetrk_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void superbug_state::superbug(machine_config &config)
{
	firetrk(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &superbug_state::program_map);

	// video hardware
	m_screen->set_screen_update(FUNC(superbug_state::screen_update));

	m_gfxdecode->set_info(gfx_superbug);

	// sound hardware
	DISCRETE(config.replace(), m_discrete, superbug_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


void montecar_state::montecar(machine_config &config)
{
	firetrk(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &montecar_state::program_map);

	// video hardware
	m_screen->set_screen_update(FUNC(montecar_state::screen_update));

	m_gfxdecode->set_info(gfx_montecar);

	m_palette->set_entries(46);
	m_palette->set_init(FUNC(montecar_state::palette));

	/* sound hardware */
	DISCRETE(config.replace(), m_discrete, montecar_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( firetrk )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD(          "032823-02.c1", 0x2000, 0x800, CRC(9570bdd3) SHA1(4d26a9490d05d53da55fc59459a4dce5bca6c761) )
	ROM_LOAD(          "032824-01.d1", 0x2800, 0x800, CRC(a5fc5629) SHA1(bf20510d8623eda2740ff296a7813a3e6f7ec76e) )
	ROM_LOAD_NIB_HIGH( "032816-01.k1", 0x3000, 0x800, CRC(c0535598) SHA1(15cb6985b0b22140b7fae1e050e0b63dd4d0f793) ) // one PCB has been found with this ROM labeled 032816-02.k1, CRC matches
	ROM_LOAD_NIB_LOW ( "032820-01.k2", 0x3000, 0x800, CRC(5733f9ed) SHA1(0f19a40793dadfb7de2c2b54a44929b414d0f4ed) )
	ROM_LOAD_NIB_HIGH( "032815-01.j1", 0x3800, 0x800, CRC(506ee759) SHA1(d111356c84f3d9942a27fbe243e716d14c258a16) )
	ROM_LOAD_NIB_LOW ( "032819-01.j2", 0x3800, 0x800, CRC(f1c3fa87) SHA1(d75cf4ad0bcac3289c068837fc24cfe84ce7542a) )

	ROM_REGION( 0x0800, "chars", 0 )
	ROM_LOAD( "032827-01.r3", 0x000, 0x800, CRC(cca31d2b) SHA1(78235176c9cb2abd73a5778b54560b87634ca0e4) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "032828-02.f5", 0x000, 0x800, CRC(68ef5f19) SHA1(df227d6a57bba6298ebdeb5a118878da21d889f6) )

	ROM_REGION( 0x0400, "cars", 0 )
	ROM_LOAD( "032831-01.p7", 0x000, 0x400, CRC(bb8d144f) SHA1(9a1355ea6f88e96926c32e0e36ac0525b0243906) )

	ROM_REGION( 0x1000, "trailer", 0 )
	ROM_LOAD( "032829-01.j5", 0x000, 0x800, CRC(e7267d71) SHA1(7132b98622e899227a378ba8c010dde39c479978) )
	ROM_LOAD( "032830-01.l5", 0x800, 0x800, CRC(e4d8b685) SHA1(30978658899c83e32dabdf554a13cf5e5235c725) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "009114.prm", 0x0000, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) // sync
ROM_END


ROM_START( firetrka ) // identical data as above, just using ROMs instead of PROMs for the 0x3000 - 0x3fff range.
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "032823-02.c1",  0x2000, 0x800, CRC(9570bdd3) SHA1(4d26a9490d05d53da55fc59459a4dce5bca6c761) )
	ROM_LOAD( "032824-01.d1",  0x2800, 0x800, CRC(a5fc5629) SHA1(bf20510d8623eda2740ff296a7813a3e6f7ec76e) )
	ROM_LOAD( "032825-02.bin", 0x3000, 0x800, CRC(fa6f050f) SHA1(531b256d536cb4da450413d7b55bcba25ce02145) )
	ROM_LOAD( "032826-02.bin", 0x3800, 0x800, CRC(e9080179) SHA1(5c0a246578a9336f89d585278cd4683782f8e006) )

	ROM_REGION( 0x0800, "chars", 0 )
	ROM_LOAD( "032827-01.r3", 0x000, 0x800, CRC(cca31d2b) SHA1(78235176c9cb2abd73a5778b54560b87634ca0e4) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "032828-02.f5", 0x000, 0x800, CRC(68ef5f19) SHA1(df227d6a57bba6298ebdeb5a118878da21d889f6) )

	ROM_REGION( 0x0400, "cars", 0 )
	ROM_LOAD( "032831-01.p7", 0x000, 0x400, CRC(bb8d144f) SHA1(9a1355ea6f88e96926c32e0e36ac0525b0243906) )

	ROM_REGION( 0x1000, "trailer", 0 )
	ROM_LOAD( "032829-01.j5", 0x000, 0x800, CRC(e7267d71) SHA1(7132b98622e899227a378ba8c010dde39c479978) )
	ROM_LOAD( "032830-01.l5", 0x800, 0x800, CRC(e4d8b685) SHA1(30978658899c83e32dabdf554a13cf5e5235c725) )

	ROM_REGION( 0x100, "proms", 0 )
	ROM_LOAD( "009114.prm", 0x0000, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) // sync
ROM_END


ROM_START( superbug )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "009121.d1", 0x0800, 0x800, CRC(350df308) SHA1(b957c830bb95e0752ea9793e3edcfdd52235e0ab) )
	ROM_LOAD( "009122.c1", 0x1000, 0x800, CRC(eb6e3e37) SHA1(5237f6bd3a7a3eca737c728296230cf0d1f436b0) )
	ROM_LOAD( "009123.a1", 0x1800, 0x800, CRC(f42c6bbe) SHA1(41470984fe951eac9f6dc77862b00ecfe8aaa51d) )

	ROM_REGION( 0x0800, "chars", 0 )
	ROM_LOAD( "009124.m3", 0x0000, 0x400, CRC(f8af8dd5) SHA1(49ab85550f546f85048e2f73163837c602dde568) )
	ROM_LOAD( "009471.n3", 0x0400, 0x400, CRC(52250698) SHA1(cc55254c54dbcd3fd1465c82a715f2e567f44951) )

	ROM_REGION( 0x1000, "tiles", 0 )
	ROM_LOAD( "009126.f5", 0x0000, 0x400, CRC(ee695137) SHA1(295fdfef88e0c841fe8ad505151ca0837e77ef83) )
	ROM_LOAD( "009472.h5", 0x0400, 0x400, CRC(5ddb80ac) SHA1(bdbbbba6efdd4cca75630d203f7c7eaf41b1a32d) )
	ROM_LOAD( "009127.e5", 0x0800, 0x400, CRC(be1386b4) SHA1(17e92df58b25075ec7a383a958db02b42066578a) )
	ROM_RELOAD(          0x0C00, 0x400 )

	ROM_REGION( 0x0400, "cars", 0 )
	ROM_LOAD( "009125.k6", 0x0000, 0x400, CRC(a3c835df) SHA1(e9b6dba1919c389bb55a8fe3c074b6702322e4e5) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "009114.prm", 0x0000, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) // sync
ROM_END


ROM_START( montecar )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "35766-01.h1", 0x2000, 0x800, CRC(d3695f09) SHA1(8aa3b3921acd0d2c3230d610843042613defcba9) )
	ROM_LOAD( "35765-01.f1", 0x2800, 0x800, CRC(9491a7ee) SHA1(712959c5f97be3db7be1d5bd70c780d4da2f6d47) )
	ROM_LOAD( "35764-01.d1", 0x3000, 0x800, CRC(899aaf4e) SHA1(84fab58d135ffc6e4b076d438b4d588b394364b6) )
	ROM_LOAD( "35763-01.c1", 0x3800, 0x800, CRC(378bfe47) SHA1(fd6b28907340a2ffc82a4e634273c3f03ab76642) )

	ROM_REGION( 0x0400, "chars", 0 )
	ROM_LOAD( "35778-01.m4", 0x0000, 0x400, CRC(294ee08e) SHA1(fbb0656468a027b2795073d811affc93c50994ec) )

	ROM_REGION( 0x0800, "tiles", 0 )
	ROM_LOAD( "35775-01.e6", 0x0000, 0x800, CRC(504106e9) SHA1(33eae2cf39b24eaf5b438a2af3060b2fdc0012b5) )

	ROM_REGION( 0x0800, "cars", 0 )
	ROM_LOAD( "35779-01.m6", 0x0000, 0x800, CRC(4fbb3fe1) SHA1(4267cd098a19892322d21f8fa7b55896158f8d6a) )

	ROM_REGION( 0x0800, "drone", 0 )
	ROM_LOAD( "35780-01.b6", 0x0000, 0x800, CRC(9d0f1374) SHA1(52d1130d48dc877e1e47e26b2e4548633ed91b21) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "35785-01.e7", 0x0000, 0x200, CRC(386c543a) SHA1(04edda180e6ff432b438947ffa46621ca0a823b4) ) // color
	ROM_LOAD( "9114.prm",    0x0200, 0x100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) ) // sync
ROM_END

} // anonymous namespace


GAMEL( 1977, superbug, 0,       superbug, superbug, superbug_state, empty_init, ROT270, "Atari (Kee Games)", "Super Bug",                              MACHINE_SUPPORTS_SAVE, layout_superbug )
GAME(  1978, firetrk,  0,       firetrk,  firetrk,  firetrk_state,  empty_init, ROT270, "Atari",             "Fire Truck / Smokey Joe (PROM version)", MACHINE_SUPPORTS_SAVE )
GAME(  1978, firetrka, firetrk, firetrk,  firetrk,  firetrk_state,  empty_init, ROT270, "Atari",             "Fire Truck / Smokey Joe (ROM version)",  MACHINE_SUPPORTS_SAVE )
GAME(  1979, montecar, 0,       montecar, montecar, montecar_state, empty_init, ROT270, "Atari",             "Monte Carlo",                            MACHINE_SUPPORTS_SAVE )
