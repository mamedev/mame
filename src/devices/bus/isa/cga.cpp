// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

  Color Graphics Adapter (CGA) section


  Notes on Port 3D8
  (http://www.clipx.net/ng/interrupts_and_ports/ng2d045.php)

    Port 3D8  -  Color/VGA Mode control register

            xx1x xxxx  Attribute bit 7. 0=blink, 1=Intesity
            xxx1 xxxx  640x200 mode
            xxxx 1xxx  Enable video signal
            xxxx x1xx  Select B/W mode
            xxxx xx1x  Select graphics
            xxxx xxx1  80x25 text


    The usage of the above control register for various modes is:
            xx10 1100  40x25 alpha B/W
            xx10 1000  40x25 alpha color
            xx10 1101  80x25 alpha B/W
            xx10 1001  80x25 alpha color
            xxx0 1110  320x200 graph B/W
            xxx0 1010  320x200 graph color
            xxx1 1110  640x200 graph B/W


    PC1512 display notes

    The PC1512 built-in display adaptor is an emulation of IBM's CGA.  Unlike a
    real CGA, it is not built around a real MC6845 controller, and so attempts
    to get custom video modes out of it may not work as expected. Its 640x200
    CGA mode can be set up to be a 16-color mode rather than mono.

    If you program it with BIOS calls, the PC1512 behaves just like a real CGA,
    except:

    - The 'greyscale' text modes (0 and 2) behave just like the 'color'
      ones (1 and 3). On a color monitor both are in color; on a mono
      monitor both are in greyscale.
    - Mode 5 (the 'greyscale' graphics mode) displays in color, using
      an alternative color palette: Cyan, Red and White.
    - The undocumented 160x100x16 "graphics" mode works correctly.

    (source John Elliott http://www.seasip.info/AmstradXT/pc1512disp.html)


  Cursor signal handling:

  The alpha dots signal is set when a character pixel should be set. This signal is
  also set when the cursor should be displayed. The following formula for alpha
  dots is derived from the schematics:
  ALPHA DOTS = ( ( CURSOR DLY ) AND ( CURSOR BLINK ) ) OR ( ( ( NOT AT7 ) OR CURSOR DLY OR -BLINK OR NOT ENABLE BLINK ) AND ( CHG DOTS ) )

  -CURSOR BLINK = VSYNC DLY (LS393) (changes every 8 vsyncs)
  -BLINK = -CURSOR BLINK (LS393) (changes every 16 vsyncs)
  -CURSOR DLY = -CURSOR signal from mc6845 and LS174
  CHG DOTS = character pixel (from character rom)

  For non-blinking modes this formula reduces to:
  ALPHA DOTS = ( ( CURSOR DLY ) AND ( CURSOR BLINK ) ) OR ( CHG DOTS )

  This means the cursor switches on/off state every 8 vsyncs.


  For blinking modes this formula reduces to:
  ALPHA DOTS = ( ( CURSOR DLY ) AND ( CURSOR BLINK ) ) OR ( ( ( NOT AT7 ) OR CURSOR DLY OR -BLINK ) AND ( CHG DOTS ) )

  So, at the cursor location the attribute blinking is ignored and only regular
  cursor blinking takes place (state switches every 8 vsyncs). On non-cursor
  locations with the highest attribute bits set the character will switch
  on/off every 16 vsyncs. In all other cases the character is displayed as
  usual.


TODO:
- Update more drivers in MESS and MAME and unify with src/emu/video/pc_cga.c
- Separate out more cards/implementations

***************************************************************************/

#include "emu.h"
#include "video/mc6845.h"
#include "cga.h"
#include "video/cgapal.h"

#define VERBOSE_CGA 0       /* CGA (Color Graphics Adapter) */

#define CGA_SCREEN_NAME "screen"
#define CGA_MC6845_NAME "mc6845_cga"

#define CGA_LOG(N,M,A) \
	do { \
		if(VERBOSE_CGA>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine().time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

enum
{
	CGA_TEXT_INTEN = 0,
	CGA_TEXT_INTEN_ALT,
	CGA_TEXT_INTEN_CG,
	CGA_TEXT_BLINK,
	CGA_TEXT_BLINK_ALT,
	CGA_TEXT_BLINK_SI,
	CGA_GFX_1BPP,
	CGA_GFX_2BPP,
	CGA_GFX_4BPPL,
	CGA_GFX_4BPPH,
	PC1512_GFX_4BPP
};

/***************************************************************************

    Static declarations

***************************************************************************/

static INPUT_PORTS_START( cga )
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


static INPUT_PORTS_START( pc1512 )
	PORT_START( "cga_config" )
	PORT_CONFNAME( 0x03, 0x03, "CGA character set")
	PORT_CONFSETTING(0x00, "Greek")
	PORT_CONFSETTING(0x01, "Danish 2")
	PORT_CONFSETTING(0x02, "Danish 1")
	PORT_CONFSETTING(0x03, "Default")
	PORT_CONFNAME( 0x1C, 0x00, "CGA monitor type")
	PORT_CONFSETTING(0x00, "Colour RGB")
	PORT_CONFSETTING(0x04, "Mono RGB")
	PORT_BIT ( 0xE0, 0x20, IPT_UNUSED ) /* Chipset is always PC1512 */
INPUT_PORTS_END


/* Dipswitch for font selection */
#define CGA_FONT        (m_cga_config->read() & m_font_selection_mask)

/* Dipswitch for monitor selection */
#define CGA_MONITOR     (m_cga_config->read()&0x1C)
#define CGA_MONITOR_RGB         0x00    /* Colour RGB */
#define CGA_MONITOR_MONO        0x04    /* Greyscale RGB */
#define CGA_MONITOR_COMPOSITE   0x08    /* Colour composite */
#define CGA_MONITOR_TELEVISION  0x0C    /* Television */
#define CGA_MONITOR_LCD         0x10    /* LCD, eg PPC512 */


/* Dipswitch for chipset selection */
/* TODO: Get rid of this; these should be handled by separate classes */
#define CGA_CHIPSET     (m_cga_config->read() & 0xE0)
#define CGA_CHIPSET_IBM         0x00    /* Original IBM CGA */
#define CGA_CHIPSET_PC1512      0x20    /* PC1512 CGA subset */
#define CGA_CHIPSET_PC200       0x40    /* PC200 in CGA mode */
#define CGA_CHIPSET_ATI         0x60    /* ATI (supports Plantronics) */
#define CGA_CHIPSET_PARADISE    0x80    /* Paradise (used in PC1640) */

MC6845_UPDATE_ROW( isa8_cga_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	y = m_y;
	if(m_y >= bitmap.height())
		return;

	switch (m_update_row_type)
	{
		case CGA_TEXT_INTEN:
			cga_text_inten_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_INTEN_ALT:
			cga_text_inten_alt_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_INTEN_CG:
			cga_text_inten_comp_grey_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_BLINK:
			cga_text_blink_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_BLINK_ALT:
			cga_text_blink_alt_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		case CGA_TEXT_BLINK_SI:
			cga_text_blink_update_row_si(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
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


MC6845_UPDATE_ROW( isa8_cga_pc1512_device::crtc_update_row )
{
	if (m_update_row_type == -1)
		return;

	switch (m_update_row_type)
	{
		case PC1512_GFX_4BPP:
			pc1512_gfx_4bpp_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
		default:
			isa8_cga_device::crtc_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
			break;
	}
}


#define CGA_HCLK (XTAL_14_31818MHz/8)
#define CGA_LCLK (XTAL_14_31818MHz/16)


static MACHINE_CONFIG_FRAGMENT( cga )
	MCFG_SCREEN_ADD(CGA_SCREEN_NAME, RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_14_31818MHz,912,0,640,262,0,200)
	MCFG_SCREEN_UPDATE_DEVICE( DEVICE_SELF, isa8_cga_device, screen_update )

	MCFG_PALETTE_ADD("palette", /* CGA_PALETTE_SETS * 16*/ 65536 )

	MCFG_MC6845_ADD(CGA_MC6845_NAME, MC6845, CGA_SCREEN_NAME, XTAL_14_31818MHz/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(isa8_cga_device, crtc_update_row)
	MCFG_MC6845_OUT_HSYNC_CB(WRITELINE(isa8_cga_device, hsync_changed))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(isa8_cga_device, vsync_changed))
	MCFG_MC6845_RECONFIGURE_CB(isa8_cga_device, reconfigure)
	MCFG_VIDEO_SET_SCREEN(nullptr)
MACHINE_CONFIG_END


ROM_START( cga )
	/* IBM 1501981(CGA) and 1501985(MDA) Character rom */
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD("5788005.u33", 0x00000, 0x2000, CRC(0bf56d70) SHA1(c2a8b10808bf51a3c123ba3eb1e9dd608231916f)) /* "AMI 8412PI // 5788005 // (C) IBM CORP. 1981 // KOREA" */
ROM_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_CGA = &device_creator<isa8_cga_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor isa8_cga_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cga );
}

ioport_constructor isa8_cga_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( cga );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_cga_device::device_rom_region() const
{
	return ROM_NAME( cga );
}


//-------------------------------------------------
//  isa8_cga_device - constructor
//-------------------------------------------------

isa8_cga_device::isa8_cga_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		device_t(mconfig, ISA8_CGA, "IBM Color/Graphics Monitor Adapter", tag, owner, clock, "cga", __FILE__),
		device_isa8_card_interface(mconfig, *this),
		m_cga_config(*this, "cga_config"), m_framecnt(0), m_mode_control(0), m_color_select(0),
		m_update_row_type(-1), m_y(0), m_chr_gen_base(nullptr), m_chr_gen(nullptr), m_vsync(0), m_hsync(0),
		m_vram_size( 0x4000 ), m_plantronics(0),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
{
	m_chr_gen_offset[0] = m_chr_gen_offset[2] = 0x1800;
	m_chr_gen_offset[1] = m_chr_gen_offset[3] = 0x1000;
	m_font_selection_mask = 0x01;
	m_start_offset = 0;
	m_superimpose = false;
}

isa8_cga_device::isa8_cga_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_isa8_card_interface(mconfig, *this),
		m_cga_config(*this, "cga_config"), m_framecnt(0), m_mode_control(0), m_color_select(0),
		m_update_row_type(-1), m_y(0), m_chr_gen_base(nullptr), m_chr_gen(nullptr), m_vsync(0), m_hsync(0),
		m_vram_size( 0x4000 ), m_plantronics(0),
		m_palette(*this, "palette"),
		m_screen(*this, "screen")
{
	m_chr_gen_offset[0] = m_chr_gen_offset[2] = 0x1800;
	m_chr_gen_offset[1] = m_chr_gen_offset[3] = 0x1000;
	m_font_selection_mask = 0x01;
	m_start_offset = 0;
	m_superimpose = false;
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_cga_device::device_start()
{
	if (m_palette != nullptr && !m_palette->started())
		throw device_missing_dependencies();

	set_isa_device();
	m_vram.resize(m_vram_size);
	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_cga_device::io_read), this ), write8_delegate( FUNC(isa8_cga_device::io_write), this ) );
	m_isa->install_bank(0xb8000, 0xb8000 + MIN(0x8000,m_vram_size) - 1, 0, m_vram_size & 0x4000, "bank_cga", &m_vram[0]);

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

	m_chr_gen_base = memregion(subtag("gfx1").c_str())->base();
	m_chr_gen = m_chr_gen_base + m_chr_gen_offset[1];

	save_item(NAME(m_framecnt));
	save_item(NAME(m_mode_control));
	save_item(NAME(m_color_select));
	//save_item(NAME(m_status)); uncomment when used
	save_item(NAME(m_update_row_type));
	save_item(NAME(m_vsync));
	save_item(NAME(m_hsync));
	save_item(NAME(m_vram));
	save_item(NAME(m_plantronics));
	save_item(NAME(m_y));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_cga_device::device_reset()
{
	m_framecnt = 0;
	m_mode_control = 0;
	m_vsync = 0;
	m_hsync = 0;
	m_color_select = 0;
	m_y = 0;
	memset(m_palette_lut_2bpp, 0, sizeof(m_palette_lut_2bpp));
}

/***************************************************************************

    Methods

***************************************************************************/


UINT32 isa8_cga_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(CGA_MC6845_NAME);

	mc6845->screen_update( screen, bitmap, cliprect);

	/* Check for changes in font dipsetting */
	switch ( CGA_FONT )
	{
	case 0:
		m_chr_gen = m_chr_gen_base + m_chr_gen_offset[0];
		break;
	case 1:
		m_chr_gen = m_chr_gen_base + m_chr_gen_offset[1];
		break;
	case 2:
		m_chr_gen = m_chr_gen_base + m_chr_gen_offset[2];
		break;
	case 3:
		m_chr_gen = m_chr_gen_base + m_chr_gen_offset[3];
		break;
	}
	return 0;
}


const device_type ISA8_CGA_POISK2 = &device_creator<isa8_cga_poisk2_device>;

//-------------------------------------------------
//  isa8_cga_poisk2_device - constructor
//-------------------------------------------------

isa8_cga_poisk2_device::isa8_cga_poisk2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_POISK2, "ISA8_CGA_POISK2", tag, owner, clock, "cga_poisk2", __FILE__)
{
	m_chr_gen_offset[0] = 0x0000;
	m_chr_gen_offset[1] = 0x0800;
}

ROM_START( cga_poisk2 )
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD( "p2_ecga.rf4", 0x0000, 0x2000, CRC(d537f665) SHA1(d70f085b9b0cbd53df7c3122fbe7592998ba8fed))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_cga_poisk2_device::device_rom_region() const
{
	return ROM_NAME( cga_poisk2 );
}


/* for superimposing CGA over a different source video (i.e. tetriskr) */
const device_type ISA8_CGA_SUPERIMPOSE = &device_creator<isa8_cga_superimpose_device>;

//-------------------------------------------------
//  isa8_cga_superimpose_device - constructor
//-------------------------------------------------

isa8_cga_superimpose_device::isa8_cga_superimpose_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_SUPERIMPOSE, "ISA8_CGA_SUPERIMPOSE", tag, owner, clock, "cga_superimpose", __FILE__)
{
	m_superimpose = true;
}

isa8_cga_superimpose_device::isa8_cga_superimpose_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
		isa8_cga_device( mconfig, type, name, tag, owner, clock, shortname, source)
{
	m_superimpose = true;
}

/***************************************************************************
  Draw text mode with 40x25 characters (default) with high intensity bg.
  The character cell size is 16x8
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_text_inten_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_text_inten_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_chr_gen[ chr * 8 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = attr >> 4;

		if ( i == cursor_x && ( m_framecnt & 0x08 ) )
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
  Draw text mode with 40x25 characters (default) with high intensity bg.
  The character cell size is 16x8. Composite monitor, greyscale.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_text_inten_comp_grey_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_text_inten_comp_grey_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_chr_gen[ chr * 8 + ra ];
		UINT16 fg = 0x10 + ( attr & 0x0F );
		UINT16 bg = 0x10 + ( ( attr >> 4 ) & 0x07 );

		if ( i == cursor_x && ( m_framecnt & 0x08 ) )
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
  Draw text mode with 40x25 characters (default) with high intensity bg.
  The character cell size is 16x8
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_text_inten_alt_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_text_inten_alt_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_chr_gen[ chr * 8 + ra ];
		UINT16 fg = attr & 0x0F;

		if ( i == cursor_x && ( m_framecnt & 0x08 ) )
		{
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


/***************************************************************************
  Draw text mode with 40x25 characters (default) and blinking colors.
  The character cell size is 16x8
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_text_blink_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_text_blink_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_chr_gen[ chr * 8 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = (attr >> 4) & 0x07;

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

MC6845_UPDATE_ROW( isa8_cga_device::cga_text_blink_update_row_si )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_text_blink_update_row_si",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_chr_gen[ chr * 8 + ra ];
		UINT16 fg = attr & 0x0F;
		UINT16 bg = (attr >> 4) & 0x07;
		UINT8 xi;

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

		for(xi=0;xi<8;xi++)
		{
			UINT8 pen_data, dot;

			dot = (data & (1 << (7-xi)));
			pen_data = dot ? fg : bg;
			if(pen_data || dot)
				*p = palette[pen_data];
			p++;
		}
	}
}

/***************************************************************************
  Draw text mode with 40x25 characters (default) and blinking colors.
  The character cell size is 16x8
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_text_blink_alt_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_text_blink_alt_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ma + i ) << 1 ) & 0x3fff;
		UINT8 chr = videoram[ offset ];
		UINT8 attr = videoram[ offset +1 ];
		UINT8 data = m_chr_gen[ chr * 8 + ra ];
		UINT16 fg = attr & 0x07;
		UINT16 bg = 0;

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


/* The lo-res (320x200) graphics mode on a colour composite monitor */

MC6845_UPDATE_ROW( isa8_cga_device::cga_gfx_4bppl_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_gfx_4bppl_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
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


#if 0
/* The hi-res graphics mode on a colour composite monitor
 *
 * The different scaling factors mean that the '160x200' versions of screens
 * are the same size as the normal colour ones.
 */

static const UINT8 yc_lut2[4] = { 0, 182, 71, 255 };

static const UINT8 yc_lut[16][8] =
{
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, /* black */
	{ 0, 0, 0, 0, 1, 1, 1, 1 }, /* blue */
	{ 0, 1, 1, 1, 1, 0, 0, 0 }, /* green */
	{ 0, 0, 1, 1, 1, 1, 0, 0 }, /* cyan */
	{ 1, 1, 0, 0, 0, 0, 1, 1 }, /* red */
	{ 1, 0, 0, 0, 0, 1, 1, 1 }, /* magenta */
	{ 1, 1, 1, 1, 0, 0, 0, 0 }, /* yellow */
	{ 1, 1, 1, 1, 1, 1, 1, 1 }, /* white */
	/* Intensity set */
	{ 2, 2, 2, 2, 2, 2, 2, 2 }, /* black */
	{ 2, 2, 2, 2, 3, 3, 3, 3 }, /* blue */
	{ 2, 3, 3, 3, 3, 2, 2, 2 }, /* green */
	{ 2, 2, 3, 3, 3, 3, 2, 2 }, /* cyan */
	{ 3, 3, 2, 2, 2, 2, 3, 3 }, /* red */
	{ 3, 2, 2, 2, 2, 3, 3, 3 }, /* magenta */
	{ 3, 3, 3, 3, 2, 2, 2, 2 }, /* yellow */
	{ 3, 3, 3, 3, 3, 3, 3, 3 }, /* white */
};
#endif

MC6845_UPDATE_ROW( isa8_cga_device::cga_gfx_4bpph_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_gfx_4bpph_update_row",("\n"));

	for ( i = 0; i < x_count; i++ )
	{
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


/***************************************************************************
  Draw graphics mode with 320x200 pixels (default) with 2 bits/pixel.
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
  cga fetches 2 byte per mc6845 access.
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_gfx_2bpp_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_gfx_2bpp_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( ra & 1 ) << 13 );
		UINT8 data = videoram[ offset ];

		*p = palette[m_palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[m_palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[m_palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[m_palette_lut_2bpp[   data        & 0x03 ]]; p++;

		data = videoram[ offset+1 ];

		*p = palette[m_palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[m_palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[m_palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[m_palette_lut_2bpp[   data        & 0x03 ]]; p++;
	}
}



/***************************************************************************
  Draw graphics mode with 640x200 pixels (default).
  The cell size is 1x1 (1 scanline is the real default)
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
***************************************************************************/

MC6845_UPDATE_ROW( isa8_cga_device::cga_gfx_1bpp_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8   fg = m_color_select & 0x0F;
	int i;

	if ( y == 0 ) CGA_LOG(1,"cga_gfx_1bpp_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
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


WRITE_LINE_MEMBER( isa8_cga_device::hsync_changed )
{
	m_hsync = state ? 1 : 0;
	if(state && !m_vsync)
	{
		m_screen->update_now();
		m_y++;
	}
}


WRITE_LINE_MEMBER( isa8_cga_device::vsync_changed )
{
	if ( state )
	{
		m_framecnt++;
	}
	else
	{
		m_screen->reset_origin();
		m_y = 0;
	}
	m_vsync = state ? 9 : 0;
}

MC6845_RECONFIGURE( isa8_cga_device::reconfigure )
{
	rectangle curvisarea = m_screen->visible_area();
	m_screen->set_visible_area(visarea.min_x, visarea.max_x, curvisarea.min_y, curvisarea.max_y);
}

void isa8_cga_device::set_palette_luts(void)
{
	/* Setup 2bpp palette lookup table */
	if ( m_mode_control & 0x10 )
	{
		m_palette_lut_2bpp[0] = 0;
	}
	else
	{
		m_palette_lut_2bpp[0] = m_color_select & 0x0F;
	}
	if ( m_mode_control & 0x04 )
	{
		m_palette_lut_2bpp[1] = ( ( m_color_select & 0x10 ) >> 1 ) | 3;
		m_palette_lut_2bpp[2] = ( ( m_color_select & 0x10 ) >> 1 ) | 4;
		m_palette_lut_2bpp[3] = ( ( m_color_select & 0x10 ) >> 1 ) | 7;
	}
	else
	{
		if ( m_color_select & 0x20 )
		{
			m_palette_lut_2bpp[1] = ( ( m_color_select & 0x10 ) >> 1 ) | 3;
			m_palette_lut_2bpp[2] = ( ( m_color_select & 0x10 ) >> 1 ) | 5;
			m_palette_lut_2bpp[3] = ( ( m_color_select & 0x10 ) >> 1 ) | 7;
		}
		else
		{
			m_palette_lut_2bpp[1] = ( ( m_color_select & 0x10 ) >> 1 ) | 2;
			m_palette_lut_2bpp[2] = ( ( m_color_select & 0x10 ) >> 1 ) | 4;
			m_palette_lut_2bpp[3] = ( ( m_color_select & 0x10 ) >> 1 ) | 6;
		}
	}
	//logerror("2bpp lut set to %d,%d,%d,%d\n", cga.palette_lut_2bpp[0], cga.palette_lut_2bpp[1], cga.palette_lut_2bpp[2], cga.palette_lut_2bpp[3]);
}

/*
 *  rW  CGA mode control register (see #P138)
 *
 *  x x x 0 1 0 0 0 - 320x200, 40x25 text. Colour on RGB and composite monitors.
 *  x x x 0 1 0 0 1 - 640x200, 80x25 text. Colour on RGB and composite monitors.
 *  x x x 0 1 0 1 0 - 320x200 graphics. Colour on RGB and composite monitors.
 *  x x x 0 1 0 1 1 - unknown/invalid.
 *  x x x 0 1 1 0 0 - 320x200, 40x25 text. Colour on RGB, greyscale on composite monitors.
 *  x x x 0 1 1 0 1 - 640x200, 80x25 text. Colour on RGB, greyscale on composite monitors.
 *  x x x 0 1 1 1 0 - 320x200 graphics. Alternative palette on RGB, greyscale on composite monitors.
 *  x x x 0 1 1 1 1 - unknown/invalid.
 *  x x x 1 1 0 0 0 - unknown/invalid.
 *  x x x 1 1 0 0 1 - unknown/invalid.
 *  x x x 1 1 0 1 0 - 160x200/640x200 graphics. 640x200 ?? on RGB monitor, 160x200 on composite monitor.
 *  x x x 1 1 0 1 1 - unknown/invalid.
 *  x x x 1 1 1 0 0 - unknown/invalid.
 *  x x x 1 1 1 0 1 - unknown/invalid.
 *  x x x 1 1 1 1 0 - 640x200 graphics. Colour on black on RGB monitor, monochrome on composite monitor.
 *  x x x 1 1 1 1 1 - unknown/invalid.
 */
void isa8_cga_device::mode_control_w(UINT8 data)
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(CGA_MC6845_NAME);
	UINT8 monitor = CGA_MONITOR;

	m_mode_control = data;

	//logerror("mode set to %02X\n", cga.mode_control & 0x3F );
	switch ( m_mode_control & 0x3F )
	{
	case 0x08: case 0x09: case 0x0C: case 0x0D:
		mc6845->set_hpixels_per_column( 8 );
		if ( monitor == CGA_MONITOR_COMPOSITE )
		{
			if ( m_mode_control & 0x04 )
			{
				/* Composite greyscale */
				m_update_row_type = CGA_TEXT_INTEN_CG;
			}
			else
			{
				/* Composite colour */
				m_update_row_type = CGA_TEXT_INTEN;
			}
		}
		else
		{
			/* RGB colour */
			m_update_row_type = CGA_TEXT_INTEN;
		}
		break;
	case 0x0A: case 0x0B: case 0x2A: case 0x2B:
		mc6845->set_hpixels_per_column( 8 );
		if ( monitor == CGA_MONITOR_COMPOSITE )
		{
			m_update_row_type = CGA_GFX_4BPPL;
		}
		else
		{
			m_update_row_type = CGA_GFX_2BPP;
		}
		break;
	case 0x0E: case 0x0F: case 0x2E: case 0x2F:
		mc6845->set_hpixels_per_column( 8 );
		m_update_row_type = CGA_GFX_2BPP;
		break;
	case 0x18: case 0x19: case 0x1C: case 0x1D:
		mc6845->set_hpixels_per_column( 8 );
		m_update_row_type = CGA_TEXT_INTEN_ALT;
		break;
	case 0x1A: case 0x1B: case 0x3A: case 0x3B:
		mc6845->set_hpixels_per_column( 16 );
		if ( monitor == CGA_MONITOR_COMPOSITE )
		{
			m_update_row_type = CGA_GFX_4BPPH;
		}
		else
		{
			m_update_row_type = CGA_GFX_1BPP;
		}
		break;
	case 0x1E: case 0x1F: case 0x3E: case 0x3F:
		mc6845->set_hpixels_per_column( 16 );
		m_update_row_type = CGA_GFX_1BPP;
		break;
	case 0x28: case 0x29: case 0x2C: case 0x2D:
		mc6845->set_hpixels_per_column( 8 );
		if ( monitor == CGA_MONITOR_COMPOSITE )
		{
			if ( m_mode_control & 0x04 )
			{
				/* Composite greyscale */
				m_update_row_type = m_superimpose ? CGA_TEXT_BLINK_SI : CGA_TEXT_BLINK;
			}
			else
			{
				/* Composite colour */
				m_update_row_type = m_superimpose ? CGA_TEXT_BLINK_SI : CGA_TEXT_BLINK;
			}
		}
		else
		{
			/* RGB colour */
			m_update_row_type = m_superimpose ? CGA_TEXT_BLINK_SI : CGA_TEXT_BLINK;
		}
		break;
	case 0x38: case 0x39: case 0x3C: case 0x3D:
		mc6845->set_hpixels_per_column( 8 );
		m_update_row_type = CGA_TEXT_BLINK_ALT;
		break;
	default:
		m_update_row_type = -1;
		break;
	}

	// The lowest bit of the mode register selects, among others, the
	// input clock to the 6845.
	mc6845->set_clock( ( m_mode_control & 1 ) ? CGA_HCLK : CGA_LCLK );

	set_palette_luts();
}



/*
 * Select Plantronics modes
 */
void isa8_cga_device::plantronics_w(UINT8 data)
{
	if ( ( CGA_CHIPSET ) != CGA_CHIPSET_ATI) return;

	data &= 0x70;   /* Only bits 6-4 are used */
	m_plantronics = data;
}



/*************************************************************************
 *
 *      CGA
 *      color graphics adapter
 *
 *************************************************************************/


READ8_MEMBER( isa8_cga_device::io_read )
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(CGA_MC6845_NAME);
	UINT8 data = 0xff;

	switch( offset )
	{
		case 0: case 2: case 4: case 6:
			/* return last written mc6845 address value here? */
			break;
		case 1: case 3: case 5: case 7:
			data = mc6845->register_r( space, offset );
			break;
		case 10:
			data = m_vsync | ( ( data & 0x40 ) >> 4 ) | m_hsync;
			break;
	}
	return data;
}



WRITE8_MEMBER( isa8_cga_device::io_write )
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(CGA_MC6845_NAME);

	switch(offset) {
	case 0: case 2: case 4: case 6:
		mc6845->address_w( space, offset, data );
		break;
	case 1: case 3: case 5: case 7:
		mc6845->register_w( space, offset, data );
		break;
	case 8:
		mode_control_w(data);
		break;
	case 9:
		m_color_select = data;
		set_palette_luts();
		break;
	case 0x0d:
		plantronics_w(data);
		break;
	}
}



/* Old plantronics rendering code, leaving it uncommented until we have re-implemented it */

//
// From choosevideomode:
//
//      /* Plantronics high-res */
//      if ((cga.mode_control & 2) && (cga.plantronics & 0x20))
//          proc = cga_pgfx_2bpp;
//      /* Plantronics low-res */
//      if ((cga.mode_control & 2) && (cga.plantronics & 0x10))
//          proc = cga_pgfx_4bpp;
//

//INLINE void pgfx_plot_unit_4bpp(bitmap_ind16 &bitmap,
//                           int x, int y, int offs)
//{
//  int color, values[2];
//  int i;
//
//  if (cga.plantronics & 0x40)
//  {
//      values[0] = videoram[offs | 0x4000];
//      values[1] = videoram[offs];
//  }
//  else
//  {
//      values[0] = videoram[offs];
//      values[1] = videoram[offs | 0x4000];
//  }
//  for (i=3; i>=0; i--)
//  {
//      color = ((values[0] & 0x3) << 1) |
//          ((values[1] & 2)   >> 1) |
//          ((values[1] & 1)   << 3);
//      bitmap.pix16(y, x+i) = Machine->pens[color];
//      values[0]>>=2;
//      values[1]>>=2;
//  }
//}
//
//
//
///***************************************************************************
//  Draw graphics mode with 640x200 pixels (default) with 2 bits/pixel.
//  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
//  Second plane at CGA_base + 0x4000 / 0x6000
//***************************************************************************/
//
//static void cga_pgfx_4bpp(bitmap_ind16 &bitmap, struct mscrtc6845 *crtc)
//{
//  int i, sx, sy, sh;
//  int offs = mscrtc6845_get_start(crtc)*2;
//  int lines = mscrtc6845_get_char_lines(crtc);
//  int height = mscrtc6845_get_char_height(crtc);
//  int columns = mscrtc6845_get_char_columns(crtc)*2;
//
//  for (sy=0; sy<lines; sy++,offs=(offs+columns)&0x1fff)
//  {
//      for (sh=0; sh<height; sh++, offs|=0x2000)
//      {
//          // char line 0 used as a12 line in graphic mode
//          if (!(sh & 1))
//          {
//              for (i=offs, sx=0; sx<columns; sx++, i=(i+1)&0x1fff)
//              {
//                  pgfx_plot_unit_4bpp(bitmap, sx*4, sy*height+sh, i);
//              }
//          }
//          else
//          {
//              for (i=offs|0x2000, sx=0; sx<columns; sx++, i=((i+1)&0x1fff)|0x2000)
//              {
//                  pgfx_plot_unit_4bpp(bitmap, sx*4, sy*height+sh, i);
//              }
//          }
//      }
//  }
//}
//
//
//
//INLINE void pgfx_plot_unit_2bpp(bitmap_ind16 &bitmap,
//                   int x, int y, const UINT16 *palette, int offs)
//{
//  int i;
//  UINT8 bmap[2], values[2];
//  UINT16 *dest;
//
//  if (cga.plantronics & 0x40)
//  {
//      values[0] = videoram[offs];
//      values[1] = videoram[offs | 0x4000];
//  }
//  else
//  {
//      values[0] = videoram[offs | 0x4000];
//      values[1] = videoram[offs];
//  }
//  bmap[0] = bmap[1] = 0;
//  for (i=3; i>=0; i--)
//  {
//      bmap[0] = bmap[0] << 1; if (values[0] & 0x80) bmap[0] |= 1;
//      bmap[0] = bmap[0] << 1; if (values[1] & 0x80) bmap[0] |= 1;
//      bmap[1] = bmap[1] << 1; if (values[0] & 0x08) bmap[1] |= 1;
//      bmap[1] = bmap[1] << 1; if (values[1] & 0x08) bmap[1] |= 1;
//      values[0] = values[0] << 1;
//      values[1] = values[1] << 1;
//  }
//
//  dest = &bitmap.pix16(y, x);
//  *(dest++) = palette[(bmap[0] >> 6) & 0x03];
//  *(dest++) = palette[(bmap[0] >> 4) & 0x03];
//  *(dest++) = palette[(bmap[0] >> 2) & 0x03];
//  *(dest++) = palette[(bmap[0] >> 0) & 0x03];
//  *(dest++) = palette[(bmap[1] >> 6) & 0x03];
//  *(dest++) = palette[(bmap[1] >> 4) & 0x03];
//  *(dest++) = palette[(bmap[1] >> 2) & 0x03];
//  *(dest++) = palette[(bmap[1] >> 0) & 0x03];
//}
//
//
//
///***************************************************************************
//  Draw graphics mode with 320x200 pixels (default) with 2 bits/pixel.
//  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
//  cga fetches 2 byte per mscrtc6845 access (not modeled here)!
//***************************************************************************/
//
//static void cga_pgfx_2bpp(bitmap_ind16 &bitmap, struct mscrtc6845 *crtc)
//{
//  int i, sx, sy, sh;
//  int offs = mscrtc6845_get_start(crtc)*2;
//  int lines = mscrtc6845_get_char_lines(crtc);
//  int height = mscrtc6845_get_char_height(crtc);
//  int columns = mscrtc6845_get_char_columns(crtc)*2;
//  int colorset = cga.color_select & 0x3F;
//  const UINT16 *palette;
//
//  /* Most chipsets use bit 2 of the mode control register to
//   * access a third palette. But not consistently. */
//  pc_cga_check_palette();
//  switch(CGA_CHIPSET)
//  {
//      /* The IBM Professional Graphics Controller behaves like
//       * the PC1512, btw. */
//      case CGA_CHIPSET_PC1512:
//      if ((colorset < 32) && (cga.mode_control & 4)) colorset += 64;
//      break;
//
//      case CGA_CHIPSET_IBM:
//      case CGA_CHIPSET_PC200:
//      case CGA_CHIPSET_ATI:
//      case CGA_CHIPSET_PARADISE:
//      if (cga.mode_control & 4) colorset = (colorset & 0x1F) + 64;
//      break;
//  }
//
//
//  /* The fact that our palette is located in cga_colortable is a vestigial
//   * aspect from when we were doing that ugly trick where drawgfx() would
//   * handle graphics drawing.  Truthfully, we should probably be using
//   * palette_set_color_rgb() here and not doing the palette lookup in the loop
//   */
//  palette = &cga_colortable[256*2 + 16*2] + colorset * 4;
//
//  for (sy=0; sy<lines; sy++,offs=(offs+columns)&0x1fff) {
//
//      for (sh=0; sh<height; sh++)
//      {
//          if (!(sh&1)) { // char line 0 used as a12 line in graphic mode
//              for (i=offs, sx=0; sx<columns; sx++, i=(i+1)&0x1fff)
//              {
//                  pgfx_plot_unit_2bpp(bitmap, sx*8, sy*height+sh, palette, i);
//              }
//          }
//          else
//          {
//              for (i=offs|0x2000, sx=0; sx<columns; sx++, i=((i+1)&0x1fff)|0x2000)
//              {
//                  pgfx_plot_unit_2bpp(bitmap, sx*8, sy*height+sh, palette, i);
//              }
//          }
//      }
//  }
//}


MC6845_UPDATE_ROW( isa8_cga_pc1512_device::pc1512_gfx_4bpp_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT16  offset_base = ra << 13;
	int j;

	if ( y == 0 ) CGA_LOG(1,"pc1512_gfx_4bpp_update_row",("\n"));
	for ( j = 0; j < x_count; j++ )
	{
		UINT16 offset = offset_base | ( ( ma + j ) & 0x1FFF );
		UINT16 i = ( m_color_select & 8 ) ? videoram[ isa8_cga_pc1512_device::vram_offset[3] | offset ] << 3 : 0;
		UINT16 r = ( m_color_select & 4 ) ? videoram[ isa8_cga_pc1512_device::vram_offset[2] | offset ] << 2 : 0;
		UINT16 g = ( m_color_select & 2 ) ? videoram[ isa8_cga_pc1512_device::vram_offset[1] | offset ] << 1 : 0;
		UINT16 b = ( m_color_select & 1 ) ? videoram[ isa8_cga_pc1512_device::vram_offset[0] | offset ]      : 0;

		*p = palette[( ( i & 0x400 ) | ( r & 0x200 ) | ( g & 0x100 ) | ( b & 0x80 ) ) >> 7]; p++;
		*p = palette[( ( i & 0x200 ) | ( r & 0x100 ) | ( g & 0x080 ) | ( b & 0x40 ) ) >> 6]; p++;
		*p = palette[( ( i & 0x100 ) | ( r & 0x080 ) | ( g & 0x040 ) | ( b & 0x20 ) ) >> 5]; p++;
		*p = palette[( ( i & 0x080 ) | ( r & 0x040 ) | ( g & 0x020 ) | ( b & 0x10 ) ) >> 4]; p++;
		*p = palette[( ( i & 0x040 ) | ( r & 0x020 ) | ( g & 0x010 ) | ( b & 0x08 ) ) >> 3]; p++;
		*p = palette[( ( i & 0x020 ) | ( r & 0x010 ) | ( g & 0x008 ) | ( b & 0x04 ) ) >> 2]; p++;
		*p = palette[( ( i & 0x010 ) | ( r & 0x008 ) | ( g & 0x004 ) | ( b & 0x02 ) ) >> 1]; p++;
		*p = palette[  ( i & 0x008 ) | ( r & 0x004 ) | ( g & 0x002 ) | ( b & 0x01 )       ]; p++;
	}
}


WRITE8_MEMBER( isa8_cga_pc1512_device::io_write )
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(CGA_MC6845_NAME);

	switch (offset)
	{
	case 0: case 2: case 4: case 6:
		data &= 0x1F;
		mc6845->address_w( space, offset, data );
		m_mc6845_address = data;
		break;

	case 1: case 3: case 5: case 7:
		if ( ! m_mc6845_locked_register[m_mc6845_address] )
		{
			mc6845->register_w( space, offset, data );
			if ( isa8_cga_pc1512_device::mc6845_writeonce_register[m_mc6845_address] )
			{
				m_mc6845_locked_register[m_mc6845_address] = 1;
			}
		}
		break;

	case 0x8:
		/* Check if we're changing to graphics mode 2 */
		if ( ( m_mode_control & 0x12 ) != 0x12 && ( data & 0x12 ) == 0x12 )
		{
			m_write = 0x0F;
		}
		else
		{
			membank("bank1")->set_base(&m_vram[isa8_cga_pc1512_device::vram_offset[0]]);
		}
		m_mode_control = data;
		switch( m_mode_control & 0x3F )
		{
		case 0x08: case 0x09: case 0x0C: case 0x0D:
			mc6845->set_hpixels_per_column( 8 );
			m_update_row_type = CGA_TEXT_INTEN;
			break;
		case 0x0A: case 0x0B: case 0x2A: case 0x2B:
			mc6845->set_hpixels_per_column( 8 );
			if ( ( CGA_MONITOR ) == CGA_MONITOR_COMPOSITE )
			{
				m_update_row_type = CGA_GFX_4BPPL;
			}
			else
			{
				m_update_row_type = CGA_GFX_2BPP;
			}
			break;
		case 0x0E: case 0x0F: case 0x2E: case 0x2F:
			mc6845->set_hpixels_per_column( 8 );
			m_update_row_type = CGA_GFX_2BPP;
			break;
		case 0x18: case 0x19: case 0x1C: case 0x1D:
			mc6845->set_hpixels_per_column( 8 );
			m_update_row_type = CGA_TEXT_INTEN_ALT;
			break;
		case 0x1A: case 0x1B: case 0x3A: case 0x3B:
			mc6845->set_hpixels_per_column( 8 );
			m_update_row_type = PC1512_GFX_4BPP;
			break;
		case 0x1E: case 0x1F: case 0x3E: case 0x3F:
			mc6845->set_hpixels_per_column( 16 );
			m_update_row_type = CGA_GFX_1BPP;
			break;
		case 0x28: case 0x29: case 0x2C: case 0x2D:
			mc6845->set_hpixels_per_column( 8 );
			m_update_row_type = CGA_TEXT_BLINK;
			break;
		case 0x38: case 0x39: case 0x3C: case 0x3D:
			mc6845->set_hpixels_per_column( 8 );
			m_update_row_type = CGA_TEXT_BLINK_ALT;
			break;
		default:
			m_update_row_type = -1;
			break;
		}
		break;

	case 0xd:
		m_write = data;
		break;

	case 0xe:
		m_read = data;
		if ( ( m_mode_control & 0x12 ) == 0x12 )
		{
			membank("bank1")->set_base(&m_vram[isa8_cga_pc1512_device::vram_offset[data & 3]]);
		}
		break;

	default:
		isa8_cga_device::io_write(space, offset,data);
		break;
	}
}


READ8_MEMBER( isa8_cga_pc1512_device::io_read )
{
	UINT8 data;

	switch (offset)
	{
	case 0xd:
		data = m_write;
		break;

	case 0xe:
		data = m_read;
		break;

	default:
		data = isa8_cga_device::io_read(space, offset);
		break;
	}
	return data;
}


WRITE8_MEMBER( isa8_cga_pc1512_device::vram_w )
{
	if ( ( m_mode_control & 0x12 ) == 0x12 )
	{
		if (m_write & 1)
			m_vram[offset+isa8_cga_pc1512_device::vram_offset[0]] = data; /* blue plane */
		if (m_write & 2)
			m_vram[offset+isa8_cga_pc1512_device::vram_offset[1]] = data; /* green */
		if (m_write & 4)
			m_vram[offset+isa8_cga_pc1512_device::vram_offset[2]] = data; /* red */
		if (m_write & 8)
			m_vram[offset+isa8_cga_pc1512_device::vram_offset[3]] = data; /* intensity (text, 4color) */
	}
	else
	{
		m_vram[offset + isa8_cga_pc1512_device::vram_offset[0]] = data;
	}
}


const device_type ISA8_CGA_PC1512 = &device_creator<isa8_cga_pc1512_device>;

const offs_t isa8_cga_pc1512_device::vram_offset[4]= { 0x0000, 0x4000, 0x8000, 0xC000 };
const UINT8 isa8_cga_pc1512_device::mc6845_writeonce_register[31] =
{
	1, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//-------------------------------------------------
//  isa8_cga_pc1512_device - constructor
//-------------------------------------------------

isa8_cga_pc1512_device::isa8_cga_pc1512_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_PC1512, "ISA8_CGA_PC1512", tag, owner, clock, "cga_pc1512", __FILE__), m_write(0), m_read(0), m_mc6845_address(0)
{
	m_vram_size = 0x10000;
	m_chr_gen_offset[0] = 0x0000;
	m_chr_gen_offset[1] = 0x0800;
	m_chr_gen_offset[2] = 0x1000;
	m_chr_gen_offset[3] = 0x1800;
}


const rom_entry *isa8_cga_pc1512_device::device_rom_region() const
{
	return nullptr;
}


ioport_constructor isa8_cga_pc1512_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pc1512 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_cga_pc1512_device::device_start()
{
	isa8_cga_device::device_start();

	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_cga_pc1512_device::io_read), this ), write8_delegate( FUNC(isa8_cga_pc1512_device::io_write), this ) );
	m_isa->install_bank(0xb8000, 0xbbfff, 0, 0, "bank1", &m_vram[0]);

	address_space &space = machine().firstcpu->space( AS_PROGRAM );

	space.install_write_handler( 0xb8000, 0xbbfff, 0, 0x0C000, write8_delegate( FUNC(isa8_cga_pc1512_device::vram_w), this ) );
}

void isa8_cga_pc1512_device::device_reset()
{
	isa8_cga_device::device_reset();

	m_write = 0x0f;
	m_read = 0;
	m_mc6845_address = 0;
	for ( int i = 0; i < 31; i++ )
	{
		m_mc6845_locked_register[i] = 0;
	}

	membank("bank1")->set_base(&m_vram[isa8_cga_pc1512_device::vram_offset[0]]);
}

void isa8_wyse700_device::change_resolution(UINT8 mode)
{
	int width = 0, height = 0;
	if (mode & 2) {
		machine().root_device().membank("bank_wy1")->set_base(&m_vram[0x10000]);
	} else {
		machine().root_device().membank("bank_wy1")->set_base(&m_vram[0x00000]);
	}
	if ((m_control & 0xf0) == (mode & 0xf0)) return;

	switch(mode & 0xf0) {
		case 0xc0: width = 1280; height = 800; break;
		case 0xa0: width = 1280; height = 400; break;
		case 0x80: width = 640; height = 400; break;
		case 0x00: width = 640; height = 400; break; // unhandled
	}
	rectangle visarea(0, width-1, 0, height-1);
	subdevice<screen_device>(CGA_SCREEN_NAME)->configure(width, height, visarea, HZ_TO_ATTOSECONDS(60));

}

WRITE8_MEMBER( isa8_wyse700_device::io_write )
{
	switch (offset)
	{
	case 0xd:
		m_bank_offset = data;
		break;

	case 0xe:
		m_bank_base = data;
		break;

	case 0xf:
		change_resolution(data);
		m_control = data;
		break;
	default:
		isa8_cga_device::io_write(space, offset,data);
		break;
	}
}


READ8_MEMBER( isa8_wyse700_device::io_read )
{
	UINT8 data;

	switch (offset)
	{
	case 0xd:
		data = m_bank_offset;
		break;

	case 0xe:
		data = m_bank_base;
		break;

	case 0xf:
		data = m_control;
		break;
	default:
		data = isa8_cga_device::io_read(space, offset);
		break;
	}
	return data;
}


const device_type ISA8_WYSE700 = &device_creator<isa8_wyse700_device>;


//-------------------------------------------------
//  isa8_wyse700_device - constructor
//-------------------------------------------------

isa8_wyse700_device::isa8_wyse700_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_WYSE700, "Wyse 700", tag, owner, clock, "wyse700", __FILE__), m_bank_offset(0), m_bank_base(0), m_control(0)
{
	m_vram_size = 0x20000;
	m_start_offset = 0x18000;
}

/*
Character ROMs:

250211-03.e5: Character ROM  Label: "(C) WYSE TECH / REV.A / 250211-03"
250212-03.f5: Character ROM  Label: "(C) WYSE TECH / REV.A / 250212-03"

Not dumped:

250026-03.2d: MC68705 MCU  Label: "(C) WYSE TECH / REV.1 / 250026-03"
250270-01.8b: PAL?         Label: "250270-01"
250024-01.8g: PAL?         Label: "250024-01"
250210-01.c2: PAL?         Label: "250210-01"
*/
ROM_START( wyse700 )
	ROM_REGION(0x4000,"gfx1", 0)
	ROM_LOAD( "250211-03.e5", 0x0000, 0x2000, CRC(58b61f63) SHA1(29ecb7cf7d07d692f0fc54e2dea8389f17a65f1a))
	ROM_LOAD( "250212-03.f5", 0x2000, 0x2000, CRC(6930d741) SHA1(1beeb133c5e39eee9914bdc5924039d70b5edcad))
ROM_END

const rom_entry *isa8_wyse700_device::device_rom_region() const
{
	return ROM_NAME( wyse700 );
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_wyse700_device::device_start()
{
	isa8_cga_device::device_start();

	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_wyse700_device::io_read), this ), write8_delegate( FUNC(isa8_wyse700_device::io_write), this ) );
	m_isa->install_bank(0xa0000, 0xaffff, 0, 0, "bank_wy1", &m_vram[0x00000]);
	m_isa->install_bank(0xb0000, 0xbffff, 0, 0, "bank_cga", &m_vram[0x10000]);
}

void isa8_wyse700_device::device_reset()
{
	isa8_cga_device::device_reset();
	m_control = 0;
	m_bank_offset = 0;
	m_bank_base = 0;
}

UINT32 isa8_wyse700_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (m_control & 0x08) {
		const rgb_t *palette = m_palette->palette()->entry_list_raw();
		UINT8 fg = m_color_select & 0x0F;
		UINT32 addr = 0;
		for (int y = 0; y < 800; y++) {
			UINT8 *src = &m_vram[addr];

			if (y & 1) {
				src += 0x10000;
				addr += 160;
			}

			for (int x = 0; x < (1280 / 8); x++) {
				UINT8 val = src[x];

				for (int i = 0; i < 8; i++) {
					bitmap.pix32(y,x*8+i) = (val & 0x80) ? palette[fg] : palette[0x00];
					val <<= 1;
				}
			}
		}
	} else {
		return isa8_cga_device::screen_update(screen, bitmap, cliprect);
	}
	return 0;
}


const device_type ISA8_EC1841_0002 = &device_creator<isa8_ec1841_0002_device>;

//-------------------------------------------------
//  isa8_ec1841_0002_device - constructor
//-------------------------------------------------

isa8_ec1841_0002_device::isa8_ec1841_0002_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_EC1841_0002, "EC 1841.0002 (CGA)", tag, owner, clock, "ec1841_0002", __FILE__), m_p3df(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_ec1841_0002_device::device_start()
{
	isa8_cga_device::device_start();

	m_isa->install_device(0x3d0, 0x3df, 0, 0, read8_delegate( FUNC(isa8_ec1841_0002_device::io_read), this ), write8_delegate( FUNC(isa8_ec1841_0002_device::io_write), this ) );
}

void isa8_ec1841_0002_device::device_reset()
{
	isa8_cga_device::device_reset();
	m_p3df = 0;
}

WRITE8_MEMBER( isa8_ec1841_0002_device::char_ram_write )
{
	offset ^= BIT(offset, 12);
//  logerror("write char ram %04x %02x\n",offset,data);
	m_chr_gen_base[offset + 0x0000] = data;
	m_chr_gen_base[offset + 0x0800] = data;
	m_chr_gen_base[offset + 0x1000] = data;
	m_chr_gen_base[offset + 0x1800] = data;
}

READ8_MEMBER( isa8_ec1841_0002_device::char_ram_read )
{
	offset ^= BIT(offset, 12);
	return m_chr_gen_base[offset];
}

WRITE8_MEMBER( isa8_ec1841_0002_device::io_write )
{
	switch (offset)
	{
	case 0x0f:
		m_p3df = data;
		if (data & 1) {
			m_isa->install_memory(0xb8000, 0xb9fff, 0, m_vram_size & 0x4000,
				read8_delegate( FUNC(isa8_ec1841_0002_device::char_ram_read), this),
				write8_delegate(FUNC(isa8_ec1841_0002_device::char_ram_write), this) );
		} else {
			m_isa->install_bank(0xb8000, 0xb8000 + MIN(0x8000,m_vram_size) - 1, 0, m_vram_size & 0x4000, "bank_cga", &m_vram[0]);
		}
		break;
	default:
		isa8_cga_device::io_write(space, offset, data);
		break;
	}
}

READ8_MEMBER( isa8_ec1841_0002_device::io_read )
{
	UINT8 data;

	switch (offset)
	{
	case 0x0f:
		data = m_p3df;
		break;
	default:
		data = isa8_cga_device::io_read(space, offset);
		break;
	}
	return data;
}

const device_type ISA8_CGA_MC1502 = &device_creator<isa8_cga_mc1502_device>;

//-------------------------------------------------
//  isa8_cga_mc1502_device - constructor
//-------------------------------------------------

isa8_cga_mc1502_device::isa8_cga_mc1502_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_MC1502, "MC1502 CGA", tag, owner, clock, "cga_mc1502", __FILE__)
{
	m_vram_size = 0x8000;
	m_chr_gen_offset[0] = 0x0000;
	m_chr_gen_offset[1] = 0x0800;

}

ROM_START( cga_iskr1031 )
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD( "iskra-1031_font.bin", 0x0000, 0x2000, CRC(f4d62e80) SHA1(ad7e81a0c9abc224671422bbcf6f6262da92b510))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_cga_iskr1031_device::device_rom_region() const
{
	return ROM_NAME( cga_iskr1031 );
}

const device_type ISA8_CGA_ISKR1031 = &device_creator<isa8_cga_iskr1031_device>;

//-------------------------------------------------
//  isa8_cga_iskr1031_device - constructor
//-------------------------------------------------

isa8_cga_iskr1031_device::isa8_cga_iskr1031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_ISKR1031, "Iskra-1031 CGA", tag, owner, clock, "cga_iskr1031", __FILE__)
{
}

ROM_START( cga_iskr1030m )
	ROM_REGION(0x2000,"gfx1", 0)
	ROM_LOAD( "iskra-1030m.chr", 0x0000, 0x2000, CRC(50b162eb) SHA1(5bd7cb1705a69bd16115a4c9ed1c2748a5c8ad51))
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *isa8_cga_iskr1030m_device::device_rom_region() const
{
	return ROM_NAME( cga_iskr1030m );
}

const device_type ISA8_CGA_ISKR1030M = &device_creator<isa8_cga_iskr1030m_device>;

//-------------------------------------------------
//  isa8_cga_iskr1030m_device - constructor
//-------------------------------------------------

isa8_cga_iskr1030m_device::isa8_cga_iskr1030m_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_ISKR1030M, "Iskra-1030M CGA", tag, owner, clock, "cga_iskr1030m", __FILE__)
{
}

// XXX

ROM_START( mc1502 )
	ROM_REGION(0x2000,"gfx1", 0)
	// taken from mc1502
	ROM_LOAD( "symgen.rom", 0x0000, 0x2000, CRC(b2747a52) SHA1(6766d275467672436e91ac2997ac6b77700eba1e))
ROM_END

const rom_entry *isa8_cga_mc1502_device::device_rom_region() const
{
	return ROM_NAME( mc1502 );
}

const device_type ISA8_CGA_M24 = &device_creator<isa8_cga_m24_device>;

static MACHINE_CONFIG_DERIVED( m24, cga )
	MCFG_DEVICE_MODIFY(CGA_SCREEN_NAME)
	MCFG_SCREEN_RAW_PARAMS(XTAL_14_31818MHz,912,0,640,462,0,400)
	MCFG_DEVICE_MODIFY(CGA_MC6845_NAME)
	MCFG_MC6845_RECONFIGURE_CB(isa8_cga_m24_device, reconfigure)
MACHINE_CONFIG_END

machine_config_constructor isa8_cga_m24_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m24 );
}
isa8_cga_m24_device::isa8_cga_m24_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
		isa8_cga_device( mconfig, ISA8_CGA_M24, "Olivetti M24 CGA", tag, owner, clock, "cga_m24", __FILE__), m_mode2(0), m_index(0)
{
	m_vram_size = 0x8000;
}

void isa8_cga_m24_device::device_reset()
{
	isa8_cga_device::device_reset();
	m_mode2 = 0;
	m_start_offset = 0;
}

MC6845_RECONFIGURE( isa8_cga_m24_device::reconfigure )
{
	// just reconfigure the screen, the apb sets it to 256 lines rather than 400
	m_screen->configure(width, height, visarea, frame_period);
}

WRITE8_MEMBER( isa8_cga_m24_device::io_write )
{
	mc6845_device *mc6845 = subdevice<mc6845_device>(CGA_MC6845_NAME);
	switch(offset)
	{
		case 0: case 2: case 4: case 6:
			m_index = data;
			mc6845->address_w( space, offset, data );
			break;
		case 1: case 3: case 5: case 7:
			switch(m_index & 0x1f) // TODO: this is handled by a pal and prom
			{
				case 0:
					data &= 0x7f;
					break;
				case 9:
					if((data < 0x80) && (data != 3))
						data = (data << 1) + 1;
					break;
				case 10:
					data = ((data << 1) & 0x1f) | (data & 0x60);
					break;
				case 11:
					data <<= 1;
					break;
			}
			mc6845->register_w( space, offset, data );
			break;
		case 0x0e:
			m_mode2 = data;
			if((data & 8) && !(data & 1))
				m_start_offset = 0x4000;
			else
				m_start_offset = 0;
			break;
		default:
			isa8_cga_device::io_write(space, offset, data);
			break;
	}
}

READ8_MEMBER( isa8_cga_m24_device::io_read )
{
	UINT8 data = 0xff;

	switch(offset)
	{
		case 0x0a:
			data = 0xc0 | m_vsync | ( ( data & 0x40 ) >> 4 ) | m_hsync; // 0xc0 == no expansion
			break;
		case 0x0e:
			data = m_mode2;
			break;
		default:
			data = isa8_cga_device::io_read(space, offset);
			break;
	}
	return data;
}

MC6845_UPDATE_ROW( isa8_cga_m24_device::crtc_update_row )
{
	if(m_mode2 & 1)
		m24_gfx_1bpp_m24_update_row(bitmap, cliprect, ma, ra, y, x_count, cursor_x, de, hbp, vbp);
	else
		isa8_cga_device::crtc_update_row(bitmap, cliprect, ma, ra >> 1, y, x_count, cursor_x, de, hbp, vbp);
}

MC6845_UPDATE_ROW( isa8_cga_m24_device::m24_gfx_1bpp_m24_update_row )
{
	UINT8 *videoram = &m_vram[m_start_offset];
	UINT32  *p = &bitmap.pix32(y);
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT8   fg = m_color_select & 0x0F;
	int i;

	if ( y == 0 ) CGA_LOG(1,"m24_gfx_1bpp_m24_update_row",("\n"));
	for ( i = 0; i < x_count; i++ )
	{
		UINT16 offset = ( ( ( ma + i ) << 1 ) & 0x1fff ) | ( ( ra & 3 ) << 13 );
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
