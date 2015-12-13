// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * aga.c
 *
 ****************************************************************************/
#include "aga.h"
#include "video/cgapal.h"

#define CGA_HCLK (XTAL_14_31818MHz/8)
#define CGA_LCLK (XTAL_14_31818MHz/16)

#define AGA_SCREEN_NAME "screen"
#define AGA_MC6845_NAME "mc6845_aga"

enum
{
	MDA_TEXT_INTEN = 0,
	MDA_TEXT_BLINK,
	CGA_TEXT_INTEN,
	CGA_TEXT_INTEN_ALT,
	CGA_TEXT_BLINK,
	CGA_TEXT_BLINK_ALT,
	CGA_GFX_1BPP,
	CGA_GFX_2BPP,
	CGA_GFX_4BPPL,
	CGA_GFX_4BPPH
};

static INPUT_PORTS_START( aga )
	PORT_START( "cga_config" )
	PORT_CONFNAME( 0x03, 0x00, "CGA character set")
	PORT_CONFSETTING(0x00, DEF_STR( Normal ))
	PORT_CONFSETTING(0x01, "Alternative")
	PORT_CONFNAME( 0x1C, 0x00, "CGA monitor type")
	PORT_CONFSETTING(0x00, "Colour RGB")
	PORT_CONFSETTING(0x04, "Mono RGB")
	PORT_CONFSETTING(0x08, "Colour composite")
	PORT_CONFSETTING(0x0C, "Television")
	PORT_CONFSETTING(0x10, "LCD")
	PORT_CONFNAME( 0xE0, 0x00, "CGA chipset")
	PORT_CONFSETTING(0x00, "IBM")
	PORT_CONFSETTING(0x20, "Amstrad PC1512")
	PORT_CONFSETTING(0x40, "Amstrad PPC512")
	PORT_CONFSETTING(0x60, "ATI")
	PORT_CONFSETTING(0x80, "Paradise")
INPUT_PORTS_END

#define CGA_MONITOR     (m_cga_config->read()&0x1C)
#define CGA_MONITOR_COMPOSITE   0x08    /* Colour composite */

const device_type ISA8_AGA = &device_creator<isa8_aga_device>;

//-------------------------------------------------
//  isa8_aga_device - constructor
//-------------------------------------------------

isa8_aga_device::isa8_aga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t( mconfig, ISA8_AGA, "AGA", tag, owner, clock, "aga", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_palette(*this, "palette"),
		m_mc6845(*this, AGA_MC6845_NAME),
		m_cga_config(*this, "cga_config"),
		m_update_row_type(-1),
		m_mode(),
		m_mda_mode_control(0),
		m_mda_status(0),
		m_mda_chr_gen(nullptr),
		m_cga_mode_control(0),
		m_cga_color_select(0),
		m_cga_status(0),
		m_cga_chr_gen(nullptr),
		m_framecnt(0),
		m_vsync(0),
		m_hsync(0),
		m_videoram(nullptr)
{
}

isa8_aga_device::isa8_aga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_isa8_card_interface(mconfig, *this),
		m_palette(*this, "palette"),
		m_mc6845(*this, AGA_MC6845_NAME),
		m_cga_config(*this, "cga_config"),
		m_update_row_type(-1), m_mode(),
		m_mda_mode_control(0),
		m_mda_status(0), m_mda_chr_gen(nullptr),
		m_cga_mode_control(0),
		m_cga_color_select(0),
		m_cga_status(0), m_cga_chr_gen(nullptr),
		m_framecnt(0),
		m_vsync(0),
		m_hsync(0), m_videoram(nullptr)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_aga_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	m_mode = AGA_COLOR;
	m_mda_chr_gen = memregion("gfx1")->base() + 0x1000;
	m_cga_chr_gen = memregion("gfx1")->base();
	m_videoram = auto_alloc_array(machine(), UINT8, 0x10000);

	set_isa_device();
	m_isa->install_memory(0xb0000, 0xbffff, 0, 0, read8_delegate(FUNC(isa8_aga_device::pc_aga_videoram_r),this), write8_delegate(FUNC(isa8_aga_device::pc_aga_videoram_w),this));
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate( FUNC(isa8_aga_device::pc_aga_mda_r), this ), write8_delegate( FUNC(isa8_aga_device::pc_aga_mda_w), this ) );
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_aga_device::pc_aga_cga_r), this ), write8_delegate( FUNC(isa8_aga_device::pc_aga_cga_w), this ) );

	/* Initialise the cga palette */
	int i;

	for ( i = 0; i < CGA_PALETTE_SETS * 16; i++ )
	{
		m_palette->set_pen_color( i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2] );
	}

	i = 0x8000;
	for ( int r = 0; r < 32; r++ )
	{
		for ( int g = 0; g < 32; g++ )
		{
			for ( int b = 0; b < 32; b++ )
			{
				m_palette->set_pen_color( i, r << 3, g << 3, b << 3 );
				i++;
			}
		}
	}

	UINT8 *gfx = &memregion("gfx1")->base()[0x8000];
	/* just a plain bit pattern for graphics data generation */
	for (i = 0; i < 256; i++)
		gfx[i] = i;
}

ROM_START( aga )
	ROM_REGION(0x8100,"gfx1", 0)
	ROM_LOAD("50146 char d1.0 euro.u16", 0x00000, 0x02000, CRC(1305dcf5) SHA1(aca488a16ae4ff05a1f4d14574379ff49cd48343)) //D1.0
ROM_END

const rom_entry *isa8_aga_device::device_rom_region() const
{
	return ROM_NAME( aga );
}

ioport_constructor isa8_aga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( aga );
}



const device_type ISA8_AGA_PC200 = &device_creator<isa8_aga_pc200_device>;

//-------------------------------------------------
//  isa8_aga_pc200_device - constructor
//-------------------------------------------------

isa8_aga_pc200_device::isa8_aga_pc200_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_aga_device( mconfig, ISA8_AGA_PC200, "AGA PC200", tag, owner, clock, "aga_pc200", __FILE__),
		m_port8(0),
		m_portd(0),
		m_porte(0)
{
}

ROM_START( aga_pc200 )
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("40109.ic159",     0x00000, 0x08000, CRC(a8b67639) SHA1(99663bfb61798526e092205575370c2ad34249a1))
ROM_END

const rom_entry *isa8_aga_pc200_device::device_rom_region() const
{
	return ROM_NAME( aga_pc200 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_aga_pc200_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	m_mode = AGA_COLOR;
	m_mda_chr_gen = memregion("gfx1")->base();
	m_cga_chr_gen = memregion("gfx1")->base() + 0x1000;
	m_videoram = auto_alloc_array(machine(), UINT8, 0x10000);

	set_isa_device();
	m_isa->install_memory(0xb0000, 0xbffff, 0, 0, read8_delegate(FUNC(isa8_aga_pc200_device::pc200_videoram_r),this), write8_delegate(FUNC(isa8_aga_pc200_device::pc200_videoram_w),this));
	m_isa->install_device(0x3b0, 0x3bf, 0, 0, read8_delegate( FUNC(isa8_aga_device::pc_aga_mda_r), this ), write8_delegate( FUNC(isa8_aga_device::pc_aga_mda_w), this ) );
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_aga_pc200_device::pc200_cga_r), this ), write8_delegate( FUNC(isa8_aga_pc200_device::pc200_cga_w), this ) );

	/* Initialise the cga palette */
	int i;

	for ( i = 0; i < CGA_PALETTE_SETS * 16; i++ )
	{
		m_palette->set_pen_color( i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2] );
	}

	i = 0x8000;
	for ( int r = 0; r < 32; r++ )
	{
		for ( int g = 0; g < 32; g++ )
		{
			for ( int b = 0; b < 32; b++ )
			{
				m_palette->set_pen_color( i, r << 3, g << 3, b << 3 );
				i++;
			}
		}
	}

	UINT8 *gfx = &memregion("gfx1")->base()[0x8000];
	/* just a plain bit pattern for graphics data generation */
	for (i = 0; i < 256; i++)
		gfx[i] = i;
}

WRITE_LINE_MEMBER( isa8_aga_device::hsync_changed )
{
	m_hsync = state ? 1 : 0;
}


WRITE_LINE_MEMBER( isa8_aga_device::vsync_changed )
{
	m_vsync = state ? 8 : 0;
	if ( state )
	{
		m_framecnt++;
	}
}


MC6845_UPDATE_ROW( isa8_aga_device::aga_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case MDA_TEXT_INTEN:
			mda_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case MDA_TEXT_BLINK:
			mda_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_INTEN:
			cga_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_INTEN_ALT:
			cga_text_inten_alt_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_BLINK:
			cga_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_BLINK_ALT:
			cga_text_blink_alt_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_GFX_1BPP:
			cga_gfx_1bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_GFX_2BPP:
			cga_gfx_2bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_GFX_4BPPL:
			cga_gfx_4bppl_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_GFX_4BPPH:
			cga_gfx_4bpph_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


MACHINE_CONFIG_FRAGMENT( pcvideo_aga )
	MCFG_SCREEN_ADD( AGA_SCREEN_NAME, RASTER )
	MCFG_SCREEN_RAW_PARAMS( XTAL_14_31818MHz,912,0,640,262,0,200 )
	MCFG_SCREEN_UPDATE_DEVICE( AGA_MC6845_NAME, mc6845_device, screen_update )

	MCFG_PALETTE_ADD( "palette", /* CGA_PALETTE_SETS * 16*/ 65536 )

	MCFG_MC6845_ADD(AGA_MC6845_NAME, MC6845, AGA_SCREEN_NAME, XTAL_14_31818MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(isa8_aga_device, aga_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(isa8_aga_device, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(isa8_aga_device, vsync_changed))
MACHINE_CONFIG_END

machine_config_constructor isa8_aga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcvideo_aga );
}

/*************************************
 *
 * row update functions
 *
 *************************************/

/* colors need fixing in the mda_text_* functions ! */
MC6845_UPDATE_ROW( isa8_aga_device::mda_text_inten_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *videoram = m_videoram;
	UINT32  *p = &bitmap.pix32(y);
	UINT16  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) logerror("mda_text_inten_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset + 1 ];
		UINT8 data = m_mda_chr_gen[ chr_base + chr * 8 ];
		UINT8 fg = ( attr & 0x08 ) ? 3 : 2;
		UINT8 bg = 0;

		if ( ( attr & ~0x88 ) == 0 ) {
			data = 0x00;
		}

		switch( attr ) {
		case 0x70:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
			bg = 2;
			fg = 1;
			break;
		case 0xF0:
			bg = 3;
			fg = 0;
			break;
		case 0xF8:
			bg = 3;
			fg = 1;
			break;
		}

		if ( i == cursor_x || ( attr & 0x07 ) == 0x01 ) {
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		if ( ( chr & 0xE0 ) == 0xC0 ) {
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		} else {
			*p = palette[bg]; p++;
		}
	}
}


MC6845_UPDATE_ROW( isa8_aga_device::mda_text_blink_update_row )
{
	UINT8 *videoram = m_videoram;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(y);
	UINT16  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) logerror("mda_text_blink_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset + 1 ];
		UINT8 data = m_mda_chr_gen[ chr_base + chr * 8 ];
		UINT8 fg = ( attr & 0x08 ) ? 3 : 2;
		UINT8 bg = 0;

		if ( ( attr & ~0x88 ) == 0 ) {
			data = 0x00;
		}

		switch( attr ) {
		case 0x70:
		case 0xF0:
			bg = 2;
			fg = 0;
			break;
		case 0x78:
		case 0xF8:
			bg = 2;
			fg = 1;
			break;
		}

		if ( i == cursor_x ) {
			data = 0xFF;
		} else {
			if ( ( attr & 0x07 ) == 0x01 ) {
				data = 0xFF;
			}
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x40 ) ) {
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		if ( ( chr & 0xE0 ) == 0xC0 ) {
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		} else {
			*p = palette[bg]; p++;
		}
	}
}


MC6845_UPDATE_ROW( isa8_aga_device::cga_text_inten_update_row )
{
	UINT8 *videoram = m_videoram;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_text_inten_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = ( attr >> 4 ) & 0x07;

		if ( i == cursor_x ) {
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_text_inten_alt_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *videoram = m_videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_text_inten_alt_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x0F;

		if ( i == cursor_x ) {
			data = 0xFF;
		}

		*p = palette[( data & 0x80 ) ? fg : 0]; p++;
		*p = palette[( data & 0x40 ) ? fg : 0]; p++;
		*p = palette[( data & 0x20 ) ? fg : 0]; p++;
		*p = palette[( data & 0x10 ) ? fg : 0]; p++;
		*p = palette[( data & 0x08 ) ? fg : 0]; p++;
		*p = palette[( data & 0x04 ) ? fg : 0]; p++;
		*p = palette[( data & 0x02 ) ? fg : 0]; p++;
		*p = palette[( data & 0x01 ) ? fg : 0]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_text_blink_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *videoram = m_videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = (attr >> 4) & 0x07;

		if ( i == cursor_x ) {
			data = 0xFF;
		} else {
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) ) {
				data = 0x00;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_text_blink_alt_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *videoram = m_videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_text_blink_alt_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_cga_chr_gen[ chr * 16 + ra ];
		UINT16 fg = attr & 0x07;
		UINT16 bg = 0;

		if ( i == cursor_x ) {
			data = 0xFF;
		} else {
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) ) {
				data = 0x00;
				bg = ( attr >> 4 ) & 0x07;
			}
		}

		*p = palette[( data & 0x80 ) ? fg : bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_gfx_4bppl_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8 *videoram = m_videoram;
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_gfx_4bppl_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( y & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;

		data = videoram[ offset + 1 ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_gfx_4bpph_update_row )
{
	UINT8 *videoram = m_videoram;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(y);
	int i;

	if ( y == 0 ) logerror("cga_gfx_4bpph_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( y & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;

		data = videoram[ offset + 1 ];

		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data >> 4]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
		*p = palette[data & 0x0F]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_gfx_2bpp_update_row )
{
	UINT8 *videoram = m_videoram;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(y);
	int i;

//  if ( y == 0 ) logerror("cga_gfx_2bpp_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( y & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[m_cga_palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[m_cga_palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[m_cga_palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[m_cga_palette_lut_2bpp[   data        & 0x03 ]]; p++;

		data = videoram[ offset+1 ];

		*p = palette[m_cga_palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[m_cga_palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[m_cga_palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[m_cga_palette_lut_2bpp[   data        & 0x03 ]]; p++;
	}
}

MC6845_UPDATE_ROW( isa8_aga_device::cga_gfx_1bpp_update_row )
{
	UINT8 *videoram = m_videoram;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(y);
	UINT8   fg = m_cga_color_select & 0x0F;
	int i;

	if ( y == 0 ) logerror("cga_gfx_1bpp_update_row\n");
	for ( i = 0; i < x_count; i++ ) {
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( ra & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[( data & 0x80 ) ? fg : 0]; p++;
		*p = palette[( data & 0x40 ) ? fg : 0]; p++;
		*p = palette[( data & 0x20 ) ? fg : 0]; p++;
		*p = palette[( data & 0x10 ) ? fg : 0]; p++;
		*p = palette[( data & 0x08 ) ? fg : 0]; p++;
		*p = palette[( data & 0x04 ) ? fg : 0]; p++;
		*p = palette[( data & 0x02 ) ? fg : 0]; p++;
		*p = palette[( data & 0x01 ) ? fg : 0]; p++;

		data = videoram[ offset + 1 ];

		*p = palette[( data & 0x80 ) ? fg : 0]; p++;
		*p = palette[( data & 0x40 ) ? fg : 0]; p++;
		*p = palette[( data & 0x20 ) ? fg : 0]; p++;
		*p = palette[( data & 0x10 ) ? fg : 0]; p++;
		*p = palette[( data & 0x08 ) ? fg : 0]; p++;
		*p = palette[( data & 0x04 ) ? fg : 0]; p++;
		*p = palette[( data & 0x02 ) ? fg : 0]; p++;
		*p = palette[( data & 0x01 ) ? fg : 0]; p++;
	}
}

/*************************************
 *
 *  AGA MDA/CGA read/write handlers
 *
 *************************************/

READ8_MEMBER ( isa8_aga_device::pc_aga_mda_r )
{
	UINT8 data = 0xFF;

	if ( m_mode == AGA_MONO ) {
		switch( offset )
		{
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;
		case 1: case 3: case 5: case 7:
			data = m_mc6845->register_r(space, offset);
			break;
		case 10:
			data = m_vsync | 0x08 | m_hsync;
			break;
		/* 12, 13, 14  are the LPT1 ports */
		}
	}
	return data;
}

WRITE8_MEMBER ( isa8_aga_device::pc_aga_mda_w )
{
	if ( m_mode == AGA_MONO ) {
		switch( offset )
		{
			case 0: case 2: case 4: case 6:
				m_mc6845->address_w( space, offset, data );
				break;
			case 1: case 3: case 5: case 7:
				m_mc6845->register_w( space, offset, data );
				break;
			case 8:
				m_mda_mode_control = data;

				switch (m_mda_mode_control & 0x2a)
				{
				case 0x08:
					m_update_row_type = MDA_TEXT_INTEN;
					break;
				case 0x28:
					m_update_row_type = MDA_TEXT_BLINK;
					break;
				default:
					m_update_row_type = -1;
				}
				break;
		}
	}
}

READ8_MEMBER ( isa8_aga_device::pc_aga_cga_r )
{
	UINT8 data = 0xFF;

	if ( m_mode == AGA_COLOR ) {
		switch( offset ) {
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;
		case 1: case 3: case 5: case 7:
			data = m_mc6845->register_r( space, offset);
			break;
		case 10:
			data = m_vsync | ( ( data & 0x40 ) >> 4 ) | m_hsync;
			break;
		}
	}
	return data;
}

void isa8_aga_device::set_palette_luts(void)
{
	/* Setup 2bpp palette lookup table */
	if ( m_cga_mode_control & 0x10 )
	{
		m_cga_palette_lut_2bpp[0] = 0;
	}
	else
	{
		m_cga_palette_lut_2bpp[0] = m_cga_color_select & 0x0F;
	}
	if ( m_cga_mode_control & 0x04 )
	{
		m_cga_palette_lut_2bpp[1] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 3;
		m_cga_palette_lut_2bpp[2] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 4;
		m_cga_palette_lut_2bpp[3] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 7;
	}
	else
	{
		if ( m_cga_color_select & 0x20 )
		{
			m_cga_palette_lut_2bpp[1] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 3;
			m_cga_palette_lut_2bpp[2] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 5;
			m_cga_palette_lut_2bpp[3] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 7;
		}
		else
		{
			m_cga_palette_lut_2bpp[1] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 2;
			m_cga_palette_lut_2bpp[2] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 4;
			m_cga_palette_lut_2bpp[3] = ( ( m_cga_color_select & 0x10 ) >> 1 ) | 6;
		}
	}
	//logerror("2bpp lut set to %d,%d,%d,%d\n", cga.palette_lut_2bpp[0], cga.palette_lut_2bpp[1], cga.palette_lut_2bpp[2], cga.palette_lut_2bpp[3]);
}


WRITE8_MEMBER (isa8_aga_device:: pc_aga_cga_w )
{
	if ( m_mode == AGA_COLOR ) {
		switch(offset) {
		case 0: case 2: case 4: case 6:
			m_mc6845->address_w( space, offset, data );
			break;
		case 1: case 3: case 5: case 7:
			m_mc6845->register_w( space, offset, data );
			break;
		case 8:
			m_cga_mode_control = data;

			//logerror("mode set to %02X\n", m_cga_mode_control & 0x3F );
			switch ( m_cga_mode_control & 0x3F ) {
			case 0x08: case 0x09: case 0x0C: case 0x0D:
				m_mc6845->set_hpixels_per_column( 8 );
				m_update_row_type = CGA_TEXT_INTEN;
				break;
			case 0x0A: case 0x0B: case 0x2A: case 0x2B:
				m_mc6845->set_hpixels_per_column( 8 );
				if ( CGA_MONITOR == CGA_MONITOR_COMPOSITE ) {
					m_update_row_type = CGA_GFX_4BPPL;
				} else {
					m_update_row_type = CGA_GFX_2BPP;
				}
					break;
			case 0x0E: case 0x0F: case 0x2E: case 0x2F:
				m_mc6845->set_hpixels_per_column( 8 );
				m_update_row_type = CGA_GFX_2BPP;
				break;
			case 0x18: case 0x19: case 0x1C: case 0x1D:
				m_mc6845->set_hpixels_per_column( 8 );
				m_update_row_type = CGA_TEXT_INTEN_ALT;
				break;
			case 0x1A: case 0x1B: case 0x3A: case 0x3B:
				m_mc6845->set_hpixels_per_column( 8 );
				if ( CGA_MONITOR == CGA_MONITOR_COMPOSITE ) {
					m_update_row_type = CGA_GFX_4BPPH;
				} else {
					m_update_row_type = CGA_GFX_1BPP;
				}
					break;
			case 0x1E: case 0x1F: case 0x3E: case 0x3F:
				m_mc6845->set_hpixels_per_column( 16 );
				m_update_row_type = CGA_GFX_1BPP;
				break;
			case 0x28: case 0x29: case 0x2C: case 0x2D:
				m_mc6845->set_hpixels_per_column( 8 );
				m_update_row_type = CGA_TEXT_BLINK;
				break;
			case 0x38: case 0x39: case 0x3C: case 0x3D:
				m_mc6845->set_hpixels_per_column( 8 );
				m_update_row_type = CGA_TEXT_BLINK_ALT;
				break;
			default:
				m_update_row_type = -1;
				break;
			}

			set_palette_luts();
			break;
		case 9:
			m_cga_color_select = data;
			set_palette_luts();
			break;
		}
	}
}

/*************************************/

void isa8_aga_device::pc_aga_set_mode( AGA_MODE mode)
{
	m_mode = mode;

	switch (m_mode) {
	case AGA_COLOR:
		m_mc6845->set_clock( XTAL_14_31818MHz/8 );
		break;
	case AGA_MONO:
		m_mc6845->set_clock( 16257000/9 );
		break;
	case AGA_OFF:
		break;
	}
}


WRITE8_MEMBER ( isa8_aga_device::pc_aga_videoram_w )
{
	switch (m_mode) {
	case AGA_COLOR:
		if (offset>=0x8000)
			m_videoram[offset-0x8000]=data;
		break;
	case AGA_MONO:
		m_videoram[offset]=data;
		break;
	case AGA_OFF: break;
	}
}

READ8_MEMBER( isa8_aga_device::pc_aga_videoram_r )
{
	switch (m_mode) {
	case AGA_COLOR:
		if (offset>=0x8000) return m_videoram[offset-0x8000];
		return 0;
	case AGA_MONO:
		return m_videoram[offset];
	case AGA_OFF: break;
	}
	return 0;
}

READ8_MEMBER( isa8_aga_pc200_device::pc200_videoram_r )
{
	switch (m_mode)
	{
		default:
			if (offset>=0x8000) return m_videoram[offset-0x8000];
			return 0;
		case AGA_MONO:
			return m_videoram[offset];
	}
}

WRITE8_MEMBER ( isa8_aga_pc200_device::pc200_videoram_w )
{
	switch (m_mode)
	{
		default:
			if (offset>=0x8000)
				m_videoram[offset-0x8000]=data;
			break;
		case AGA_MONO:
			m_videoram[offset]=data;
			break;
	}
}

// in reality it is of course only 1 graphics adapter,
// but now cga and mda are splitted in mess
WRITE8_MEMBER( isa8_aga_pc200_device::pc200_cga_w )
{
	pc_aga_cga_w(space, offset,data,mem_mask);
	switch(offset) {
	case 4:
		m_portd |= 0x20;
		break;
	case 8:
		m_port8 = data;
		m_portd |= 0x80;
		break;
	case 0xe:
		m_portd = 0x1f;
		if (data & 0x80)
			m_portd |= 0x40;

/* The bottom 3 bits of this port are:
 * Bit 2: Disable AGA
 * Bit 1: Select MDA
 * Bit 0: Select external display (monitor) rather than internal display
 *       (TV for PC200; LCD for PPC512) */
		if ((m_porte & 7) != (data & 7))
		{
			if (data & 4)
				pc_aga_set_mode(AGA_OFF);
			else if (data & 2)
				pc_aga_set_mode(AGA_MONO);
			else
				pc_aga_set_mode(AGA_COLOR);
		}
		m_porte = data;
		break;

	default:
		break;
	}
}

READ8_MEMBER ( isa8_aga_pc200_device::pc200_cga_r )
{
	UINT8 result = 0xff;

	switch(offset) {
	case 8:
		result = m_port8;
		break;

	case 0xd:
		// after writing 0x80 to 0x3de, bits 7..5 of 0x3dd from the 2nd read must be 0
		result=m_portd;
		m_portd&=0x1f;
		break;

	case 0xe:
		// 0x20 low cga
		// 0x10 low special
		result = machine().root_device().ioport("DSW0")->read() & 0x38;
		break;

	default:
		result = pc_aga_cga_r(space, offset, mem_mask);
		break;
	}
	return result;
}
