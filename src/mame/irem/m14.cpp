// license: BSD-3-Clause
// copyright-holders: Angelo Salese
/**************************************************************************************************

M14 Hardware (c) 1979 Irem
アイレム『PTリーチ麻雀』

TODO:
- Actual discrete sound emulation;
- Colors might be not 100% accurate (needs video references);
- Complete dipswitches;
- Complete outputs;
- To start a game needs holding start for a bit too long compared to standard (verify);

Notes:
- Unlike most Arcade games, if you call a ron but you don't have a legit hand you'll automatically
  lose the match. This is commonly named chombo in rii'chi mahjong rules;
- After getting a completed hand, press start 1 + ron + discard at the same time to go back
  into attract mode (!);

===================================================================================================
x (Mystery Rom)
(c)1978-1981? Irem?
PCB No. :M14S-2
    :M14L-2
CPU :NEC D8085AC
Sound   :?
OSC :6MHz x2

mgpb1.bin
mgpa2.bin
mgpa3.bin
mgpa4.bin
mgpa5.bin
mgpb6.bin
mgpa7.bin
mgpb8.bin

mgpa9.bin
mgpa10.bin


--- Team Japump!!! ---
Dumped by Chackn
01/30/2000

**************************************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "sound/samples.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class m14_state : public driver_device
{
public:
	m14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_video_ram(*this, "video_ram")
		, m_color_ram(*this, "color_ram")
		, m_samples(*this, "samples")
	{ }

	void m14(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(left_coin_inserted);
	DECLARE_INPUT_CHANGED_MEMBER(right_coin_inserted);
	ioport_value rng_r();

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_video_ram;
	required_shared_ptr<uint8_t> m_color_ram;
	required_device<samples_device> m_samples;

	void vram_w(offs_t offset, uint8_t data);
	void cram_w(offs_t offset, uint8_t data);
	void output_w(uint8_t data);
	void ball_x_w(uint8_t data);
	void ball_y_w(uint8_t data);
	void paddle_x_w(uint8_t data);
	void sound_w(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	INTERRUPT_GEN_MEMBER(vblank_irq);

	void io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	virtual void video_start() override ATTR_COLD;
	tilemap_t *m_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(m14_get_tile_info);
	void palette_init(palette_device &palette) const;
	void draw_ball_and_paddle(bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	uint8_t m_ballx = 0, m_bally = 0;
	uint8_t m_paddlex = 0;
};


/*************************************
 *
 *  Video Hardware
 *
 *************************************/

void m14_state::palette_init(palette_device &palette) const
{
	// TODO: background color is unconfirmed
	const rgb_t green_pen = rgb_t(pal1bit(0), pal3bit(3), pal1bit(0));

	for (int i = 0; i < 0x20; i++)
	{
		rgb_t color;
		if (i & 0x01)
			color = rgb_t(pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 3));
		else
			color = (i & 0x10) ? rgb_t::white() : green_pen;

		palette.set_pen_color(i, color);
	}
}

TILE_GET_INFO_MEMBER(m14_state::m14_get_tile_info)
{
	u8 code = m_video_ram[tile_index];
	// TODO: colorram & 0xf0 used on tiles (background pen select?)
	u8 color = m_color_ram[tile_index] & 0x0f;

	tileinfo.set(0, code, color, 0);
}

void m14_state::video_start()
{
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(m14_state::m14_get_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

void m14_state::draw_ball_and_paddle(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//const rgb_t white_pen =  rgb_t::white();
	const u8 white_pen = 0x1f;
	const int xoffs = -8; // matches left-right wall bounces
	const int p_ybase = 184; // matches ball bounce to paddle

	rectangle ball_clip;
	ball_clip.min_x = flip_screen() ? 32 * 8 - (m_ballx + xoffs) : m_ballx + xoffs;
	ball_clip.max_x = ball_clip.min_x + 4;
	ball_clip.min_y = flip_screen() ? 28 * 8 - (m_bally) : m_bally;
	ball_clip.max_y = ball_clip.min_y + 4;
	ball_clip &= cliprect;
	bitmap.fill(white_pen, ball_clip);

	rectangle paddle_clip;
	paddle_clip.min_x = flip_screen() ? 32 * 8 - (m_paddlex + xoffs) : m_paddlex + xoffs;
	paddle_clip.max_x = paddle_clip.min_x + 12;
	paddle_clip.min_y = flip_screen() ? 28 * 8 - (p_ybase) : p_ybase;
	paddle_clip.max_y = paddle_clip.min_y + 4;
	paddle_clip &= cliprect;
	bitmap.fill(white_pen, paddle_clip);
}


uint32_t m14_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	draw_ball_and_paddle(bitmap,cliprect);

	return 0;
}


void m14_state::vram_w(offs_t offset, uint8_t data)
{
	m_video_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

void m14_state::cram_w(offs_t offset, uint8_t data)
{
	m_color_ram[offset] = data;
	m_tilemap->mark_tile_dirty(offset);
}

/*************************************
 *
 *  I/O
 *
 *************************************/

void m14_state::output_w(uint8_t data)
{
	/* ---- x--- active after calling a winning hand */
	/* ---- --x- lamp? */
	/* ---- ---x flip screen */
	//m_hop_mux = data & 2;
	flip_screen_set(data & 1);
	//popmessage("%02x",data);
}

void m14_state::ball_x_w(uint8_t data)
{
	m_ballx = data;
}

void m14_state::ball_y_w(uint8_t data)
{
	m_bally = data;
}

void m14_state::paddle_x_w(uint8_t data)
{
	m_paddlex = data;
}

/*************************************
 *
 *  Sound section
 *
 *************************************/

static const char *const m14_sample_names[] =
{
	"*ptrmj",
	"wall_hit", // 1
	"tile_hit", // 2
	"tick",     // 0x40
	"ball_drop", // 8
	"paddle_hit",
	nullptr
};

void m14_state::sound_w(uint8_t data)
{
	switch(data)
	{
		case 1: // wall hit
			m_samples->start(0,0);
			break;
		case 2: // tile hit
			m_samples->start(1,1);
			break;
		case 8: // ball drop
			m_samples->start(3,3);
			break;
		case 0x40: // tick
			m_samples->start(2,2);
			break;
		case 0x80:
		case 0: // flips with previous, unknown purpose
			break;
		default:
			logerror("%s: Sound start with %02x param\n",m_samples->tag(),data);
			break;
	}
}


/*************************************
 *
 *  Memory Map
 *
 *************************************/

void m14_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0xe000, 0xe3ff).ram().w(FUNC(m14_state::vram_w)).share("video_ram");
	map(0xe400, 0xe7ff).ram().w(FUNC(m14_state::cram_w)).share("color_ram");
}

void m14_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xf8, 0xf8).portr("AN_PADDLE").w(FUNC(m14_state::ball_x_w));
	map(0xf9, 0xf9).portr("IN0").w(FUNC(m14_state::ball_y_w));
	map(0xfa, 0xfa).portr("IN1").w(FUNC(m14_state::paddle_x_w));
	map(0xfb, 0xfb).portr("DSW").w(FUNC(m14_state::output_w));
	map(0xfc, 0xfc).w(FUNC(m14_state::sound_w));
}

/*************************************
 *
 *  Input Port Definitions
 *
 *************************************/

// Coin A insertion causes a rst6.5 (vector 0x34)
INPUT_CHANGED_MEMBER(m14_state::left_coin_inserted)
{
	if (newval)
		m_maincpu->set_input_line(I8085_RST65_LINE, HOLD_LINE);
}

// Coin B insertion causes a rst5.5 (vector 0x2c)
INPUT_CHANGED_MEMBER(m14_state::right_coin_inserted)
{
	if (newval)
		m_maincpu->set_input_line(I8085_RST55_LINE, HOLD_LINE);
}

/*
 * graphic artifacts happens if this doesn't return random values.
 * guess directly tied to screen frame number
 */
ioport_value m14_state::rng_r()
{
	return m_screen->frame_number() & 0x7f;
}

static INPUT_PORTS_START( m14 )
	PORT_START("AN_PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Fire / Discard")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Ron")
	// Coinage displays as "yen amount" | "number of tokens" | "number of turns"
	// cfr. flyer
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x04, u8"Coin A 5\u30de\u30a4 Coin B 1\u30de\u30a4" )
	PORT_DIPSETTING(    0x00, u8"Coin A & B 1\u30de\u30a4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(m14_state, rng_r)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Reach")

	PORT_START("DSW") // this whole port is stored at work ram $2112.
	PORT_DIPNAME( 0x01, 0x01, "Show available tiles" ) // difficulty even
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("FAKE")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, m14_state,left_coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(1) PORT_CHANGED_MEMBER(DEVICE_SELF, m14_state,right_coin_inserted, 0)
INPUT_PORTS_END

static GFXDECODE_START( gfx_m14 )
	GFXDECODE_ENTRY( "gfx1", 0, gfx_8x8x1,     0, 0x10 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(m14_state::vblank_irq)
{
	device.execute().set_input_line(I8085_RST75_LINE, ASSERT_LINE);
	device.execute().set_input_line(I8085_RST75_LINE, CLEAR_LINE);
}

void m14_state::machine_start()
{
	//save_item(NAME(m_hop_mux));
	save_item(NAME(m_ballx));
	save_item(NAME(m_bally));
	save_item(NAME(m_paddlex));
}

void m14_state::machine_reset()
{
	//m_hop_mux = 0;
}


void m14_state::m14(machine_config &config)
{
	I8085A(config, m_maincpu, 6000000); //guess: 6 Mhz internally divided by 2
	m_maincpu->set_addrmap(AS_PROGRAM, &m14_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &m14_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(m14_state::vblank_irq));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); //not accurate
	m_screen->set_size(32*8, 32*8);
	m_screen->set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	m_screen->set_screen_update(FUNC(m14_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_m14);
	PALETTE(config, m_palette, FUNC(m14_state::palette_init), 0x20);

	SPEAKER(config, "mono").front_center();
	SAMPLES(config, m_samples);
	m_samples->set_channels(5);
	m_samples->set_samples_names(m14_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.6);

//  DISCRETE(config, m_discrete);
//  m_discrete->set_intf(m14);
//  m_discrete->add_route(ALL_OUTPUTS, "mono", 1.0);
}

/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( ptrmj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "mgpb1.bin",   0x0000, 0x0400, CRC(47c041b8) SHA1(e834c375e689f99a13964863fc9847a8e148ec91) )
	ROM_LOAD( "mgpa2.bin",   0x0400, 0x0400, CRC(cf8bfa23) SHA1(091055e803255f1b5520f50b31af7135d71d0a40) )
	ROM_LOAD( "mgpa3.bin",   0x0800, 0x0400, CRC(a07a3093) SHA1(5b86bb11e83c06f828956e7db6dd2c105b023b03) )
	ROM_LOAD( "mgpa4.bin",   0x0c00, 0x0400, CRC(a420241c) SHA1(7497d90014dabb49f9db1d5d8e3014c634045725) )
	ROM_LOAD( "mgpa5.bin",   0x1000, 0x0400, CRC(a2df92a3) SHA1(97a3d4b188d26f172881f8cf86bdd83d549f5b74) )
	ROM_LOAD( "mgpb6.bin",   0x1400, 0x0400, CRC(f5c0fcd4) SHA1(14e2d04be105caeb221dfc226f84cb1722ae2627) )
	ROM_LOAD( "mgpa7.bin",   0x1800, 0x0400, CRC(56ed7eb2) SHA1(acf954bf4daadb225475937dd3c36baa996f7b65) )
	ROM_LOAD( "mgpb8.bin",   0x1c00, 0x0400, CRC(3ecb8214) SHA1(8575bcb49f693aa8798b8a7d9a76392bfcc90e0e) )

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "mgpa9.bin",   0x0000, 0x0400, CRC(cb68b4ec) SHA1(2cf596affb155ae38729fcd95cae424073faf74d) )
	ROM_LOAD( "mgpa10.bin",  0x0400, 0x0400, CRC(e1a4ebdc) SHA1(d9df42424ede17f0634d8d0a56c0374a33c55333) )
ROM_END

} // anonymous namespace


GAME( 1979, ptrmj, 0, m14, m14, m14_state, empty_init, ROT0, "Irem", "PT Reach Mahjong (Japan)", MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // was already Irem according to the official flyer
