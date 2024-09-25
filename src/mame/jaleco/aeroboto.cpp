// license:BSD-3-Clause
// copyright-holders: Carlos A. Lozano, Uki

/****************************************************************************

Formation Z / Aeroboto

PCB ID: JALECO FZ-8420

Driver by Carlos A. Lozano

TODO:
- star field
  Uki's report:
  - The color of stars:
    at 1st title screen = neutral tints of blue and aqua (1 color only)
    at 2nd title screen and attract mode (purple surface) = light & dark aqua
    This color will not be affected by scroll. Leftmost 8 pixels are light, next
    16 pixels are dark, the next 16 pixels are light, and so on.
- Verify CPU clocks again. Main CPU is a HD68B09P, and next to it is a 8 MHz XTAL.

Revisions:
- Updated starfield according to Uki's report. (AT)

*note: Holding any key at boot puts the game in MCU test. Press F3 to quit.

****************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/gen_latch.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


// configurable logging
#define LOG_3004     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_3004)

#include "logmacro.h"

#define LOG3004(...)     LOGMASKED(LOG_3004,     __VA_ARGS__)


namespace {

class aeroboto_state : public driver_device
{
public:
	aeroboto_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_videoram(*this, "videoram"),
		m_hscroll(*this, "hscroll"),
		m_tilecolor(*this, "tilecolor"),
		m_spriteram(*this, "spriteram"),
		m_vscroll(*this, "vscroll"),
		m_starx(*this, "starx"),
		m_stary(*this, "stary"),
		m_bgcolor(*this, "bgcolor"),
		m_stars_rom(*this, "starfield_data"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_p(*this, "P%u", 1U)
	{ }

	void formatz(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// memory pointers
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_hscroll;
	required_shared_ptr<uint8_t> m_tilecolor;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_vscroll;
	required_shared_ptr<uint8_t> m_starx;
	required_shared_ptr<uint8_t> m_stary;
	required_shared_ptr<uint8_t> m_bgcolor;

	// stars layout
	required_region_ptr<uint8_t> m_stars_rom;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// I/O
	required_ioport_array<2> m_p;

	// video-related
	tilemap_t *m_bg_tilemap = nullptr;
	uint8_t m_charbank = 0U;
	uint8_t m_starsoff = 0U;
	uint8_t m_sx = 0U;
	uint8_t m_sy = 0U;
	uint8_t m_ox = 0U;
	uint8_t m_oy = 0U;

	// misc
	uint32_t m_count = 0U;
	uint8_t m_disable_irq = 0U;

	uint8_t _201_r();
	uint8_t irq_ack_r();
	uint8_t _2973_r();
	void _1a2_w(uint8_t data);
	uint8_t in0_r();
	void _3000_w(uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void tilecolor_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(int state);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


// how the starfield ROM is interpreted: 0 = 256 x 256 x 1 linear bitmap, 1 = 8 x 8 x 1 x 1024 tilemap
static constexpr uint8_t STARS_LAYOUT = 1;

// scroll speed of the stars: 1 = normal, 2 = half, 3 = one-third...etc. (positive integers only)
static constexpr uint8_t SCROLL_SPEED = 1;

/***************************************************************************

  Callbacks for the TileMap code

***************************************************************************/

TILE_GET_INFO_MEMBER(aeroboto_state::get_tile_info)
{
	uint8_t const code = m_videoram[tile_index];
	tileinfo.set(0,
			code + (m_charbank << 8),
			m_tilecolor[code],
			(m_tilecolor[code] >= 0x33) ? 0 : TILE_FORCE_LAYER0);
}
// transparency should only affect tiles with color 0x33 or higher


/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/

void aeroboto_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(aeroboto_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 64);
	m_bg_tilemap->set_transparent_pen(0);
	m_bg_tilemap->set_scroll_rows(64);

	save_item(NAME(m_charbank));
	save_item(NAME(m_starsoff));
	save_item(NAME(m_sx));
	save_item(NAME(m_sy));
	save_item(NAME(m_ox));
	save_item(NAME(m_oy));

	if (STARS_LAYOUT)
	{
		int len = m_stars_rom.bytes();
		std::vector<uint8_t> temp(len);
		memcpy(&temp[0], &m_stars_rom[0], len);

		for (int i = 0; i < len; i++)
			m_stars_rom[(i & ~0xff) + (i << 5 & 0xe0) + (i >> 3 & 0x1f)] = temp[i];
	}
}



/***************************************************************************

  Memory handlers

***************************************************************************/

uint8_t aeroboto_state::in0_r()
{
	return flip_screen() ? m_p[1]->read() : m_p[0]->read();
}

void aeroboto_state::_3000_w(uint8_t data)
{
	// bit 0 selects both flip screen and player1 / player2 controls
	flip_screen_set(data & 0x01);

	// bit 1 = char bank select
	if (m_charbank != ((data & 0x02) >> 1))
	{
		m_bg_tilemap->mark_all_dirty();
		m_charbank = (data & 0x02) >> 1;
	}

	// bit 2 = disable star field?
	m_starsoff = data & 0x04;
}

void aeroboto_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void aeroboto_state::tilecolor_w(offs_t offset, uint8_t data)
{
	if (m_tilecolor[offset] != data)
	{
		m_tilecolor[offset] = data;
		m_bg_tilemap->mark_all_dirty();
	}
}



/***************************************************************************

  Display refresh

***************************************************************************/

void aeroboto_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int offs = 0; offs < m_spriteram.bytes(); offs += 4)
	{
		int x = m_spriteram[offs + 3];
		int y = 240 - m_spriteram[offs];

		if (flip_screen())
		{
			x = 248 - x;
			y = 240 - y;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
				m_spriteram[offs + 1],
				m_spriteram[offs + 2] & 0x07,
				flip_screen(), flip_screen(),
				((x + 8) & 0xff) - 8, y, 0);
	}
}


uint32_t aeroboto_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	rectangle const splitrect1(0, 255, 0, 39);
	rectangle const splitrect2(0, 255, 40, 255);

	int star_color = *m_bgcolor << 2;
	int sky_color = *m_bgcolor << 2;

	// the star field is supposed to be seen through tile pen 0 when active
	if (!m_starsoff)
	{
		if (star_color < 0xd0)
		{
			star_color = 0xd0;
			sky_color = 0;
		}

		star_color += 2;

		bitmap.fill(sky_color, cliprect);

		// actual scroll speed is unknown but it can be adjusted by changing the SCROLL_SPEED constant
		m_sx += char(*m_starx - m_ox);
		m_ox = *m_starx;
		int const x = m_sx / SCROLL_SPEED;

		if (*m_vscroll != 0xff)
			m_sy += char(*m_stary - m_oy);
		m_oy = *m_stary;
		int const y = m_sy / SCROLL_SPEED;

			for (int i = 0; i < 256; i++)
		{
			int src_offsx = (x + i) & 0xff;
			int const src_colmask = 1 << (src_offsx & 7);
			src_offsx >>= 3;
			uint8_t const *const src_colptr = &m_stars_rom[src_offsx];
			int const pen = star_color + ((i + 8) >> 4 & 1);

			for (int j = 0; j < 256; j++)
			{
				uint8_t const *const src_rowptr = src_colptr + (((y + j) & 0xff) << 5 );
				if (!((unsigned)*src_rowptr & src_colmask))
					bitmap.pix(j, i) = pen;
			}
		}
	}
	else
	{
		m_sx = m_ox = *m_starx;
		m_sy = m_oy = *m_stary;
		bitmap.fill(sky_color, cliprect);
	}

	for (int y = 0; y < 64; y++)
		m_bg_tilemap->set_scrollx(y, m_hscroll[y]);

	// the playfield is part of a splitscreen and should not overlap with status display
	m_bg_tilemap->set_scrolly(0, *m_vscroll);
	m_bg_tilemap->draw(screen, bitmap, splitrect2, 0, 0);

	draw_sprites(bitmap, cliprect);

	// the status display behaves more closely to a 40-line splitscreen than an overlay
	m_bg_tilemap->set_scrolly(0, 0);
	m_bg_tilemap->draw(screen, bitmap, splitrect1, 0, 0);
	return 0;
}


uint8_t aeroboto_state::_201_r()
{
	/* if you keep a button pressed during boot, the game will expect this
	   series of values to be returned from 3004, and display "PASS 201" if it is */
	static const uint8_t res[4] = { 0xff, 0x9f, 0x1b, 0x03 };

	LOG3004("PC %04x: read 3004\n", m_maincpu->pc());
	return res[(m_count++) & 3];
}


void aeroboto_state::vblank_irq(int state)
{
	if (state)
	{
		if (!m_disable_irq)
			m_maincpu->set_input_line(0, ASSERT_LINE);
		else
			m_disable_irq--;

		m_audiocpu->set_input_line(0, HOLD_LINE);
	}
}

uint8_t aeroboto_state::irq_ack_r()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	return 0xff;
}

uint8_t aeroboto_state::_2973_r()
{
	m_mainram[0x02be] = 0;
	return 0xff;
}

void aeroboto_state::_1a2_w(uint8_t data)
{
	m_mainram[0x01a2] = data;
	if (data)
		m_disable_irq = 1;
}

void aeroboto_state::main_map(address_map &map)
{
	map(0x0000, 0x07ff).ram().share(m_mainram);
	map(0x01a2, 0x01a2).w(FUNC(aeroboto_state::_1a2_w));           // affects IRQ line (more protection?)
	map(0x0800, 0x08ff).ram();                             // tile color buffer; copied to 0x2000
	map(0x0900, 0x09ff).writeonly().share("colors2");      // a backup of default tile colors
	map(0x1000, 0x17ff).ram().w(FUNC(aeroboto_state::videoram_w)).share(m_videoram);
	map(0x1800, 0x183f).ram().share(m_hscroll);
	map(0x1840, 0x27ff).nopw();                    // cleared during custom LSI test
	map(0x2000, 0x20ff).ram().w(FUNC(aeroboto_state::tilecolor_w)).share(m_tilecolor);
	map(0x2800, 0x28ff).ram().share(m_spriteram);
	map(0x2900, 0x2fff).nopw();                    // cleared along with sprite RAM
	map(0x2973, 0x2973).r(FUNC(aeroboto_state::_2973_r));           // protection read
	map(0x3000, 0x3000).rw(FUNC(aeroboto_state::in0_r), FUNC(aeroboto_state::_3000_w));
	map(0x3001, 0x3001).portr("DSW1").w("soundlatch", FUNC(generic_latch_8_device::write));
	map(0x3002, 0x3002).portr("DSW2").w("soundlatch2", FUNC(generic_latch_8_device::write));
	map(0x3003, 0x3003).writeonly().share(m_vscroll);
	map(0x3004, 0x3004).r(FUNC(aeroboto_state::_201_r)).writeonly().share(m_starx);
	map(0x3005, 0x3005).writeonly().share(m_stary); // usable but probably wrong
	map(0x3006, 0x3006).writeonly().share(m_bgcolor);
	map(0x3800, 0x3800).r(FUNC(aeroboto_state::irq_ack_r));        // watchdog or IRQ ack
	map(0x4000, 0xffff).rom();
}

void aeroboto_state::sound_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x9000, 0x9001).w("ay1", FUNC(ay8910_device::address_data_w));
	map(0x9002, 0x9002).r("ay1", FUNC(ay8910_device::data_r));
	map(0xa000, 0xa001).w("ay2", FUNC(ay8910_device::address_data_w));
	map(0xa002, 0xa002).r("ay2", FUNC(ay8910_device::data_r));
	map(0xf000, 0xffff).rom();
}



static INPUT_PORTS_START( formatz )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "Infinite (Cheat)")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "30000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x04, "70000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x00, "SW1:5" )        // Listed as "Unused"
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	/* The last dip switch is directly connected to the video hardware and
	   flips the screen. The program instead sees the coin input, which must
	   stay low for exactly 2 frames to be consistently recognized. */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(2) PORT_DIPLOCATION("SW1:8") /* "Screen Inversion" */

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x18, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x00, "SW2:6" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x00, "SW2:7" )        // Listed as "Unused"
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x00, "SW2:8" )        // Listed as "Unused"
INPUT_PORTS_END



static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	2,
	{ 4, 0 },
	{ 0, 1, 2, 3, RGN_FRAC(1,2)+0, RGN_FRAC(1,2)+1, RGN_FRAC(1,2)+2, RGN_FRAC(1,2)+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};
/*
// exact star layout unknown... could be anything
static const gfx_layout starlayout =
{
    8,8,
    RGN_FRAC(1,1),
    1,
    { 0 },
    { 0, 1, 2, 3, 4, 5, 6, 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8
};
*/
static const gfx_layout spritelayout =
{
	8,16,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3), RGN_FRAC(1,3), RGN_FRAC(0,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static GFXDECODE_START( gfx_aeroboto )
	GFXDECODE_ENTRY( "tiles", 0,          charlayout,     0,  64 )
//  GFXDECODE_ENTRY( "starfield_data", 0, starlayout,     0, 128 )
	GFXDECODE_ENTRY( "sprites", 0,        spritelayout,   0,   8 )
GFXDECODE_END

void aeroboto_state::machine_start()
{
	save_item(NAME(m_disable_irq));
	save_item(NAME(m_count));
}

void aeroboto_state::machine_reset()
{
	m_disable_irq = 0;
	m_count = 0;

	m_charbank = 0;
	m_starsoff = 0;
	m_ox = 0;
	m_oy = 0;
	m_sx = 0;
	m_sy = 0;
}

void aeroboto_state::formatz(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, XTAL(10'000'000) / 2); // verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &aeroboto_state::main_map);

	MC6809(config, m_audiocpu, XTAL(10'000'000) / 4); // verified on PCB
	m_audiocpu->set_addrmap(AS_PROGRAM, &aeroboto_state::sound_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 31*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(aeroboto_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(aeroboto_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aeroboto);

	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, "soundlatch");
	GENERIC_LATCH_8(config, "soundlatch2");

	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'000'000) / 8)); // verified on PCB
	ay1.port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	ay1.port_b_read_callback().set("soundlatch2", FUNC(generic_latch_8_device::read));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, "ay2", XTAL(10'000'000) / 16).add_route(ALL_OUTPUTS, "mono", 0.25); // verified on PCB
}



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( formatz )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "format_z.8",   0x4000, 0x4000, CRC(81a2416c) SHA1(d43c6bcc079847cb4c8e77fdc4d9d5bb9c2cc41a) )
	ROM_LOAD( "format_z.7",   0x8000, 0x4000, CRC(986e6052) SHA1(4d39eda38fa17695f8217b0032a750cbe71c5674) )
	ROM_LOAD( "format_z.6",   0xc000, 0x4000, CRC(baa0d745) SHA1(72b6cf31c9bbf9b5c55ef3f4ca5877ce576beda9) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "format_z.9",   0xf000, 0x1000, CRC(6b9215ad) SHA1(3ab416d070bf6b9a8be3e19d4dbc3a399d9ab5cb) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "format_z.5",   0x0000, 0x2000, CRC(ba50be57) SHA1(aa37b644e8c1944b4c0ba81164d5a52be8ab491f) )

	ROM_REGION( 0x2000, "starfield_data", 0 ) // starfield data
	ROM_LOAD( "format_z.4",   0x0000, 0x2000, CRC(910375a0) SHA1(1044e0f45ce34c15986d9ab520c0e7d08fd46dde) )

	ROM_REGION( 0x3000, "sprites", 0 )
	ROM_LOAD( "format_z.1",   0x0000, 0x1000, CRC(5739afd2) SHA1(3a645bc8a5ac69f1dc878a589c580f2bf033d3cb) )
	ROM_LOAD( "format_z.2",   0x1000, 0x1000, CRC(3a821391) SHA1(476507ba5e5d64ca3729244590beadb9b3a6a018) )
	ROM_LOAD( "format_z.3",   0x2000, 0x1000, CRC(7d1aec79) SHA1(bb19d6c91a14df26706226cfe22853bb8383c63d) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "10c",          0x0000, 0x0100, CRC(b756dd6d) SHA1(ea79f87f84ded2f0a66458af24cbc792e5ff77e3) )
	ROM_LOAD( "10b",          0x0100, 0x0100, CRC(00df8809) SHA1(f4539c052a5ce8a63662db070c3f52139afef23d) )
	ROM_LOAD( "10a",          0x0200, 0x0100, CRC(e8733c8f) SHA1(105b44c9108ee173a417f8c79ec8381f824dd675) )
ROM_END

ROM_START( aeroboto )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "aeroboto.8",   0x4000, 0x4000, CRC(4d3fc049) SHA1(6efb8c58c025a69ac2dce99049128861f7ede690) )
	ROM_LOAD( "aeroboto.7",   0x8000, 0x4000, CRC(522f51c1) SHA1(4ea47d0b8b65e711c99701c055dbaf70a003d441) )
	ROM_LOAD( "aeroboto.6",   0xc000, 0x4000, CRC(1a295ffb) SHA1(990b3f2f883717c180089b6ba5ae381ed9272341) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "format_z.9",   0xf000, 0x1000, CRC(6b9215ad) SHA1(3ab416d070bf6b9a8be3e19d4dbc3a399d9ab5cb) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "aeroboto.5",   0x0000, 0x2000, CRC(32fc00f9) SHA1(fd912fe2ab0101057c15c846f0cc4259cd94b035) )

	ROM_REGION( 0x2000, "starfield_data", 0 ) // starfield data
	ROM_LOAD( "format_z.4",   0x0000, 0x2000, CRC(910375a0) SHA1(1044e0f45ce34c15986d9ab520c0e7d08fd46dde) )

	ROM_REGION( 0x3000, "sprites", 0 )
	ROM_LOAD( "aeroboto.1",   0x0000, 0x1000, CRC(7820eeaf) SHA1(dedd15295bb02f417d0f51a29df686b66b94dee1) )
	ROM_LOAD( "aeroboto.2",   0x1000, 0x1000, CRC(c7f81a3c) SHA1(21476a4146d5c57e2b15125c304fc61d82edf4af) )
	ROM_LOAD( "aeroboto.3",   0x2000, 0x1000, CRC(5203ad04) SHA1(d16eb370de9033793a502e23c82a3119cd633aa9) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "10c",          0x0000, 0x0100, CRC(b756dd6d) SHA1(ea79f87f84ded2f0a66458af24cbc792e5ff77e3) )
	ROM_LOAD( "10b",          0x0100, 0x0100, CRC(00df8809) SHA1(f4539c052a5ce8a63662db070c3f52139afef23d) )
	ROM_LOAD( "10a",          0x0200, 0x0100, CRC(e8733c8f) SHA1(105b44c9108ee173a417f8c79ec8381f824dd675) )
ROM_END

} // anonymous namespace


GAME( 1984, formatz,  0,       formatz, formatz, aeroboto_state, empty_init, ROT0, "Jaleco",                    "Formation Z", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1984, aeroboto, formatz, formatz, formatz, aeroboto_state, empty_init, ROT0, "Jaleco (Williams license)", "Aeroboto",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
