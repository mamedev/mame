// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "emu.h"
#include "includes/mikromik.h"
#include "screen.h"

#define HORIZONTAL_CHARACTER_PIXELS 10

//-------------------------------------------------
//  i8275 crtc display pixels
//-------------------------------------------------

I8275_DRAW_CHARACTER_MEMBER( mm1_state::crtc_display_pixels )
{
	uint8_t romdata = m_char_rom->base()[(charcode << 4) | linecount];

	int gpa0 = BIT(gpa, 0);     // general purpose attribute 0
	int llen = m_llen;          // light enable
	int compl_in = rvv;         // reverse video
	int hlt_in = hlgt;          // highlight;
	int color;                  // 0 = black, 1 = dk green, 2 = lt green; on MikroMikko 1, "highlight" is actually the darker shade of green
	int i, qh, video_in;

	int d7 = BIT(romdata, 7);   // save MSB (1 indicates that this is a Visual Attribute or Special Code instead of a normal display character)
	int d6 = BIT(romdata, 6);   // save also first and last char bitmap bits before shifting out the MSB
	int d0 = BIT(romdata, 0);
	uint8_t data = (romdata << 1) | (d7 & d0); // get rid of MSB, duplicate LSB for special characters

	if (y < 360 || x >= HORIZONTAL_CHARACTER_PIXELS || compl_in == 0) // leftmost char on the 25th row is never displayed on actual MikroMikko 1 HW if it's inversed
	{
		if (HORIZONTAL_CHARACTER_PIXELS == 10)
		{
			// Hack to stretch 8 pixels wide character bitmap to 10 pixels on screen.
			// This was needed because high res graphics use 800 pixels wide bitmap but
			// 80 chars * 8 pixels is only 640 -> characters would cover only 80% of the screen width.
			// Step 1: Instead of 8, set MCFG_I8275_CHARACTER_WIDTH(10) at the end of this file
			// Step 2: Make sure i8275_device::recompute_parameters() is called in i8275_device::device_start()
			// Step 3: Fill in missing 2 pixels in the screen bitmap by repeating last column of the char bitmap
			// (works better with MikroMikko 1 font than duplicating the first and the last column)
			qh = d7 & d6; // extend pixels on the right side only if there were two adjacent ones before shifting out the MSB
			video_in = ((((d7 & llen) | (vsp ? 0 : 1)) & (gpa0 ? 0 : 1)) & qh) | lten;
			color = (hlt_in ? 1 : 2) * (video_in ^ compl_in);
			bitmap.pix32(y, x + 8) = m_palette->pen(color);
			bitmap.pix32(y, x + 9) = m_palette->pen(color);
		}

		for (i = 0; i < 8; ++i) // ...and now the actual character bitmap bits for this scanline
		{
			qh = BIT(data, i);
			video_in = ((((d7 & llen) | (vsp ? 0 : 1)) & (gpa0 ? 0 : 1)) & qh) | lten;
			color = (hlt_in ? 1 : 2)*(video_in ^ compl_in);
			bitmap.pix32(y, x + i) = m_palette->pen(color);
		}
	}
}


//-------------------------------------------------
//  ADDRESS_MAP( mm1_upd7220_map )
//-------------------------------------------------

void mm1_state::mm1_upd7220_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x7fff).ram().share("video_ram");
}


//-------------------------------------------------
//  UPD7220 callbacks
//-------------------------------------------------

UPD7220_DISPLAY_PIXELS_MEMBER( mm1_state::hgdc_display_pixels )
{
	uint16_t data = m_video_ram[address >> 1];
	for (int i = 0; i < 16; i++)
	{
		if (BIT(data, i)) bitmap.pix32(y, x + i) = m_palette->pen(2);
	}
}


uint32_t mm1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* text */
	m_crtc->screen_update(screen, bitmap, cliprect);

	/* graphics */
	m_hgdc->screen_update(screen, bitmap, cliprect);

	return 0;
}


//-------------------------------------------------
//  gfx_layout charlayout
//-------------------------------------------------

static const gfx_layout charlayout =
{
	8, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{  0*8,  1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
		8*8,  9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};


//-------------------------------------------------
//  GFXDECODE( mm1 )
//-------------------------------------------------

static GFXDECODE_START( gfx_mm1 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 1 )
GFXDECODE_END

void mm1_state::mm1_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));
	palette.set_pen_color(1, rgb_t(0x00, 0x7f, 0x0a)); // dark green ("highlight" mode color)
	palette.set_pen_color(2, rgb_t(0x08, 0xd0, 0x1a)); // bright green (normal color)
}


//-------------------------------------------------
//  machine_config( mm1m6_video )
//-------------------------------------------------

void mm1_state::mm1m6_video(machine_config &config)
{
	screen_device &screen(SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz( 50 );
	screen.set_screen_update(FUNC(mm1_state::screen_update));
	screen.set_size( 800, 375 ); // (25 text rows * 15 vertical pixels / character)
	screen.set_visarea( 0, 800-1, 0, 375-1 );
	//screen.set_raw(XTAL(18'720'000), ...);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_mm1);
	PALETTE(config, m_palette, FUNC(mm1_state::mm1_palette), 3);

	I8275(config, m_crtc, XTAL(18'720'000)/8);
	m_crtc->set_character_width(HORIZONTAL_CHARACTER_PIXELS);
	m_crtc->set_display_callback(FUNC(mm1_state::crtc_display_pixels), this);
	m_crtc->drq_wr_callback().set(m_dmac, FUNC(am9517a_device::dreq0_w));
	m_crtc->vrtc_wr_callback().set(m_hgdc, FUNC(upd7220_device::ext_sync_w));
	m_crtc->set_screen("screen");

	UPD7220(config, m_hgdc, XTAL(18'720'000)/8);
	m_hgdc->set_addrmap(0, &mm1_state::mm1_upd7220_map);
	m_hgdc->set_display_pixels(FUNC(mm1_state::hgdc_display_pixels));
	m_hgdc->set_screen(SCREEN_TAG);
}
