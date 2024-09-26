// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    "Monkey Magic" © 1979 Nintendo


    Dumping info provided by Andrew Welburn:

    TZF-MP  - Main Board
    TZF-SOU - Sound Board

    #   device  Label   PCB     filename
    -------------------------------------------
    4   i2708   1AI*    2A      1AI.2A
    5   i2708   2AI*    3A      2AI.3A
    6   i2708   3AI*    4A      3AI.4A
    7   i2708   4AI*    4/5A    4AI.45A
    8   i2708   5AI*    5A      5AI.5A

    22  H7641   6H      6HI     6H.6HI
    23  ?? **   7H      7HI     7H.7HI
    24  H7641   6J      6JK     6J.6JK
    25  H7641   6H***   7JK

    * Note that there is a Kana character 'I' in romaji on the end of the labels, not an I.

    ** Note this device was plastic and not ceramic, but it was dumped as a Harris 7641 as
    it is logical that its compatible with the 7641. I can see the other devices all have
    similar/same Harris markings in the bottom left of the IC obscured by the labels.

    *** Note that the label for the 7643 PROM at IC25 was almost scraped off, but by its position
    in the sequence, it has to be 6H. I removed a little more of the label in order to
    work out what the inking was on it below, turned out to be 'D-2'. the prom at IC22 also
    looks like it has an inked number under the paper label, just peeking through on one side.
    Without removing the paper labels entirely, these markings wont be fully known, but were
    covered for some reason.

    SPECS:

    - CPU is an NEC D8085A
    - Crystal is marked 6.1440, but this looks to have been replaced.
    - X1/X2 clock frequency measured at pins 1 + 2 is 6.14330 mhz
    - Test point with stable readings is :
    - TP4 (HS) = 15.9982 khz (Horizontal sync)
    - TP5 (VS) = 60.5992 hz  (Vertical Sync)

***************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"
#include "emupal.h"
#include "speaker.h"
#include "screen.h"
#include "tilemap.h"

#define LOG_AUDIO (1U << 1)

#define VERBOSE (LOG_AUDIO)
#include "logmacro.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mmagic_state : public driver_device
{
public:
	mmagic_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_in(*this, "IN%u", 0),
		m_analog(*this, "ANALOG%u", 0),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tilemap(*this, "tilemap"),
		m_samples(*this,"samples"),
		m_tile_colors(*this, "colors"),
		m_ball_x(0x00),
		m_ball_y(0x00),
		m_tile_color_base(0x00),
		m_audio(0x00)
	{ }

	void mmagic(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_ioport_array<2> m_in;
	required_ioport_array<2> m_analog;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tilemap_device> m_tilemap;
	required_device<samples_device> m_samples;
	required_region_ptr<uint8_t> m_tile_colors;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t paddle_r();
	uint8_t buttons_r();

	uint8_t vblank_r();
	void ball_x_w(uint8_t data);
	void ball_y_w(uint8_t data);
	void color_w(uint8_t data);
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);
	void palette_init(palette_device& palette) const;
	TILE_GET_INFO_MEMBER(tile_info);

	void audio_w(uint8_t data);

	uint8_t m_ball_x;
	uint8_t m_ball_y;
	uint8_t m_tile_color_base;
	uint8_t m_audio;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void mmagic_state::mem_map(address_map& map)
{
	map.unmap_value_high();
	map(0x0000, 0x17ff).rom();
	map(0x2000, 0x21ff).ram();
	map(0x3000, 0x31ff).ram().w(m_tilemap, FUNC(tilemap_device::write8)).share("tilemap");
	map(0x8002, 0x8002).w(FUNC(mmagic_state::ball_x_w));
	map(0x8003, 0x8003).w(FUNC(mmagic_state::ball_y_w));
	map(0x8004, 0x8004).r(FUNC(mmagic_state::vblank_r));
}

void mmagic_state::io_map(address_map& map)
{
	map.global_mask(0xff);
	map(0x80, 0x80).w(FUNC(mmagic_state::color_w));
	map(0x81, 0x81).w(FUNC(mmagic_state::audio_w));
	map(0x85, 0x85).r(FUNC(mmagic_state::paddle_r));
	map(0x86, 0x86).r(FUNC(mmagic_state::buttons_r));
	map(0x87, 0x87).portr("DIPSW");
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( mmagic )
	PORT_START("DIPSW")
	PORT_SERVICE_DIPLOC(0x01, IP_ACTIVE_LOW, "DSW:1")
	PORT_DIPNAME(0x06, 0x06, DEF_STR(Bonus_Life)) PORT_DIPLOCATION ("DSW:2,3")
	PORT_DIPSETTING(0x00, "30000")
	PORT_DIPSETTING(0x02, "20000")
	PORT_DIPSETTING(0x04, "15000")
	PORT_DIPSETTING(0x06, "10000")
	PORT_DIPNAME(0x18, 0x18, DEF_STR(Lives)) PORT_DIPLOCATION ("DSW:4,5")
	PORT_DIPSETTING(0x00, "6")
	PORT_DIPSETTING(0x08, "5")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x18, "3")
	PORT_DIPUNUSED_DIPLOC(0x20, IP_ACTIVE_LOW, "DSW:6" )
	PORT_DIPUNUSED_DIPLOC(0x40, IP_ACTIVE_LOW, "DSW:7" )
	PORT_DIPUNUSED_DIPLOC(0x80, IP_ACTIVE_LOW, "DSW:8" )

	PORT_START("IN0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_START1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_START2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Debug?") // checked once at startup

	PORT_START("IN1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)

	PORT_START("ANALOG0")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_CENTERDELTA(0)

	PORT_START("ANALOG1")
	PORT_BIT(0xff, 0x80, IPT_PADDLE) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_CENTERDELTA(0) PORT_PLAYER(2)
INPUT_PORTS_END

uint8_t mmagic_state::paddle_r()
{
	return flip_screen() ? m_analog[0]->read() : m_analog[1]->read();
}

uint8_t mmagic_state::buttons_r()
{
	return flip_screen() ? m_in[0]->read() : m_in[1]->read() | (m_in[0]->read() & ~1);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

uint8_t mmagic_state::vblank_r()
{
	// bit 0 = vblank, other bits unused
	return 0xfe | m_screen->vblank();
}

void mmagic_state::ball_x_w(uint8_t data)
{
	m_ball_x = data;
}

void mmagic_state::ball_y_w(uint8_t data)
{
	m_ball_y = data;
}

void mmagic_state::color_w(uint8_t data)
{
	// bit 3 is flip screen
	flip_screen_set(BIT(data, 3));

	// bit 6 switches the palette (actually there is only a single differently colored tile)
	int tile_color_base = BIT(data, 6) << 7;
	if (m_tile_color_base != tile_color_base)
	{
		m_tilemap->mark_all_dirty();
		m_tile_color_base = tile_color_base;
	}

	// other bits are always 0
}

uint32_t mmagic_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	// draw ball (if not disabled)
	if (m_ball_x != 0xff)
	{
		static const int BALL_SIZE = 4;
		int ball_y = (m_ball_y >> 4) * 12 + (m_ball_y & 0x0f);
		bitmap.plot_box(flip_screen() ? 255 - m_ball_x : m_ball_x, flip_screen() ? 191 - ball_y : ball_y, BALL_SIZE, BALL_SIZE, rgb_t::white());
	}

	return 0;
}

void mmagic_state::palette_init(palette_device& palette) const
{
	for (int i = 0; i < 16; i++)
	{
		int const r = (!BIT(i, 0) && BIT(i, 1)) ? 0xff : 0x00;
		int const g = (!BIT(i, 0) && BIT(i, 2)) ? 0xff : 0x00;
		int const b = (!BIT(i, 0) && BIT(i, 3)) ? 0xff : 0x00;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

static constexpr gfx_layout tile_layout =
{
	8, 12, // 8x12 tiles
	128,   // 128 tiles
	1,     // 1 bits per pixel
	{ 0 }, // no bitplanes
	// x offsets
	{ 3, 2, 1, 0, 7, 6, 5, 4 },
	// y offsets
	{ 0 * 8, 1 * 8, 2 * 8, 3 * 8, 4 * 8, 5 * 8, 6 * 8, 7 * 8, 8 * 8, 9 * 8, 10 * 8, 11 * 8 },
	16 * 8 // every tile takes 16 bytes
};

static constexpr GFXDECODE_START(gfxdecode)
	GFXDECODE_ENTRY("tiles", 0x0000, tile_layout, 0, 8)
GFXDECODE_END

TILE_GET_INFO_MEMBER(mmagic_state::tile_info)
{
	uint32_t code = m_tilemap->basemem_read(tile_index) & 0x7f;
	uint32_t color = m_tile_colors[code | m_tile_color_base];

	tileinfo.set(0, code, color, 0);
}


//**************************************************************************
//  AUDIO EMULATION
//**************************************************************************

static constexpr const char* const sample_names[] =
{
	"*mmagic",
	"4",
	"3",
	"5",
	"2",
	"2-2",
	"6",
	"6-2",
	"1",
	nullptr
};

void mmagic_state::audio_w(uint8_t data)
{
	LOGMASKED(LOG_AUDIO, "audio_w: %02x\n", data);

	data ^= 0xff;
	if (data != m_audio)
	{
		if (BIT(data, 7))
			m_samples->start(0, m_audio & 7);

		m_audio = data;
	}
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

void mmagic_state::machine_start()
{
	save_item(NAME(m_ball_x));
	save_item(NAME(m_ball_y));
	save_item(NAME(m_tile_color_base));
	save_item(NAME(m_audio));
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void mmagic_state::mmagic(machine_config& config)
{
	// basic machine hardware
	I8085A(config, m_maincpu, 6.144_MHz_XTAL); // NEC D8085A
	m_maincpu->set_addrmap(AS_PROGRAM, &mmagic_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mmagic_state::io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(6.144_MHz_XTAL, 384, 0, 256, 264, 0, 192);
	m_screen->set_screen_update(FUNC(mmagic_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfxdecode);

	PALETTE(config, m_palette, FUNC(mmagic_state::palette_init), 16);

	TILEMAP(config, m_tilemap, m_gfxdecode, 1, 8, 12, TILEMAP_SCAN_ROWS, 32, 16).set_info_callback(FUNC(mmagic_state::tile_info));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SAMPLES(config, m_samples);
	m_samples->set_channels(1);
	m_samples->set_samples_names(sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.5);
	// TODO: replace samples with SN76477 + discrete sound
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( mmagic )
	ROM_REGION(0x1800, "maincpu", 0)
	ROM_LOAD("1ai.2a",  0x0000, 0x0400, CRC(ec772e2e) SHA1(7efc1bbb24b2ed73c518aea1c4ef4b9a93034e31))
	ROM_LOAD("2ai.3a",  0x0400, 0x0400, CRC(e5d482ca) SHA1(208b808e9208bb6f5f5f89ffbeb5a885be33733a))
	ROM_LOAD("3ai.4a",  0x0800, 0x0400, CRC(e8d38deb) SHA1(d7384234fb47e4b1d0421f58571fa748662b05f5))
	ROM_LOAD("4ai.45a", 0x0c00, 0x0400, CRC(3048bd6c) SHA1(740051589f6ba44b2ee68edf76a3177bb973d78e))
	ROM_LOAD("5ai.5a",  0x1000, 0x0400, CRC(2cab8f04) SHA1(203a3c005f18f968cd14c972bbb9fd7e0fc3b670))
	// location 6a is unpopulated, if the "debug" switch is activated on bootup it would jump here
	ROM_FILL(0x1400, 0x0400, 0xff)

	ROM_REGION(0x800, "tiles", 0)
	ROM_LOAD("6h.6hi", 0x000, 0x200, CRC(b6321b6f) SHA1(06611f7419d2982e006a3e81b79677e59e194f38))
	ROM_LOAD("7h.7hi", 0x200, 0x200, CRC(9ec0e82c) SHA1(29983f690a1b6134bb1983921f42c14898788095))
	ROM_LOAD("6j.6jk", 0x400, 0x200, CRC(7ce83302) SHA1(1870610ff07ab11622e183e04e3fce29328ff291))
	ROM_FILL(0x600, 0x200, 0xff)

	ROM_REGION(0x200, "colors", ROMREGION_INVERT)
	ROM_LOAD("7j.7jk", 0x000, 0x200, CRC(b7eb8e1c) SHA1(b65a8efb88668dcf1c1d00e31a9b15a67c2972c8))
ROM_END


} // anonymous namespace


//**************************************************************************
//  GAME DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  MACHINE INPUT   CLASS         INIT        ROT     COMPANY     FULLNAME        FLAGS
GAME( 1979, mmagic, 0,      mmagic, mmagic, mmagic_state, empty_init, ROT90, "Nintendo", "Monkey Magic", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
