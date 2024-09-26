// license:BSD-3-Clause
// copyright-holders:Frank Palazzolo, Stefan Jokisch

/***************************************************************************

Atari Starship 1 driver

  "starshp1" -> regular version, bonus time for 3500 points
  "starshpp" -> possible prototype, bonus time for 2700 points

 *  The schematics don't seem to make a lot of sense when it
 *  comes to the video timing chain::
 *
 *    * there are clear typos -- what is H132???
 *    * there are two HBLANK/HSYNC periods as the horizontal
 *      chain is drawn, which would give an alternating long
 *      line followed by a much shorter one.  This cannot be right
 *    * the carry-out/load circuit on LS161@J8 is superfluous
 *
 *  These values also give a frame rate of about 45Hz, which is
 *  probably too low.  I suspect that screen is not really
 *  512 pixels wide -- most likely 384, which would give 60Hz
 *
 *  Based on photographs of the PCB, and analysis of videos of
 *  actual gameplay, the horizontal screen really is 384 clocks.
 *
 *  However, some of the graphics, like the starfield, are
 *  clocked with the 12MHz signal.  This effectively doubles
 *  the horizontal resolution:
 *
 *                             6.048Mhz clocks     12.096Mhz clocks
 *  Horizontal Visible Area    384 (0x180)         768 (0x300)
 *  Horizontal Blanking Time   128 (0x080)         256 (0x100)

***************************************************************************/

#include "emu.h"

#include "starshp1_a.h"

#include "cpu/m6502/m6502.h"
#include "machine/74259.h"
#include "sound/discrete.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class starshp1_state : public driver_device
{
public:
	starshp1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_playfield_ram(*this, "playfield_ram"),
		m_hpos_ram(*this, "hpos_ram"),
		m_vpos_ram(*this, "vpos_ram"),
		m_obj_ram(*this, "obj_ram"),
		m_system(*this, "SYSTEM"),
		m_playtime(*this, "PLAYTIME"),
		m_stick(*this, "STICK%c", 'X')
	{ }

	void starshp1(machine_config &config);

	ioport_value analog_r();
	ioport_value collision_latch_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<discrete_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_playfield_ram;
	required_shared_ptr<uint8_t> m_hpos_ram;
	required_shared_ptr<uint8_t> m_vpos_ram;
	required_shared_ptr<uint8_t> m_obj_ram;

	required_ioport m_system;
	required_ioport m_playtime;
	required_ioport_array<2> m_stick;

	uint8_t m_analog_in_select = 0;
	uint8_t m_attract = 0;
	uint8_t m_ship_explode = 0;
	uint8_t m_ship_picture = 0;
	uint8_t m_ship_hoffset = 0;
	uint8_t m_ship_voffset = 0;
	uint8_t m_ship_size = 0;
	uint8_t m_circle_hpos = 0;
	uint8_t m_circle_vpos = 0;
	uint8_t m_circle_size = 0;
	uint8_t m_circle_mod = 0;
	uint8_t m_circle_kill = 0;
	uint8_t m_phasor = 0;
	uint8_t m_collision_latch = 0;
	uint8_t m_starfield_kill = 0;
	uint8_t m_mux = 0;
	uint8_t m_inverse = 0;
	std::unique_ptr<uint16_t[]> m_lsfr;
	bitmap_ind16 m_helper;
	tilemap_t *m_bg_tilemap = nullptr;

	void collision_reset_w(uint8_t data);
	void analog_in_w(offs_t offset, uint8_t data);
	void ship_explode_w(int state);
	void circle_mod_w(int state);
	void circle_kill_w(int state);
	void starfield_kill_w(int state);
	void inverse_w(int state);
	void mux_w(int state);
	uint8_t rng_r();
	void ssadd_w(offs_t offset, uint8_t data);
	void sspic_w(uint8_t data);
	void playfield_w(offs_t offset, uint8_t data);
	void attract_w(int state);
	void phasor_w(int state);
	void analog_out_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);
	INTERRUPT_GEN_MEMBER(interrupt);
	void set_pens();
	void draw_starfield(bitmap_ind16 &bitmap, const rectangle &cliprect);
	int get_sprite_hpos(int i);
	int get_sprite_vpos(int i);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_spaceship(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_phasor(bitmap_ind16 &bitmap);
	int get_radius();
	int get_circle_hpos();
	int get_circle_vpos();
	void draw_circle_line(bitmap_ind16 &bitmap, int x, int y, int l);
	void draw_circle(bitmap_ind16 &bitmap);
	int spaceship_collision(bitmap_ind16 &bitmap, const rectangle &rect);
	int point_in_circle(int x, int y, int center_x, int center_y, int r);
	int circle_collision(const rectangle &rect);
	void program_map(address_map &map) ATTR_COLD;
};


void starshp1_state::set_pens()
{
	m_palette->set_indirect_color(m_inverse ? 7 : 0, rgb_t(0x00, 0x00, 0x00));
	m_palette->set_indirect_color(m_inverse ? 6 : 1, rgb_t(0x1e, 0x1e, 0x1e));
	m_palette->set_indirect_color(m_inverse ? 5 : 2, rgb_t(0x4e, 0x4e, 0x4e));
	m_palette->set_indirect_color(m_inverse ? 4 : 3, rgb_t(0x6c, 0x6c, 0x6c));
	m_palette->set_indirect_color(m_inverse ? 3 : 4, rgb_t(0x93, 0x93, 0x93));
	m_palette->set_indirect_color(m_inverse ? 2 : 5, rgb_t(0xb1, 0xb1, 0xb1));
	m_palette->set_indirect_color(m_inverse ? 1 : 6, rgb_t(0xe1, 0xe1, 0xe1));
	m_palette->set_indirect_color(m_inverse ? 0 : 7, rgb_t(0xff, 0xff, 0xff));
}


void starshp1_state::palette(palette_device &palette) const
{
	static constexpr uint16_t colortable_source[] =
	{
		0, 3,       // 0x00 - 0x01 - alpha numerics
		0, 2,       // 0x02 - 0x03 - sprites (Z=0)
		0, 5,       // 0x04 - 0x05 - sprites (Z=1)
		0, 2, 4, 6, // 0x06 - 0x09 - spaceship (EXPLODE=0)
		0, 6, 6, 7, // 0x0a - 0x0d - spaceship (EXPLODE=1)
		5, 2,       // 0x0e - 0x0f - star field
		7,          // 0x10        - phasor
		5, 7        // 0x11        - circle
	};

	for (unsigned i = 0; i < std::size(colortable_source); i++)
		palette.set_pen_indirect(i, colortable_source[i]);
}


TILE_GET_INFO_MEMBER(starshp1_state::get_tile_info)
{
	uint8_t const code = m_playfield_ram[tile_index];

	tileinfo.set(0, code & 0x3f, 0, 0);
}


void starshp1_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(starshp1_state::get_tile_info)), TILEMAP_SCAN_ROWS, 16, 8, 32, 8);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scrollx(0, -8);

	m_lsfr = std::make_unique<uint16_t[]>(0x10000);

	uint16_t val = 0;
	for (int i = 0; i < 0x10000; i++)
	{
		int bit = (val >> 0xf) ^
					(val >> 0xc) ^
					(val >> 0x7) ^
					(val >> 0x1) ^ 1;

		m_lsfr[i] = val;

		val = (val << 1) | (bit & 1);
	}

	m_screen->register_screen_bitmap(m_helper);
}


uint8_t starshp1_state::rng_r()
{
	int const width = m_screen->width();
	int const height = m_screen->height();
	int x = m_screen->hpos();
	int y = m_screen->vpos();

	/* the LFSR is only running in the non-blank region
	   of the screen, so this is not quite right */
	if (x > width - 1)
		x = width - 1;
	if (y > height - 1)
		y = height - 1;

	return m_lsfr[x + (uint16_t) (512 * y)];
}


void starshp1_state::ssadd_w(offs_t offset, uint8_t data)
{
	/*
	 * The range of sprite position values doesn't suffice to
	 * move the zoomed spaceship sprite over the top and left
	 * edges of the screen. These additional values are used
	 * to compensate for this. Technically, they cut off the
	 * first columns and rows of the spaceship sprite, but in
	 * practice they work like offsets in zoomed pixels.
	 */

	m_ship_voffset = ((offset & 0xf0) >> 4);
	m_ship_hoffset = ((offset & 0x0f) << 2) | (data & 3);
}


void starshp1_state::sspic_w(uint8_t data)
{
	/*
	 * Some mysterious game code at address $2CCE causes
	 * erratic images in the target explosion sequence. But
	 * this is the way the actual game worked!
	 */

	m_ship_picture = data;
}


void starshp1_state::playfield_w(offs_t offset, uint8_t data)
{
	if (m_mux != 0)
	{
		offset ^= 0x1f;
		m_playfield_ram[offset] = data;
		m_bg_tilemap->mark_tile_dirty(offset);
	}
}


void starshp1_state::draw_starfield(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/*
	 * The LSFR is reset once per frame at the position of
	 * sprite 15. This behavior is quite pointless and not
	 * really needed by the game. Not emulated.
	 */

	for (int y = 0; y <= cliprect.bottom(); y++)
	{
		uint16_t const *const p = m_lsfr.get() + (uint16_t) (512 * y);
		uint16_t *const pLine = &bitmap.pix(y);

		for (int x = 0; x <= cliprect.right(); x++)
			if ((p[x] & 0x5b56) == 0x5b44)
				pLine[x] = (p[x] & 0x0400) ? 0x0e : 0x0f;
	}
}


int starshp1_state::get_sprite_hpos(int i)
{
	return 2 * (m_hpos_ram[i] ^ 0xff);
}
int starshp1_state::get_sprite_vpos(int i)
{
	return 1 * (m_vpos_ram[i] - 0x07);
}


void starshp1_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int i = 0; i < 14; i++)
	{
		int const code = (m_obj_ram[i] & 0xf) ^ 0xf;

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			code % 8,
			code / 8,
			0, 0,
			get_sprite_hpos(i),
			get_sprite_vpos(i), 0);
	}
}


void starshp1_state::draw_spaceship(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	double scaler = -5 * log(1 - m_ship_size / 256.0); /* ? */

	unsigned xzoom = 2 * 0x10000 * scaler;
	unsigned yzoom = 1 * 0x10000 * scaler;

	int x = get_sprite_hpos(14);
	int y = get_sprite_vpos(14);

	if (x <= 0)
		x -= (xzoom * m_ship_hoffset) >> 16;

	if (y <= 0)
		y -= (yzoom * m_ship_voffset) >> 16;

	m_gfxdecode->gfx(2)->zoom_transpen(bitmap, cliprect,
		m_ship_picture & 0x03,
		m_ship_explode,
		m_ship_picture & 0x80, 0,
		x, y,
		xzoom, yzoom, 0);
}


void starshp1_state::draw_phasor(bitmap_ind16 &bitmap)
{
	for (int i = 128; i < 240; i++)
		if (i >= get_sprite_vpos(13))
		{
			bitmap.pix(i, 2 * i + 0) = 0x10;
			bitmap.pix(i, 2 * i + 1) = 0x10;
			bitmap.pix(i, 2 * (255 - i) + 0) = 0x10;
			bitmap.pix(i, 2 * (255 - i) + 1) = 0x10;
		}
}


int starshp1_state::get_radius()
{
	return 6 * sqrt((double)m_circle_size);  // size calibrated by hand
}
int starshp1_state::get_circle_hpos()
{
	return 2 * (3 * m_circle_hpos / 2 - 64);
}
int starshp1_state::get_circle_vpos()
{
	return 1 * (3 * m_circle_vpos / 2 - 64);
}


void starshp1_state::draw_circle_line(bitmap_ind16 &bitmap, int x, int y, int l)
{
	if (y >= 0 && y <= bitmap.height() - 1)
	{
		uint16_t const *const p = m_lsfr.get() + uint16_t(512 * y);

		uint16_t *const pLine = &bitmap.pix(y);

		int h1 = x - 2 * l;
		int h2 = x + 2 * l;

		if (h1 < 0)
			h1 = 0;
		if (h2 > bitmap.width() - 1)
			h2 = bitmap.width() - 1;

		for (x = h1; x <= h2; x++)
			if (m_circle_mod)
			{
				if (p[x] & 1)
					pLine[x] = 0x11;
			}
			else
				pLine[x] = 0x12;
	}
}


void starshp1_state::draw_circle(bitmap_ind16 &bitmap)
{
	int const cx = get_circle_hpos();
	int const cy = get_circle_vpos();

	int x = 0;
	int y = get_radius();

	// Bresenham's circle algorithm

	int d = 3 - 2 * get_radius();

	while (x <= y)
	{
		draw_circle_line(bitmap, cx, cy - x, y);
		draw_circle_line(bitmap, cx, cy + x, y);
		draw_circle_line(bitmap, cx, cy - y, x);
		draw_circle_line(bitmap, cx, cy + y, x);

		x++;

		if (d < 0)
			d += 4 * x + 6;
		else
			d += 4 * (x - y--) + 10;
	}
}


int starshp1_state::spaceship_collision(bitmap_ind16 &bitmap, const rectangle &rect)
{
	for (int y = rect.top(); y <= rect.bottom(); y++)
	{
		uint16_t const *const pLine = &m_helper.pix(y);

		for (int x = rect.left(); x <= rect.right(); x++)
			if (pLine[x] != 0)
				return 1;
	}

	return 0;
}


int starshp1_state::point_in_circle(int x, int y, int center_x, int center_y, int r)
{
	int const dx = abs(x - center_x) / 2;
	int const dy = abs(y - center_y) / 1;

	return dx * dx + dy * dy < r * r;
}


int starshp1_state::circle_collision(const rectangle &rect)
{
	int const center_x = get_circle_hpos();
	int const center_y = get_circle_vpos();

	int const r = get_radius();

	return point_in_circle(rect.left(), rect.top(), center_x, center_y, r) ||
			point_in_circle(rect.left(), rect.bottom(), center_x, center_y, r) ||
			point_in_circle(rect.right(), rect.top(), center_x, center_y, r) ||
			point_in_circle(rect.right(), rect.bottom(), center_x, center_y, r);
}


uint32_t starshp1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();

	bitmap.fill(0, cliprect);

	if (m_starfield_kill == 0)
		draw_starfield(bitmap, cliprect);

	draw_sprites(bitmap, cliprect);

	if (m_circle_kill == 0 && m_circle_mod != 0)
		draw_circle(bitmap);

	if (m_attract == 0)
		draw_spaceship(bitmap, cliprect);

	if (m_circle_kill == 0 && m_circle_mod == 0)
		draw_circle(bitmap);

	rectangle tilemaprect(0, m_bg_tilemap->width(), 0, m_bg_tilemap->height());
	tilemaprect &= cliprect;
	m_bg_tilemap->draw(screen, bitmap, tilemaprect, 0, 0);

	if (m_phasor != 0)
		draw_phasor(bitmap);

	return 0;
}


void starshp1_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		const rectangle &visarea = m_screen->visible_area();

		rectangle rect(get_sprite_hpos(13), 0, get_sprite_vpos(13), 0);
		rect.set_size(m_gfxdecode->gfx(1)->width(), m_gfxdecode->gfx(1)->height());

		rect &= m_helper.cliprect();

		m_helper.fill(0, visarea);

		if (m_attract == 0)
			draw_spaceship(m_helper, visarea);

		if (circle_collision(visarea))
			m_collision_latch |= 1;

		if (circle_collision(rect))
			m_collision_latch |= 2;

		if (spaceship_collision(m_helper, rect))
			m_collision_latch |= 4;

		if (spaceship_collision(m_helper, visarea))
			m_collision_latch |= 8;
	}
}


INTERRUPT_GEN_MEMBER(starshp1_state::interrupt)
{
	if ((m_system->read() & 0x90) != 0x90)
		device.execute().pulse_input_line(0, device.execute().minimum_quantum_time());
}


void starshp1_state::attract_w(int state)
{
	m_attract = state;
	m_discrete->write(STARSHP1_ATTRACT, state);

	machine().bookkeeping().coin_lockout_w(0, !m_attract);
	machine().bookkeeping().coin_lockout_w(1, !m_attract);
}


void starshp1_state::phasor_w(int state)
{
	m_phasor = state;
	m_discrete->write(STARSHP1_PHASOR_ON, state);
}


void starshp1_state::collision_reset_w(uint8_t data)
{
	m_collision_latch = 0;
}


ioport_value starshp1_state::analog_r()
{
	int val = 0;

	switch (m_analog_in_select)
	{
	case 0:
		val = m_stick[1]->read();
		break;
	case 1:
		val = m_stick[0]->read();
		break;
	case 2:
		val = 0x20; // DAC feedback, not used
		break;
	case 3:
		val = m_playtime->read();
		break;
	}

	return val & 0x3f;
}


ioport_value starshp1_state::collision_latch_r()
{
	return m_collision_latch & 0x0f;
}


void starshp1_state::analog_in_w(offs_t offset, uint8_t data)
{
	m_analog_in_select = offset & 3;
}


void starshp1_state::analog_out_w(offs_t offset, uint8_t data)
{
	switch (offset & 7)
	{
	case 1:
		m_ship_size = data;
		break;
	case 2:
		m_discrete->write(STARSHP1_NOISE_AMPLITUDE, data);
		break;
	case 3:
		m_discrete->write(STARSHP1_TONE_PITCH, data);
		break;
	case 4:
		m_discrete->write(STARSHP1_MOTOR_SPEED, data);
		break;
	case 5:
		m_circle_hpos = data;
		break;
	case 6:
		m_circle_vpos = data;
		break;
	case 7:
		m_circle_size = data;
		break;
	}
}


void starshp1_state::ship_explode_w(int state)
{
	m_ship_explode = state;
}


void starshp1_state::circle_mod_w(int state)
{
	m_circle_mod = state;
}


void starshp1_state::circle_kill_w(int state)
{
	m_circle_kill = !state;
}


void starshp1_state::starfield_kill_w(int state)
{
	m_starfield_kill = state;
}


void starshp1_state::inverse_w(int state)
{
	m_inverse = state;
}


void starshp1_state::mux_w(int state)
{
	m_mux = state;
}


void starshp1_state::program_map(address_map &map)
{
	map(0x0000, 0x00ff).ram().mirror(0x100);
	map(0x2c00, 0x3fff).rom();
	map(0xa000, 0xa000).portr("SYSTEM");
	map(0xb000, 0xb000).portr("VBLANK");
	map(0xc300, 0xc3ff).w(FUNC(starshp1_state::sspic_w)); // spaceship picture
	map(0xc400, 0xc400).portr("COINAGE");
	map(0xc400, 0xc4ff).w(FUNC(starshp1_state::ssadd_w)); // spaceship address
	map(0xc800, 0xc8ff).ram().w(FUNC(starshp1_state::playfield_w)).share(m_playfield_ram);
	map(0xcc00, 0xcc0f).writeonly().share(m_hpos_ram);
	map(0xd000, 0xd00f).writeonly().share(m_vpos_ram);
	map(0xd400, 0xd40f).writeonly().share(m_obj_ram);
	map(0xd800, 0xd800).r(FUNC(starshp1_state::rng_r));
	map(0xd800, 0xd80f).w(FUNC(starshp1_state::collision_reset_w));
	map(0xdc00, 0xdc07).mirror(0x0008).w("misclatch", FUNC(f9334_device::write_d0));
	map(0xdd00, 0xdd0f).w(FUNC(starshp1_state::analog_in_w));
	map(0xde00, 0xde07).mirror(0x0008).w("audiolatch", FUNC(f9334_device::write_d0));
	map(0xdf00, 0xdf0f).w(FUNC(starshp1_state::analog_out_w));
	map(0xf000, 0xffff).rom();
}


void starshp1_state::machine_start()
{
	save_item(NAME(m_analog_in_select));
	save_item(NAME(m_attract));
	save_item(NAME(m_ship_explode));
	save_item(NAME(m_ship_picture));
	save_item(NAME(m_ship_hoffset));
	save_item(NAME(m_ship_voffset));
	save_item(NAME(m_ship_size));
	save_item(NAME(m_circle_hpos));
	save_item(NAME(m_circle_vpos));
	save_item(NAME(m_circle_size));
	save_item(NAME(m_circle_mod));
	save_item(NAME(m_circle_kill));
	save_item(NAME(m_phasor));
	save_item(NAME(m_collision_latch));
	save_item(NAME(m_starfield_kill));
	save_item(NAME(m_mux));
	save_item(NAME(m_inverse));
}


static INPUT_PORTS_START( starshp1 )
	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // SWA1?
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x20, 0x20, "Extended Play" )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Thrust Control") // Speed lever - spring-returned to SLOW unless held down for FAST
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("VBLANK")
	PORT_BIT( 0x3f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(starshp1_state, analog_r)   // analog in
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("COINAGE")
	PORT_BIT( 0x0f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(starshp1_state, collision_latch_r)   // collision latch
	PORT_DIPNAME( 0x70, 0x20, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED ) // ground

	PORT_START("PLAYTIME")
	PORT_DIPNAME( 0x3f, 0x20, "Play Time" ) // potentiometer
	PORT_DIPSETTING(    0x00, "60 Seconds" )
	PORT_DIPSETTING(    0x20, "90 Seconds" )
	PORT_DIPSETTING(    0x3f, "120 Seconds" )

	PORT_START("STICKY")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_Y ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE

	PORT_START("STICKX")
	PORT_BIT( 0x3f, 0x20, IPT_AD_STICK_X ) PORT_MINMAX(0,63) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE
INPUT_PORTS_END


static const gfx_layout tilelayout =
{
	16, 8,  // 16x8 tiles
	64,     // 64 tiles
	1,      // 1 bit per pixel
	{ 0 },
	{
		0x204, 0x204, 0x205, 0x205, 0x206, 0x206, 0x207, 0x207,
		0x004, 0x004, 0x005, 0x005, 0x006, 0x006, 0x007, 0x007
	},
	{
		0x0000, 0x0400, 0x0800, 0x0c00,
		0x1000, 0x1400, 0x1800, 0x1c00
	},
	8       // step
};


static const gfx_layout spritelayout =
{
	16, 8,  // 16x8 sprites
	8,      // 8 sprites
	1,      // 1 bit per pixel
	{ 0 },
	{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f
	},
	{
		0x00, 0x20, 0x40, 0x60, 0x80, 0xa0, 0xc0, 0xe0
	},
	0x100   // step
};

static const uint32_t shiplayout_xoffset[64] =
{
		0x04, 0x05, 0x06, 0x07, 0x0c, 0x0d, 0x0e, 0x0f,
		0x14, 0x15, 0x16, 0x17, 0x1c, 0x1d, 0x1e, 0x1f,
		0x24, 0x25, 0x26, 0x27, 0x2c, 0x2d, 0x2e, 0x2f,
		0x34, 0x35, 0x36, 0x37, 0x3c, 0x3d, 0x3e, 0x3f,
		0x44, 0x45, 0x46, 0x47, 0x4c, 0x4d, 0x4e, 0x4f,
		0x54, 0x55, 0x56, 0x57, 0x5c, 0x5d, 0x5e, 0x5f,
		0x64, 0x65, 0x66, 0x67, 0x6c, 0x6d, 0x6e, 0x6f,
		0x74, 0x75, 0x76, 0x77, 0x7c, 0x7d, 0x7e, 0x7f
};

static const gfx_layout shiplayout =
{
	64, 16, // 64x16 sprites
	4,      // 4 sprites
	2,      // 2 bits per pixel
	{ 0, 0x2000 },
	EXTENDED_XOFFS,
	{ STEP16(0x000, 0x080) },
	0x800,  // step
	shiplayout_xoffset,
	nullptr
};


static GFXDECODE_START( gfx_starshp1 )
	GFXDECODE_ENTRY( "tiles",   0, tilelayout,   0, 1 )
	GFXDECODE_ENTRY( "sprites", 0, spritelayout, 2, 2 )
	GFXDECODE_ENTRY( "ship",    0, shiplayout,   6, 2 )
GFXDECODE_END


void starshp1_state::starshp1(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = 12.096_MHz_XTAL;
	constexpr XTAL CPU_CLOCK = MASTER_CLOCK / 16;
	constexpr XTAL PIXEL_CLOCK = MASTER_CLOCK;

	// basic machine hardware

	M6502(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &starshp1_state::program_map);
	m_maincpu->set_vblank_int("screen", FUNC(starshp1_state::interrupt));

	f9334_device &misclatch(F9334(config, "misclatch")); // C8
	misclatch.q_out_cb<0>().set(FUNC(starshp1_state::ship_explode_w));
	misclatch.q_out_cb<1>().set(FUNC(starshp1_state::circle_mod_w));
	misclatch.q_out_cb<2>().set(FUNC(starshp1_state::circle_kill_w));
	misclatch.q_out_cb<3>().set(FUNC(starshp1_state::starfield_kill_w));
	misclatch.q_out_cb<4>().set(FUNC(starshp1_state::inverse_w));
	misclatch.q_out_cb<5>().set_nop(); // BLACK HOLE, not used
	misclatch.q_out_cb<6>().set(FUNC(starshp1_state::mux_w));
	misclatch.q_out_cb<7>().set_output("led0").invert();

	// video hardware

	constexpr int HTOTAL = 0x300;
	constexpr int HBEND = 0x000;
	constexpr int HBSTART = 0x200;
	constexpr int VTOTAL = 0x106;
	constexpr int VBEND = 0x000;
	constexpr int VBSTART = 0x0f0;

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	m_screen->set_screen_update(FUNC(starshp1_state::screen_update));
	m_screen->screen_vblank().set(FUNC(starshp1_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_starshp1);
	PALETTE(config, m_palette, FUNC(starshp1_state::palette), 19, 8);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	DISCRETE(config, m_discrete, starshp1_discrete).add_route(ALL_OUTPUTS, "mono", 1.0);

	f9334_device &audiolatch(F9334(config, "audiolatch")); // D9
	audiolatch.q_out_cb<0>().set(FUNC(starshp1_state::attract_w));
	audiolatch.q_out_cb<1>().set(FUNC(starshp1_state::phasor_w));
	audiolatch.q_out_cb<2>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_KICKER>));
	audiolatch.q_out_cb<3>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_SL1>));
	audiolatch.q_out_cb<4>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_SL2>));
	audiolatch.q_out_cb<5>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_MOLVL>));
	audiolatch.q_out_cb<6>().set("discrete", FUNC(discrete_device::write_line<STARSHP1_NOISE_FREQ>));
}


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( starshp1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "7529-02.c2", 0x2c00, 0x0400, CRC(f191c328) SHA1(5d44be879bcf16a142a69e4f1501533e02720fe5) )
	ROM_LOAD_NIB_LOW ( "7528-02.c1", 0x2c00, 0x0400, CRC(605ed4df) SHA1(b0d892bcd08b611d2c01ab23b491c1d9db498e7b) )
	ROM_LOAD(          "7530-02.h3", 0x3000, 0x0800, CRC(4b2d466c) SHA1(2104c4d163adbf53f9853334868622752ccb01b8) )
	ROM_RELOAD(                      0xf000, 0x0800 )
	ROM_LOAD(          "7531-02.e3", 0x3800, 0x0800, CRC(b35b2c0e) SHA1(e52240cdfbba3dc380ba63f24cfc07b44feafd53) )
	ROM_RELOAD(                      0xf800, 0x0800 )

	ROM_REGION( 0x0400, "tiles", 0 )
	ROM_LOAD( "7513-01.n7",  0x0000, 0x0400, CRC(8fb0045d) SHA1(fb311c6977dec6e2a04179406e9ffdb920989a47) )

	ROM_REGION( 0x0100, "sprites", 0 )
	ROM_LOAD( "7515-01.j5",  0x0000, 0x0100, CRC(fcbcbf2e) SHA1(adf3cc43b77ad18eddbe39ee11625e552d1abab9) )

	ROM_REGION( 0x0800, "ship", 0 )
	ROM_LOAD( "7517-01.r1",  0x0000, 0x0400, CRC(1531f85f) SHA1(291822614fc6d3a71bf56607c796e18779f8cfc9) )
	ROM_LOAD( "7516-01.p1",  0x0400, 0x0400, CRC(64fbfe4c) SHA1(b2dfdcc1c9927c693fe43b2e1411d0f14375fdeb) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "7518-01.r10", 0x0000, 0x0100, CRC(80877f7e) SHA1(8b28f48936a4247c583ca6713bfbaf4772c7a4f5) ) // video output
	ROM_LOAD( "7514-01.n9",  0x0100, 0x0100, CRC(3610b453) SHA1(9e33ee04f22a9174c29fafb8e71781fa330a7a08) ) // sync
	ROM_LOAD( "7519-01.b5",  0x0200, 0x0020, CRC(23b9cd3c) SHA1(220f9f73d86cdcf1b390c52c591750a73402af50) ) // address
ROM_END

ROM_START( starshpp )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD_NIB_HIGH( "7529-02.c2", 0x2c00, 0x0400, CRC(f191c328) SHA1(5d44be879bcf16a142a69e4f1501533e02720fe5) )
	ROM_LOAD_NIB_LOW ( "7528-02.c1", 0x2c00, 0x0400, CRC(605ed4df) SHA1(b0d892bcd08b611d2c01ab23b491c1d9db498e7b) )
	ROM_LOAD_NIB_HIGH( "7521.h2", 0x3000, 0x0400, CRC(6e3525db) SHA1(b615c60e4958d6576f4c179bbead9e8d330bba99) )
	ROM_RELOAD(                   0xf000, 0x0400 )
	ROM_LOAD_NIB_LOW ( "7520.h1", 0x3000, 0x0400, CRC(2fbed61b) SHA1(5cbe1aee82a32edbf33780a46e4166ec45c88170) )
	ROM_RELOAD(                   0xf000, 0x0400 )
	ROM_LOAD_NIB_HIGH( "f2",      0x3400, 0x0400, CRC(590ea913) SHA1(4baf5a6f6c9dcc5916163f85cec01d78a339ae20) )
	ROM_RELOAD(                   0xf400, 0x0400 )
	ROM_LOAD_NIB_LOW ( "f1",      0x3400, 0x0400, CRC(84fce404) SHA1(edd78f5439c4087c4a853d66446433f9a356b17f) )
	ROM_RELOAD(                   0xf400, 0x0400 )
	ROM_LOAD_NIB_HIGH( "7525.e2", 0x3800, 0x0400, CRC(5c6d12d9) SHA1(7078b685d859fd4122b814e473c83647b81ef7cd) )
	ROM_RELOAD(                   0xf800, 0x0400 )
	ROM_LOAD_NIB_LOW ( "7524.e1", 0x3800, 0x0400, CRC(6193a7bd) SHA1(3c9eab14481cb29ba2627bc73434f579d6b96a6e) )
	ROM_RELOAD(                   0xf800, 0x0400 )
	ROM_LOAD_NIB_HIGH( "d2",      0x3c00, 0x0400, CRC(a17df2ea) SHA1(ec488f4af47594e20b3d51882ee862a92e2f38fd) )
	ROM_RELOAD(                   0xfc00, 0x0400 )
	ROM_LOAD_NIB_LOW ( "d1",      0x3c00, 0x0400, CRC(be4050b6) SHA1(03ca4833769efb10f18f52b7ba4d016568d3cab9) )
	ROM_RELOAD(                   0xfc00, 0x0400 )

	ROM_REGION( 0x0400, "tiles", 0 )
	ROM_LOAD( "7513-01.n7", 0x0000, 0x0400, CRC(8fb0045d) SHA1(fb311c6977dec6e2a04179406e9ffdb920989a47) )

	ROM_REGION( 0x0100, "sprites", 0 )
	ROM_LOAD( "7515-01.j5", 0x0000, 0x0100, CRC(fcbcbf2e) SHA1(adf3cc43b77ad18eddbe39ee11625e552d1abab9) )

	ROM_REGION( 0x0800, "ship", 0 )
	ROM_LOAD( "7517-01.r1", 0x0000, 0x0400, CRC(1531f85f) SHA1(291822614fc6d3a71bf56607c796e18779f8cfc9) )
	ROM_LOAD( "7516-01.p1", 0x0400, 0x0400, CRC(64fbfe4c) SHA1(b2dfdcc1c9927c693fe43b2e1411d0f14375fdeb) )

	ROM_REGION( 0x0220, "proms", 0 )
	ROM_LOAD( "7518-01.r10", 0x0000, 0x0100, CRC(80877f7e) SHA1(8b28f48936a4247c583ca6713bfbaf4772c7a4f5) ) // video output
	ROM_LOAD( "7514-01.n9",  0x0100, 0x0100, CRC(3610b453) SHA1(9e33ee04f22a9174c29fafb8e71781fa330a7a08) ) // sync
	ROM_LOAD( "7519-01.b5",  0x0200, 0x0020, CRC(23b9cd3c) SHA1(220f9f73d86cdcf1b390c52c591750a73402af50) ) // address
ROM_END

} // anonymous namespace


GAME( 1977, starshp1, 0,        starshp1, starshp1, starshp1_state, empty_init, ORIENTATION_FLIP_X, "Atari", "Starship 1",              MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1977, starshpp, starshp1, starshp1, starshp1, starshp1_state, empty_init, ORIENTATION_FLIP_X, "Atari", "Starship 1 (prototype?)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
