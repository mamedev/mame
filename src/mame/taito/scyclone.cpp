// license:BSD-3-Clause
// copyright-holders:David Haywood
/*
    Space Cyclone

    3 board stack? - the PCB pictures make it very difficult to figure much out

    seems a bit like 8080bw hardware but with extra sprites and a Z80 cpu?
    maybe an evolution of Polaris etc.?


    there's an MB14241 near the Sprite ROM

    1x 8DSW, 1x 4DSW

    there are 11 HM4716AP RAM chips near the main CPU (1 is unmarked, could be different?)
    HM4716AP is 16384 * 1-bit (2048 [0x800] bytes)

    1bpp of video needs 0x3800 bytes

    1bpp video would need 4 of these (with 0x400 bytes leftover)
    2bpp video would need 7 of these
    3bpp video would need 11 of these (with 0x400 bytes leftover)
    mainram is 0x400 bytes (so the leftover in above calc?)


    Notes:

    (to check on hardware)
    The stars scroll backwards when in cocktail mode, the direction changes when the screen
    is flipped but is still reversed compared to upright.  We would have no way of knowing
    if we're in cocktail mode without checking the dipswitch, so I think this is a game bug.

    Furthermore, the game seems to set what I believe to be the 'flipscreen' bit for player 2
    even when in 'upright' mode, so it's possible this romset was only really made for a
    cocktail table?

    The speech chip is unknown, it seems like it's some sort of 1-bit delta modulation.
    Probably HC55516 or something similar.

    SN76477 parameters are unknown, so some sound effects are missing.
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/gen_latch.h"
#include "machine/mb14241.h"
#include "sound/dac.h"
#include "sound/hc55516.h"
#include "sound/sn76477.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class scyclone_state : public driver_device
{
public:
	scyclone_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_stars(*this, "stars"),
		m_gfx1pal(*this, "gfx1pal"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_snsnd(*this, "snsnd%u", 0),
		m_cvsd(*this, "cvsd"),
		m_cvsd_clock(*this, "cvsd_clock"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void scyclone(machine_config &config);

	int collision_r();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_region_ptr<uint8_t> m_stars;
	required_region_ptr<uint8_t> m_gfx1pal;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device_array<sn76477_device, 2> m_snsnd;
	required_device<hc55516_device> m_cvsd;
	required_device<clock_device> m_cvsd_clock;
	required_device<generic_latch_8_device> m_soundlatch;

	void scyclone_iomap(address_map &map) ATTR_COLD;
	void scyclone_map(address_map &map) ATTR_COLD;
	void scyclone_sub_iomap(address_map &map) ATTR_COLD;
	void scyclone_sub_map(address_map &map) ATTR_COLD;

	INTERRUPT_GEN_MEMBER(main_irq);

	// video-related
	std::unique_ptr<uint8_t[]> m_vram;

	uint8_t m_videowritemask = 0;
	uint8_t m_videowritemask2 = 0;

	uint8_t m_sprite_xpos = 0;
	uint8_t m_sprite_ypos = 0;
	uint8_t m_sprite_colour = 0;
	uint8_t m_sprite_tile = 0;
	uint8_t m_starscroll = 0;
	uint8_t m_p0e = 0;
	uint8_t m_vidctrl = 0;

	uint8_t m_hascollided = 0;

	const uint8_t get_sprite_pixel(int x, int y);
	const uint8_t get_bitmap_pixel(int x, int y);
	uint32_t draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t draw_bitmap_and_sprite(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint32_t screen_update_scyclone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vidctrl_w(uint8_t data);
	void sprite_xpos_w(uint8_t data);
	void sprite_ypos_w(uint8_t data);
	void sprite_colour_w(uint8_t data);
	void sprite_tile_w(uint8_t data);
	void starscroll_w(uint8_t data);
	void port0e_w(uint8_t data);
	void port06_w(uint8_t data);
	void videomask1_w(uint8_t data);
	void videomask2_w(uint8_t data);
	void vram_w(offs_t offset, uint8_t data);
	uint8_t vram_r(offs_t offset);

	// sound related
	uint8_t m_speech_data = 0;

	void snd_3001_w(uint8_t data);
	void snd_3002_w(uint8_t data);
	void snd_3003_w(uint8_t data);
	void snd_3004_w(uint8_t data);
	void snd_3005_w(uint8_t data);
	void cvsd_tick(int state);
};

void scyclone_state::machine_start()
{
	save_item(NAME(m_videowritemask));
	save_item(NAME(m_videowritemask2));
	save_item(NAME(m_sprite_xpos));
	save_item(NAME(m_sprite_ypos));
	save_item(NAME(m_sprite_colour));
	save_item(NAME(m_sprite_tile));
	save_item(NAME(m_starscroll));
	save_item(NAME(m_p0e));
	save_item(NAME(m_vidctrl));
	save_item(NAME(m_hascollided));
	save_item(NAME(m_speech_data));
}



/***************************************************************************

  Video hardware

***************************************************************************/

void scyclone_state::video_start()
{
	m_vram = std::make_unique<uint8_t []>(0x3800*3);
	save_pointer(&m_vram[0], "m_vram", 0x3800*3);

	for (int i = 0; i < 8; i++)
		m_palette->set_pen_color(i, rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2)));

	for (int i = 0; i < 16; i++)
	{
		uint8_t col = m_gfx1pal[i];
		m_palette->set_pen_color(i+8, rgb_t(pal1bit(col >> 2), pal1bit(col >> 1), pal1bit(col >> 0)));
	}
}

const uint8_t scyclone_state::get_sprite_pixel(int x, int y)
{
	int minx = 0xe0 - m_sprite_xpos;
	int miny = 0xe0 - m_sprite_ypos;
	int col = m_sprite_colour & 3;
	int code = m_sprite_tile & 7;

	int maxx = minx + 32;
	int maxy = miny + 32;

	// if we're not even inside the sprite, return nothing
	if (x < minx || x >= maxx)
		return 0;

	if (y < miny || y >= maxy)
		return 0;

	int sprx = x-minx;
	int spry = y-miny;

	const uint8_t* srcdata = m_gfxdecode->gfx(0)->get_data(code);

	uint8_t pix = srcdata[spry * 32 + sprx] + col * 4;

	return pix;
}


const uint8_t scyclone_state::get_bitmap_pixel(int x, int y)
{
	const uint8_t video_data0 = m_vram[(y * 32 + (x / 8)) + 0x3800 * 0];
	const uint8_t video_data1 = m_vram[(y * 32 + (x / 8)) + 0x3800 * 1];
	const uint8_t video_data2 = m_vram[(y * 32 + (x / 8)) + 0x3800 * 2];

	int const bit(x & 0x07);

	const uint8_t plane0(BIT(video_data0, bit));
	const uint8_t plane1(BIT(video_data1, bit));
	const uint8_t plane2(BIT(video_data2, bit));

	const uint8_t pal = plane0 | (plane1 << 1) | (plane2 << 2);

	return pal;
}

uint32_t scyclone_state::draw_starfield(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();

	for (int x = 0; x < 32; x++)
	{
		for (int y = 0; y < 32; y++)
		{
			const uint8_t sx = (x + (m_starscroll >> 3)) & 0x1f;
			const uint8_t star = m_stars[y << 5 | sx] & 0xf;
			const uint8_t color = (m_screen->frame_number() + (star << 2)) >> 3 & 7;

			uint8_t dx = x << 3 | (~m_starscroll & 7);
			uint8_t dy = (y << 3) - 32 + 7;

			bool noclipped = false;

			if (flip_screen())
			{
				dx = 255 - dx;
				dy = 255 - 32 - dy;

				noclipped = dx < 64*3;
			}
			else
				noclipped = dx >= 64;

			if (noclipped && star)
				bitmap.pix(dy, dx) = paldata[color];
		}
	}

	return 0;
}

uint32_t scyclone_state::draw_bitmap_and_sprite(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const pen_t *paldata = m_palette->pens();
	m_hascollided = 0;

	for (int y = 0; y < 256-32; y++)
	{
		for (int x = 0; x < 256/8; x++)
		{
			for (int i = 0; i < 8; i++)
			{
				int realx = 0;
				int realy = 0;

				if (flip_screen())
				{
					realx = 255 - ((x*8) + i);
					realy = 255 - 32 - y;
				}
				else
				{
					realx = (x*8)+i;
					realy = y;
				}

				uint8_t pal = get_bitmap_pixel(realx, realy);

				if (pal) bitmap.pix(y, (x*8) + i) = paldata[pal];

				uint8_t pal2 = get_sprite_pixel(realx, realy);

				if (pal2 & 0x3)
				{
					bitmap.pix(y, (x*8) + i) = paldata[8 + pal2];
					if (pal == 0x7) m_hascollided = 1;
				}
			}
		}
	}

	return 0;
}


uint32_t scyclone_state::screen_update_scyclone(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	draw_starfield(screen, bitmap, cliprect);
	draw_bitmap_and_sprite(screen, bitmap, cliprect);

	//popmessage("%02x %02x %02x %02x %02x %02x", m_sprite_xpos, m_sprite_ypos, m_sprite_colour, m_sprite_tile, m_starscroll, m_p0e);

	return 0;
}


// Memory handlers

uint8_t scyclone_state::vram_r(offs_t offset)
{
	uint8_t ret = 0;

	// not sure, collisions depend on readback
	ret |= m_vram[offset + 0x3800 * 0];
	ret |= m_vram[offset + 0x3800 * 1];
	ret |= m_vram[offset + 0x3800 * 2];

	return ret;
}


void scyclone_state::vram_w(offs_t offset, uint8_t data)
{
#if 0
	// halves are NOT equal in between level cutscenes
	// this is used when drawing the pointing stick, need closeup reference

	// 0x40 and 0x80 ports always seem equal tho
	if ((m_videowritemask & 0xf) != ((m_videowritemask >> 4) & 0xf))
	{
		logerror("m_videowritemask halves not equal %02x\n", m_videowritemask);
	}

	if ((m_videowritemask2 & 0xf) != ((m_videowritemask2 >> 4) & 0xf))
	{
		logerror("m_videowritemask2 halves not equal %02x\n", m_videowritemask2);
	}

	if ((m_videowritemask) != (m_videowritemask2))
	{
		logerror("m_videowritemask != m_videowritemask2 %02x %02x\n", m_videowritemask, m_videowritemask2);
	}
#endif

	for (int i = 0; i < 8; i++)
	{
		const uint8_t databit = data & (1 << i);
		const uint8_t nodatabit = (1 << i);

		uint8_t videowritemask = 0;

		if (i >= 6) videowritemask = (m_videowritemask >> 4) & 0x0f;
		else if (i >= 4) videowritemask = (m_videowritemask >> 0) & 0x0f;
		else if (i >= 2) videowritemask = (m_videowritemask2 >> 4) & 0x0f;
		else videowritemask = (m_videowritemask2 >> 0) & 0x0f;

		if (!(videowritemask & 1) && databit) m_vram[offset + 0x3800 * 0] |= databit;
		else m_vram[offset + 0x3800 * 0] &= ~nodatabit;

		if (!(videowritemask & 2) && databit) m_vram[offset + 0x3800 * 1] |= databit;
		else m_vram[offset + 0x3800 * 1] &= ~nodatabit;

		if (!(videowritemask & 4) && databit) m_vram[offset + 0x3800 * 2] |= databit;
		else m_vram[offset + 0x3800 * 2] &= ~nodatabit;
	}
}


void scyclone_state::vidctrl_w(uint8_t data)
{
	// ---- facu
	// f = flipscreen? (always set during player 2 turn, even in upright mode?!)
	// a = alternates during attract mode, enabled during gameplay
	// u = unknown but used (set to 1 during gameplay)
	// c = coinlock
	m_vidctrl = data;

	// breaks upright mode, so it's commented out
	//flip_screen_set(BIT(data, 3));

	if (data & 0xf0)
	{
		logerror("vidctrl_w %02x\n", data);
	}
}

void scyclone_state::sprite_xpos_w(uint8_t data)
{
	m_sprite_xpos = data;
}

void scyclone_state::sprite_ypos_w(uint8_t data)
{
	m_sprite_ypos = data;
}

void scyclone_state::sprite_colour_w(uint8_t data)
{
	m_sprite_colour = data;
}

void scyclone_state::sprite_tile_w(uint8_t data)
{
	m_sprite_tile = data;
}

void scyclone_state::starscroll_w(uint8_t data)
{
	m_starscroll = data;
}

void scyclone_state::port0e_w(uint8_t data)
{
	// could be related to basic sound effects?
	//logerror("port0e_w %02x\n",data);
	m_p0e = data;
}

void scyclone_state::port06_w(uint8_t data)
{
	// watchdog?
}

// these seem to select where the vram writes go
// 3 possible planes, probably one 4 bits per 2 pixels?
void scyclone_state::videomask1_w(uint8_t data)
{
	// format seems to be -xxx -xxx
	m_videowritemask = data;
}

void scyclone_state::videomask2_w(uint8_t data)
{
	// format seems to be -xxx -xxx
	m_videowritemask2 = data;
}


int scyclone_state::collision_r()
{
	// Appears to be when a white bitmap pixel (col 0x7) collides with a large sprite?
	// If you simply set it to 1 and shoot in the left corner, the game gets stuck,
	// but if you have it set to 0 there are no collisions with large objects.
	return m_hascollided;
}



/***************************************************************************

  Sound hardware

***************************************************************************/

void scyclone_state::snd_3001_w(uint8_t data)
{
	// need to clear the latch somewhere, the command value is written back here and at 3005
	// after acknowledging a command
	// might actually reset the DACs as there are (at least) 2 of them?
	m_soundlatch->clear_w();
}

void scyclone_state::snd_3002_w(uint8_t data)
{
	m_speech_data = data;
}

void scyclone_state::snd_3003_w(uint8_t data)
{
	m_cvsd->fzq_w(BIT(~data, 1));
}

void scyclone_state::snd_3004_w(uint8_t data)
{
	// always writes 00?
}

void scyclone_state::snd_3005_w(uint8_t data)
{
	// written at the same time, with the same data as 0x3001
}

void scyclone_state::cvsd_tick(int state)
{
	if (state)
	{
		m_cvsd->digin_w(BIT(m_speech_data, 7));
		m_speech_data <<= 1;
	}
}



/***************************************************************************

  Address maps

***************************************************************************/

void scyclone_state::scyclone_map(address_map &map)
{
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x43ff).ram();
	map(0x4400, 0x5fff).rw(FUNC(scyclone_state::vram_r), FUNC(scyclone_state::vram_w));
	map(0x6000, 0x60ff).noprw(); // this just seems to be overflow from the VRAM writes, probably goes nowhere
}


void scyclone_state::scyclone_iomap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("mb14241", FUNC(mb14241_device::shift_result_r)).w("mb14241", FUNC(mb14241_device::shift_count_w));
	map(0x01, 0x01).portr("IN0").w("mb14241", FUNC(mb14241_device::shift_data_w));
	map(0x02, 0x02).portr("IN1");
	map(0x03, 0x03).portr("DSW0").w(FUNC(scyclone_state::vidctrl_w));
	map(0x04, 0x04).w(FUNC(scyclone_state::sprite_xpos_w));
	map(0x05, 0x05).w(FUNC(scyclone_state::sprite_ypos_w));
	map(0x06, 0x06).w(FUNC(scyclone_state::port06_w));
	map(0x08, 0x08).w(FUNC(scyclone_state::sprite_colour_w));
	map(0x09, 0x09).w(FUNC(scyclone_state::sprite_tile_w));
	map(0x0a, 0x0a).w(FUNC(scyclone_state::starscroll_w));
	map(0x0e, 0x0e).w(FUNC(scyclone_state::port0e_w));
	map(0x0f, 0x0f).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x40, 0x40).w(FUNC(scyclone_state::videomask1_w));
	map(0x80, 0x80).w(FUNC(scyclone_state::videomask2_w));
}


void scyclone_state::scyclone_sub_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();

	map(0x3000, 0x3000).r(m_soundlatch, FUNC(generic_latch_8_device::read)).w("dac", FUNC(dac_byte_interface::data_w)); // music
	map(0x3001, 0x3001).w(FUNC(scyclone_state::snd_3001_w));
	map(0x3002, 0x3002).w(FUNC(scyclone_state::snd_3002_w)); // speech
	map(0x3003, 0x3003).w(FUNC(scyclone_state::snd_3003_w));
	map(0x3004, 0x3004).w(FUNC(scyclone_state::snd_3004_w));
	map(0x3005, 0x3005).w(FUNC(scyclone_state::snd_3005_w));
}

void scyclone_state::scyclone_sub_iomap(address_map &map)
{
	map.global_mask(0xff);
}



/***************************************************************************

  Input ports

***************************************************************************/

static INPUT_PORTS_START( scyclone )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(scyclone_state::collision_r)) // hw collision?
	// maybe these 4 are the 4xdsw bank?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL PORT_CONDITION("DSW0", 0x04, EQUALS, 0x00)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Lives ) )      PORT_DIPLOCATION("DSW0:1,2")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Cabinet ) )    PORT_DIPLOCATION("DSW0:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Upright ) )
	PORT_DIPNAME( 0x08, 0x00, "Disable Collision (Buggy!)" ) PORT_DIPLOCATION("DSW0:4") // this causes the game to malfunction, likely leftover debug feature
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Show Copyright Date" ) PORT_DIPLOCATION("DSW0:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "DSW0:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "DSW0:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "DSW0:8" )
INPUT_PORTS_END



/***************************************************************************

  GFX Layouts

***************************************************************************/

static const gfx_layout tiles32x32_layout =
{
	32,32,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{
		0,      1,      2,      3,      256+0,  256+1,  256+2,  256+3,
		512+0,  512+1,  512+2,  512+3,  768+0,  768+1,  768+2,  768+3,
		1024+0, 1024+1, 1024+2, 1024+3, 1280+0, 1280+1, 1280+2, 1280+3,
		1536+0, 1536+1, 1536+2, 1536+3, 1792+0, 1792+1, 1792+2, 1792+3
	},
	{
		0*8,  1*8,  2*8 , 3*8,  4*8,  5*8,  6*8,  7*8,
		8*8,  9*8,  10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
		16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
		24*8, 25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	8*32*8
};

static GFXDECODE_START( gfx_scyclone )
	GFXDECODE_ENTRY( "gfx1", 0, tiles32x32_layout, 8, 4 )
GFXDECODE_END



/***************************************************************************

  Machine configs

***************************************************************************/

INTERRUPT_GEN_MEMBER(scyclone_state::main_irq)
{
	// CPU runs in IM0
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xd7); // Z80 - RST 10h
}

void scyclone_state::scyclone(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5_MHz_XTAL/2); // MOSTEK Z80-CPU 2.5MHz? (there's also a 9.987MHz XTAL), intermissions seem driven directly by CPU speed for reference
	m_maincpu->set_addrmap(AS_PROGRAM, &scyclone_state::scyclone_map);
	m_maincpu->set_addrmap(AS_IO, &scyclone_state::scyclone_iomap);
	m_maincpu->set_vblank_int("screen", FUNC(scyclone_state::main_irq));

	Z80(config, m_subcpu, 5_MHz_XTAL/2); // LH0080 Z80-CPU SHARP 2.5MHz (5Mhz XTAL on this sub-pcb)
	m_subcpu->set_addrmap(AS_PROGRAM, &scyclone_state::scyclone_sub_map);
	m_subcpu->set_addrmap(AS_IO, &scyclone_state::scyclone_sub_iomap);

	GENERIC_LATCH_8(config, m_soundlatch);

	MB14241(config, "mb14241");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-32-1);
	m_screen->set_screen_update(FUNC(scyclone_state::screen_update_scyclone));
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // due to hw collisions

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_scyclone);
	PALETTE(config, m_palette).set_entries(8 + 4*4);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	SN76477(config, m_snsnd[0]);
	m_snsnd[0]->set_enable(1);
	m_snsnd[0]->add_route(ALL_OUTPUTS, "speaker", 0.1);

	SN76477(config, m_snsnd[1]);
	m_snsnd[1]->set_enable(1);
	m_snsnd[1]->add_route(ALL_OUTPUTS, "speaker", 0.1);

	DAC_8BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.1); // unknown DAC

	HC55516(config, m_cvsd, 0).add_route(ALL_OUTPUTS, "speaker", 0.4);
	CLOCK(config, m_cvsd_clock, 14400); // unknown clock source, but this matches PCB recording
	m_cvsd_clock->signal_handler().set(FUNC(scyclone_state::cvsd_tick));
	m_cvsd_clock->signal_handler().append(m_cvsd, FUNC(hc55516_device::mclock_w));
	m_cvsd_clock->signal_handler().append_inputline(m_subcpu, 0, HOLD_LINE);
}



/***************************************************************************

  ROM definitions

***************************************************************************/

ROM_START( scyclone )
	ROM_REGION( 0x3000, "maincpu", 0 )
	ROM_LOAD( "de07.1h.2716", 0x0000, 0x0800, CRC(fa9c93a3) SHA1(701a3196ee67533d69f1264e14085d04dfb1b915) )
	ROM_LOAD( "de08.3h.2716", 0x0800, 0x0800, CRC(bdff3e77) SHA1(e3fc6840348fa6df834cb1fb8b3832db05b5c585) )
	ROM_LOAD( "de09.4h.2716", 0x1000, 0x0800, CRC(eed05875) SHA1(e00a1b2d2d723c2deedf5f5e3159936a5d27891d) )
	ROM_LOAD( "de10.5h.2716", 0x1800, 0x0800, CRC(71301be6) SHA1(70c1983d256a6a9e18c7a15b93f5e3781d884fdb) )
	ROM_LOAD( "de11.6h.2716", 0x2000, 0x0800, CRC(d241db51) SHA1(53c6489d715baf5f03032a7ac367a6af64bce040) )
	ROM_LOAD( "de12.7h.2716", 0x2800, 0x0800, CRC(208e3e9a) SHA1(07f0e6c5417584eef171d8dcad83d741f88c9348) )

	ROM_REGION( 0x2000, "subcpu", 0 )
	ROM_LOAD( "de18.ic1.2716",  0x0000, 0x0800, CRC(182a5c24) SHA1(0ba29b6b3e7ec97c0b0748a6e60bb1a737dc55fa) )
	ROM_LOAD( "de04.ic9.2716",  0x0800, 0x0800, CRC(098269ac) SHA1(138af858bbbd73380086f971bcdd52ae47e2b667) )
	ROM_LOAD( "de05.ic14.2716", 0x1000, 0x0800, CRC(878f68b2) SHA1(2c2fa1b9053664ec26452ab0b35e2ae550601cc9) )
	ROM_LOAD( "de06.ic19.2716", 0x1800, 0x0800, CRC(0b1ead90) SHA1(38322b39f4420408c223cdbed3b75692a3d70746) )

	ROM_REGION( 0x0800, "gfx1", 0 ) // 32x32x2 sprites
	ROM_LOAD( "de01.11c.2716", 0x0000, 0x0800, CRC(bf770730) SHA1(1d0f9235b0618e3f4dd6db47efbdf92e2b00f5f6) )

	ROM_REGION( 0x0400, "gfx1pal", 0 ) // only 16 bytes of this are actually used
	ROM_LOAD( "de02.5b.82s137", 0x0000, 0x0400, CRC(fe4b278c) SHA1(c03080ab4d3fb84b8eec7087b925d1a1d8565fcc) )

	ROM_REGION( 0x0400, "stars", 0 ) // starfield bitmap
	ROM_LOAD( "de15.3a.82s137", 0x0000, 0x0400, CRC(c2816161) SHA1(2d0e7b4dbdb2af1481a4b7e45b1f9e3640f69aec) )

	ROM_REGION( 0x0040, "proms", 0 )
	ROM_LOAD( "de16.4a.82s123", 0x0000, 0x0020, CRC(5178e9c5) SHA1(dd2f81894069282f37feae21c5cfacf50f77dcd5) )
	ROM_LOAD( "de17.2e.82s123", 0x0020, 0x0020, CRC(3c8572e4) SHA1(c908c4ed99828fff576c3d0963cd8b99edeb993b) )
ROM_END

} // anonymous namespace


GAME( 1980, scyclone, 0, scyclone, scyclone, scyclone_state, empty_init, ROT270, "Taito Corporation", "Space Cyclone (Japan)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
