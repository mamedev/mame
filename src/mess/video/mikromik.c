// license:BSD-3-Clause
// copyright-holders:Curt Coder
#include "includes/mikromik.h"



//-------------------------------------------------
//  i8275 crtc display pixels
//-------------------------------------------------

I8275_DRAW_CHARACTER_MEMBER( mm1_state::crtc_display_pixels )
{
	UINT8 romdata = m_char_rom->base()[(charcode << 4) | linecount];

	int d0 = BIT(romdata, 0);
	int d7 = BIT(romdata, 7);
	int gpa0 = BIT(gpa, 0);
	int llen = m_llen;
	int i;

	UINT8 data = (romdata << 1) | (d7 & d0);

	for (i = 0; i < 8; i++)
	{
		int qh = BIT(data, i);
		int video_in = ((((d7 & llen) | !vsp) & !gpa0) & qh) | lten;
		int compl_in = rvv;
		int hlt_in = hlgt;

		int color = hlt_in ? 2 : (video_in ^ compl_in);

		bitmap.pix32(y, x + i) = m_palette->pen(color);
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
	MCFG_SCREEN_SIZE( 800, 400 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 800-1, 0, 400-1 )
	//MCFG_SCREEN_RAW_PARAMS(XTAL_18_720MHz, ...)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mm1)
	MCFG_PALETTE_ADD_MONOCHROME_GREEN_HIGHLIGHT("palette")

	MCFG_DEVICE_ADD(I8275_TAG, I8275, XTAL_18_720MHz/8)
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(mm1_state, crtc_display_pixels)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE(I8237_TAG, am9517a_device, dreq0_w))
	MCFG_I8275_VRTC_CALLBACK(DEVWRITELINE(UPD7220_TAG, upd7220_device, ext_sync_w))
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)

	MCFG_DEVICE_ADD(UPD7220_TAG, UPD7220, XTAL_18_720MHz/8)
	MCFG_DEVICE_ADDRESS_MAP(AS_0, mm1_upd7220_map)
	MCFG_UPD7220_DISPLAY_PIXELS_CALLBACK_OWNER(mm1_state, hgdc_display_pixels)
	MCFG_VIDEO_SET_SCREEN(SCREEN_TAG)
MACHINE_CONFIG_END
