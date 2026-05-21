// license:BSD-3-Clause
// copyright-holders: Tomasz Slanina

/*
 'Swinging Singles' US distribution by Ent. Ent. Ltd
 Original Japan release is 'Utamaro' by 'Yachiyo' (undumped!)
 driver by Tomasz Slanina

 Crap XXX game.
 Three ROMs contain text "BY YACHIYO"

 Upper half of 7.bin = upper half of 8.bin = intentional or bad dump ?

 TODO:
 - colors (missing PROM(s) ?)
 - samples (at least two of unused ROMs contains samples (unkn. format, ADPCM ?)
 - dips (one is tested in game (difficulty related?), another 2 are tested at start)

 Unknown reads/writes:
 - AY i/o ports (writes)
 - mem $c000, $c001 = protection device ? if tests fails, game crashes
   (problems with stack - skipped code with "pop af")
 - i/o port $8 = data read used for $e command arg for one of AY chips
   (volume? - could be a sample player (based on volume changes?)

*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

static constexpr uint8_t NUM_PENS = 4 * 8;

class ssingles_state : public driver_device
{
public:
	ssingles_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_videoram(*this, "videoram")
		, m_colorram(*this, "colorram")
		, m_gfx_rom(*this, "gfx")
		, m_extra(*this, "EXTRA")
	{ }

	void ssingles(machine_config &config);

	ioport_value controls_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_region_ptr<uint8_t> m_gfx_rom;

	required_ioport m_extra;

	pen_t m_pens[NUM_PENS];
	uint8_t m_prot_data = 0;

	uint8_t c000_r();
	uint8_t c001_r();
	void c001_w(uint8_t data);

	void palette(palette_device &palette) const;

	MC6845_UPDATE_ROW(ssingles_update_row);

	void ssingles_io_map(address_map &map) ATTR_COLD;
	void ssingles_map(address_map &map) ATTR_COLD;
};

void ssingles_state::machine_start()
{
	save_item(NAME(m_prot_data));
}


// fake palette
static constexpr rgb_t ssingles_colors[NUM_PENS] =
{
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0x00,0x00 }, { 0xaa,0x00,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0xaa,0x00 }, { 0xaa,0x55,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0xff }, { 0xff,0x00,0x00 }, { 0xff,0xaa,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xaa,0x00 }, { 0xff,0x00,0x00 }, { 0xff,0x55,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0xff,0x00 }, { 0xff,0xaa,0x00 }, { 0xaa,0x55,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0x00 }, { 0x00,0xff,0x00 }, { 0xff,0xff,0x00 },
	{ 0x00,0x00,0x00 }, { 0x00,0x00,0xff }, { 0xaa,0xff,0x00 }, { 0xff,0xaa,0x00 },
	{ 0x00,0x00,0x00 }, { 0xff,0x00,0xff }, { 0xaa,0x00,0xaa }, { 0x55,0x00,0x55 }
};

void ssingles_state::palette(palette_device &palette) const
{
	for (int i = 0; i < NUM_PENS; ++i)
		palette.set_pen_color(i, ssingles_colors[i]);
}

void ssingles_state::video_start()
{
	for (int i = 0; i < NUM_PENS; ++i)
		m_pens[i] = ssingles_colors[i];
}

MC6845_UPDATE_ROW(ssingles_state::ssingles_update_row)
{
	for (int cx = 0; cx < x_count; ++cx)
	{
		int const address = ((ma >> 1) + (cx >> 1)) & 0xff;

		uint16_t const cell = m_videoram[address] + (m_colorram[address] << 8);

		uint32_t const tile_address = ((cell & 0x3ff) << 4) + ra;
		uint16_t const palette = (cell >> 10) & 0x1c;

		uint16_t const cxo = (cx & 1) ? 0x4000 : 0;
		uint8_t b0 = m_gfx_rom[tile_address + 0x0000 + cxo];
		uint8_t b1 = m_gfx_rom[tile_address + 0x8000 + cxo];

		for (int x = 7; x >= 0; --x)
		{
			bitmap.pix(y, (cx << 3) | x) = m_pens[palette + ((b0 & 1) | ((b1 & 1) << 1))];
			b0 >>= 1;
			b1 >>= 1;
		}
	}
}

uint8_t ssingles_state::c000_r()
{
	return m_prot_data;
}

uint8_t ssingles_state::c001_r()
{
	if (!machine().side_effects_disabled())
		m_prot_data = 0xc4;
	return 0;
}

void ssingles_state::c001_w(uint8_t data)
{
	m_prot_data ^= data ^ 0x11;
}

ioport_value ssingles_state::controls_r()
{
	// multiplexed
	return count_leading_zeros_32(m_extra->read() & 0x7f) - 25;
}

void ssingles_state::ssingles_map(address_map &map)
{
	map(0x0000, 0x00ff).writeonly().share(m_videoram);
	map(0x0800, 0x08ff).writeonly().share(m_colorram);
	map(0x0000, 0x1fff).rom();
	map(0xc000, 0xc000).r(FUNC(ssingles_state::c000_r));
	map(0xc001, 0xc001).rw(FUNC(ssingles_state::c001_r), FUNC(ssingles_state::c001_w));
	map(0x6000, 0xbfff).rom();
	map(0xf800, 0xffff).ram();
}

void ssingles_state::ssingles_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("ay1", FUNC(ay8910_device::address_w));
	map(0x04, 0x04).w("ay1", FUNC(ay8910_device::data_w));
	map(0x06, 0x06).w("ay2", FUNC(ay8910_device::address_w));
	map(0x08, 0x08).nopr();
	map(0x0a, 0x0a).w("ay2", FUNC(ay8910_device::data_w));
	map(0x16, 0x16).portr("DSW0");
	map(0x18, 0x18).portr("DSW1");
	map(0x1c, 0x1c).portr("INPUTS");
//  map(0x1a, 0x1a).nopw(); // flip screen
	map(0xfe, 0xfe).w("crtc", FUNC(mc6845_device::address_w));
	map(0xff, 0xff).w("crtc", FUNC(mc6845_device::register_w));
}


static INPUT_PORTS_START( ssingles )
	PORT_START("INPUTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // must be LOW
	PORT_BIT( 0x1c, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(ssingles_state::controls_r))
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON1 )

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x04, 0x00, "Unk1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unk2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unk3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Cocktail ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x00, "Unk6" )
	PORT_DIPSETTING(    0x01, "Pos 1" )
	PORT_DIPSETTING(    0x03, "Pos 2" )
	PORT_DIPSETTING(    0x00, "Pos 3" )
	PORT_DIPSETTING(    0x02, "Pos 4" )
	PORT_DIPNAME( 0x04, 0x00, "Unk7" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Unk8" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Unk9" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "UnkA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "UnkB" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "UnkC" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static GFXDECODE_START( gfx_ssingles )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x2_planar, 0, 8 )
GFXDECODE_END

void ssingles_state::ssingles(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 14_MHz_XTAL / 4); // 3.5 MHz?
	m_maincpu->set_addrmap(AS_PROGRAM, &ssingles_state::ssingles_map);
	m_maincpu->set_addrmap(AS_IO, &ssingles_state::ssingles_io_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(14_MHz_XTAL / 2, 424, 0, 288, 256, 0, 224); // from CRTC
	m_screen->set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, "palette", FUNC(ssingles_state::palette), NUM_PENS);

	GFXDECODE(config, "gfxdecode", "palette", gfx_ssingles);

	mc6845_device &crtc(MC6845(config, "crtc", 14_MHz_XTAL / 16));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(ssingles_state::ssingles_update_row));
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay1", 14_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5);
	AY8910(config, "ay2", 14_MHz_XTAL / 8).add_route(ALL_OUTPUTS, "mono", 0.5);
}

ROM_START( ssingles )
	ROM_REGION( 0x10000, "maincpu", 0 ) // Z80
	ROM_LOAD( "1.bin",  0x0000, 0x2000, CRC(43f02215) SHA1(9f04a7d4671ff39fd2bd8ec7afced4981ee7be05) )
	ROM_LOAD( "2.bin",  0x6000, 0x2000, CRC(281f27e4) SHA1(cef28717ab2ed991a5709464c01490f0ab1dc17c) )
	ROM_LOAD( "3.bin",  0x8000, 0x2000, CRC(14fdcb65) SHA1(70f7fcb46e74937de0e4037c9fe79349a30d0d07) )
	ROM_LOAD( "4.bin",  0xa000, 0x2000, CRC(acb44685) SHA1(d68aab8b7e68d842a350d3fb76985ac857b1d972) )

	ROM_REGION( 0x10000, "gfx", 0 )
	ROM_LOAD( "12.bin", 0x0000, 0x4000, CRC(e5585a93) SHA1(04d55699b56d869066f2be2c6ac48042aa6c3108) )
	ROM_LOAD( "11.bin", 0x4000, 0x4000, CRC(f7107b29) SHA1(a405926fd3cb4b3d2a1c705dcde25d961dba5884) )
	ROM_LOAD( "10.bin", 0x8000, 0x4000, CRC(cd3ba260) SHA1(2499ad9982cc6356e2eb3a0f10d77886872a0c9f) )
	ROM_LOAD( "9.bin",  0xc000, 0x4000, CRC(57fac6f9) SHA1(12f6695c9831399e599a95008ebf9db943725437) )

	ROM_REGION( 0x08000, "user1", 0) // samples ? data ?
	ROM_LOAD( "5.bin",  0x0000, 0x2000, CRC(242a8dda) SHA1(e140893cc05fb8cee75904d98b02626f2565ed1b) )
	ROM_LOAD( "6.bin",  0x2000, 0x2000, CRC(85ab8aab) SHA1(566f034e1ba23382442f27457447133a0e0f1cfc) )
	ROM_LOAD( "7.bin",  0x4000, 0x2000, CRC(57cc112d) SHA1(fc861c58ae39503497f04d302a9f16fca19b37fb) )
	ROM_LOAD( "8.bin",  0x6000, 0x2000, CRC(52de717a) SHA1(e60399355165fb46fac862fb7fcdff16ff351631) )
ROM_END

} // anonymous namespace


GAME( 1983, ssingles, 0, ssingles, ssingles, ssingles_state, empty_init, ROT90, "Yachiyo Denki (Entertainment Enterprises, Ltd. license)", "Swinging Singles (US)", MACHINE_SUPPORTS_SAVE | MACHINE_WRONG_COLORS | MACHINE_IMPERFECT_SOUND | MACHINE_NO_COCKTAIL )
