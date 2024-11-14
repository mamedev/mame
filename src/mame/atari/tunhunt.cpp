// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino, David Haywood

/***************************************************************************

    Atari Tunnel Hunt hardware

    Games supported:
        * Tunnel Hunt

    Known issues:
        * see below

****************************************************************************

    MAME driver for Tunnel Hunt (C)1981
        (aka Tube Chase)
        Developed by Atari
        Hardware by Dave Sherman
        Game Programmed by Owen Rubin
        Licensed and Distributed by Centuri

    Many thanks to Owen Rubin for invaluable hardware information and
    game description!

    Known Issues:

    Colors:
    - Hues are hardcoded.  There doesn't appear to be any logical way to
        map the color PROMs so that the correct colors appear.
        See last page of schematics for details.  Are color PROMs bad?
        (shouldn't be, both sets were the same)

    Shell Objects:
    - vstretch/placement/color handling isn't confirmed
    - two bitplanes per character or two banks?

    Motion Object:
    - enemy ships look funny when they get close (to ram player)
    - stretch probably isn't implemented correctly (see splash screen
        with zooming "TUNNEL HUNT" logo.
    - colors may not be mapped correctly.

    Square Generator:
    - needs optimization

***************************************************************************/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "machine/rescap.h"
#include "sound/pokey.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class tunhunt_state : public driver_device
{
public:
	tunhunt_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_workram(*this, "workram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_paletteram(*this, "paletteram"),
		m_in0(*this, "IN0"),
		m_dsw(*this, "DSW"),
		m_led(*this, "led0")
	{ }

	void tunhunt(machine_config &config);

protected:
	virtual void machine_start() override { m_led.resolve(); }
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_paletteram;
	required_ioport m_in0;
	required_ioport m_dsw;
	output_finder<> m_led;

	uint8_t m_control = 0;
	tilemap_t *m_fg_tilemap = nullptr;
	bitmap_ind16 m_tmpbitmap;

	uint8_t m_mobsc0 = 0;
	uint8_t m_mobsc1 = 0;
	uint8_t m_lineh[13]{};
	uint8_t m_shl0st = 0;
	uint8_t m_shl1st = 0;
	uint8_t m_vstrlo = 0;
	uint8_t m_linesh = 0;
	uint8_t m_shl0pc = 0;
	uint8_t m_shl1pc = 0;
	uint8_t m_linec[13]{};
	uint8_t m_shl0v = 0;
	uint8_t m_shl1v = 0;
	uint8_t m_mobjh = 0;
	uint8_t m_linev[13]{};
	uint8_t m_shl0vs = 0;
	uint8_t m_shl1vs = 0;
	uint8_t m_mobvs = 0;
	uint8_t m_linevs[13]{};
	uint8_t m_shel0h = 0;
	uint8_t m_mobst = 0;
	uint8_t m_shel1h = 0;
	uint8_t m_mobjv = 0;

	void control_w(uint8_t data);
	uint8_t button_r(offs_t offset);
	void videoram_w(offs_t offset, uint8_t data);
	template <uint16_t Mask> uint8_t dsw2_r();

	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void set_pens();
	void draw_motion_object(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_shell(bitmap_ind16 &bitmap, const rectangle &cliprect, int picture_code, int hposition, int vstart, int vstop, int vstretch, int hstretch);
	void main_map(address_map &map) ATTR_COLD;
};


/****************************************************************************************/

void tunhunt_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(tunhunt_state::get_fg_tile_info)
{
	int const attr = m_videoram[tile_index];
	int const code = attr & 0x3f;
	int const color = attr >> 6;
	int const flags = color ? TILE_FORCE_LAYER0 : 0;

	tileinfo.set(0, code, color, flags);
}

void tunhunt_state::video_start()
{
	/*
	Motion Object RAM contains 64 lines of run-length encoded data.
	We keep track of dirty lines and cache the expanded bitmap.
	With max RLE expansion, bitmap size is 256x64.
	*/

	m_tmpbitmap.allocate(256, 64, m_screen->format());

	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tunhunt_state::get_fg_tile_info)), TILEMAP_SCAN_COLS, 8, 8, 8, 32);

	m_fg_tilemap->set_transparent_pen(0);
	m_fg_tilemap->set_scrollx(0, 64);

	save_item(NAME(m_control));
	save_item(NAME(m_mobsc0));
	save_item(NAME(m_mobsc1));
	save_item(NAME(m_lineh));
	save_item(NAME(m_shl0st));
	save_item(NAME(m_shl1st));
	save_item(NAME(m_vstrlo));
	save_item(NAME(m_linesh));
	save_item(NAME(m_shl0pc));
	save_item(NAME(m_shl1pc));
	save_item(NAME(m_linec));
	save_item(NAME(m_shl0v));
	save_item(NAME(m_shl1v));
	save_item(NAME(m_mobjh));
	save_item(NAME(m_linev));
	save_item(NAME(m_shl0vs));
	save_item(NAME(m_shl1vs));
	save_item(NAME(m_mobvs));
	save_item(NAME(m_linevs));
	save_item(NAME(m_shel0h));
	save_item(NAME(m_mobst));
	save_item(NAME(m_shel1h));
	save_item(NAME(m_mobjv));
}

void tunhunt_state::palette(palette_device &palette) const
{
	/* Tunnel Hunt uses a combination of color PROMs and palette RAM to specify a 16 color
	 * palette.  Here, we manage only the mappings for alphanumeric characters and SHELL
	 * graphics, which are unpacked ahead of time and drawn using MAME's drawgfx primitives.
	 */

	// motion objects/box
	for (int i = 0; i < 0x10; i++)
		palette.set_pen_indirect(i, i);

	/* AlphaNumerics (1bpp)
	 *  2 bits of hilite select from 4 different background colors
	 *  Foreground color is always pen#4
	 *  Background color is mapped as follows:
	 */

	// alpha hilite#0
	palette.set_pen_indirect(0x10, 0x0); // background color#0 (transparent)
	palette.set_pen_indirect(0x11, 0x4); // foreground color

	// alpha hilite#1
	palette.set_pen_indirect(0x12, 0x5); // background color#1
	palette.set_pen_indirect(0x13, 0x4); // foreground color

	// alpha hilite#2
	palette.set_pen_indirect(0x14, 0x6); // background color#2
	palette.set_pen_indirect(0x15, 0x4); // foreground color

	// alpha hilite#3
	palette.set_pen_indirect(0x16, 0xf); // background color#3
	palette.set_pen_indirect(0x17, 0x4); // foreground color

	/* shell graphics; these are either 1bpp (2 banks) or 2bpp.  It isn't clear which.
	 * In any event, the following pens are associated with the shell graphics:
	 */
	palette.set_pen_indirect(0x18, 0);
	palette.set_pen_indirect(0x19, 4);//1;
}

/*
Color Array RAM Assignments:
    Location
        0               Blanking, border
        1               Mot Obj (10) (D), Shell (01)
        2               Mot Obj (01) (G), Shell (10)
        3               Mot Obj (00) (W)
        4               Alpha & Shell (11) - shields
        5               Hilight 1
        6               Hilight 2
        8-E             Lines (as normal) background
        F               Hilight 3
*/
void tunhunt_state::set_pens()
{
/*
    The actual contents of the color PROMs (unused by this driver)
    are as follows:

    D11 "blue/green"
    0000:   00 00 8b 0b fb 0f ff 0b
            00 00 0f 0f fb f0 f0 ff

    C11 "red"
    0020:   00 f0 f0 f0 b0 b0 00 f0
            00 f0 f0 00 b0 00 f0 f0
*/
	//const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 16; i++)
	{
		int color = m_paletteram[i];
		int const shade = 0xf^(color>>4);
		int red, green, blue;

		color &= 0xf; // hue select
		switch (color)
		{
		default:
		case 0x0: red = 0xff; green = 0xff; blue = 0xff; break; // white
		case 0x1: red = 0xff; green = 0x00; blue = 0xff; break; // purple
		case 0x2: red = 0x00; green = 0x00; blue = 0xff; break; // blue
		case 0x3: red = 0x00; green = 0xff; blue = 0xff; break; // cyan
		case 0x4: red = 0x00; green = 0xff; blue = 0x00; break; // green
		case 0x5: red = 0xff; green = 0xff; blue = 0x00; break; // yellow
		case 0x6: red = 0xff; green = 0x00; blue = 0x00; break; // red
		case 0x7: red = 0x00; green = 0x00; blue = 0x00; break; // black?

		case 0x8: red = 0xff; green = 0x7f; blue = 0x00; break; // orange
		case 0x9: red = 0x7f; green = 0xff; blue = 0x00; break; // ?
		case 0xa: red = 0x00; green = 0xff; blue = 0x7f; break; // ?
		case 0xb: red = 0x00; green = 0x7f; blue = 0xff; break; // ?
		case 0xc: red = 0xff; green = 0x00; blue = 0x7f; break; // ?
		case 0xd: red = 0x7f; green = 0x00; blue = 0xff; break; // ?
		case 0xe: red = 0xff; green = 0xaa; blue = 0xaa; break; // ?
		case 0xf: red = 0xaa; green = 0xaa; blue = 0xff; break; // ?
		}

	// combine color components with shade value (0..0xf)
		#define APPLY_SHADE( C,S ) ((C*S)/0xf)
		red = APPLY_SHADE(red, shade);
		green = APPLY_SHADE(green, shade);
		blue = APPLY_SHADE(blue, shade);

		m_palette->set_indirect_color(i, rgb_t(red, green, blue));
	}
}

void tunhunt_state::draw_motion_object(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
 *      VSTRLO  0x1202
 *          normally 0x02 (gameplay, attract1)
 *          in attract2 (with "Tunnel Hunt" graphic), decrements from 0x2f down to 0x01
 *          goes to 0x01 for some enemy shots
 *
 *      MOBSC0  0x1080
 *      MOBSC1  0x1081
 *          always 0x00?
 */

	bitmap_ind16 &tmpbitmap = m_tmpbitmap;
	//int skip = m_mobst;
	const int x0 = 255 - m_mobjv;
	const int y0 = 255 - m_mobjh;

	for (int line = 0; line < 64; line++)
	{
		int x = 0;
		const uint8_t *const source = &m_spriteram[line * 0x10];
		for (int span = 0; span < 0x10; span++)
		{
			const int span_data = source[span];
			if (span_data == 0xff) break;
			const int color = ((span_data >> 6) & 0x3) ^ 0x3;
			int count = (span_data & 0x1f) + 1;
			while (count-- && x < 256)
				tmpbitmap.pix(line, x++) = color;
		}
		while (x < 256)
			tmpbitmap.pix(line, x++) = 0;
	}

	int scaley;
	switch (m_vstrlo)
	{
	case 0x01:
		scaley = (1 << 16) * 0.33; // seems correct
		break;

	case 0x02:
		scaley = (1 << 16) * 0.50; // seems correct
		break;

	default:
		scaley = (1 << 16) * m_vstrlo / 4; // ???
		break;
	}
	const int scalex = 1 << 16;

	copyrozbitmap_trans(
			bitmap, cliprect, tmpbitmap,
			-x0 * scalex, // startx
			-y0 * scaley, // starty
			scalex, // incxx
			0, 0, // incxy, incyx
			scaley, // incyy
			false, // no wraparound
			0);
}

void tunhunt_state::draw_box(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
/*
    This is unnecessarily slow, but the box priorities aren't completely understood,
    yet.  Once understood, this function should be converted to use bitmap_fill with
    rectangular chunks instead of BITMAP_ADDR.

    Tunnels:
        1080: 00 00 00      01  e7 18   ae 51   94 6b   88 77   83 7c   80 7f   x0
        1480: 00 f0 17      00  22 22   5b 5b   75 75   81 81   86 86   89 89   y0
        1400: 00 00 97      ff  f1 f1   b8 b8   9e 9e   92 92   8d 8d   8a 8a   y1
        1280: 07 03 00      07  07 0c   0c 0d   0d 0e   0e 08   08 09   09 0a   palette select

    Color Bars:
        1080: 00 00 00      01  00 20 40 60 80 a0 c0 e0     01 2a   50 7a       x0
        1480: 00 f0 00      00  40 40 40 40 40 40 40 40     00 00   00 00       y0
        1400: 00 00 00      ff  ff ff ff ff ff ff ff ff     40 40   40 40       y1
        1280: 07 03 00      01  07 06 04 05 02 07 03 00     09 0a   0b 0c       palette select
        ->hue 06 02 ff      60  06 05 03 04 01 06 02 ff     d2 00   c2 ff
*/

	for (int y = 0; y < 256; y++)
	{
		if (0xff - y >= cliprect.top() && 0xff - y <= cliprect.bottom())
			for (int x = 0; x < 256; x++)
			{
				int color = 0;
				int z = 0;
				for (int span = 0; span < 13; span++)
				{
					int const x0 = m_lineh[span];
					int const y0 = m_linevs[span];
					int const y1 = m_linev[span];

					if (y >= y0 && y <= y1 && x >= x0 && x0 >= z)
					{
						color = m_linec[span] & 0xf;
						z = x0; // give priority to rightmost spans
					}
				}
				if (x >= cliprect.left() && x <= cliprect.right())
					bitmap.pix(0xff - y, x) = color;
			}
	}
}

// "shell" graphics are 16x16 pixel tiles used for player shots and targeting cursor
void tunhunt_state::draw_shell(bitmap_ind16 &bitmap,
		const rectangle &cliprect,
		int picture_code,
		int hposition,
		int vstart,
		int vstop,
		int vstretch,
		int hstretch )
{
	if (hstretch)
	{
		for (int sx = 0; sx < 256; sx += 16)
		{
			for (int sy = 0; sy < 256; sy += 16)
			{
					m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
					picture_code,
					0, // color
					0, 0, // flip
					sx, sy, 0);
			}
		}
	}
	else
	/*
	    vstretch is normally 0x01

	    targeting cursor:
	        hposition   = 0x78
	        vstart      = 0x90
	        vstop       = 0x80

	    during grid test:
	        vstretch    = 0xff
	        hposition   = 0xff
	        vstart      = 0xff
	        vstop       = 0x00

	*/

			m_gfxdecode->gfx(1)->transpen(bitmap, cliprect,
			picture_code,
			0, // color
			0, 0, // flip
			255 - hposition - 16, vstart - 32, 0);
}

uint32_t tunhunt_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	set_pens();

	draw_box(bitmap, cliprect);

	draw_motion_object(bitmap, cliprect);

	draw_shell(bitmap, cliprect,
		m_shl0pc,  // picture code
		m_shel0h,  // hposition
		m_shl0v,   // vstart
		m_shl0vs,  // vstop
		m_shl0st,  // vstretch
		m_control & 0x08); // hstretch

	draw_shell(bitmap, cliprect,
		m_shl1pc,  // picture code
		m_shel1h,  // hposition
		m_shl1v,   // vstart
		m_shl1vs,  // vstop
		m_shl1st,  // vstretch
		m_control & 0x10); // hstretch

	rectangle cr = cliprect;
	if (cr.min_x < 192)
		cr.min_x = 192;

	m_fg_tilemap->draw(screen, bitmap, cr, 0, 0);
	return 0;
}


/*************************************
 *
 *  Output ports
 *
 *************************************/

void tunhunt_state::control_w(uint8_t data)
{
	/*
	    0x01    coin counter#2  "right counter"
	    0x02    coin counter#1  "center counter"
	    0x04    "left counter"
	    0x08    cover screen (shell0 hstretch)
	    0x10    cover screen (shell1 hstretch)
	    0x40    start LED
	    0x80    in-game
	*/

	m_control = data;
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
	m_led = BIT(data , 6); // start
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

uint8_t tunhunt_state::button_r(offs_t offset)
{
	int const data = m_in0->read();
	return ((data >> offset) & 1) ? 0x00 : 0x80;
}


template <uint16_t Mask>
uint8_t tunhunt_state::dsw2_r()
{
	return (m_dsw->read()& Mask) ? 0x80 : 0x00;
}


void tunhunt_state::machine_reset()
{
	m_mobsc0 = 0;
	m_mobsc1 = 0;
	memset(m_lineh, 0, 13);
	m_shl0st = 0;
	m_shl1st = 0;
	m_vstrlo = 0;
	m_linesh = 0;
	m_shl0pc = 0;
	m_shl1pc = 0;
	memset(m_linec, 0, 13);
	m_shl0v = 0;
	m_shl1v = 0;
	m_mobjh = 0;
	memset(m_linev, 0, 13);
	m_shl0vs = 0;
	m_shl1vs = 0;
	m_mobvs = 0;
	memset(m_linevs, 0, 13);
	m_shel0h = 0;
	m_mobst = 0;
	m_shel1h = 0;
	m_mobjv = 0;
}



/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void tunhunt_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x03ff).ram().share(m_workram);

	map(0x1080, 0x1080).lw8(NAME([this](uint8_t data) { m_mobsc0 = data; }));  // SCAN ROM START FOR MOBJ (unused?)
	map(0x1081, 0x1081).lw8(NAME([this](uint8_t data) { m_mobsc1 = data; }));  // (unused?)
	map(0x1083, 0x108f).lw8(NAME([this](offs_t offset, uint8_t data) { m_lineh[offset] = data; }));   // LINES HORIZ START
	map(0x1200, 0x1200).lw8(NAME([this](uint8_t data) { m_shl0st = data; }));  // SHELL VSTRETCH (LIKE MST OBJ STRECTH)
	map(0x1201, 0x1201).lw8(NAME([this](uint8_t data) { m_shl1st = data; }));
	map(0x1202, 0x1202).lw8(NAME([this](uint8_t data) { m_vstrlo = data; }));  // VERT (SCREEN ON SIDE) STRETCH MOJ OBJ
	map(0x1203, 0x1203).lw8(NAME([this](uint8_t data) { m_linesh = data; }));  // LINES SLOPE 4 BITS D0-D3 (signed)
	map(0x1280, 0x1280).lw8(NAME([this](uint8_t data) { m_shl0pc = data; }));  // SHELL PICTURE CODE (D3-D0)
	map(0x1280, 0x1281).lw8(NAME([this](uint8_t data) { m_shl1pc = data; }));
	map(0x1283, 0x128f).lw8(NAME([this](offs_t offset, uint8_t data) { m_linec[offset] = data; }));   // LINE COLOR, 4 BITS D0-D3
	map(0x1400, 0x1400).lw8(NAME([this](uint8_t data) { m_shl0v = data; }));   // SHELL V START(NORMAL SCREEN)
	map(0x1401, 0x1401).lw8(NAME([this](uint8_t data) { m_shl1v = data; }));
	map(0x1402, 0x1402).lw8(NAME([this](uint8_t data) { m_mobjh = data; }));   // H POSITON (SCREEN ON SIDE) (VSTART - NORMAL SCREEN)
	map(0x1403, 0x140f).lw8(NAME([this](offs_t offset, uint8_t data) { m_linev[offset] = data; }));   // LINES VERTICAL START
	map(0x1480, 0x1480).lw8(NAME([this](uint8_t data) { m_shl0vs = data; }));  // SHELL V STOP (NORMAL SCREEN)
	map(0x1481, 0x1481).lw8(NAME([this](uint8_t data) { m_shl1vs = data; }));
	map(0x1482, 0x1482).lw8(NAME([this](uint8_t data) { m_mobvs = data; }));   // V STOP OF MOTION OBJECT (NORMAL SCREEN)
	map(0x1483, 0x148f).lw8(NAME([this](offs_t offset, uint8_t data) { m_linevs[offset] = data; }));  // LINES VERT STOP
	map(0x1600, 0x160f).writeonly().share(m_paletteram);                       // COLRAM (D7-D4 SHADE; D3-D0 COLOR)
	map(0x1800, 0x1800).lw8(NAME([this](uint8_t data) { m_shel0h = data; }));  // SHELL H POSITON (NORMAL SCREEN)
	map(0x1802, 0x1802).lw8(NAME([this](uint8_t data) { m_mobst = data; }));   // STARTING LINE FOR RAM SCAN ON MOBJ
	map(0x1a00, 0x1a00).lw8(NAME([this](uint8_t data) { m_shel1h = data; }));
	map(0x1c00, 0x1c00).lw8(NAME([this](uint8_t data) { m_mobjv = data; }));   // V POSITION (SCREEN ON SIDE)
	map(0x1e00, 0x1eff).w(FUNC(tunhunt_state::videoram_w)).share(m_videoram);  // ALPHA

	map(0x2000, 0x2000).nopw();    // watchdog
	map(0x2000, 0x2007).r(FUNC(tunhunt_state::button_r));
	map(0x2400, 0x2400).nopw();    // INT ACK
	map(0x2800, 0x2800).w(FUNC(tunhunt_state::control_w));
	map(0x2c00, 0x2fff).writeonly().share(m_spriteram);

	map(0x3000, 0x300f).rw("pokey1", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x4000, 0x400f).rw("pokey2", FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0x5000, 0x7fff).rom();
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( tunhunt )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_SERVICE( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT ( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN1")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4)

	PORT_START("IN2")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_SENSITIVITY(100) PORT_KEYDELTA(4) PORT_REVERSE

	PORT_START("DSW")
	PORT_DIPNAME (0x0003, 0x0002, DEF_STR( Coinage ) )      PORT_DIPLOCATION("B4:1,2")
	PORT_DIPSETTING (     0x0003, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING (     0x0002, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING (     0x0001, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING (     0x0000, DEF_STR( Free_Play ) )
	PORT_DIPNAME (0x000c, 0x0000, "Coin 2 Multiplier" )     PORT_DIPLOCATION("B4:3,4")
	PORT_DIPSETTING (     0x0000, "1" )
	PORT_DIPSETTING (     0x0004, "4" )
	PORT_DIPSETTING (     0x0008, "5" )
	PORT_DIPSETTING (     0x000c, "6" )
	PORT_DIPNAME (0x0010, 0x0000, "Coin 1 Multiplier" )     PORT_DIPLOCATION("B4:5")
	PORT_DIPSETTING (     0x0000, "1" )
	PORT_DIPSETTING (     0x0010, "2" )
	PORT_DIPNAME (0x0060, 0x0000, "Bonus Credits" )         PORT_DIPLOCATION("B4:6,7")
	PORT_DIPSETTING (     0x0000, DEF_STR( None ) )
	PORT_DIPSETTING (     0x0060, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING (     0x0040, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING (     0x0020, DEF_STR( 2C_1C ) )
	PORT_DIPNAME (0x0880, 0x0000, DEF_STR( Language ) )     PORT_DIPLOCATION("B3:3,B4:8")
	PORT_DIPSETTING (     0x0000, DEF_STR( English ) )
	PORT_DIPSETTING (     0x0080, DEF_STR( German ) )
	PORT_DIPSETTING (     0x0800, DEF_STR( French ) )
	PORT_DIPSETTING (     0x0880, DEF_STR( Spanish ) )
	PORT_DIPNAME (0x1100, 0x0100, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("B3:2,1")
	PORT_DIPSETTING (     0x0000, DEF_STR( None ) )
	PORT_DIPSETTING (     0x1000, "30K, 100K" )
	PORT_DIPSETTING (     0x0100, "60K, 100K" )
	PORT_DIPSETTING (     0x1100, "90K, 100K" )
	PORT_DIPNAME (0x0600, 0x0200, DEF_STR( Lives ) )        PORT_DIPLOCATION("B3:4,5")
	PORT_DIPSETTING (     0x0000, "2" )
	PORT_DIPSETTING (     0x0200, "3" )
	PORT_DIPSETTING (     0x0400, "4" )
	PORT_DIPSETTING (     0x0600, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x0000, "B3:6" ) // N/C
	PORT_DIPUNUSED_DIPLOC( 0x4000, 0x0000, "B3:7" ) // N/C
	PORT_DIPUNUSED_DIPLOC( 0x8000, 0x0000, "B3:8" ) // N/C
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout alpha_layout =
{
	8,8,
	0x40,
	1,
	{ 4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70 },
	0x80
};


static const gfx_layout obj_layout =
{
	16,16,
	8, // number of objects
	1, // number of bitplanes
	{ 4 }, // plane offsets
	{
		0x00+0,0x00+1,0x00+2,0x00+3,
		0x08+0,0x08+1,0x08+2,0x08+3,
		0x10+0,0x10+1,0x10+2,0x10+3,
		0x18+0,0x18+1,0x18+2,0x18+3
		}, // x offsets
	{
		0x0*0x20, 0x1*0x20, 0x2*0x20, 0x3*0x20,
		0x4*0x20, 0x5*0x20, 0x6*0x20, 0x7*0x20,
		0x8*0x20, 0x9*0x20, 0xa*0x20, 0xb*0x20,
		0xc*0x20, 0xd*0x20, 0xe*0x20, 0xf*0x20
	}, // y offsets
	0x200
};


static GFXDECODE_START( gfx_tunhunt )
	GFXDECODE_ENTRY( "chars",   0x000, alpha_layout, 0x10, 4 )
	GFXDECODE_ENTRY( "sprites", 0x200, obj_layout,   0x18, 1 )
	GFXDECODE_ENTRY( "sprites", 0x000, obj_layout,   0x18, 1 ) // second bank, or second bitplane?
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void tunhunt_state::tunhunt(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 12.096_MHz_XTAL / 6); // ???
	m_maincpu->set_addrmap(AS_PROGRAM, &tunhunt_state::main_map);
	m_maincpu->set_periodic_int(FUNC(tunhunt_state::irq0_line_hold), attotime::from_hz(4*60));  // 48V, 112V, 176V, 240V

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); // not accurate
	m_screen->set_size(256, 256-16);
	m_screen->set_visarea(0, 255, 0, 255-16);
	m_screen->set_screen_update(FUNC(tunhunt_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_tunhunt);
	PALETTE(config, m_palette, FUNC(tunhunt_state::palette), 0x1a, 16);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	pokey_device &pokey1(POKEY(config, "pokey1", 12.096_MHz_XTAL / 10));
	pokey1.allpot_r().set_ioport("DSW");
	pokey1.set_output_rc(RES_K(1), CAP_U(0.047), 5.0);
	pokey1.add_route(ALL_OUTPUTS, "mono", 0.50);

	pokey_device &pokey2(POKEY(config, "pokey2", 12.096_MHz_XTAL / 10));
	pokey2.pot_r<0>().set_ioport("IN1");
	pokey2.pot_r<1>().set_ioport("IN2");
	pokey2.pot_r<2>().set(FUNC(tunhunt_state::dsw2_r<0x100>));
	pokey2.pot_r<3>().set(FUNC(tunhunt_state::dsw2_r<0x200>));
	pokey2.pot_r<4>().set(FUNC(tunhunt_state::dsw2_r<0x400>));
	pokey2.pot_r<5>().set(FUNC(tunhunt_state::dsw2_r<0x800>));
	pokey2.pot_r<6>().set(FUNC(tunhunt_state::dsw2_r<0x1000>));
	pokey2.set_output_rc(RES_K(1), CAP_U(0.047), 5.0);
	pokey2.add_route(ALL_OUTPUTS, "mono", 0.50);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

/*

ATARI TUBE CHASE

136000-101      5000    6/16
136000-102      5800    6/16
136000-103      6000    6/16
136000-104      6800    6/16
136000-105      7000    6/16
136000-106      7800    6/16

136000-015      SYNC
136000-017      SW 1            B8
136000-016      SW 0            A8
136000-018      PRIORITY        H9
136000-019      A/N             C10
136000-013      RED             C11
136000-014      B/G             D11

*/

ROM_START( tunhunt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "001.lm1",    0x5000, 0x800, CRC(2601a3a4) SHA1(939bafc54576fdaccf688b49cc9d201b03feec3a) )
	ROM_LOAD( "002.k1",     0x5800, 0x800, CRC(29bbf3df) SHA1(4a0ec4cfab362a976d3962b347f687db45095cfd) )
	ROM_LOAD( "136000.103", 0x6000, 0x800, CRC(1a6a60a4) SHA1(7c60cc92595f1b90f421eabbaa20f657181ed4f0) )
	ROM_LOAD( "004.fh1",    0x6800, 0x800, CRC(4d6c920e) SHA1(2ef274356f4b8a0170a267cd6a3758b2bda693b5) )
	ROM_LOAD( "005.ef1",    0x7000, 0x800, CRC(e17badf0) SHA1(6afbf517486340fe54b01fa26258877b2a8fc510) )
	ROM_LOAD( "006.d1",     0x7800, 0x800, CRC(c3ae8519) SHA1(2b2e49065bc38429894ef29a29ffc60f96e64840) )

	ROM_REGION( 0x400, "chars", 0 )
	ROM_LOAD( "019.c10",    0x000, 0x400, CRC(d6fd45a9) SHA1(c86ea3790c29c554199af8ad6f3d563dcb7723c7) )

	ROM_REGION( 0x400, "sprites", 0 ) // "SHELL" objects (16x16 pixel sprites)
	ROM_LOAD( "016.a8",     0x000, 0x200, CRC(830e6c34) SHA1(37a5eeb722dd80c4224c7f622b0edabb3ac1ca19) )
	ROM_LOAD( "017.b8",     0x200, 0x200, CRC(5bef8b5a) SHA1(bfd9c592a34ed4861a6ad76ef10ea0d9b76a92b2) )

	ROM_REGION( 0x540, "proms", 0 )
	ROM_LOAD( "013.d11",    0x000, 0x020, CRC(66f1f5eb) SHA1(bcf5348ae328cf943d2bf6e38df727c0c4c466b7) )    // hue: BBBBGGGG?
	ROM_LOAD( "014.c11",    0x020, 0x020, CRC(662444b2) SHA1(2e510c1d9b7e34a3045048a46045e61fabaf918e) )    // hue: RRRR----?
	ROM_LOAD( "015.n4",     0x040, 0x100, CRC(00e224a0) SHA1(1a384ef488791c62566c91b18d6a1fb4a5def2ba) )    // timing?
	ROM_LOAD( "018.h9",     0x140, 0x400, CRC(6547c208) SHA1(f19c334f9b4a1cfcbc913c0920688db2730dded0) )    // color lookup table?
ROM_END

ROM_START( tunhuntc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "001.lm1",    0x5000, 0x800, CRC(2601a3a4) SHA1(939bafc54576fdaccf688b49cc9d201b03feec3a) )
	ROM_LOAD( "002.k1",     0x5800, 0x800, CRC(29bbf3df) SHA1(4a0ec4cfab362a976d3962b347f687db45095cfd) )
	ROM_LOAD( "003.j1",     0x6000, 0x800, CRC(360c0f47) SHA1(8e3d815836504c7651812e0e26423b0c7045621c) ) // bad CRC? fails self-test
	// 0xcaa6bb2a: alternate PROM (re)dumped by Al also fails, they simply modified the ROM without fixing the checksum routine?
	ROM_LOAD( "004.fh1",    0x6800, 0x800, CRC(4d6c920e) SHA1(2ef274356f4b8a0170a267cd6a3758b2bda693b5) )
	ROM_LOAD( "005.ef1",    0x7000, 0x800, CRC(e17badf0) SHA1(6afbf517486340fe54b01fa26258877b2a8fc510) )
	ROM_LOAD( "006.d1",     0x7800, 0x800, CRC(c3ae8519) SHA1(2b2e49065bc38429894ef29a29ffc60f96e64840) )

	ROM_REGION( 0x400, "chars", 0 )
	ROM_LOAD( "019.c10",    0x000, 0x400, CRC(d6fd45a9) SHA1(c86ea3790c29c554199af8ad6f3d563dcb7723c7) )

	ROM_REGION( 0x400, "sprites", 0 ) // "SHELL" objects (16x16 pixel sprites)
	ROM_LOAD( "016.a8",     0x000, 0x200, CRC(830e6c34) SHA1(37a5eeb722dd80c4224c7f622b0edabb3ac1ca19) )
	ROM_LOAD( "017.b8",     0x200, 0x200, CRC(5bef8b5a) SHA1(bfd9c592a34ed4861a6ad76ef10ea0d9b76a92b2) )

	ROM_REGION( 0x540, "proms", 0 )
	ROM_LOAD( "013.d11",    0x000, 0x020, CRC(66f1f5eb) SHA1(bcf5348ae328cf943d2bf6e38df727c0c4c466b7) )    // hue: BBBBGGGG?
	ROM_LOAD( "014.c11",    0x020, 0x020, CRC(662444b2) SHA1(2e510c1d9b7e34a3045048a46045e61fabaf918e) )    // hue: RRRR----?
	ROM_LOAD( "015.n4",     0x040, 0x100, CRC(00e224a0) SHA1(1a384ef488791c62566c91b18d6a1fb4a5def2ba) )    // timing?
	ROM_LOAD( "018.h9",     0x140, 0x400, CRC(6547c208) SHA1(f19c334f9b4a1cfcbc913c0920688db2730dded0) )    // color lookup table?
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979,tunhunt,  0,       tunhunt,   tunhunt, tunhunt_state, empty_init, ORIENTATION_SWAP_XY, "Atari",                   "Tunnel Hunt",           MACHINE_SUPPORTS_SAVE )
GAME( 1981,tunhuntc, tunhunt, tunhunt,   tunhunt, tunhunt_state, empty_init, ORIENTATION_SWAP_XY, "Atari (Centuri license)", "Tunnel Hunt (Centuri)", MACHINE_SUPPORTS_SAVE )
