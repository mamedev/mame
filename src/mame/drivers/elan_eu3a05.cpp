// license:BSD-3-Clause
// copyright-holders:David Haywood, R.Belmont

/*
    Radica Games 6502 based 'TV Game' hardware

    These use a 6502 derived CPU under a glob
    The CPU die is marked 'ELAN EU3A05'

    There is a second glob surrounded by TSOP48 pads
    this contains the ROM

    Space Invaders uses a 3rd glob marked
    AMIC (C) (M) 1998-1 AM3122A
    this is presumably for the bitmap layer on Qix

    --
    Known games on this hardware

    Tetris
    Space Invaders

    Possible other games on this hardawre

    ConnecTV Football (aka ConnecTV International Football)
     - Soccer game, not the same as Play TV Football, or Play TV Soccer
       (it has the same XTAL etc. as this driver at least)

    ---
    The XaviX ones seem to have a XaviX logo on the external packaging while the
    ones for this driver don't seem to have any specific marking.


    Notes:
    To access internal test on Tetris hold P1 Down + P1 Anticlockwise (Button 2) on boot
    There appears to be a similar mode for Invaders but I don't know if it's accessible


    RAM 0xa0 and 0xa1 contain the ACD0 and AD1 values and player 2 controls if between
    certain values? probably read via serial??

    Custom Interrupt purposes

    TETRIS

    ffb0
    nothing of note?

    ffb4
    stuff with 500e, 500c and 500d

    ffb8
    stuff with 50a4 / 50a5 / 50a6  and memory address e2

    ffbc
    stuff with 50a4 / 50a5 / 50a6  and memory address e2 (similar to above, different bits)

    ffc0 - doesn't exist
    ffc4 - doesn't exist
    ffc8 - doesn't exist
    ffd0 - doesn't exist

    ffd4
    main irq?

    ffd8
    jumps straight to an rti

    ffdc
    accesses 501d / 501b

    SPACE INVADERS

    ffb0
    rti

    ffb4
    rti

    ffb8
    rti

    ffbc
    decreases 301  bit 02
    stuff wit 50a5

    ffc0
    decreases 302
    stuff with 50a5 bit 04

    ffc4
    decreases 303
    stuff with 50a5  bit 08

    ffc8
    decreases 304
    stuff with 50a5  bit 10

    ffcc
    uses 307
    stuff with 50a5  bit 20

    ffd0
    dead loop

    ffd4
    main interrupt

    ffd8
    dead loop

    ffdc
    dead loop

    ffe0
    dead loop

    ffe4
    rti

    ffe8
    dead loop

    ffec
    dead loop


    Flaws (NOT emulation bugs, happen on hardware):
    --

    In QIX the sprites lag behind the line drawing, so you see the line infront of your player until you stop moving

    In Space Invaders the UFO can sometimes glitch for a frame when appearing, and wraps around at the edges
      (even if the hardware supports having higher priority tiles to prevent this, as used by Lunar Rescue, it isn't
       used here)

    Colony 7 has a typo in the instructions

    The fake 'colour band' effect does not apply to the thruster (and several other elements) in Lunar Rescue

    Enemies in Phoenix are rendered above the score panel

    The 200pt right facing bird on the Phoenix score table is corrupt

    Uncertain (to check)

    Space Invaders seems to be using a darker than expected palette, there are lighter colours in the palette but
    they don't seem to be used.  It's difficult to judge from hardware videos, although it definitely isn't as
    white as the menu, so this might also be a non-bug.


*/

#include "emu.h"
#include "cpu/m6502/m6502.h"
//#include "cpu/m6502/m65c02.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/bankdev.h"
#include "audio/elan_eu3a05.h"
#include "machine/elan_eu3a05gpio.h"

class radica_eu3a05_state : public driver_device
{
public:
	radica_eu3a05_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, "ram"),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_palram(*this, "palram"),
		m_pixram(*this, "pixram"),
		m_bank(*this, "bank"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void radicasi(machine_config &config);

private:
	// screen updates
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	// system
	DECLARE_READ8_MEMBER(radicasi_5003_r);
	DECLARE_READ8_MEMBER(radicasi_pal_ntsc_r);
	DECLARE_READ8_MEMBER(radicasi_rombank_lo_r);
	DECLARE_WRITE8_MEMBER(radicasi_rombank_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_rombank_hi_w);

	// DMA
	DECLARE_WRITE8_MEMBER(radicasi_dmasrc_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_dmasrc_md_w);
	DECLARE_WRITE8_MEMBER(radicasi_dmasrc_hi_w);
	DECLARE_READ8_MEMBER(radicasi_dmasrc_lo_r);
	DECLARE_READ8_MEMBER(radicasi_dmasrc_md_r);
	DECLARE_READ8_MEMBER(radicasi_dmasrc_hi_r);
	DECLARE_WRITE8_MEMBER(radicasi_dmadst_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_dmadst_hi_w);
	DECLARE_READ8_MEMBER(radicasi_dmadst_lo_r);
	DECLARE_READ8_MEMBER(radicasi_dmadst_hi_r);
	DECLARE_WRITE8_MEMBER(radicasi_dmasize_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_dmasize_hi_w);
	DECLARE_READ8_MEMBER(radicasi_dmasize_lo_r);
	DECLARE_READ8_MEMBER(radicasi_dmasize_hi_r);
	DECLARE_READ8_MEMBER(radicasi_dmatrg_r);
	DECLARE_WRITE8_MEMBER(radicasi_dmatrg_w);

	// VIDEO
	// tile bases
	DECLARE_WRITE8_MEMBER(radicasi_tile_gfxbase_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_tile_gfxbase_hi_w);
	DECLARE_READ8_MEMBER(radicasi_tile_gfxbase_lo_r);
	DECLARE_READ8_MEMBER(radicasi_tile_gfxbase_hi_r);
	// sprite tile bases
	DECLARE_WRITE8_MEMBER(radicasi_sprite_gfxbase_lo_w);
	DECLARE_WRITE8_MEMBER(radicasi_sprite_gfxbase_hi_w);
	DECLARE_READ8_MEMBER(radicasi_sprite_gfxbase_lo_r);
	DECLARE_READ8_MEMBER(radicasi_sprite_gfxbase_hi_r);

	DECLARE_WRITE8_MEMBER(radicasi_vidctrl_w);

	DECLARE_READ8_MEMBER(radicasi_sprite_bg_scroll_r);
	DECLARE_WRITE8_MEMBER(radicasi_sprite_bg_scroll_w);

	// more sound regs?
	DECLARE_READ8_MEMBER(radicasi_50a9_r);
	DECLARE_WRITE8_MEMBER(radicasi_50a9_w);

	INTERRUPT_GEN_MEMBER(interrupt);

	DECLARE_READ8_MEMBER(radicasi_nmi_vector_r);
	DECLARE_READ8_MEMBER(radicasi_irq_vector_r);

	// for callback
	DECLARE_READ8_MEMBER(read_full_space);

	void radicasi_bank_map(address_map &map);
	void radicasi_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_ram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_palram;
	required_shared_ptr<uint8_t> m_pixram;
	required_device<address_map_bank_device> m_bank;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t m_rombank_hi;
	uint8_t m_rombank_lo;
	uint8_t m_vidctrl;
	uint8_t m_50a9_data;

	uint8_t m_dmasrc_lo_data;
	uint8_t m_dmasrc_md_data;
	uint8_t m_dmasrc_hi_data;
	uint8_t m_dmadst_lo_data;
	uint8_t m_dmadst_hi_data;
	uint8_t m_dmasize_lo_data;
	uint8_t m_dmasize_hi_data;

	uint8_t m_tile_gfxbase_lo_data;
	uint8_t m_tile_gfxbase_hi_data;

	uint8_t m_sprite_gfxbase_lo_data;
	uint8_t m_sprite_gfxbase_hi_data;

	uint8_t m_bg_scroll[2];

	int m_custom_irq;
	int m_custom_nmi;
	uint16_t m_custom_irq_vector;
	uint16_t m_custom_nmi_vector;

	bool get_tile_data(int base, int drawpri, int& tile, int &attr, int &unk2);
	void draw_tilemaps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int drawpri);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

void radica_eu3a05_state::video_start()
{
}

/* (m_tile_gfxbase_lo_data | (m_tile_gfxbase_hi_data << 8)) * 0x100
   gives you the actual rom address, everything references the 3MByte - 4MByte region, like the banking so
   the system can probably have up to a 4MByte rom, all games we have so far just use the upper 1MByte of
   that space (Tetris seems to rely on mirroring? as it sets all addresses up for the lower 1MB instead)
*/

void radica_eu3a05_state::draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);

	/*
	    Sprites
	    AA yy xx cc XX YY aa bb

	    yy = y position
	    xx = x position
	    XX = texture x start
	    YY = texture y start
	    aa = same as unk2 on tiles
	    bb = sometimes set in invaders
	    cc = same as attr on tiles (colour / priority?)

	    AA = attributes
	    e--- fFsS
	    e = enable
	    S = SizeX
	    s = SizeY
	    F = FlipX
	    f = FlipY (assumed, not seen)

	*/

	for (int i = 0; i < 512; i += 8)
	{
		uint8_t x = m_spriteram[i + 2];
		uint8_t y = m_spriteram[i + 1];

		/*
		   Space Invaders draws the player base with this specific y value before the start of each life
		   and expects it to NOT wrap around.  There are no high priority tiles or anything else to hide
		   and it doesn't appear on real hardware.

		   it's possible sprites don't wrap around at all (but then you couldn't have smooth entry at the
		   top of the screen - there are no extra y co-ordinate bits.  However there would have been easier
		   ways to hide this tho as there are a bunch of unseen lines at the bottom of the screen anyway!

		   needs further investigation.
		*/
		if (y==255)
			continue;

		uint8_t tex_x = m_spriteram[i + 4];
		uint8_t tex_y = m_spriteram[i + 5];

		uint8_t flags = m_spriteram[i + 0];
		uint8_t attr = m_spriteram[i + 3];
		uint8_t unk2 = m_spriteram[i + 6];


		//int priority = attr & 0x0f;
		int colour = attr & 0xf0;

		// ? game select has this set to 0xff, but clearly doesn't want the palette to change!
		// while in Space Invaders this has to be palette for the UFO to be red.
		if (colour & 0x80)
			colour = 0;

		int transpen = 0;

		 /* HACK - how is this calculated
		   phoenix and the game select need it like this
		   it isn't a simple case of unk2 being transpen either because Qix has some elements set to 0x07
		   where the transpen needs to be 0x00 and Space Invaders has it set to 0x04
		   it could be a global register rather than something in the spritelist?
		*/
		if ((attr == 0xff) && (unk2 == 0xff))
			transpen = 0xff;


		if (!(flags & 0x80))
			continue;

		int sizex = 8;
		int sizey = 8;

		if (flags & 0x01)
		{
			sizex = 16;
		}

		if (flags & 0x02)
		{
			sizey = 16;
		}

		int base = (m_sprite_gfxbase_lo_data | (m_sprite_gfxbase_hi_data << 8)) * 0x100;

		for (int yy = 0; yy < sizey; yy++)
		{
			uint16_t* row;

			if (flags & 0x08) // guess flipy
			{
				row = &bitmap.pix16((y + (sizey - 1 - yy)) & 0xff);
			}
			else
			{
				row = &bitmap.pix16((y + yy) & 0xff);
			}

			for (int xx = 0; xx < sizex; xx++)
			{
				int realaddr = base + ((tex_x + xx) & 0xff);
				realaddr += ((tex_y + yy) & 0xff) * 256;

				uint8_t pix = fullbankspace.read_byte(realaddr);

				if (pix != transpen)
				{
					if (flags & 0x04) // flipx
					{
						row[(x + (sizex - 1 - xx)) & 0xff] = (pix + ((colour & 0x70) << 1)) & 0xff;
					}
					else
					{
						row[(x + xx) & 0xff] = (pix + ((colour & 0x70) << 1)) & 0xff;
					}
				}
			}
		}
	}
}

double hue2rgb(double p, double q, double t)
{
	if (t < 0) t += 1;
	if (t > 1) t -= 1;
	if (t < 1 / 6.0f) return p + (q - p) * 6 * t;
	if (t < 1 / 2.0f) return q;
	if (t < 2 / 3.0f) return p + (q - p) * (2 / 3.0f - t) * 6;
	return p;
}

// a hacky mess for now
bool radica_eu3a05_state::get_tile_data(int base, int drawpri, int& tile, int &attr, int &unk2)
{
	tile = m_vram[base * 4] + (m_vram[(base * 4) + 1] << 8);

	// these seem to be the basically the same as attr/unk2 in the sprites, which also make
	// very little sense.
	attr = m_vram[(base * 4) + 2];
	unk2 = m_vram[(base * 4) + 3];

	/* hack for phoenix title screens.. the have attr set to 0x3f which change the colour bank in ways we don't want
	   and also by our interpretation of 0x0f bits sets the tiles to priority over sprites (although in this case
	   that might tbe ok, because it looks like the sprites also have that set */
	if (unk2 == 0x07)
		attr = 0;

	int priority = attr & 0x0f;

	// likely wrong
	if ((drawpri == 0 && priority == 0x0f) || (drawpri == 1 && priority != 0x0f))
		return false;

	return true;
}
void radica_eu3a05_state::draw_tilemaps(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int drawpri)
{
	/* this doesn't handle 8x8 4bpp, haven't seen it used
	   nor is there support for horizontal scrolling etc.
	   for the same reasons
	*/

	int scroll = (m_bg_scroll[1] << 8) | m_bg_scroll[0];
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);

	if (m_vidctrl & 0x40) // 16x16 tiles
	{
		int startrow = (scroll >> 4) & 0x1f;

		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				int realstartrow = (startrow + y);

				if (realstartrow >= 28)
					realstartrow -= 28;

				int base = (((realstartrow + y) & 0x1f) * 8) + x;

				int tile,attr,unk2;

				if (!get_tile_data(base, drawpri, tile,attr,unk2))
					continue;

				int colour = attr & 0xf0;

				if (m_vidctrl & 0x20) // 4bpp mode
				{
					tile = (tile & 0xf) + ((tile & ~0xf) * 16);
					tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);
				}
				else
				{
					tile = (tile & 0xf) + ((tile & ~0xf) * 16);
					tile <<= 1;

					tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);
				}

				for (int i = 0; i < 16; i++)
				{
					int drawline = (y * 16) + i;
					drawline -= scroll & 0xf;

					if ((drawline >= 0) && (drawline < 256))
					{
						uint16_t* row = &bitmap.pix16(drawline);

						if (m_vidctrl & 0x20) // 4bpp
						{
							for (int xx = 0; xx < 16; xx += 2)
							{
								int realaddr = ((tile + i * 16) << 3) + (xx >> 1);
								uint8_t pix = fullbankspace.read_byte(realaddr);
								row[x * 16 + xx + 0] = ((pix & 0xf0) >> 4) + colour;
								row[x * 16 + xx + 1] = ((pix & 0x0f) >> 0) + colour;
							}
						}
						else // 8bpp
						{
							for (int xx = 0; xx < 16; xx++)
							{
								int realaddr = ((tile + i * 32) << 3) + xx;
								uint8_t pix = fullbankspace.read_byte(realaddr);
								row[x * 16 + xx] = (pix + ((colour & 0x70) << 1)) & 0xff;
							}
						}

					}
				}
			}
		}
	}
	else // 8x8 tiles
	{
		// Phoenix scrolling actually skips a pixel, jumping from 0x001 to 0x1bf, scroll 0x000 isn't used, maybe it has other meanings?

		int startrow = (scroll >> 3) & 0x3f;

		for (int y = 0; y < 32; y++)
		{
			for (int x = 0; x < 32; x++)
			{
				int realstartrow = (startrow + y);

				if (realstartrow >= 56)
					realstartrow -= 56;

				int base = (((realstartrow) & 0x3f) * 32) + x;

				int tile,attr,unk2;

				if (!get_tile_data(base, drawpri, tile,attr,unk2))
					continue;

				int colour = attr & 0xf0;

				tile = (tile & 0x1f) + ((tile & ~0x1f) * 8);
				tile += ((m_tile_gfxbase_lo_data | m_tile_gfxbase_hi_data << 8) << 5);

				for (int i = 0; i < 8; i++)
				{
					int drawline = (y * 8) + i;
					drawline -= scroll & 0x7;

					if ((drawline >= 0) && (drawline < 256))
					{
						uint16_t* row = &bitmap.pix16(drawline);

						for (int xx = 0; xx < 8; xx++)
						{
							int realaddr = ((tile + i * 32) << 3) + xx;
							uint8_t pix = fullbankspace.read_byte(realaddr);
							row[x * 8 + xx] = (pix + ((colour & 0x70) << 1)) & 0xff;
						}
					}
				}
			}
		}
	}
}

uint32_t radica_eu3a05_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// Palette
	int offs = 0;
	for (int index = 0;index < 256; index++)
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
		} else {
			double q = l < 0.5f ? l * (1 + s) : l + s - l * s;
			double p = 2 * l - q;
			r = hue2rgb(p, q, h + 1/3.0f);
			g = hue2rgb(p, q, h);
			b = hue2rgb(p, q, h - 1/3.0f);
		}

		int r_real = r * 255.0f;
		int g_real = g * 255.0f;
		int b_real = b * 255.0f;

		m_palette->set_pen_color(index, r_real, g_real, b_real);
	}


	draw_tilemaps(screen,bitmap,cliprect,0);
	draw_sprites(screen,bitmap,cliprect);
	draw_tilemaps(screen,bitmap,cliprect,1);

	return 0;
}

// System control

WRITE8_MEMBER(radica_eu3a05_state::radicasi_rombank_hi_w)
{
	// written with the banking?
	//logerror("%s: radicasi_rombank_hi_w (set ROM bank) %02x\n", machine().describe_context(), data);
	m_rombank_hi = data;

	m_bank->set_bank(m_rombank_lo | (m_rombank_hi << 8));
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_rombank_lo_w)
{
	//logerror("%s: radicasi_rombank_lo_w (select ROM bank) %02x\n", machine().describe_context(), data);
	m_rombank_lo = data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_rombank_lo_r)
{
	return m_rombank_lo;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_pal_ntsc_r)
{
	// how best to handle this, we probably need to run the PAL machine at 50hz
	// the text under the radica logo differs between regions
	logerror("%s: radicasi_pal_ntsc_r (region + more?)\n", machine().describe_context());
	return 0xff; // NTSC
	//return 0x00; // PAL
}

READ8_MEMBER(radica_eu3a05_state::radicasi_5003_r)
{
	/* masked with 0x0f, 0x01 or 0x03 depending on situation..

	  I think it might just be an RNG because if you return 0x00
	  your shots can never hit the stage 3 enemies in Phoenix and
	  if you return 0xff they always hit.  On the real hardware it
	  seems to be random.  Could also just be a crude frame counter
	  used for the same purpose.

	*/

	logerror("%s: radicasi_5003_r (RNG?)\n", machine().describe_context());

	return machine().rand();
}


// Video device

// Tile bases

WRITE8_MEMBER(radica_eu3a05_state::radicasi_tile_gfxbase_lo_w)
{
	//logerror("%s: radicasi_tile_gfxbase_lo_w (select GFX base lower) %02x\n", machine().describe_context(), data);
	m_tile_gfxbase_lo_data = data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_tile_gfxbase_hi_w)
{
	//logerror("%s: radicasi_tile_gfxbase_hi_w (select GFX base upper) %02x\n", machine().describe_context(), data);
	m_tile_gfxbase_hi_data = data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_tile_gfxbase_lo_r)
{
	//logerror("%s: radicasi_tile_gfxbase_lo_r (GFX base lower)\n", machine().describe_context());
	return m_tile_gfxbase_lo_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_tile_gfxbase_hi_r)
{
	//logerror("%s: radicasi_tile_gfxbase_hi_r (GFX base upper)\n", machine().describe_context());
	return m_tile_gfxbase_hi_data;
}

// Sprite Tile bases

WRITE8_MEMBER(radica_eu3a05_state::radicasi_sprite_gfxbase_lo_w)
{
	//logerror("%s: radicasi_sprite_gfxbase_lo_w (select Sprite GFX base lower) %02x\n", machine().describe_context(), data);
	m_sprite_gfxbase_lo_data = data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_sprite_gfxbase_hi_w)
{
	//logerror("%s: radicasi_sprite_gfxbase_hi_w (select Sprite GFX base upper) %02x\n", machine().describe_context(), data);
	m_sprite_gfxbase_hi_data = data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_sprite_gfxbase_lo_r)
{
	//logerror("%s: radicasi_sprite_gfxbase_lo_r (Sprite GFX base lower)\n", machine().describe_context());
	return m_sprite_gfxbase_lo_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_sprite_gfxbase_hi_r)
{
	//logerror("%s: radicasi_sprite_gfxbase_hi_r (Sprite GFX base upper)\n", machine().describe_context());
	return m_sprite_gfxbase_hi_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_sprite_bg_scroll_r)
{
	return m_bg_scroll[offset];

}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_sprite_bg_scroll_w)
{
	m_bg_scroll[offset] = data;
}


WRITE8_MEMBER(radica_eu3a05_state::radicasi_vidctrl_w)
{
	logerror("%s: radicasi_vidctrl_w %02x (video control?)\n", machine().describe_context(), data);
	/*
	    c3  8bpp 16x16         1100 0011
	    e3  4bpp 16x16         1110 0011
	    83  8bpp 8x8           1000 0011
	    02  8bpp 8x8 (phoenix) 0000 0010
	*/
	m_vidctrl = data;
}

// DMA device

WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmasrc_lo_w)
{
	logerror("%s: radicasi_dmasrc_lo_w (select DMA source low) %02x\n", machine().describe_context(), data);
	m_dmasrc_lo_data = data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmasrc_md_w)
{
	logerror("%s: radicasi_dmasrc_md_w (select DMA source middle) %02x\n", machine().describe_context(), data);
	m_dmasrc_md_data = data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmasrc_hi_w)
{
	logerror("%s: radicasi_dmasrc_hi_w (select DMA source upper) %02x\n", machine().describe_context(), data);
	m_dmasrc_hi_data = data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmasrc_lo_r)
{
	logerror("%s: radicasi_dmasrc_lo_r (DMA source low)\n", machine().describe_context());
	return m_dmasrc_lo_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmasrc_md_r)
{
	logerror("%s: radicasi_dmasrc_md_r (DMA source middle)\n", machine().describe_context());
	return m_dmasrc_md_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmasrc_hi_r)
{
	logerror("%s: radicasi_dmasrc_hi_r (DMA source upper)\n", machine().describe_context());
	return m_dmasrc_hi_data;
}



WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmadst_lo_w)
{
	logerror("%s: radicasi_dmadst_lo_w (select DMA Dest lower) %02x\n", machine().describe_context(), data);
	m_dmadst_lo_data = data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmadst_hi_w)
{
	logerror("%s: radicasi_dmadst_hi_w (select DMA Dest upper) %02x\n", machine().describe_context(), data);
	m_dmadst_hi_data = data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmadst_lo_r)
{
	logerror("%s: radicasi_dmadst_lo_r (DMA Dest lower)\n", machine().describe_context());
	return m_dmadst_lo_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmadst_hi_r)
{
	logerror("%s: radicasi_dmadst_hi_r (DMA Dest upper)\n", machine().describe_context());
	return m_dmadst_hi_data;
}


WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmasize_lo_w)
{
	logerror("%s: radicasi_dmasize_lo_w (select DMA Size lower) %02x\n", machine().describe_context(), data);
	m_dmasize_lo_data = data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmasize_hi_w)
{
	logerror("%s: radicasi_dmasize_hi_w (select DMA Size upper) %02x\n", machine().describe_context(), data);
	m_dmasize_hi_data = data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmasize_lo_r)
{
	logerror("%s: radicasi_dmasize_lo_r (DMA Size lower)\n", machine().describe_context());
	return m_dmasize_lo_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmasize_hi_r)
{
	logerror("%s: radicasi_dmasize_hi_r (DMA Size upper)\n", machine().describe_context());
	return m_dmasize_hi_data;
}

READ8_MEMBER(radica_eu3a05_state::radicasi_dmatrg_r)
{
	logerror("%s: radicasi_dmatrg_r (DMA operation state?)\n", machine().describe_context());
	return 0x00;//m_dmatrg_data;
}


WRITE8_MEMBER(radica_eu3a05_state::radicasi_dmatrg_w)
{
	logerror("%s: radicasi_dmatrg_w (trigger DMA operation) %02x\n", machine().describe_context(), data);
	//m_dmatrg_data = data;

	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	address_space& destspace = m_maincpu->space(AS_PROGRAM);

	if (data)
	{
		int src = (m_dmasrc_lo_data << 0) | (m_dmasrc_md_data << 8) | (m_dmasrc_hi_data << 16);
		uint16_t dest = (m_dmadst_lo_data | (m_dmadst_hi_data << 8));
		uint16_t size = (m_dmasize_lo_data | (m_dmasize_hi_data << 8));

		logerror(" Doing DMA %06x to %04x size %04x\n", src, dest, size);

		for (int i = 0; i < size; i++)
		{
			uint8_t dat = fullbankspace.read_byte(src + i);
			destspace.write_byte(dest + i, dat);
		}
	}
}




// probably also sound device, maybe for forcing channels to stop?
READ8_MEMBER(radica_eu3a05_state::radicasi_50a9_r)
{
	logerror("%s: radicasi_50a9_r\n", machine().describe_context());
	return m_50a9_data;
}

WRITE8_MEMBER(radica_eu3a05_state::radicasi_50a9_w)
{
	logerror("%s: radicasi_50a9_w %02x\n", machine().describe_context(), data);
	m_50a9_data = data;
}

// sound callback
READ8_MEMBER(radica_eu3a05_state::read_full_space)
{
	address_space& fullbankspace = m_bank->space(AS_PROGRAM);
	return fullbankspace.read_byte(offset);
}

void radica_eu3a05_state::radicasi_map(address_map &map)
{
	// can the addresses move around?
	map(0x0000, 0x05ff).ram().share("ram");
	map(0x0600, 0x3dff).ram().share("vram");
	map(0x3e00, 0x3fff).ram().share("spriteram");
	map(0x4800, 0x49ff).ram().share("palram");

	// 500x system regs?
	map(0x5003, 0x5003).r(FUNC(radica_eu3a05_state::radicasi_5003_r));
	map(0x500b, 0x500b).r(FUNC(radica_eu3a05_state::radicasi_pal_ntsc_r)); // PAL / NTSC flag at least
	map(0x500c, 0x500c).w(FUNC(radica_eu3a05_state::radicasi_rombank_hi_w));
	map(0x500d, 0x500d).rw(FUNC(radica_eu3a05_state::radicasi_rombank_lo_r), FUNC(radica_eu3a05_state::radicasi_rombank_lo_w));

	// 501x DMA controller
	map(0x500F, 0x500F).rw(FUNC(radica_eu3a05_state::radicasi_dmasrc_lo_r), FUNC(radica_eu3a05_state::radicasi_dmasrc_lo_w));
	map(0x5010, 0x5010).rw(FUNC(radica_eu3a05_state::radicasi_dmasrc_md_r), FUNC(radica_eu3a05_state::radicasi_dmasrc_md_w));
	map(0x5011, 0x5011).rw(FUNC(radica_eu3a05_state::radicasi_dmasrc_hi_r), FUNC(radica_eu3a05_state::radicasi_dmasrc_hi_w));

	map(0x5012, 0x5012).rw(FUNC(radica_eu3a05_state::radicasi_dmadst_lo_r), FUNC(radica_eu3a05_state::radicasi_dmadst_lo_w));
	map(0x5013, 0x5013).rw(FUNC(radica_eu3a05_state::radicasi_dmadst_hi_r), FUNC(radica_eu3a05_state::radicasi_dmadst_hi_w));

	map(0x5014, 0x5014).rw(FUNC(radica_eu3a05_state::radicasi_dmasize_lo_r), FUNC(radica_eu3a05_state::radicasi_dmasize_lo_w));
	map(0x5015, 0x5015).rw(FUNC(radica_eu3a05_state::radicasi_dmasize_hi_r), FUNC(radica_eu3a05_state::radicasi_dmasize_hi_w));

	map(0x5016, 0x5016).rw(FUNC(radica_eu3a05_state::radicasi_dmatrg_r), FUNC(radica_eu3a05_state::radicasi_dmatrg_w));

	// 502x - 503x video regs area?
	map(0x5020, 0x5026).ram(); // unknown, space invaders sets these to fixed values, tetris has them as 00
	map(0x5027, 0x5027).w(FUNC(radica_eu3a05_state::radicasi_vidctrl_w));

	map(0x5029, 0x5029).rw(FUNC(radica_eu3a05_state::radicasi_tile_gfxbase_lo_r), FUNC(radica_eu3a05_state::radicasi_tile_gfxbase_lo_w)); // tilebase
	map(0x502a, 0x502a).rw(FUNC(radica_eu3a05_state::radicasi_tile_gfxbase_hi_r), FUNC(radica_eu3a05_state::radicasi_tile_gfxbase_hi_w)); // tilebase

	map(0x502b, 0x502b).rw(FUNC(radica_eu3a05_state::radicasi_sprite_gfxbase_lo_r), FUNC(radica_eu3a05_state::radicasi_sprite_gfxbase_lo_w)); // tilebase (spr?)
	map(0x502c, 0x502c).rw(FUNC(radica_eu3a05_state::radicasi_sprite_gfxbase_hi_r), FUNC(radica_eu3a05_state::radicasi_sprite_gfxbase_hi_w)); // tilebase (spr?)

	map(0x5031, 0x5032).rw(FUNC(radica_eu3a05_state::radicasi_sprite_bg_scroll_r), FUNC(radica_eu3a05_state::radicasi_sprite_bg_scroll_w));


	// 504x GPIO area?
	map(0x5040, 0x5046).rw("gpio", FUNC(radica6502_gpio_device::gpio_r), FUNC(radica6502_gpio_device::gpio_w));
	map(0x5048, 0x504a).w("gpio", FUNC(radica6502_gpio_device::gpio_unk_w));

	// 506x unknown
	map(0x5060, 0x506d).ram(); // read/written by tetris

	// 508x sound
	map(0x5080, 0x5091).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_addr_r), FUNC(radica6502_sound_device::radicasi_sound_addr_w));
	map(0x5092, 0x50a3).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_size_r), FUNC(radica6502_sound_device::radicasi_sound_size_w));
	map(0x50a4, 0x50a4).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_unk_r), FUNC(radica6502_sound_device::radicasi_sound_unk_w));
	map(0x50a5, 0x50a5).rw("6ch_sound", FUNC(radica6502_sound_device::radicasi_sound_trigger_r), FUNC(radica6502_sound_device::radicasi_sound_trigger_w));

	map(0x50a8, 0x50a8).r("6ch_sound", FUNC(radica6502_sound_device::radicasi_50a8_r)); // possible 'stopped' status of above channels, waits for it to be 0x3f in places

	map(0x50a9, 0x50a9).rw(FUNC(radica_eu3a05_state::radicasi_50a9_r), FUNC(radica_eu3a05_state::radicasi_50a9_w));

	//AM_RANGE(0x5000, 0x50ff) AM_RAM

	map(0x6000, 0xdfff).m(m_bank, FUNC(address_map_bank_device::amap8));

	map(0xe000, 0xffff).rom().region("maincpu", 0x3f8000);
	// not sure how these work, might be a modified 6502 core instead.
	map(0xfffa, 0xfffb).r(FUNC(radica_eu3a05_state::radicasi_nmi_vector_r));
	map(0xfffe, 0xffff).r(FUNC(radica_eu3a05_state::radicasi_irq_vector_r));
}


void radica_eu3a05_state::radicasi_bank_map(address_map &map)
{
	map(0x000000, 0xffffff).noprw(); // shut up any logging when video params are invalid
	map(0x000000, 0x3fffff).rom().region("maincpu", 0);
	map(0x400000, 0x40ffff).ram(); // ?? only ever cleared maybe a mirror of below?
	map(0x800000, 0x80ffff).ram().share("pixram"); // Qix writes here and sets the tile base here instead of ROM so it can have a pixel layer
}

static INPUT_PORTS_START( rad_sinv )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) // MENU
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( rad_tetr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Anticlockwise
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Clockwise
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) // and Select
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	/* Player 2 inputs must be read via serial or similar
	   the game doesn't read them directly, or even let
	   you select player 2 mode by default
	*/
INPUT_PORTS_END

/* both NMI and IRQ vectors just point to RTI
    there is a table of jumps just before that, those appear to be the real interrupt functions?

    patch the main IRQ to be the one that decreases an address the code is waiting for
    the others look like they might be timer service routines
*/

READ8_MEMBER(radica_eu3a05_state::radicasi_nmi_vector_r)
{
	if (m_custom_nmi)
	{
		return m_custom_nmi_vector >> (offset*8);
	}
	else
	{
		uint8_t *rom = memregion("maincpu")->base();
		return rom[0x3f9ffa + offset];
	}
}

READ8_MEMBER(radica_eu3a05_state::radicasi_irq_vector_r)
{
	if (m_custom_irq)
	{
		return m_custom_irq_vector >> (offset*8);
	}
	else
	{
		uint8_t *rom = memregion("maincpu")->base();
		return rom[0x3f9ffe + offset];
	}
}

void radica_eu3a05_state::machine_start()
{
	m_custom_irq = 0;
	m_custom_irq_vector = 0x0000;

	m_custom_nmi = 0;
	m_custom_nmi_vector = 0x0000;

	m_bank->set_bank(0x7f);
}

void radica_eu3a05_state::machine_reset()
{
	/* the 6502 core sets the default stack value to 0x01bd
	   and Tetris does not initialize it to anything else

	   Tetris stores the playfield data at 0x100 - 0x1c7 and
	   has a clear routine that will erase that range and
	   trash the stack

	   It seems likely this 6502 sets it to 0x1ff by default
	   at least.

	   According to
	   http://mametesters.org/view.php?id=6486
	   this isn't right for known 6502 types either
	*/
	m_maincpu->set_state_int(M6502_S, 0x1ff);
}

static const gfx_layout helper_4bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	{ STEP8(0,4) },
	{ 0 },
	8 * 4
};

static const gfx_layout helper_8bpp_8_layout =
{
	8,1,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ STEP8(0,8) },
	{ 0 },
	8 * 8
};


// these are fake just to make looking at the texture pages easier
static const uint32_t texlayout_xoffset_8bpp[256] = { STEP256(0,8) };
static const uint32_t texlayout_yoffset_8bpp[256] = { STEP256(0,256*8) };
static const gfx_layout texture_helper_8bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*8,
	texlayout_xoffset_8bpp,
	texlayout_yoffset_8bpp
};

static const uint32_t texlayout_xoffset_4bpp[256] = { STEP256(0,4) };
static const uint32_t texlayout_yoffset_4bpp[256] = { STEP256(0,256*4) };
static const gfx_layout texture_helper_4bpp_layout =
{
	256, 256,
	RGN_FRAC(1,1),
	4,
	{ 0,1,2,3 },
	EXTENDED_XOFFS,
	EXTENDED_YOFFS,
	256*256*4,
	texlayout_xoffset_4bpp,
	texlayout_yoffset_4bpp
};

static GFXDECODE_START( gfx_radicasi_fake )
	GFXDECODE_ENTRY( "maincpu", 0, helper_4bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_4bpp_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, helper_8bpp_8_layout,  0x0, 1  )
	GFXDECODE_ENTRY( "maincpu", 0, texture_helper_8bpp_layout,  0x0, 1  )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(radica_eu3a05_state::interrupt)
{
	m_custom_irq = 1;
	m_custom_irq_vector = 0xffd4;

	m_maincpu->set_input_line(INPUT_LINE_IRQ0,HOLD_LINE);
	/*
	m_custom_nmi = 1;
	m_custom_nmi_vector = 0xffd4;

	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	*/
}

void radica_eu3a05_state::radicasi(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, XTAL(21'281'370)/2); // Tetris has a XTAL(21'281'370), not confirmed on Space Invaders, actual CPU clock unknown.
	m_maincpu->set_addrmap(AS_PROGRAM, &radica_eu3a05_state::radicasi_map);
	m_maincpu->set_vblank_int("screen", FUNC(radica_eu3a05_state::interrupt));

	ADDRESS_MAP_BANK(config, "bank").set_map(&radica_eu3a05_state::radicasi_bank_map).set_options(ENDIANNESS_LITTLE, 8, 24, 0x8000);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(radica_eu3a05_state::screen_update));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 28*8-1);
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(256);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_radicasi_fake);

	radica6502_gpio_device &gpio(RADICA6502_GPIO(config, "gpio", 0));
	gpio.read_0_callback().set_ioport("IN0");

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	radica6502_sound_device &sound(RADICA6502_SOUND(config, "6ch_sound", 8000));
	sound.space_read_callback().set(FUNC(radica_eu3a05_state::read_full_space));
	sound.add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( rad_tetr )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "tetrisrom.bin", 0x000000, 0x100000, CRC(40538e08) SHA1(1aef9a2c678e39243eab8d910bb7f9f47bae0aee) )
	ROM_RELOAD(0x100000, 0x100000)
	ROM_RELOAD(0x200000, 0x100000)
	ROM_RELOAD(0x300000, 0x100000)
ROM_END

ROM_START( rad_sinv )
	ROM_REGION( 0x400000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "spaceinvadersrom.bin", 0x000000, 0x100000, CRC(5ffb2c8f) SHA1(9bde42ec5c65d9584a802de7d7c8b842ebf8cbd8) )
	ROM_RELOAD(0x100000, 0x100000)
	ROM_RELOAD(0x200000, 0x100000)
	ROM_RELOAD(0x300000, 0x100000)
ROM_END

CONS( 2004, rad_sinv, 0, 0, radicasi, rad_sinv, radica_eu3a05_state, empty_init, "Radica (licensed from Taito)",                      "Space Invaders [Lunar Rescue, Colony 7, Qix, Phoenix] (Radica, Arcade Legends TV Game)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND ) // "5 Taito games in 1"
CONS( 2004, rad_tetr, 0, 0, radicasi, rad_tetr, radica_eu3a05_state, empty_init, "Radica (licensed from Elorg / The Tetris Company)", "Tetris (Radica, Arcade Legends TV Game)", MACHINE_NOT_WORKING ) // "5 Tetris games in 1"
