// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
    These use a 6502 derived CPU under a glob
    The CPU die is marked 'ELAN EU3A14'

    There is a second glob surrounded by TSOP48 pads
    this contains the ROM

    Known to be on this hardware

    name                          PCB ID      ROM width   TSOP pads   ROM size        SEEPROM         die markings
    Golden Tee Golf Home Edition  ?           x16         48          4MB             no              ELAN EU3A14   (developed by FarSight Studios)
    Real Swing Golf               74037       x16         48          4MB             no              ELAN EU3A14   (developed by FarSight Studios)
    Baseball 3                    ?           x16         48          4MB             no              ELAN EU3A14   (developed by FarSight Studios)
    Connectv Football             ?           x16         48          4MB             no              ELAN EU3A14   (developed by Medialink)
    Huntin’3                      ?           x16         48          4MB             no              Elan ?        (developed by V-Tac Technology Co Ltd.)
    Play TV Basketball            75029       x16         48          4MB             no              ELAN EU3A14

    In many ways this is similar to the rad_eu3a05.cpp hardware
    but the video system has changed, here the sprites are more traditional non-tile based, rather
    than coming from 'pages'

    --

    Compared to the XaviXport games camera hookups, Real Swing Golf just has 6 wires, Its camera PCB is the only one with a ceramic resonator.
    Maybe the CU5502 chip offloads some processing from the CPU?

    The Basketball camera also uses an ETOMS CU5502.  It’s different from the others (XaviXport + Real Swing Golf) in that the sensor is on a small PCB with
    a 3.58MHz resonator with 16 wires going to another small PCB that has a glob and a 4MHz resonator.  6 wires go from that PCB to the main game PCB.

    To access hidden test mode in Football hold enter and right during power on.

    Football test mode tests X pos, Y pos, Z pos, direction and speed.  This data must all be coming from the camera in the unit as the shinpads are simply
    reflective objects, they don't contain any electronics.  It could be a useful test case for better understanding these things.

    To access hidden test mode in Golden Tee Home hold back/backspin and left during power on.

    To access hidden test mode in Basketball hold left and Button 1 during power on.

    To access hidden test mode in Real Swing Golf hold left and down during power on.
     - test mode check
     77B6: lda $5041
     77B9: eor #$ed
     77BB: beq $77be

    To access hidden test mode in Baseball 3 hold down during power on.
    - test mode check
    686E: lda $5041
    6871: eor #$f7
    6873: bne $68c8

    It is not clear how to access Huntin'3 Test Mode (if possible) there do appear to be tiles for it tho

    Huntin'3 makes much more extensive use of the video hardware than the other titles, including
     - Table based Rowscroll (most first person views)
     - RAM based tiles (status bar in "Target Range", text descriptions on menus etc.)
     - Windowing effects (to highlight menu items, timer in "Target Range") NOT YET EMULATED / PROPERLY UNDERSTOOD

*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"
#include "audio/elan_eu3a05.h"
#include "machine/timer.h"

/*

TODO: fill these in for other games, this is for Golden Tee Home

ffb0  rti
ffb4  rti
ffb8  rti
ffbc  rti

ffc0  rti
ffc4  rti
ffc8  rti
ffcc  rti

ffd0  rti
ffd4  main irq?
ffd8  rti
ffdc  rti

ffe0  something with 5045 bit 0x08 and 9d in ram (increase or decrease)  (ADC interrupt)
ffe4  something with 5045 bit 0x20 and 9c in ram (increase of decrease)  (ADC interrupt)

ffe8  rti
ffec  rti

regular NMI (e3f0 - jump to ($19e2) which seems to point to rti, but could move..)
regular IRQ (e3f3 - points to rti)

*/


class radica_eu3a14_state : public driver_device
{
public:
	radica_eu3a14_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainregion(*this, "maincpu"),
		m_palram(*this, "palram"),
		m_scrollregs(*this, "scrollregs"),
		m_tilecfg(*this, "tilecfg"),
		m_rowscrollregs(*this, "rowscrollregs"),
		m_rowscrollsplit(*this, "rowscrollsplit"),
		m_rowscrollcfg(*this, "rowscrollcfg"),
		m_ramtilecfg(*this, "ramtilecfg"),
		m_spriteaddr(*this, "spriteaddr"),
		m_spritebase(*this, "spritebase"),
		m_mainram(*this, "mainram"),
		m_dmaparams(*this, "dmaparams"),
		m_bank(*this, "bank"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_tvtype(*this, "TV")
	{ }


	void radica_eu3a14(machine_config &config);
	void radica_eu3a14_adc(machine_config &config);
	void radica_eu3a14p(machine_config &config);

	void init_rad_gtg();
	void init_rad_foot();
	void init_rad_hnt3();

private:
	READ8_MEMBER(irq_vector_r);

	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	int m_custom_irq;
	uint16_t m_custom_irq_vector;

	INTERRUPT_GEN_MEMBER(interrupt);

	DECLARE_READ8_MEMBER(radicasi_pal_ntsc_r);

	DECLARE_READ8_MEMBER(dma_trigger_r);
	DECLARE_WRITE8_MEMBER(dma_trigger_w);

	DECLARE_READ8_MEMBER(radicasi_rombank_lo_r);
	DECLARE_WRITE8_MEMBER(radicasi_rombank_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_rombank_hi_w);

	DECLARE_WRITE8_MEMBER(porta_dir_w);
	DECLARE_WRITE8_MEMBER(portb_dir_w);
	DECLARE_WRITE8_MEMBER(portc_dir_w);

	DECLARE_WRITE8_MEMBER(porta_dat_w);
	DECLARE_WRITE8_MEMBER(portb_dat_w);
	DECLARE_WRITE8_MEMBER(portc_dat_w);


	DECLARE_READ8_MEMBER(radica_5009_unk_r) { return machine().rand(); };

	DECLARE_READ8_MEMBER(random_r) { return machine().rand(); };

	TIMER_DEVICE_CALLBACK_MEMBER(scanline_cb);

	// for callback
	DECLARE_READ8_MEMBER(read_full_space);

	void bank_map(address_map &map);
	void radica_eu3a14_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	double hue2rgb(double p, double q, double t);

	required_device<cpu_device> m_maincpu;
	required_region_ptr<uint8_t> m_mainregion;
	required_shared_ptr<uint8_t> m_palram;
	required_shared_ptr<uint8_t> m_scrollregs;
	required_shared_ptr<uint8_t> m_tilecfg;
	required_shared_ptr<uint8_t> m_rowscrollregs;
	required_shared_ptr<uint8_t> m_rowscrollsplit;
	required_shared_ptr<uint8_t> m_rowscrollcfg;
	required_shared_ptr<uint8_t> m_ramtilecfg;
	required_shared_ptr<uint8_t> m_spriteaddr;
	required_shared_ptr<uint8_t> m_spritebase;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_dmaparams;
	required_device<address_map_bank_device> m_bank;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_ioport m_tvtype;

	uint8_t m_rombank_hi;
	uint8_t m_rombank_lo;
	int m_tilerambase;
	int m_spriterambase;

	bitmap_ind8 m_prioritybitmap;

	uint8_t m_portdir[3];

	void handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void draw_background_ramlayer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect);
	int get_xscroll_for_screenypos(int line);
	void draw_background_tile(bitmap_ind16 &bitmap, const rectangle &cliprect, int bpp, int tileno, int palette, int priority, int flipx, int flipy, int xpos, int ypos, int transpen, int size, int base, int drawfromram);
	void draw_background_page(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ramstart, int ramend, int xbase, int ybase, int size, int bpp, int base, int pagewidth,int pageheight, int bytespertile, int palettepri, int drawfromram);
	void draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprite_pix(const rectangle& cliprect, uint16_t* dst, uint8_t* pridst, int realx, int priority, uint8_t pix, uint8_t mask, uint8_t shift, int palette);
	void draw_sprite_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int offset, int line, int pal, int flipx, int pri, int xpos, int ypos, int bpp);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


void radica_eu3a14_state::video_start()
{
	m_screen->register_screen_bitmap(m_prioritybitmap);
}

double radica_eu3a14_state::hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}

void radica_eu3a14_state::handle_palette(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// Palette
	int offs = 0;
	for (int index = 0; index < 512; index++)
	{
		uint16_t dat = m_palram[offs++] << 8;
		dat |= m_palram[offs++];

		// llll lsss ---h hhhh
		int l_raw = (dat & 0xf800) >> 11;
		int sl_raw = (dat & 0x0700) >> 8;
		int h_raw = (dat & 0x001f) >> 0;

		double l = (double)l_raw / 31.0f;
		double s = (double)sl_raw / 7.0f;
		double h = (double)h_raw / 24.0f;

		double r, g, b;

		if (s == 0) {
			r = g = b = l; // greyscale
		}
		else {
			double q = l < 0.5f ? l * (1 + s) : l + s - l * s;
			double p = 2 * l - q;
			r = hue2rgb(p, q, h + 1 / 3.0f);
			g = hue2rgb(p, q, h);
			b = hue2rgb(p, q, h - 1 / 3.0f);
		}

		int r_real = r * 255.0f;
		int g_real = g * 255.0f;
		int b_real = b * 255.0f;

		m_palette->set_pen_color(index, r_real, g_real, b_real);
	}
}

void radica_eu3a14_state::draw_background_tile(bitmap_ind16& bitmap, const rectangle& cliprect, int bpp, int tileno, int palette, int priority, int flipx, int flipy, int xpos, int ypos, int transpen, int size, int base, int drawfromram)
{
	int baseaddr = base * 256;

	int xstride = 8;

	if (bpp == 8) // 8bpp selection
	{
		if (size == 8)
		{
			xstride = size / 1; baseaddr += tileno * 64; // 8x8 8bpp
		}
		else
		{
			xstride = size / 1; baseaddr += tileno * 256; // 16x16 8bpp
		}

		palette &= 0x100; // only top bit valid, as there are only 2 palettes?
	}
	else if (bpp == 4) // 4bpp selection
	{
		if (size == 8)
		{
			xstride = size / 2; baseaddr += tileno * 32; // 8x8 4bpp
		}
		else
		{
			xstride = size / 2; baseaddr += tileno * 128; // 16x16 4bpp
		}
	}
	else if (bpp == 2) // 2bpp?
	{
		xstride = size / 4; baseaddr += tileno * 64; // 16x16 2bpp
	}
	else
	{
		popmessage("draw_background_tile unsupported bpp %d\n", bpp);
		return;
	}



	uint8_t* gfxdata;

	if (drawfromram)
	{
		gfxdata = &m_mainram[baseaddr & 0x3fff];
	}
	else
	{
		gfxdata = &m_mainregion[baseaddr & 0x3fffff];
	}


	int count = 0;
	for (int y = 0; y < size; y++)
	{
		int realy = ypos + y;
		int xposwithscroll = 0;

		if (!drawfromram)
		{
			xposwithscroll = xpos - get_xscroll_for_screenypos(realy);
		}
		else
		{
			xposwithscroll = xpos;
			// RAM tile layer has no scrolling? (or we've never seen it used / enabled)
		}

		uint16_t* dst = &bitmap.pix16(ypos + y);
		uint8_t* pridst = &m_prioritybitmap.pix8(ypos + y);

		for (int x = 0; x < xstride; x++)
		{
			int pix = gfxdata[count];

			if (realy >= cliprect.min_y && realy <= cliprect.max_y)
			{
				if (bpp == 8) // 8bpp
				{
					int realx = x + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = pix | palette;
								pridst[realx] = priority;
							}

						}
					}
				}
				else if (bpp == 7)
				{
					popmessage("draw_background_tile bpp == 7");
				}
				else if (bpp == 6)
				{
					popmessage("draw_background_tile bpp == 6");
				}
				else if (bpp == 5)
				{
					popmessage("draw_background_tile bpp == 5");
				}
				else if (bpp == 4) // 4bpp
				{
					int realx = (x * 2) + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0xf0)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0xf0) >> 4) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x0f)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = (pix & 0x0f) | palette;
								pridst[realx] = priority;
							}
						}
					}
				}
				else if (bpp == 3)
				{
					popmessage("draw_background_tile bpp == 3");
				}
				else if (bpp == 2) // 2bpp (hnt3 ram text)
				{
					int realx = (x * 4) + xposwithscroll;
					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0xc0)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0xc0) >> 6) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x30)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0x30) >> 4) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x0c)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0x0c) >> 2) | palette;
								pridst[realx] = priority;
							}
						}
					}

					realx++;

					if (realx >= cliprect.min_x && realx <= cliprect.max_x)
					{
						if (pix & 0x03)
						{
							if (pridst[realx] <= priority)
							{
								dst[realx] = ((pix & 0x03) >> 0) | palette;
								pridst[realx] = priority;
							}
						}
					}
				}
				else if (bpp == 1)
				{
					popmessage("draw_background_tile bpp == 1");
				}
			}
			count++;
		}
	}
}

int radica_eu3a14_state::get_xscroll_for_screenypos(int ydraw)
{
	if ((ydraw < 0) || (ydraw >= 224))
		return 0;

	int xscroll = m_scrollregs[0] | (m_scrollregs[1] << 8);

	if (m_rowscrollcfg[1] == 0x01) // GUESS! could be anything, but this bit is set in Huntin'3
	{
		int split0 = m_rowscrollregs[0] | (m_rowscrollregs[1] << 8);
		int split1 = m_rowscrollregs[2] | (m_rowscrollregs[3] << 8);
		int split2 = m_rowscrollregs[4] | (m_rowscrollregs[5] << 8);
		int split3 = m_rowscrollregs[6] | (m_rowscrollregs[7] << 8);

		if (ydraw < m_rowscrollsplit[0])
		{
			return xscroll;
		}
		else if (ydraw < m_rowscrollsplit[1])
		{
			return split0;
		}
		else if (ydraw < m_rowscrollsplit[2])
		{
			return split1;
		}
		else if (ydraw < m_rowscrollsplit[3])
		{
			return split2;
		}
		else if (ydraw < m_rowscrollsplit[4])
		{
			return split3;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return xscroll;
	}
}


void radica_eu3a14_state::draw_background_page(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect, int ramstart, int ramend, int xbase, int ybase, int size, int bpp, int base, int pagewidth, int pageheight, int bytespertile, int palettepri, int drawfromram)
{

	int palette = ((palettepri & 0xf0) >> 4) | ((palettepri & 0x08) << 1);
	palette = palette << 4;
	int priority = palettepri & 0x07;

	int xdraw = xbase;
	int ydraw = ybase;
	int count = 0;

	for (int i = ramstart; i < ramend; i += bytespertile)
	{
		int tile = 0;
		int realpalette = palette;
		int realpriority = priority;
		int realbpp = bpp;

		if (bytespertile == 2)
		{
			tile = m_mainram[i + 0] | (m_mainram[i + 1] << 8);
		}
		else if (bytespertile == 4) // rad_foot hidden test mode, rad_hnt3 "Target Range" (not yet correct)
		{
			tile = m_mainram[i + 0] | (m_mainram[i + 1] << 8);// | (m_mainram[i + 2] << 16) |  | (m_mainram[i + 3] << 24);

			// m_mainram[i + 3] & 0x04 is set in both seen cases, maybe per-tile bpp?
			// this would match up with this mode being inline replacements for m_tilecfg[1] (palettepri) and m_tilecfg[2] (bpp);

			int newpalette = ((m_mainram[i + 2] & 0xf0) >> 4) | ((m_mainram[i + 2] & 0x08) << 1);
			newpalette = newpalette << 4;
			realpalette = newpalette;
			realpriority = m_mainram[i + 2] & 0x07;
			realbpp = m_mainram[i + 3] & 0x07;
			if (realbpp == 0)
				realbpp = 8;

		}


		draw_background_tile(bitmap, cliprect, realbpp, tile, realpalette, realpriority, 0, 0, xdraw, ydraw, 0, size, base, drawfromram);

		xdraw += size;

		count++;
		if (((count % pagewidth) == 0))
		{
			xdraw -= size * pagewidth;
			ydraw += size;
		}
	}
}

void radica_eu3a14_state::draw_background_ramlayer(screen_device& screen, bitmap_ind16& bitmap, const rectangle& cliprect)
{
	// this register use is questionable
	if (m_ramtilecfg[0] & 0x80)
	{
		int rtm_size;;
		int rtm_pagewidth;
		int rtm_pageheight;
		int rtm_yscroll;
		int rtm_bpp;
		int rtm_bytespertile = 2;
		uint8_t palettepri = m_ramtilecfg[1];

		rtm_yscroll = 0;

		// this is the gfxbase in ram for all cases seen
		int rtm_base = (0x2000 - 0x200) / 256;

		// same as regular layer?
		if (m_ramtilecfg[0] & 0x10)
		{
			rtm_size = 8;
			rtm_pagewidth = 32;
			rtm_pageheight = 28;
		}
		else
		{
			rtm_size = 16;
			rtm_pagewidth = 32 / 2;
			rtm_pageheight = 28 / 2;
		}

		rtm_bpp = m_ramtilecfg[2] & 0x07;
		if (rtm_bpp == 0)
			rtm_bpp = 8;

		// this is in the same place even when the first tilemap is in 16x16 mode, probably a base register somewhere
		int ramstart = m_tilerambase + 0x700;
		int ramend = m_tilerambase + 0x700 + 0x700;

		// hack for "Target Range" mode
		if (m_ramtilecfg[5] == 0x06)
		{
			ramstart = 0x3980-0x200;
			ramend = 0x3980-0x200 + 0x700;
		}

		// normal
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
		// wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (rtm_size * rtm_pagewidth), 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
		// wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, (rtm_size * rtm_pageheight) + 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
		// wrap x+y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (rtm_size * rtm_pagewidth), (rtm_size * rtm_pageheight) + 0 - rtm_yscroll, rtm_size, rtm_bpp, rtm_base, rtm_pagewidth, rtm_pageheight, rtm_bytespertile, palettepri, 1);
	}
}


void radica_eu3a14_state::draw_background(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int yscroll = m_scrollregs[2] | (m_scrollregs[3] << 8);

	int base = (m_tilecfg[5] << 8) | m_tilecfg[4];
	uint8_t palettepri = m_tilecfg[1];

	int pagewidth = 1, pageheight = 1;
	int bytespertile = 2;
	int size;

	// m_tilecfg[0]   b-as ?-hh    b = bytes per tile  s = tilesize / page size?  a = always set when tilemaps are in use - check? h = related to page positions, when set uses 2x2 pages? ? = used
	// m_tilecfg[1]   pppp x--?    ? = used foot x = used, huntin3 summary (palette bank?) p = palette (used for different stages in huntin3 and the hidden test modes in others)
	// m_tilecfg[2]   ---- bbbb    b = bpp mode (0 = 8bpp)
	// m_tilecfg[3]   ---- ----     (unused or more gfxbase?)
	// m_tilecfg[4]   gggg gggg     gfxbase (lower bits)
	// m_tilecfg[5]   gggg gggg     gfxbase (upper bits)

	// ramtilecfg appears to be a similar format, except for the other layer with ram base tiles
	// however 'a' in m_tilecfg[0] is NOT set
	// also m_tilecfg[0] has 0x80 set, which would be 4 bytes per tile, but it isn't?
	// the layer seems to be disabled by setting m_tilecfg[0] to 0?

	if (m_tilecfg[0] & 0x10)
	{
		size = 8;
		pagewidth = 32;
		pageheight = 28;
	}
	else
	{
		size = 16;
		pagewidth = 16;
		pageheight = 14;
	}

	if (m_tilecfg[0] & 0x80)
	{
		bytespertile = 4;
	}
	else
	{
		bytespertile = 2;
	}

	int bpp = (m_tilecfg[2] & 0x07);
	if (bpp == 0)
		bpp = 8;

	int ramstart = 0;
	int ramend = 0;

	int pagesize = pagewidth * pageheight * 2;

	if (bytespertile == 4)
	{
		pagesize <<= 1; // shift because we need twice as much ram for this mode
	}

	if ((m_tilecfg[0] & 0x03) == 0x00) // tilemaps arranged as 2x2 pages?
	{
		ramstart = m_tilerambase + pagesize * 0;
		ramend = m_tilerambase + pagesize * 1;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                        0 - yscroll,                          size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2),   0 - yscroll,                          size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                       (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2),  (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 1;
		ramend = m_tilerambase + pagesize * 2;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     0 - yscroll,                           size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), 0 - yscroll,                           size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight * 2) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 2;
		ramend = m_tilerambase + pagesize * 3;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                      (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2), (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, 0,                      (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 2), (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y

		ramstart = m_tilerambase + pagesize * 3;
		ramend = m_tilerambase + pagesize * 4;

		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // normal
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight) - yscroll,     size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth),     (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);// wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart,ramend, (size * pagewidth * 3), (size * pageheight * 3) - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0); // wrap x+y
	}
	else if ((m_tilecfg[0] & 0x03) == 0x03) // individual tilemaps? multiple layers?
	{
	//  popmessage("m_tilecfg[0] & 0x03 multiple layers config %04x", base);
		ramstart = m_tilerambase + pagesize * 0;
		ramend = m_tilerambase + pagesize * 1;

		// normal
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);
		// wrap x
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (size * pagewidth), 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);
		// wrap y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, 0, (size * pageheight) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);
		// wrap x+y
		draw_background_page(screen, bitmap, cliprect, ramstart, ramend, (size * pagewidth), (size * pageheight) + 0 - yscroll, size, bpp, base, pagewidth,pageheight, bytespertile, palettepri, 0);

		// RAM based tile layer
		draw_background_ramlayer(screen, bitmap, cliprect);
	}
	else
	{
		popmessage("m_tilecfg[0] & 0x03 unknown config");
	}

}

void radica_eu3a14_state::draw_sprite_pix(const rectangle& cliprect, uint16_t* dst, uint8_t* pridst, int realx, int priority, uint8_t pix, uint8_t mask, uint8_t shift, int palette)
{
	if (realx >= cliprect.min_x && realx <= cliprect.max_x)
	{
		if (pridst[realx] <= priority)
		{
			if (pix & mask)
			{
				dst[realx] = ((pix & mask) >> shift) | palette;
				pridst[realx] = priority;
			}
		}
	}
}

void radica_eu3a14_state::draw_sprite_line(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int offset, int line, int palette, int flipx, int priority, int xpos, int ypos, int bpp)
{
	offset = offset * 2;

	int bppdiv = 0;

	switch (bpp)
	{
	default:
	case 0x8:
	case 0x7:
	case 0x6:
	case 0x5:
		offset += line * 8;
		bppdiv = 1;
		break;

	case 0x4:
	case 0x3:
		offset += line * 4;
		bppdiv = 2;
		break;

	case 0x2:
		offset += line * 2;
		bppdiv = 4;
		break;

	case 0x1:
		offset += line * 1;
		bppdiv = 8;
		break;
	}

	uint8_t* gfxdata = &m_mainregion[offset & 0x3fffff];

	if (ypos >= cliprect.min_y && ypos <= cliprect.max_y)
	{
		uint16_t* dst = &bitmap.pix16(ypos);
		uint8_t* pridst = &m_prioritybitmap.pix8(ypos);

		int count = 0;
		for (int x = 0; x < 8/bppdiv;x++)
		{
			if (bpp == 8)
			{
				int pix,mask,shift;
				if (flipx) { pix = gfxdata[7 - count]; } else { pix = gfxdata[count]; }
				int realx = xpos + x * 1;
				if (flipx) { mask = 0xff; shift = 0; } else { mask = 0xff; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 7)
			{
				int pix,mask,shift;
				if (flipx) { pix = gfxdata[7 - count]; } else { pix = gfxdata[count]; }
				int realx = xpos + x * 1;
				// stride doesn't change, data isn't packed, just don't use top bit
				if (flipx) { mask = 0x7f; shift = 0; } else { mask = 0x7f; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 6)
			{
				popmessage("6bpp sprite\n");
			}
			else if (bpp == 5)
			{
				popmessage("5bpp sprite\n");
			}
			else if (bpp == 4)
			{
				int pix,mask,shift;
				if (flipx) { pix = gfxdata[3 - count]; } else { pix = gfxdata[count]; }
				int realx = xpos + x * 2;
				if (flipx) { mask = 0x0f; shift = 0; } else { mask = 0xf0; shift = 4; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0xf0; shift = 4; } else { mask = 0x0f; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 3)
			{
				popmessage("3bpp sprite\n");
			}
			else if (bpp == 2)
			{
				int pix,mask,shift;
				if (flipx) { pix = gfxdata[1 - count]; } else { pix = gfxdata[count]; }
				int realx = xpos + x * 4;
				if (flipx) { mask = 0x03; shift = 0; } else { mask = 0xc0; shift = 6; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0x0c; shift = 2; } else { mask = 0x30; shift = 4; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0x30; shift = 4; } else { mask = 0x0c; shift = 2; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
				realx++;
				if (flipx) { mask = 0xc0; shift = 6; } else { mask = 0x03; shift = 0; }
				draw_sprite_pix(cliprect, dst, pridst, realx, priority, pix, mask, shift, palette);
			}
			else if (bpp == 1)
			{
				popmessage("1bpp sprite\n");
			}

			count++;
		}
	}

}


void radica_eu3a14_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	// first 4 sprite entries seem to be garbage sprites, so we start at 0x20
	// likely we're just interpreting them wrong and they're used for blanking things or clipping?
	for (int i = m_spriterambase; i < m_spriterambase + 0x800; i += 8)
	{
		/*
		+0  e-ff hhww  flip yx, enable, height, width
		+1  yyyy yyyy  ypos
		+2  xxxx xxxx  xpos
		+3  pppp Pzzz  p = palette, P = upper palette bank, z = priority
		+4  tttt tttt  tile bits
		+5  tttt tttt
		+6  --TT TPPP  TTT = tile bank PPP = bpp select (+more?)
		+7  ---- ----

		*/

		int attr = m_mainram[i + 0];
		int y = m_mainram[i + 1];
		int x = m_mainram[i + 2];
		int palettepri = m_mainram[i + 3];

		int h = attr & 0x0c;
		int w = attr & 0x03;
		int flipx = (attr & 0x10) >> 4;
		int flipy = (attr & 0x20) >> 5;

		int height = 0;
		int width = 0;

		int pri = palettepri & 0x07;

		int palette = ((palettepri & 0xf0) >> 4) | ((palettepri & 0x08) << 1);
		palette = palette << 4;

		switch (h)
		{
		case 0x0:height = 2; break;
		case 0x4:height = 4; break;
		case 0x8:height = 8; break;
		case 0xc:height = 16; break;
		}

		switch (w)
		{
		case 0x0:width = 1; break;
		case 0x1:width = 2; break;
		case 0x2:width = 4; break;
		case 0x3:width = 8; break;
		}

		y -= ((height * 2) - 4);

		x -= ((width * 4) - 4);

		height *= 4;

		x -= 6;
		y -= 4;

		int offset = ((m_mainram[i + 5] << 8) + (m_mainram[i + 4] << 0));
		int extra = m_mainram[i + 6];

		int spritebase = (m_spritebase[1] << 8) | m_spritebase[0];

		offset += (extra & 0xf8) << 13;
		offset += spritebase << 7;

		int bpp = extra & 0x07;
		if (bpp == 0)
			bpp = 8;

		if (attr & 0x80)
		{
			int count = 0;
			for (int yy = 0; yy < height; yy++)
			{
				int yoff = flipy ? height-1-yy : yy;

				for (int xx = 0; xx < width; xx++)
				{
					int xoff = flipx ? (((width - 1) * 8) - (xx * 8)) : (xx * 8);

					draw_sprite_line(screen, bitmap, cliprect, offset, count, palette, flipx, pri, x + xoff, y + yoff, bpp);
					count++;
				}
			}
		}
	}
}



uint32_t radica_eu3a14_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_spriterambase = (m_spriteaddr[0] * 0x200) - 0x200;

	bitmap.fill(0, cliprect);
	m_prioritybitmap.fill(0, cliprect);

	handle_palette(screen, bitmap, cliprect);
	draw_background(screen, bitmap, cliprect);

	draw_sprites(screen, bitmap, cliprect);

	return 0;
}

// sound callback
READ8_MEMBER(radica_eu3a14_state::read_full_space)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	return fullbankspace.read_byte(offset);
}

// irq controller seems to be like the Radica Space Invaders
READ8_MEMBER(radica_eu3a14_state::irq_vector_r)
{
	if (m_custom_irq)
	{
		return m_custom_irq_vector >> (offset*8);
	}
	else
	{
		uint8_t *rom = memregion("maincpu")->base();
		return rom[0x001ffe + offset];
	}
}

/*
   code at 0000 maps to e000
   code at 1000 maps to f000

   data at 2000
   data at 3000
   data at 4000
   blank   5000
   blank   6000

   code at 7000 maps to 3000
   code at 8000 maps to 6000
           9000 maps to 7000
           a000 maps to 8000
           b000 maps to 9000
           c000 maps to a000
           d000 maps to b000
           e000 maps to c000
*/

WRITE8_MEMBER(radica_eu3a14_state::radicasi_rombank_hi_w)
{
	// written with the banking?
	//logerror("%s: radicasi_rombank_hi_w (set ROM bank) %02x\n", machine().describe_context(), data);
	m_rombank_hi = data;

	m_bank->set_bank(m_rombank_lo | (m_rombank_hi << 8));
}

WRITE8_MEMBER(radica_eu3a14_state::radicasi_rombank_lo_w)
{
	//logerror("%s: radicasi_rombank_lo_w (select ROM bank) %02x\n", machine().describe_context(), data);
	m_rombank_lo = data;
}

READ8_MEMBER(radica_eu3a14_state::radicasi_rombank_lo_r)
{
	return m_rombank_lo;
}

READ8_MEMBER(radica_eu3a14_state::radicasi_pal_ntsc_r)
{
	// how best to handle this, we probably need to run the PAL machine at 50hz
	// the text under the radica logo differs between regions
	logerror("%s: radicasi_pal_ntsc_r (region + more?)\n", machine().describe_context());
	return m_tvtype->read();
}

WRITE8_MEMBER(radica_eu3a14_state::porta_dir_w)
{
	m_portdir[0] = data;
	// update state
}

WRITE8_MEMBER(radica_eu3a14_state::portb_dir_w)
{
	m_portdir[1] = data;
	// update state
}

WRITE8_MEMBER(radica_eu3a14_state::portc_dir_w)
{
	m_portdir[2] = data;
	// update state
}

WRITE8_MEMBER(radica_eu3a14_state::porta_dat_w)
{
}

WRITE8_MEMBER(radica_eu3a14_state::portb_dat_w)
{
}

WRITE8_MEMBER(radica_eu3a14_state::portc_dat_w)
{
}



void radica_eu3a14_state::bank_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom().region("maincpu", 0);
}

void radica_eu3a14_state::radica_eu3a14_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	map(0x0200, 0x3fff).ram().share("mainram"); // 200-9ff is sprites? a00 - ??? is tilemap?

	map(0x4800, 0x4bff).ram().share("palram");

	// similar to eu3a05, at least for pal flags and rom banking
	// 5001 write
	// 5004 write
	// 5006 write
	map(0x5007, 0x5007).noprw();
	map(0x5008, 0x5008).nopw(); // startup (read too)
	map(0x5009, 0x5009).r(FUNC(radica_eu3a14_state::radica_5009_unk_r)); // rad_hnt3 polls this on startup
	map(0x500a, 0x500a).nopw(); // startup
	map(0x500b, 0x500b).r(FUNC(radica_eu3a14_state::radicasi_pal_ntsc_r)).nopw(); // PAL / NTSC flag at least
	map(0x500c, 0x500c).w(FUNC(radica_eu3a14_state::radicasi_rombank_hi_w));
	map(0x500d, 0x500d).rw(FUNC(radica_eu3a14_state::radicasi_rombank_lo_r), FUNC(radica_eu3a14_state::radicasi_rombank_lo_w));

	// DMA is similar to, but not the same as eu3a05
	map(0x500f, 0x5017).ram().share("dmaparams");
	map(0x5018, 0x5018).rw(FUNC(radica_eu3a14_state::dma_trigger_r), FUNC(radica_eu3a14_state::dma_trigger_w));
	// 5019 - 46 on startup (hnt3) 22 (bb3, foot) na (gtg) 09    (rsg)
	// 501a - 01 on startup (hnt3) 03 (bb3, foot) na (gtg) 02,01 (rsg)

	// probably GPIO like eu3a05, although it access 47/48 as unknown instead of 48/49/4a
	map(0x5040, 0x5040).w(FUNC(radica_eu3a14_state::porta_dir_w));
	map(0x5041, 0x5041).portr("IN0").w(FUNC(radica_eu3a14_state::porta_dat_w));
	map(0x5042, 0x5042).w(FUNC(radica_eu3a14_state::portb_dir_w));
	map(0x5043, 0x5043).portr("IN1").w(FUNC(radica_eu3a14_state::portb_dat_w));
	map(0x5044, 0x5044).w(FUNC(radica_eu3a14_state::portc_dir_w));
	map(0x5045, 0x5045).portr("IN2").w(FUNC(radica_eu3a14_state::portc_dat_w));

	map(0x5046, 0x5046).nopw();
	map(0x5047, 0x5047).nopw();
	map(0x5048, 0x5048).nopw();

	// 5060 - 506e  r/w during startup on foot

	// sound appears to be the same as eu3a05
	map(0x5080, 0x5091).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_addr_r), FUNC(radica6502_sound_device::radicasi_sound_addr_w));
	map(0x5092, 0x50a3).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_size_r), FUNC(radica6502_sound_device::radicasi_sound_size_w));
	map(0x50a4, 0x50a4).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_unk_r), FUNC(radica6502_sound_device::radicasi_sound_unk_w)); // read frequently on this
	map(0x50a5, 0x50a5).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_trigger_r), FUNC(radica6502_sound_device::radicasi_sound_trigger_w));
	map(0x50a6, 0x50a6).nopw(); // startup
	map(0x50a7, 0x50a7).nopw(); // startup
	map(0x50a8, 0x50a8).r("6ch_sound", FUNC(radica6502_sound_device::radicasi_50a8_r));
	map(0x50a9, 0x50a9).nopw(); // startup, read foot

	// video regs are in the 51xx range

	// huntin'3 seems to use some registers for a windowing / highlight effect on the trophy room names and "Target Range" mode timer??
	// 5100 - 0x0f when effect is enabled, 0x00 otherwise?
	// 5101 - 0x0e in both modes
	// 5102 - 0x86 in both modes
	// 5103 - 0x0e in tropy room (left?)                                  / 0x2a in "Target Range" mode (left position?)
	// 5104 - trophy room window / highlight top, move with cursor        / 0xbf in "Target Range" mode (top?)
	// 5105 - 0x52 in trophy room (right?)                                / counts from 0xa1 to 0x2a in "Target Range" mode (right position?)
	// 5106 - trophy room window / highlight bottom, move with cursor     / 0xcb in "Target Range" mode (bottom?)
	// 5107 - 0x00
	// 5108 - 0x04 in both modes
	// 5109 - 0xc2 in both modes

	map(0x5100, 0x5100).ram();
	map(0x5101, 0x5101).ram();
	map(0x5102, 0x5102).ram();
	map(0x5103, 0x5106).ram();
	map(0x5107, 0x5107).ram(); // on transitions, maybe layer disables?
	map(0x5108, 0x5109).ram(); // hnt3, frequently rewrites same values, maybe something to do with raster irq?

	// layer specific regs?
	map(0x5110, 0x5115).ram().share("tilecfg");
	map(0x5116, 0x5117).ram().share("rowscrollcfg"); // 00 01 in hnt3 (could just be extra tile config bits, purpose guessed)   set to 00 05 in rad_gtg overhead part (no rowscroll)
	//  0x5118, 0x5119  not used
	map(0x511a, 0x511e).ram().share("rowscrollsplit"); // hnt3 (60 68 78 90 b8 - rowscroll position list see note below

	// register value notes for 511a-511e and how they relate to screen
	// 00-6f normal scroll reg
	// 60-67 is where the first extra scroll reg (rowscrollregs) is onscreen
	// 68-77 is the 2nd
	// 78-8f is the 3rd
	// 90-b7 is the 4th
	// b8-ff no scroll?

	map(0x5121, 0x5124).ram().share("scrollregs");
	map(0x5125, 0x512c).ram().share("rowscrollregs"); // 4 extra x scroll regs

	// layer specific regs?
	map(0x5140, 0x5145).ram().share("ramtilecfg"); // hnt3
	map(0x5148, 0x514b).ram(); // hnt3 (always 0 tho?)

	// sprite specific regs?
	map(0x5150, 0x5150).ram().share("spriteaddr"); // startup 01 bb3,gtg,rsg, (na) foot 0c hnt3
	map(0x5151, 0x5152).ram().share("spritebase");
	map(0x5153, 0x5153).ram(); // startup

	map(0x6000, 0xdfff).m(m_bank, FUNC(address_map_bank_device::amap8));

	map(0xe000, 0xffff).rom().region("maincpu", 0x0000);

	map(0xfffe, 0xffff).r(FUNC(radica_eu3a14_state::irq_vector_r));
}

READ8_MEMBER(radica_eu3a14_state::dma_trigger_r)
{
	logerror("%s: dma_trigger_r\n", machine().describe_context());
	return 0;
}

WRITE8_MEMBER(radica_eu3a14_state::dma_trigger_w)
{
	uint32_t dmasrc = (m_dmaparams[2] << 16) | (m_dmaparams[1] << 8) | (m_dmaparams[0] << 0);
	uint32_t dmadst = (m_dmaparams[5] << 16) | (m_dmaparams[4] << 8) | (m_dmaparams[3] << 0);
	uint32_t dmalen = (m_dmaparams[8] << 16) | (m_dmaparams[7] << 8) | (m_dmaparams[6] << 0);

	//logerror("%s: dma_trigger_w %02x (src %08x dst %08x size %08x)\n", machine().describe_context(), data, dmasrc, dmadst, dmalen);

	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	address_space& destspace = m_maincpu->space(AS_PROGRAM);

	if (data == 0x08)
	{
		for (int i = 0; i < dmalen; i++)
		{
			uint8_t dat = fullbankspace.read_byte(dmasrc + i);
			destspace.write_byte(dmadst + i, dat);
		}
	}
	else
	{
		logerror("UNKNOWN DMA TRIGGER VALUE\n");
	}
}


static INPUT_PORTS_START( rad_gtg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) // back / backspin
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // up and down in the menus should be the trackball
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x08, 0x08, "Track Y test" ) // trackball up/down direction bit? (read in interrupt, increases / decreases a counter)
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x20, 0x20, "Track X test" ) // trackball left / right direction bit? (read in interrupt, increases / decreases a counter)
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("TV")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( rad_rsg ) // base unit just has 4 directions + enter and a sensor to swing the club over
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) // aiming
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // select in menus?
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) // previous in menus?
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // next in menus?
	PORT_DIPNAME( 0x20, 0x20, "IN0" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TV")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( rad_rsgp )
	PORT_INCLUDE(rad_rsg)

	PORT_MODIFY("TV")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( radica_foot )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) // enter?
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TV")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_hnt3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Menu Previous")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Menu Next")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Menu Select")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON4 ) // pause?

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Fire Gun") // maybe
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Safety") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON7 ) PORT_NAME("Fire Gun (alt)") // maybe
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TV")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_hnt3p )
	PORT_INCLUDE(radica_hnt3)

	PORT_MODIFY("TV")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( radica_bask )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x08, 0x08, "IN0" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TV")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_baskp )
	PORT_INCLUDE(radica_bask)

	PORT_MODIFY("TV")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_bb3 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("TV")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( radica_bb3p )
	PORT_INCLUDE(radica_bb3)

	PORT_MODIFY("TV")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

void radica_eu3a14_state::machine_start()
{
}

void radica_eu3a14_state::machine_reset()
{
	// rather be safe
	m_maincpu->set_state_int(M6502_S, 0x1ff);

	m_bank->set_bank(0x01);

	m_portdir[0] = 0x00;
	m_portdir[1] = 0x00;
	m_portdir[2] = 0x00;

	m_spriteaddr[0] = 0x14; // ?? rad_foot never writes, other games seem to use it to set sprite location
}


TIMER_DEVICE_CALLBACK_MEMBER(radica_eu3a14_state::scanline_cb)
{
	// these interrupts need to occur based on how fast the trackball is
	// being moved, the direction is read in a port.

	int scanline = param;

	if (scanline == 20)
	{
		// vertical trackball
		m_custom_irq_vector = 0xffe0;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);
	}

	if (scanline == 40)
	{
		// horizontal trackball
		m_custom_irq_vector = 0xffe4;
		m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);
	}
}

INTERRUPT_GEN_MEMBER(radica_eu3a14_state::interrupt)
{
	m_custom_irq = 1;
	m_custom_irq_vector = 0xffd4;
	m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);
}


// background
static const gfx_layout helper16x16x8_layout =
{
	16,16,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP16(0,8) },
	{ STEP16(0,16*8) },
	16 * 16 * 8
};

static const gfx_layout helper16x16x4_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP16(0,4) },
	{ STEP16(0,16*4) },
	16 * 16 * 4
};

static const gfx_layout helper8x8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	{ STEP8(0,8*8) },
	8 * 8 * 8
};

static const gfx_layout helper8x8x4_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,8*4)  },
	8 * 8 * 4
};


static GFXDECODE_START( gfx_helper )
	// dummy standard decodes to see background tiles, not used for drawing
	GFXDECODE_ENTRY( "maincpu", 0, helper16x16x8_layout,  0x0, 2  )
	GFXDECODE_ENTRY( "maincpu", 0, helper16x16x4_layout,  0x0, 32  )
	GFXDECODE_ENTRY( "maincpu", 0, helper8x8x8_layout,    0x0, 2  )
	GFXDECODE_ENTRY( "maincpu", 0, helper8x8x4_layout,    0x0, 32  )
GFXDECODE_END



void radica_eu3a14_state::radica_eu3a14(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(21'477'272)/2); // marked as 21'477'270
	m_maincpu->set_addrmap(AS_PROGRAM, &radica_eu3a14_state::radica_eu3a14_map);
	m_maincpu->set_vblank_int("screen", FUNC(radica_eu3a14_state::interrupt));

	ADDRESS_MAP_BANK(config, "bank").set_map(&radica_eu3a14_state::bank_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x8000);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_helper);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(radica_eu3a14_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(512);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	radica6502_sound_device &sound(RADICA6502_SOUND(config, "6ch_sound", 8000));
	sound.space_read_callback().set(FUNC(radica_eu3a14_state::read_full_space));
	sound.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void radica_eu3a14_state::radica_eu3a14_adc(machine_config &config)
{
	radica_eu3a14(config);

	TIMER(config, "scantimer").configure_scanline(FUNC(radica_eu3a14_state::scanline_cb), "screen", 0, 1);
}


void radica_eu3a14_state::radica_eu3a14p(machine_config &config) // TODO, clocks differ too, what are they on PAL?
{
	radica_eu3a14(config);

	subdevice<screen_device>("screen")->set_refresh_hz(50);
}


void radica_eu3a14_state::init_rad_gtg()
{
	// must be registers to control this
	m_tilerambase = 0x0a00 - 0x200;
}

void radica_eu3a14_state::init_rad_foot()
{
	// must be registers to control this
	m_tilerambase = 0x0200 - 0x200;
}

void radica_eu3a14_state::init_rad_hnt3()
{
	// must be registers to control this
	m_tilerambase = 0x0200 - 0x200;
}


ROM_START( rad_gtg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "goldentee.bin", 0x000000, 0x400000, CRC(2d6cdb85) SHA1(ce6ed39d692ff16ea407f39c37b6e731f952b9d5) )
ROM_END

ROM_START( rad_rsg )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "realswinggolf.bin", 0x000000, 0x400000, CRC(89e5b6a6) SHA1(0b14aa84d7e7ae7190cd64e3eb125de2104342bc) )
ROM_END

ROM_START( rad_rsgp )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "realswinggolf.bin", 0x000000, 0x400000, CRC(89e5b6a6) SHA1(0b14aa84d7e7ae7190cd64e3eb125de2104342bc) )
ROM_END


ROM_START( rad_foot )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "connectvfootball.bin", 0x000000, 0x400000, CRC(00ac4fc0) SHA1(2b60ae5c6bc7e9ef7cdbd3f6a0a0657ed3ab5afe) )
ROM_END

ROM_START( rad_bb3 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "baseball3.bin", 0x000000, 0x400000, CRC(af86aab0) SHA1(5fed48a295f045ca839f87b0f9b78ecc51104cdc) )
ROM_END

ROM_START( rad_bb3p )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "baseball3.bin", 0x000000, 0x400000, CRC(af86aab0) SHA1(5fed48a295f045ca839f87b0f9b78ecc51104cdc) )
ROM_END

ROM_START( rad_hnt3 )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "huntin3.bin", 0x000000, 0x400000, CRC(c8e3e40b) SHA1(81eb16ac5ab6d93525fcfadbc6703b2811d7de7f) )
ROM_END

ROM_START( rad_hnt3p )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "huntin3.bin", 0x000000, 0x400000, CRC(c8e3e40b) SHA1(81eb16ac5ab6d93525fcfadbc6703b2811d7de7f) )
ROM_END

ROM_START( rad_bask )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "basketball.bin", 0x000000, 0x400000, CRC(7d6ff53c) SHA1(1c75261d55e0107a3b8e8d4c1eb2854750f2d0e8) )
ROM_END

ROM_START( rad_baskp )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "basketball.bin", 0x000000, 0x400000, CRC(7d6ff53c) SHA1(1c75261d55e0107a3b8e8d4c1eb2854750f2d0e8) )
ROM_END

CONS( 2006, rad_gtg,  0,        0, radica_eu3a14_adc, rad_gtg,       radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios (licensed from Incredible Technologies)", "Golden Tee Golf: Home Edition", MACHINE_NOT_WORKING )

CONS( 2005, rad_rsg,  0,        0, radica_eu3a14,     rad_rsg,       radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios",                                         "Play TV Real Swing Golf", MACHINE_NOT_WORKING )
// some Connectv branded Real Swing Golf units have a language selection, so there are likely other PAL revisions of this
CONS( 2005, rad_rsgp, rad_rsg,  0, radica_eu3a14p,    rad_rsgp,      radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios",                                         "Connectv Real Swing Golf", MACHINE_NOT_WORKING )

// also has a Connectv Real Soccer logo in the roms, apparently unused, maybe that was to be the US title (without the logo being changed to Play TV) but Play TV Soccer ended up being a different game licensed from Epoch instead.
CONS( 2006, rad_foot, 0,        0, radica_eu3a14p,    radica_foot,   radica_eu3a14_state, init_rad_foot, "Radica / Medialink",                                                "Connectv Football", MACHINE_NOT_WORKING )

CONS( 2005, rad_bb3,  0,        0, radica_eu3a14,     radica_bb3,    radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios",                                         "Play TV Baseball 3", MACHINE_NOT_WORKING )
CONS( 2005, rad_bb3p, rad_bb3,  0, radica_eu3a14p,    radica_bb3p,   radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios",                                         "Connectv Baseball 3", MACHINE_NOT_WORKING )

CONS( 2005, rad_hnt3, 0,        0, radica_eu3a14,     radica_hnt3,   radica_eu3a14_state, init_rad_hnt3, "Radica / V-Tac Technology Co Ltd.",                                 "Play TV Huntin' 3", MACHINE_NOT_WORKING )
CONS( 2005, rad_hnt3p,rad_hnt3, 0, radica_eu3a14p,    radica_hnt3p,  radica_eu3a14_state, init_rad_hnt3, "Radica / V-Tac Technology Co Ltd.",                                 "Connectv Huntin' 3", MACHINE_NOT_WORKING )

CONS( 2005, rad_bask, 0,        0, radica_eu3a14,     radica_bask,   radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios",                                         "Play TV Basketball", MACHINE_NOT_WORKING )
CONS( 2005, rad_baskp,rad_bask, 0, radica_eu3a14,     radica_baskp,  radica_eu3a14_state, init_rad_gtg,  "Radica / FarSight Studios",                                         "Connectv Basketball", MACHINE_NOT_WORKING )
