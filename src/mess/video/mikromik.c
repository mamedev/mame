#include "includes/mikromik.h"



//-------------------------------------------------
//  PALETTE_INIT( mm1 )
//-------------------------------------------------

PALETTE_INIT_MEMBER(mm1_state,mm1)
{
	palette_set_color(machine(), 0, RGB_BLACK); /* black */
	palette_set_color_rgb(machine(), 1, 0x00, 0xc0, 0x00); /* green */
	palette_set_color_rgb(machine(), 2, 0x00, 0xff, 0x00); /* bright green */
}


//-------------------------------------------------
//  i8275_interface crtc_intf
//-------------------------------------------------

static I8275_DISPLAY_PIXELS( crtc_display_pixels )
{
	mm1_state *state = device->machine().driver_data<mm1_state>();

	UINT8 romdata = state->m_char_rom[(charcode << 4) | linecount];

	int d0 = BIT(romdata, 0);
	int d7 = BIT(romdata, 7);
	int gpa0 = BIT(gpa, 0);
	int llen = state->m_llen;
	int i;

	UINT8 data = (romdata << 1) | (d7 & d0);

	for (i = 0; i < 8; i++)
	{
		int qh = BIT(data, i);
		int video_in = ((((d7 & llen) | !vsp) & !gpa0) & qh) | lten;
		int compl_in = rvv;
		int hlt_in = hlgt;

		int color = hlt_in ? 2 : (video_in ^ compl_in);

		state->m_bitmap.pix16(y, x + i) = color;
	}
}

static const i8275_interface crtc_intf =
{
	SCREEN_TAG,
	8,
	0,
	DEVCB_DEVICE_LINE_MEMBER(I8237_TAG, am9517a_device, dreq0_w),
	DEVCB_NULL,
	crtc_display_pixels
};


//-------------------------------------------------
//  ADDRESS_MAP( mm1_upd7220_map )
//-------------------------------------------------

static ADDRESS_MAP_START( mm1_upd7220_map, AS_0, 8, mm1_state )
	AM_RANGE(0x00000, 0x3ffff) AM_RAM AM_SHARE("video_ram")
ADDRESS_MAP_END


//-------------------------------------------------
//  UPD7220_INTERFACE( hgdc_intf )
//-------------------------------------------------

static UPD7220_DISPLAY_PIXELS( hgdc_display_pixels )
{
	mm1_state *state = device->machine().driver_data<mm1_state>();

	UINT8 data = state->m_video_ram[address * 2];

	for (int i = 0; i < 8; i++)
	{
		if (BIT(data, i)) bitmap.pix16(y, x + i) = 1;
	}
}

static UPD7220_INTERFACE( hgdc_intf )
{
	SCREEN_TAG,
	hgdc_display_pixels,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};


//-------------------------------------------------
//  VIDEO_START( mm1 )
//-------------------------------------------------

void mm1_state::video_start()
{
	// find memory regions
	m_char_rom = memregion("chargen")->base();

	machine().primary_screen->register_screen_bitmap(m_bitmap);
}


//-------------------------------------------------
//  SCREEN_UPDATE_IND16( mm1 )
//-------------------------------------------------

UINT32 mm1_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	/* text */
	i8275_update(m_crtc, bitmap, cliprect);
	copybitmap(bitmap, m_bitmap, 0, 0, 0, 0, cliprect);

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
	GFXDECODE_ENTRY( "chargen", 0, charlayout, 0, 0x100 )
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

	MCFG_GFXDECODE(mm1)
	MCFG_PALETTE_LENGTH(3)
	MCFG_PALETTE_INIT_OVERRIDE(mm1_state,mm1)

	MCFG_I8275_ADD(I8275_TAG, crtc_intf)
	MCFG_UPD7220_ADD(UPD7220_TAG, XTAL_18_720MHz/8, hgdc_intf, mm1_upd7220_map)
MACHINE_CONFIG_END
