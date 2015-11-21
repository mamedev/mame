// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/mikromik.h"

#define HORIZONTAL_CHARACTER_PIXELS 10

//-------------------------------------------------
//  i8275 crtc display pixels
//-------------------------------------------------

I8275_DRAW_CHARACTER_MEMBER( mm1_state::crtc_display_pixels )
{
	UINT8 romdata = m_char_rom->base()[(charcode << 4) | linecount];

	int gpa0 = BIT(gpa, 0);     // general purpose attribute 0
	int llen = m_llen;          // light enable
	int compl_in = rvv;         // reverse video
	int hlt_in = hlgt;          // highlight;
	int color;                  // 0 = black, 1 = dk green, 2 = lt green; on MikroMikko 1, "highlight" is actually the darker shade of green
	int i, qh, video_in;

	int d7 = BIT(romdata, 7);   // save MSB (1 indicates that this is a Visual Attribute or Special Code instead of a normal display character)
	int d6 = BIT(romdata, 6);   // save also first and last char bitmap bits before shifting out the MSB
	int d0 = BIT(romdata, 0);
	UINT8 data = (romdata << 1) | (d7 & d0); // get rid of MSB, duplicate LSB for special characters

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

static ADDRESS_MAP_START( mm1_upd7220_map, AS_0, 16, mm1_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7fff)
	AM_RANGE(0x0000, 0x7fff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END


//-------------------------------------------------
//  UPD7220_INTERFACE( hgdc_intf )
//-------------------------------------------------

UPD7220_DISPLAY_PIXELS_MEMBER( mm1_state::hgdc_display_pixels )
{
	UINT16 data = m_video_ram[address >> 1];
	for (int i = 0; i < 16; i++)
	{
		if (BIT(data, i)) bitmap.pix32(y, x + i) = m_palette->pen(1);
	}
}


UINT32 mm1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

static GFXDECODE_START( mm1 )
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 1 )
GFXDECODE_END


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( mm1m6_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( mm1m6_video )
	MCFG_SCREEN_ADD( SCREEN_TAG, RASTER )
	MCFG_SCREEN_REFRESH_RATE( 50 )
	MCFG_SCREEN_UPDATE_DRIVER(mm1_state, screen_update)
	MCFG_SCREEN_SIZE( 800, 375 ) // (25 text rows * 15 vertical pixels / character)
	MCFG_SCREEN_VISIBLE_AREA( 0, 800-1, 0, 375-1 )
	//MCFG_SCREEN_RAW_PARAMS(XTAL_18_720MHz, ...)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mm1)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN_HIGHLIGHT("palette")

	MCFG_DEVICE_ADD(I8275_TAG, I8275, XTAL_18_720MHz/8)
	MCFG_I8275_CHARACTER_WIDTH(HORIZONTAL_CHARACTER_PIXELS)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(mm1_state, crtc_display_pixels)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE(I8237_TAG, am9517a_device, dreq0_w))
	MCFG_I8275_VRTC_CALLBACK(DEVWRITELINE(UPD7220_TAG, upd7220_device, ext_sync_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_DEVICE_ADD(UPD7220_TAG, UPD7220, XTAL_18_720MHz/8)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mm1_upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(mm1_state, hgdc_display_pixels)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
MACHINE_CONFIG_END
