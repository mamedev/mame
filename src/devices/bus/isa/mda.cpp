// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Miodrag Milanovic
/***************************************************************************

  Monochrome Display Adapter (MDA) section

***************************************************************************/

#include "emu.h"
#include "mda.h"

#include "screen.h"


#define LOG_READ    (1U << 1)
#define LOG_SETUP   (1U << 2)
#define LOG_ROW     (1U << 3)
#define LOG_MODE    (1U << 4)
#define LOG_CHRG    (1U << 5)
#define LOG_STAT    (1U << 6)

//#define VERBOSE (LOG_MODE|LOG_STAT)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGROW(...)   LOGMASKED(LOG_ROW,   __VA_ARGS__)
#define LOGMODE(...)  LOGMASKED(LOG_MODE,  __VA_ARGS__)
#define LOGCHRG(...)  LOGMASKED(LOG_CHRG,  __VA_ARGS__)
#define LOGSTAT(...)  LOGMASKED(LOG_STAT,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

#define MDA_SCREEN_NAME "mda_screen"
#define MC6845_NAME "mc6845"

/*
  Hercules video card
 */
#define HERCULES_SCREEN_NAME "hercules_screen"
#define MDA_CLOCK   XTAL(16'257'000)

static const unsigned char mda_palette[4][3] =
{
	{ 0x00,0x00,0x00 },
	{ 0x00,0x55,0x00 },
	{ 0x00,0xaa,0x00 },
	{ 0x00,0xff,0x00 }
};

enum
{
	MDA_TEXT_INTEN = 0,
	MDA_TEXT_BLINK,
	HERCULES_GFX_BLINK,
	MDA_LOWRES_TEXT_INTEN,
	MDA_LOWRES_TEXT_BLINK
};

/* F4 Character Displayer */
static const gfx_layout pc_16_charlayout =
{
	8, 16,                  /* 8 x 16 characters */
	256,                    /* 256 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 2048*8, 2049*8, 2050*8, 2051*8, 2052*8, 2053*8, 2054*8, 2055*8 },
	8*8                 /* every char takes 2 x 8 bytes */
};

static const gfx_layout pc_8_charlayout =
{
	8, 8,                   /* 8 x 8 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8                 /* every char takes 8 bytes */
};

static GFXDECODE_START( gfx_pcmda )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout, 1, 1 )
	GFXDECODE_ENTRY( "gfx1", 0x1000, pc_8_charlayout, 1, 1 )
GFXDECODE_END


WRITE_LINE_MEMBER( isa8_mda_device::pc_cpu_line )
{
	m_isa->irq7_w(state);
}


ROM_START( mda )
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x08100,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x02000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_MDA, isa8_mda_device, "isa_ibm_mda", "IBM Monochrome Display and Printer Adapter")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_mda_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, MDA_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(MDA_CLOCK, 882, 0, 720, 370, 0, 350);
	screen.set_screen_update(MC6845_NAME, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(4);

	MC6845(config, m_crtc, MDA_CLOCK/9);
	m_crtc->set_screen(MDA_SCREEN_NAME);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(isa8_mda_device::crtc_update_row));
	m_crtc->out_hsync_callback().set(FUNC(isa8_mda_device::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(isa8_mda_device::vsync_changed));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pcmda);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(isa8_mda_device::pc_cpu_line));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_mda_device::device_rom_region() const
{
	return ROM_NAME( mda );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_mda_device - constructor
//-------------------------------------------------

isa8_mda_device::isa8_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_MDA, tag, owner, clock)
{
}

isa8_mda_device::isa8_mda_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this), m_crtc(*this, MC6845_NAME), m_lpt(*this, "lpt"), m_framecnt(0), m_mode_control(0),
	m_update_row_type(-1), m_chr_gen(nullptr), m_vsync(0), m_hsync(0), m_pixel(0),
	m_palette(*this, "palette")
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_mda_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();
	m_videoram.resize(0x1000);
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate(*this, FUNC(isa8_mda_device::io_read)), write8_delegate(*this, FUNC(isa8_mda_device::io_write)));
	m_isa->install_bank(0xb0000, 0xb0fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb1000, 0xb1fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb2000, 0xb2fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb3000, 0xb3fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb4000, 0xb4fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb5000, 0xb5fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb6000, 0xb6fff, "bank_mda", &m_videoram[0]);
	m_isa->install_bank(0xb7000, 0xb7fff, "bank_mda", &m_videoram[0]);

	/* Initialise the mda palette */
	for (int i = 0; i < 4; i++)
		m_palette->set_pen_color(i, rgb_t(mda_palette[i][0], mda_palette[i][1], mda_palette[i][2]));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_mda_device::device_reset()
{
	m_framecnt = 0;
	m_mode_control = 0;
	m_vsync = 0;
	m_hsync = 0;
	m_pixel = 0;

	m_chr_gen = memregion(subtag("gfx1").c_str())->base();
}

/***************************************************************************

  Monochrome Display Adapter (MDA) section

***************************************************************************/

/***************************************************************************
  Draw text mode with 80x25 characters (default) and intense background.
  The character cell size is 9x15. Column 9 is column 8 repeated for
  character codes 176 to 223.
***************************************************************************/
MC6845_UPDATE_ROW( isa8_mda_device::mda_text_inten_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ chr_base + chr * 8 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
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

		if ( ( i == cursor_x && ( m_framecnt & 0x08 ) ) || ( attr & 0x07 ) == 0x01 )
		{
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
		if ( ( chr & 0xE0 ) == 0xC0 )
		{
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		}
		else
		{
			*p = palette[bg]; p++;
		}
	}
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and blinking characters.
  The character cell size is 9x15. Column 9 is column 8 repeated for
  character codes 176 to 223.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_mda_device::mda_text_blink_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ( ra & 0x08 ) ? 0x800 | ( ra & 0x07 ) : ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ chr_base + chr * 8 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
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

		if ( ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}
		else
		{
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) )
			{
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
		if ( ( chr & 0xE0 ) == 0xC0 )
		{
			*p = palette[( data & 0x01 ) ? fg : bg]; p++;
		}
		else
		{
			*p = palette[bg]; p++;
		}
	}
}

MC6845_UPDATE_ROW( isa8_mda_device::crtc_update_row )
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
	}
}


MC6845_UPDATE_ROW( isa8_hercules_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case HERCULES_GFX_BLINK:
			hercules_gfx_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		default:
			isa8_mda_device::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


WRITE_LINE_MEMBER( isa8_mda_device::hsync_changed )
{
	m_hsync = state ? 1 : 0;
}


WRITE_LINE_MEMBER( isa8_mda_device::vsync_changed )
{
	m_vsync = state ? 0x80 : 0;
	if ( state )
	{
		m_framecnt++;
	}
}


/*
 *  rW  MDA mode control register (see #P138)
 */
WRITE8_MEMBER( isa8_mda_device::mode_control_w )
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
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
}


/*  R-  CRT status register (see #P139)
 *      (EGA/VGA) input status 1 register
 *      7    HGC vertical sync in progress
 *      6-4  adapter 000  hercules
 *                   001  hercules+
 *                   101  hercules InColor
 *                   else unknown
 *      3    pixel stream (0 black, 1 white)
 *      2-1  reserved
 *      0    horizontal drive enable
 */
READ8_MEMBER( isa8_mda_device::status_r )
{
	// Faking pixel stream here
	m_pixel++;

	return 0xF0 | (m_pixel & 0x08) | m_hsync;
}


/*************************************************************************
 *
 *      MDA
 *      monochrome display adapter
 *
 *************************************************************************/
WRITE8_MEMBER( isa8_mda_device::io_write )
{
	switch( offset )
	{
		case 0x00: case 0x02: case 0x04: case 0x06:
			m_crtc->address_w(data);
			break;
		case 0x01: case 0x03: case 0x05: case 0x07:
			m_crtc->register_w(data);
			break;
		case 0x08:
			mode_control_w(space, offset, data);
			break;
		case 0x0c: case 0x0d:  case 0x0e:
			m_lpt->write(space, offset - 0x0c, data);
			break;
	}
}

READ8_MEMBER( isa8_mda_device::io_read )
{
	int data = 0xff;
	switch( offset )
	{
		case 0x00: case 0x02: case 0x04: case 0x06:
			/* return last written mc6845 address value here? */
			break;
		case 0x01: case 0x03: case 0x05: case 0x07:
			data = m_crtc->register_r();
			break;
		case 0x0a:
			data = status_r(space, offset);
			break;
		/* LPT ports */
		case 0x0c: case 0x0d:  case 0x0e:
			data = m_lpt->read(space, offset - 0x0c);
			break;
	}
	return data;
}


/***************************************************************************

  Hercules Display Adapter section (re-uses parts from the MDA section)

***************************************************************************/

/*
When the Hercules changes to graphics mode, the number of pixels per access and
clock divider should be changed. The correct mc6845 implementation does not
allow this.

The divder/pixels per 6845 clock is 9 for text mode and 16 for graphics mode.
*/

static GFXDECODE_START( gfx_pcherc )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pc_16_charlayout, 1, 1 )
GFXDECODE_END

ROM_START( hercules )
	ROM_REGION(0x1000, "gfx1", 0)
	ROM_SYSTEM_BIOS(0, "cp437", "Code page 437")
	ROMX_LOAD("um2301.bin", 0x0000, 0x1000, CRC(0827bdac) SHA1(15f1aceeee8b31f0d860ff420643e3c7f29b5ffc), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "mzv", "Mazovia (Polish)") // dumped from a Taiwanese-made card using the SiS 86C22 chip
	ROMX_LOAD("hgc_mzv_2301.bin", 0x0000, 0x1000, CRC(9431b9e0) SHA1(3279dfeed4a0f5daa7b57d455c96eafdcbb6bf41), ROM_BIOS(1))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(ISA8_HERCULES, isa8_hercules_device, "isa_hercules", "Hercules Graphics Card")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_hercules_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, HERCULES_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(MDA_CLOCK, 882, 0, 720, 370, 0, 350);
	screen.set_screen_update(MC6845_NAME, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(4);

	MC6845(config, m_crtc, MDA_CLOCK/9);
	m_crtc->set_screen(HERCULES_SCREEN_NAME);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(isa8_hercules_device::crtc_update_row));
	m_crtc->out_hsync_callback().set(FUNC(isa8_mda_device::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(isa8_mda_device::vsync_changed));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_pcherc);

	PC_LPT(config, m_lpt);
	m_lpt->irq_handler().set(FUNC(isa8_mda_device::pc_cpu_line));
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_hercules_device::device_rom_region() const
{
	return ROM_NAME( hercules );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_hercules_device - constructor
//-------------------------------------------------

isa8_hercules_device::isa8_hercules_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_HERCULES, tag, owner, clock), m_configuration_switch(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_hercules_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	m_videoram.resize(0x10000);
	set_isa_device();
	m_isa->install_device(0x3b0, 0x3bf, read8_delegate(*this, FUNC(isa8_hercules_device::io_read)), write8_delegate(*this, FUNC(isa8_hercules_device::io_write)));
	m_isa->install_bank(0xb0000, 0xbffff, "bank_hercules", &m_videoram[0]);

	/* Initialise the mda palette */
	for(int i = 0; i < (sizeof(mda_palette) / 3); i++)
		m_palette->set_pen_color(i, mda_palette[i][0], mda_palette[i][1], mda_palette[i][2]);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_hercules_device::device_reset()
{
	isa8_mda_device::device_reset();
	m_configuration_switch = 0;

	m_chr_gen = memregion(subtag("gfx1").c_str())->base();
}

/***************************************************************************
  Draw graphics with 720x348 pixels (default); so called Hercules gfx.
  The memory layout is divided into 4 banks where of size 0x2000.
  Every bank holds data for every n'th scanline, 8 pixels per byte,
  bit 7 being the leftmost.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_hercules_device::hercules_gfx_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  gfx_base = ( ( m_mode_control & 0x80 ) ? 0x8000 : 0x0000 ) | ( ( ra & 0x03 ) << 13 );
	int i;
	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint8_t   data = m_videoram[ gfx_base + ( ( ma + i ) << 1 ) ];

		*p = palette[( data & 0x80 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x40 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x20 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x10 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x08 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x04 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x02 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x01 ) ? 2 : 0]; p++;

		data = m_videoram[ gfx_base + ( ( ma + i ) << 1 ) + 1 ];

		*p = palette[( data & 0x80 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x40 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x20 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x10 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x08 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x04 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x02 ) ? 2 : 0]; p++;
		*p = palette[( data & 0x01 ) ? 2 : 0]; p++;
	}
}


WRITE8_MEMBER( isa8_hercules_device::mode_control_w )
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_TEXT_BLINK;
		break;
	case 0x0A:          /* Hercules modes */
	case 0x2A:
		m_update_row_type = HERCULES_GFX_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}

	m_crtc->set_unscaled_clock( MDA_CLOCK / (m_mode_control & 0x02 ? 16 : 9) );
	m_crtc->set_hpixels_per_column( m_mode_control & 0x02 ? 16 : 9 );
}


WRITE8_MEMBER( isa8_hercules_device::io_write )
{
	switch( offset )
	{
	case 0x00: case 0x02: case 0x04: case 0x06:
		m_crtc->address_w(data);
		break;
	case 0x01: case 0x03: case 0x05: case 0x07:
		m_crtc->register_w(data);
		break;
	case 0x08:
		mode_control_w(space, offset, data);
		break;
	case 0x0c: case 0x0d:  case 0x0e:
		m_lpt->write(space, offset - 12, data);
		break;
	case 0x0f:
		m_configuration_switch = data;
		break;
	}
}


/*  R-  CRT status register (see #P139)
 *      (EGA/VGA) input status 1 register
 *      7    HGC vertical sync in progress
 *      6-4  adapter 000  hercules
 *                   001  hercules+
 *                   101  hercules InColor
 *                   else unknown
 *      3    pixel stream (0 black, 1 white)
 *      2-1  reserved
 *      0    horizontal drive enable
 */
READ8_MEMBER( isa8_hercules_device::status_r )
{
	// Faking pixel stream here
	m_pixel++;

	return m_vsync | ( m_pixel & 0x08 ) | m_hsync;
}


READ8_MEMBER( isa8_hercules_device::io_read )
{
	int data = 0xff;
	switch( offset )
	{
	case 0x00: case 0x02: case 0x04: case 0x06:
		/* return last written mc6845 address value here? */
		break;
	case 0x01: case 0x03: case 0x05: case 0x07:
		data = m_crtc->register_r();
		break;
	case 0x0a:
		data = status_r(space, offset);
		break;
	/* LPT ports */
	case 0xc: case 0xd:  case 0xe:
		data = m_lpt->read(space, offset - 0x0c);
		break;
	}
	return data;
}

DEFINE_DEVICE_TYPE(ISA8_EC1840_0002, isa8_ec1840_0002_device, "ec1840_0002", "EC1840.0002 (MDA)")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

// XXX
void isa8_ec1840_0002_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, MDA_SCREEN_NAME, SCREEN_TYPE_RASTER));
	screen.set_raw(MDA_CLOCK, 792, 0, 640, 370, 0, 350);
	screen.set_screen_update(MC6845_NAME, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(4);

	MC6845(config, m_crtc, MDA_CLOCK/8);
	m_crtc->set_screen(MDA_SCREEN_NAME);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(isa8_mda_device::crtc_update_row));
	m_crtc->out_hsync_callback().set(FUNC(isa8_mda_device::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(isa8_mda_device::vsync_changed));
}

//-------------------------------------------------
//  isa8_ec1840_0002_device - constructor
//-------------------------------------------------

isa8_ec1840_0002_device::isa8_ec1840_0002_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_EC1840_0002, tag, owner, clock), m_soft_chr_gen(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ec1840_0002_device::device_start()
{
	isa8_mda_device::device_start();

	m_soft_chr_gen = std::make_unique<uint8_t[]>(0x2000);
	m_isa->install_bank(0xdc000, 0xddfff, "bank_chargen", m_soft_chr_gen.get());
	m_isa->install_bank(0xde000, 0xdffff, "bank_chargen", m_soft_chr_gen.get());
}

void isa8_ec1840_0002_device::device_reset()
{
	isa8_mda_device::device_reset();

	m_chr_gen = m_soft_chr_gen.get();
}


/***************************************************************************
  Draw text mode with 80x25 characters (default) and intense background.
  The character cell size is 8x14.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_ec1840_0002_device::mda_lowres_text_inten_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ (chr_base + chr * 16) << 1 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
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

		if ( ( i == cursor_x && ( m_framecnt & 0x08 ) ) || ( attr & 0x07 ) == 0x01 )
		{
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


/***************************************************************************
  Draw text mode with 80x25 characters (default) and blinking characters.
  The character cell size is 8x14.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_ec1840_0002_device::mda_lowres_text_blink_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	if ( y == 0 ) LOGROW("%11.6f: %-24s\n", machine().time().as_double(), FUNCNAME);
	for ( i = 0; i < x_count; i++ )
	{
		uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
		uint8_t chr = m_videoram[ offset ];
		uint8_t attr = m_videoram[ offset + 1 ];
		uint8_t data = m_chr_gen[ (chr_base + chr * 16) << 1 ];
		uint8_t fg = ( attr & 0x08 ) ? 3 : 2;
		uint8_t bg = 0;

		if ( ( attr & ~0x88 ) == 0 )
		{
			data = 0x00;
		}

		switch( attr )
		{
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

		if ( ( attr & 0x07 ) == 0x01 )
		{
			data = 0xFF;
		}

		if ( i == cursor_x )
		{
			if ( m_framecnt & 0x08 )
			{
				data = 0xFF;
			}
		}
		else
		{
			if ( ( attr & 0x80 ) && ( m_framecnt & 0x10 ) )
			{
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

WRITE8_MEMBER( isa8_ec1840_0002_device::mode_control_w )
{
	m_mode_control = data;

	switch( m_mode_control & 0x2a )
	{
	case 0x08:
		m_update_row_type = MDA_LOWRES_TEXT_INTEN;
		break;
	case 0x28:
		m_update_row_type = MDA_LOWRES_TEXT_BLINK;
		break;
	default:
		m_update_row_type = -1;
	}
}

MC6845_UPDATE_ROW( isa8_ec1840_0002_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case MDA_LOWRES_TEXT_INTEN:
			mda_lowres_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case MDA_LOWRES_TEXT_BLINK:
			mda_lowres_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}

/*****************************************************************************

  Ericsson PC Monochrome HR Graphics Board 1070

******************************************************************************/

/* PCB layouts and assembly years from online pictures and physical unit.
 Ericsson   -  marked SPVT02 8301 60 53-10, assembled in 1985 indicated by chip dates
 +--------------------------------------------------------------------------------------+ ___
 |  IC1  IC2   IC3   IC4   IC5 +-IC15--EPROM-+   IC6      IC7      IC8    S1        ||
 |                                 |8363 65 14-80|                                      ||
 | IC9  IC10 IC11  IC12  IC13  IC14|CG 50821 A64 |+------------------++-IC24 EPROM--+   ||
 |                                 +-------------+| CRTC HD46505SP-1 ||10-40VP      |   ||
 | IC16 IC17 IC18  IC19  IC20  IC21     IC22      | IC23 HD68A45SP   ||402 28 A19   | J4|| not
 |                                                +------------------++-------------+   || mounted
 | IC25 IC26 IC27  IC28  IC29  IC30       IC31       IC32      IC33      IC34           ||
 |                                                                                     O-|__
 | IC35 IC36 IC37  IC38  IC39  IC40       IC41       IC42      IC43      IC44           ||  |
 |                                                                                      ||DB15
 | IC45 IC46 IC47  IC48  IC49  IC50       IC51       IC52      IC53      IC54           ||  |
 |                                                                                      ||__|
 | IC55 IC56 IC57  IC58  IC59  IC60       IC61       IC62      IC63      IC64          O-|
 |                                                                               J1A    ||
 | IC65 IC66 IC67 IC68 IC69 IC70 IC71 IC72 +--------------------------------------------+|
 +-----------------------------------------+    |||||||||  |||||||||||||||||||||||||     |
   I85565  A85571 (labels)                                                               |
                                                                                         |

 IC's (from photos)
 ------------------------------------------------------------------------------
 IC1  74F109                              IC26 74F86                                IC51 TMS4416-15NL 4 x 16Kbits DRAM
 IC2  74LS393                             IC27 74LS08                               IC52 74ALS574
 IC3  74F64                               IC28 74F153                               IC53 74LS138
 IC4  74ALS299                            IC29 74LS174                              IC54 74F86
 IC5  74LS375                             IC30 74LS374                              IC55 74F109
 IC6  74LS151                             IC31 74LS374                              IC56 74F32
 IC7  74LS153                             IC32 74ALS574                             IC57 74F109
 IC8  74LS389?                            IC33 74LS08                               IC58 74F00?
 IC9  74F02                               IC34 74LS245                              IC59 74LS244
 IC10 74ALS109                            IC35 74F10?                               IC60 TMS4416-15NL 4 x 16Kbits DRAM
 IC11 Crystal 17.040MHz                   IC36 74LS02                               IC61 TMS4416-15NL 4 x 16Kbits DRAM
 IC12 74F64                               IC37 74LS00                               IC62 74ALS574
 IC13 74ALS299                            IC38 74F374                               IC63 74LS138
 IC14 PAL? 10-70ART40101                  IC39 74LS125                              IC64 74LS245
 IC15 EPROM 8363 65 14-80 CG 50821 A64    IC40 74LS244                              IC65 74LS00
 IC16 Crystal 19.170MHz                   IC41 74LS244                              IC66 74LS02
 IC17 74LS10                              IC42 74LS574                              IC67 74LS51
 IC18 74F08                               IC43 74LS32                               IC68 74LS04
 IC19 74ALS574                            IC44 MC10124 - TTL to MECL converter      IC69 74LS153
 IC20 74LS299                             IC45 74LS109                              IC70 74LS109
 IC21 74LS273                             IC46 74LS00                               IC71 74LS138
 IC22 74ALS574                            IC47 74F194                               IC72 74LS139
 IC23 CRTC HD46505SP,HD68A45SP            IC48 74F04
 IC24 EPROM 2764, 10-40 VP 402 28 A19     IC49 74LS174
 IC25 74ALS109                            IC50 TMS4416-15NL 4 x 16Kbits DRAM

 General description
 -------------------
 The PCB has a 2 bit DIP switch S1 and a DB15 non standard video connector. There is also an unsoldered J4 connector
 above the DB15 but no hole prepared for a connector in the plate. Above the J4 connector there is a two pin PCB connector
 that probably receives the power for the monitor for the DB15 from the PSU.

 Just below IC65 and IC66 there are two labels saying "I 85565" and "A E85571" respectively

 Video cable, card DB15 <---> monitor DB25
 ---------------------------------------------------
  Ericsson       2  +VS             4  Ericsson
  Monochrome     3  VS return       2  Monochrome HR
  HR Graphics   10  +VS            17  Monitors 3111 (Amber) or
  Board 1070    11  VS return      15  3712/3715 (Black & White)
         4  VSYNC           6
        12  VSYNC          19
         5  HSYNC           7
        13  HSYNC          20
         6  High intensity  8
        14  High intensity 21
         7  Video           9
        15  Video          22
                 8  GND            11

  This board is normaly used with an Ericsson monitor due to the non standard connector.
  Trivia: https://www.pinterest.se/pin/203084264425177097/
 */

#define EPC_MDA_SCREEN "epc_mda_screen" // TODO: use a device finder reference instead

ROM_START( epc )
	ROM_REGION(0x2000,"chargen", 0)
	ROM_LOAD("8363_65_14_80_cg_50821_a64.bin",  0x00000, 0x2000, CRC(be709786) SHA1(38ab26224bbe66bbe2bb2ccac29b41cbf78bdbf8))
	//ROM_LOAD("10_40_vp_402_28_ic_24_a19.bin",  0x00000, 0x2000, CRC(2aa53b92) SHA1(87051a037249eb631d7d2191bc0e925125c60f39))
ROM_END

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************
DEFINE_DEVICE_TYPE(ISA8_EPC_MDA, isa8_epc_mda_device, "isa_epc_mda", "Ericsson PC Monochrome HR Graphics Board 1070")


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
/* There are two crystals on the board: 19.170Mhz and 17.040MHz  TODO: verify use */
/* Text modes uses 720x400 base resolution and the Graphics modes 320/640x200/400 */
/* This matches the difference between the crystals so we assume this for now     */
void isa8_epc_mda_device::device_add_mconfig(machine_config &config)
{
	screen_device &screen(SCREEN(config, EPC_MDA_SCREEN, SCREEN_TYPE_RASTER));
	screen.set_raw(XTAL(19'170'000) / 4, 720, 0, 720, 400, 0, 400);
	//screen.set_screen_update(MC6845_NAME, FUNC(h46505_device::screen_update));
	screen.set_screen_update(MC6845_NAME, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(4);

	HD6845S(config, m_crtc, XTAL(19'170'000) / 16); // clock and divider are guesswork
	m_crtc->set_screen(EPC_MDA_SCREEN);
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);

	m_crtc->set_update_row_callback(FUNC(isa8_epc_mda_device::crtc_update_row));
	m_crtc->out_hsync_callback().set(FUNC(isa8_epc_mda_device::hsync_changed));
	m_crtc->out_vsync_callback().set(FUNC(isa8_epc_mda_device::vsync_changed));

	//MCFG_GFXDECODE_ADD("gfxdecode", "palette", pcepc)
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *isa8_epc_mda_device::device_rom_region() const
{
	return ROM_NAME( epc );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_epc_mda_device - constructor
//-------------------------------------------------

isa8_epc_mda_device::isa8_epc_mda_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_mda_device(mconfig, ISA8_EPC_MDA, tag, owner, clock),
	m_soft_chr_gen(nullptr),
	m_s1(*this, "S1"),
	m_color_mode(0),
	m_mode_control2(0),
	m_screen(*this, EPC_MDA_SCREEN),
	m_io_monitor(*this, "MONITOR"),
	m_chargen(*this, "chargen"),
	m_installed(false)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_epc_mda_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	/* Palette for use with the Ericsson Amber Monochrome HR CRT monitor 3111, P3 phospor 602nm 255,183, 0 */
	m_3111_pal[0] = rgb_t(  0,   0,     0); // black
	m_3111_pal[1] = rgb_t(  143, 103,   0); // dim
	m_3111_pal[2] = rgb_t(  191, 137,   0); // normal
	m_3111_pal[3] = rgb_t(  255, 183,   0); // bright

	/* Palette for use with the Ericsson B&W Monochrome HR CRT monitor 3712/3715 */
	m_371x_pal[0] = rgb_t(    0,   0,   0); // black
	m_371x_pal[1] = rgb_t(  143, 143, 143); // dim
	m_371x_pal[2] = rgb_t(  191, 191, 191); // normal
	m_371x_pal[3] = rgb_t(  255, 255, 255); // bright

	/* Init a default palette */
	m_pal = &m_3111_pal; // In case screen starts rendering before device_reset where we read the settings

	m_videoram.resize(0x8000);
	set_isa_device();
	m_installed = false;
	m_hd6845s = subdevice<hd6845s_device>(MC6845_NAME);
}

void isa8_epc_mda_device::device_reset()
{
	m_framecnt = 0;
	m_mode_control = 0;
	m_vsync = 0;
	m_hsync = 0;

	m_color_mode = m_s1->read();
	LOGSETUP("%s: m_color_mode:%02x\n", FUNCNAME, m_color_mode);
	m_pal = (m_io_monitor-> read() & 1) == 1 ? &m_371x_pal : &m_3111_pal;
	m_vmode = 0;

	if (m_installed == false)
	{
		m_isa->install_device(0x3b0, 0x3bb, read8_delegate(*this, FUNC(isa8_epc_mda_device::io_read)), write8_delegate(*this, FUNC(isa8_epc_mda_device::io_write)));
		// The Ericsson PC MDA card 1070 doesn't respond to the LPT device addresses 3bc-3be because
		// the LPT device is on the main PCB, but requires 3bf for mode register 2 so need to create a hole here - needs verification on hw as docs are wrong 
		m_isa->install_device(0x3bf, 0x3bf, read8_delegate(*this, FUNC(isa8_epc_mda_device::io_read2)), write8_delegate(*this, FUNC(isa8_epc_mda_device::io_write2)));
		m_isa->install_bank(0xb0000, 0xb7fff, "bank_epc", &m_videoram[0]); // Monochrome emulation mode VRAM address

		// This check allows a color monitor adapter to be installed at this address range if color emulation is disabled
		if (m_color_mode & 1)
		{
			m_isa->install_device(0x3d0, 0x3df, read8_delegate(*this, FUNC(isa8_epc_mda_device::io_read)), write8_delegate(*this, FUNC(isa8_epc_mda_device::io_write)));
			m_isa->install_bank(0xb8000, 0xbffff, "bank_epc", &m_videoram[0]); // Color emulation mode VRAM address, but same 32KB areas as there are only this amount on the board
		}
		m_installed = true;
	}
}

/*
 * Register Address table from the manual
 * Ericsson name          MDA mode   CGA mode  Standard name
 *-------------------------------------------------------------------------------
 * 6845 Address Registers 0x3b4      0x3d4     wo CRT Index reg
 * 6845 Data Registers    0x3b5      0x3d5     wo CRT Data reg
 * Mode Register 1        0x3b8      0x3d8     rw MDA/CGA mode reg (bit 0,1 & 4 incompatible)
 * Mode Register 2        0x3bf      0x3df     rw CRT/CPU page reg (incompatible w PCjr only)
 * Status Register        0x3ba      0x3da     r  CGA/MDA status reg (incompatible)
 *                                              w EGA/VGA feature ccontrol reg (not used by this board)
 *
 * NOTE: The LPT device resides on the Ericsson PC main board in the 3bc-3be address range
 */

WRITE8_MEMBER(isa8_epc_mda_device::io_write2 )
{
	io_write(space, offset + 0x0f, data);
}

WRITE8_MEMBER(isa8_epc_mda_device::io_write )
{
	LOG("%s: %04x <- %02x\n", FUNCNAME, offset, data);
	switch( offset )
	{
		case 0x04:
			//LOGSETUP(" - HD6845S address write\n");
			m_hd6845s->address_w( data );
			break;
		case 0x05:
			//LOGSETUP(" - HD6845S register write\n");
			m_hd6845s->register_w( data );
			break;
		case 0x08: // Mode 1 reg
			LOGMODE(" - Mode register 1 write: %02x\n", data);
			LOGMODE("   MSB attribute: %s\n", (data & 0x20) == 0 ? "intensity" : "blink");
			LOGMODE("   Horizontal px: %s\n", (data & 0x10) == 0 ? "320/LR" : "640/HR");
			LOGMODE("   Video        : %s\n", (data & 0x08) == 0 ? "Disabled" : "Enabled");
			LOGMODE("   Mode         : %s\n", (data & 0x02) == 0 ? "Text" : "Graphics");
			LOGMODE("   Text columns : %d\n", (data & 0x01) == 0 ? 40 : 80);
			m_mode_control = data;
			m_vmode &= ~(VM_GRAPH | VM_COLS80 | VM_HOR640);
			m_vmode |= ((m_mode_control & 0x01) ? VM_COLS80 : 0);
			m_vmode |= ((m_mode_control & 0x02) ? VM_GRAPH  : 0);
			m_vmode |= ((m_mode_control & 0x10) ? VM_HOR640 : 0);
			m_update_row_type = ((data & 0x20) == 0 ? MDA_LOWRES_TEXT_INTEN : MDA_LOWRES_TEXT_BLINK);
			{
				rectangle rect(0, get_xres() - 1, 0, get_yres() -1);
				m_screen->configure(get_xres(), get_yres(), rect, HZ_TO_ATTOSECONDS(50));
			}
			LOGMODE("Video Mode:%02x\n\n", m_vmode);
			break;
		case 0x0f: // Mode 2 reg
			LOGMODE(" - Mode register 2 write: %02x\n", data);
			LOGMODE("   Vertical px  : %s\n", (data & MR2_VER400) == 0 ? "200" : "400");
			LOGMODE("   Character set: %s\n", (data & MR2_CHRSET) == 0 ? "0" : "1");
			LOGMODE("   Emulated     : %s\n", (data & MR2_COLEMU) == 0 ? "Color" : "Monochrome");
			m_mode_control2 = data;
			m_vmode &= ~(VM_MONO | VM_VER400);
			m_vmode |= ((m_mode_control2 & 0x04) ? VM_MONO   : 0);
			m_vmode |= ((m_mode_control2 & 0x80) ? VM_VER400 : 0);
			{
				rectangle rect(0, get_xres() - 1, 0, get_yres() -1);
				m_screen->configure(get_xres(), get_yres(), rect, HZ_TO_ATTOSECONDS(50));
			}
			LOGMODE("Video Mode:%02x\n\n", m_vmode);
			break;
		default:
			LOG("EPC MDA: io_write at wrong offset:%02x\n", offset);
	}
}

READ8_MEMBER( isa8_epc_mda_device::io_read2 )
{
	return io_read(space, offset + 0x0f);
}

READ8_MEMBER( isa8_epc_mda_device::io_read )
{
	LOG("%s: %04x <- ???\n", FUNCNAME, offset);
	int data = 0xff;
	switch( offset )
	{
		case 0x04:
			LOGR(" - hd6845s address read\n");
			break;
		case 0x05:
			LOGR(" - hd6845s register read\n");
			data = m_hd6845s->register_r();
			break;
		case 0x08: // Mode 1 reg
			data = m_mode_control;
			LOGMODE(" - Mode register 1 read: %02x\n", data);
			break;
		case 0x0a: // Status reg: b7-6=00 board ID; b3 vert retrace; b0 horiz retrace; b5,4,2,1 unused
			data = (m_vsync != 0 ? 0x08 : 0x00) | (m_hsync != 0 ? 0x01 : 0x00);
			LOGSTAT(" - Status register read: %02x\n", data);
			break;
		case 0x0f: // Mode 2 reg
			data = m_mode_control2;
			LOGMODE(" - Mode register 2 read: %02x\n", data);
			break;
		default:
			LOG("EPC MDA: io_read at wrong offset:%02x\n", offset);
			logerror("EPC MDA: io_read at wrong offset:%02x\n", offset);
	}
	LOG(" !!!: %04x <- %02x\n", offset, data);
	return data;
}

inline int isa8_epc_mda_device::get_xres()
{
	return (m_vmode & VM_GRAPH) ? ( (m_vmode & VM_HOR640) ? 640 : 320 ) : 720;
}

inline int isa8_epc_mda_device::get_yres()
{
	return (m_vmode & VM_GRAPH) ? ( (m_vmode & VM_VER400) ? 400 : 200 ) : 400;
}

MC6845_UPDATE_ROW(isa8_epc_mda_device::crtc_update_row)
{
	uint32_t  *p = &bitmap.pix32(y);
	uint16_t  chr_base = ra;
	int i;

	// Get som debug data from a couple of rows now and then
	if ( y < (16 * 0 + 0x20) && (m_framecnt & 0xff) == 0 )
	{
		LOGROW("%11.6f %s\n - y:%d chr_base:%d ra:%d ma:%d x_count:%d\n", machine().time().as_double(), FUNCNAME,
			   y, y % 16, ra, ma, x_count);
	}

	// Video Off handling
	if ((m_mode_control & MR1_VIDEO) == 0)
	{
		for (int i = 0; i < get_xres(); i++)
		{
			bitmap.pix32(y, i) = rgb_t::black();
		}
	}

	// Graphic modes using only pixeldata, soft fonts are 8x8 or 8x16 but this is transparant to the code
	else if ((m_vmode & VM_GRAPH) != 0)
	{
		logerror("EPC MDA: graphic modes not supported yet\n");
	}

	// Text modes using one of two 9x16 fonts in character rom
	else
	{
		// Adjust row pointer if in monochrome text mode as we insert two scanlines per row of characters (see below)
		if (m_vmode & VM_MONO)
		{
			p = &bitmap.pix32((y / 14) * 16 + y % 14);
		}

		// Loop over each character in a row
		for ( i = 0; i < x_count; i++ )
		{
			uint16_t offset = ( ( ma + i ) << 1 ) & 0x0FFF;
			uint8_t chr = m_videoram[ offset ];
			uint8_t attr = m_videoram[ offset + 1 ];
			uint8_t data = m_chargen[ ((m_mode_control2 & MR2_CHRSET) ? 0x1000 : 0) + chr_base + chr * 16];

			// Default to light text on dark background
			uint8_t fg = 2;
			uint8_t bg = 0;

			if (y == 0 && i == 0) LOGCHRG(" - Offset: %04x Chr: '%c'[%02x] Attr: %02x Chr_base: %04x\n", offset, chr, chr, attr, chr_base);

			// Prepare some special monochrome emulation cases
			if ( m_vmode & VM_MONO)
			{
				// Handle invisible characters
				if ( (attr & (ATTR_FOREG | ATTR_BACKG)) == 0 )
				{
					data = 0x00;
				}
				// Handle reversed characters
				else if ( (attr & (ATTR_BACKG)) == ATTR_BACKG )
				{
					fg = 0;
					bg = 2;
				}
			}
			else // prepare some special color emulation cases
			{
				// Handle invisible characters
				if ( (attr & (ATTR_FOREG)) == ((attr & ATTR_BACKG) >> 4))
				{
					data = 0x00;
				}
				// Handle reversed characters
				else if ( (attr & ATTR_BACKG) == ATTR_BACKG ||
					  (attr & ATTR_FOREG) == 0 )
				{
					fg = 0;
					bg = 2;
				}
			}

			// Handle intense foreground
			if ((attr & ATTR_INTEN) != 0 && fg == 2)
			{
				fg = 3;
			}

			// Handle intense background if blinking is disabled
			if ((m_mode_control & MR1_BLINK) == 0 &&
				(attr & ATTR_BLINK) != 0 && bg == 2)
			{
				bg = 3;
			}

			// Handle cursor and blinks
			if ( i == (cursor_x))
			{
				if ( m_framecnt & 0x08 )
				{
					data = 0xFF;
				}
			}
			else
			{
				if ( (m_mode_control & MR1_BLINK) &&
					 ( attr & ATTR_BLINK ) && ( m_framecnt & 0x10 ) )
				{
					data = 0x00;
				}
			}

			*p = (*m_pal)[( data & 0x80 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x40 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x20 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x10 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x08 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x04 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x02 ) ? fg : bg]; p++;
			*p = (*m_pal)[( data & 0x01 ) ? fg : bg]; p++;
			if (chr >= 0xc0 && chr <= 0xdf)
				*p = (*m_pal)[( data & 0x01 ) ? fg : bg]; // 9th pixel col is a copy of col 8
			else
				*p = (*m_pal)[bg];                        // 9th pixel col is just background
			p++;

			// Insert two extra scanlines in monochrome text mode to get 400 lines and support underline, needs verification on actual hardware.
			// The technical manual says that the character box is 9x16 pixels in 80x25 character mode which equals 720x400 resolution but the
			// CRTC calls back for only 350 lines. Assumption is that there is hardware adding these lines and that handles underlining. In color
			// emulation text mode all 400 lines are called for in 80x25 and this mode does not support underlining according to the technical manual
			if ( ra == 13 && (m_vmode & VM_MONO) )
			{
				uint16_t row = ra + (y / 14) * 16; // Calculate correct row number including the extra 2 lines per each row of characters
				for ( int j = 0; j < 9; j++)
				{
					if (chr >= 0xb3 && chr <= 0xdf) // Handle the meta graphics characters
					{
						bitmap.pix32(row + 1, j + i * 9) = (*m_pal)[( data & (0x80 >> j) ) || (j == 8 && (data & 0x01)) ? fg : bg];
						bitmap.pix32(row + 2, j + i * 9) = (*m_pal)[( data & (0x80 >> j) ) || (j == 8 && (data & 0x01)) ? fg : bg];
					}
					else
					{
						// Handle underline
						bitmap.pix32(row + 1, j + i * 9) =(*m_pal)[( attr & ATTR_FOREG ) == ATTR_ULINE ? fg : bg];
						bitmap.pix32(row + 2, j + i * 9) = (*m_pal)[bg];
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------------
//  Port definitions
//--------------------------------------------------------------------
static INPUT_PORTS_START( epc_mda )
	PORT_START( "S1" )
	PORT_DIPNAME( 0x01, 0x00, "Color emulation") PORT_DIPLOCATION("S1:1")
	PORT_DIPSETTING( 0x00, "Disabled" )
	PORT_DIPSETTING( 0x01, "Enabled" )
	PORT_DIPUNUSED_DIPLOC(0x02, 0x02, "S1:2")

	PORT_START( "MONITOR" )
	PORT_CONFNAME( 0x01, 0x00, "Ericsson Monochrome HR Monitors") PORT_CHANGED_MEMBER( DEVICE_SELF, isa8_epc_mda_device, monitor_changed, 0 )
	PORT_CONFSETTING(    0x00, "Amber 3111")
	PORT_CONFSETTING(    0x01, "B&W 3712/3715")
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( isa8_epc_mda_device::monitor_changed )
{
	if ((m_io_monitor->read() & 1) == 1)
	{
		m_pal = &m_371x_pal;
	}
	else
	{
		m_pal = &m_3111_pal;
	}
}

ioport_constructor isa8_epc_mda_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( epc_mda );
}
