// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*
 * Poisk-1 does not have a real mc6845 and always runs in graphics mode.
 * Text mode is emulated by BIOS.
 * Video RAM in native graphics mode starts at 0xB8000, in emulated text mode -- at 0xBC000.
 */

#include "emu.h"

#include "includes/poisk1.h"

#define CGA_PALETTE_SETS 83
/* one for colour, one for mono, 81 for colour composite */

#include "video/cgapal.h"

#define BG_COLOR(x) (((x) & 7)|(((x) & 0x10) >> 1))

#define VERBOSE_DBG 0

#define DBG_LOG(N,M,A) \
	do { \
		if(VERBOSE_DBG>=N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s",machine().time().as_double(),(char*)M ); \
			logerror A; \
		} \
	} while (0)

//

/*
 * Poisk-1 doesn't have a mc6845 and always runs in graphics mode.  Text mode is emulated by BIOS;
 * NMI is triggered on access to video memory and to mc6845 ports.  Address and data are latched into:
 *
 * Port 28H (offset 0) -- lower 8 bits of address
 * Port 29H (offset 1) -- high  -//- and mode bits
 * Port 2AH (offset 2) -- data
 */

READ8_MEMBER(p1_state::p1_trap_r)
{
	UINT8 data = m_video.trap[offset];
	DBG_LOG(1,"trap",("R %.2x $%02x\n", 0x28+offset, data));
	if (offset == 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	return data;
}

WRITE8_MEMBER(p1_state::p1_trap_w)
{
	DBG_LOG(1,"trap",("W %.2x $%02x\n", 0x28+offset, data));
}

READ8_MEMBER(p1_state::p1_cga_r)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	return 0;
}

WRITE8_MEMBER(p1_state::p1_cga_w)
{
	UINT16 port = offset + 0x3d0;

	DBG_LOG(1,"cga",("W %.4x $%02x\n", port, data));
	m_video.trap[2] = data;
	m_video.trap[1] = 0xC0 | ((port >> 8) & 0x3f);
	m_video.trap[0] = port & 255;
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE8_MEMBER(p1_state::p1_vram_w)
{
	DBG_LOG(1,"vram",("W %.4x $%02x\n", offset, data));
	if (m_video.videoram_base)
		m_video.videoram_base[offset] = data;
	m_video.trap[2] = data;
	m_video.trap[1] = 0x80 | ((offset >> 8) & 0x3f);
	m_video.trap[0] = offset & 255;
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

// CGA emulator
/*
068h    D42 0..2    R, G, B     XXX Foreground/Background color
        3   NMI DISABLE NMI trap  1: Disabled  0: Enabled
        4   PALETTE     XXX Colour palette  0: XXX  1: XXX
        5   I (INTENS)  XXX Foreground/Background color intensity
        6   DISPLAY BANK    XXX Video RAM page
        7   HIRES       1: 640x200  0: 320x200
*/

WRITE8_MEMBER(p1_state::p1_ppi2_porta_w)
{
	address_space &space_prg = m_maincpu->space(AS_PROGRAM);

	DBG_LOG(1,"color_select_68",("W $%02x\n", data));

	// NMI DISABLE
	if (BIT(data, 3) != BIT(m_video.color_select_68, 3)) {
		if (BIT(data, 3)) {
			space_prg.install_readwrite_bank( 0xb8000, 0xbbfff, "bank11" );
		} else {
			space_prg.install_read_bank( 0xb8000, 0xbbfff, "bank11" );
			space_prg.install_write_handler( 0xb8000, 0xbbfff, WRITE8_DELEGATE(p1_state, p1_vram_w) );
		}
	}
	// DISPLAY BANK
	if (BIT(data, 6) != BIT(m_video.color_select_68, 6)) {
		if (BIT(data, 6))
			m_video.videoram = m_video.videoram_base.get() + 0x4000;
		else
			m_video.videoram = m_video.videoram_base.get();
	}
	// HIRES -- XXX
	if (BIT(data, 7) != BIT(m_video.color_select_68, 7)) {
		if (BIT(data, 7))
			machine().first_screen()->set_visible_area(0, 640-1, 0, 200-1);
		else
			machine().first_screen()->set_visible_area(0, 320-1, 0, 200-1);
	}
	m_video.color_select_68 = data;
	set_palette_luts();
}

/*
06Ah    Dxx 6   Enable/Disable color burst (?)
        7   Enable/Disable D7H/D7L
*/

WRITE8_MEMBER(p1_state::p1_ppi_portc_w)
{
	DBG_LOG(1,"mode_control_6a",("W $%02x\n", data));

	m_video.mode_control_6a = data;
	set_palette_luts();
}

void p1_state::set_palette_luts(void)
{
	/* Setup 2bpp palette lookup table */
	// HIRES
	if ( m_video.color_select_68 & 0x80 )
	{
		m_video.palette_lut_2bpp[0] = 0;
	}
	else
	{
		m_video.palette_lut_2bpp[0] = BG_COLOR(m_video.color_select_68);
	}
	// B&W -- XXX
/*
    if ( m_video.mode_control_6a & 0x40 )
    {
        m_video.palette_lut_2bpp[1] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 3;
        m_video.palette_lut_2bpp[2] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 4;
        m_video.palette_lut_2bpp[3] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 7;
    }
    else
*/
	{
		// PALETTE
		if ( m_video.color_select_68 & 0x20 )
		{
			m_video.palette_lut_2bpp[1] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 3;
			m_video.palette_lut_2bpp[2] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 5;
			m_video.palette_lut_2bpp[3] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 7;
		}
		else
		{
			m_video.palette_lut_2bpp[1] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 2;
			m_video.palette_lut_2bpp[2] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 4;
			m_video.palette_lut_2bpp[3] = ( ( m_video.color_select_68 & 0x20 ) >> 2 ) | 6;
		}
	}
}

/***************************************************************************
  Draw graphics mode with 320x200 pixels (default) with 2 bits/pixel.
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
  cga fetches 2 byte per mc6845 access.
***************************************************************************/

POISK1_UPDATE_ROW( p1_state::cga_gfx_2bpp_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(ra);
	UINT16  odd, offset;
	int i;

	if ( ra == 0 ) DBG_LOG(1,"cga_gfx_2bpp_update_row",("\n"));
	odd = ( ra & 1 ) << 13;
	offset = ( ma & 0x1fff ) | odd;
	for ( i = 0; i < stride; i++ )
	{
		UINT8 data = videoram[ offset++ ];

		*p = palette[m_video.palette_lut_2bpp[ ( data >> 6 ) & 0x03 ]]; p++;
		*p = palette[m_video.palette_lut_2bpp[ ( data >> 4 ) & 0x03 ]]; p++;
		*p = palette[m_video.palette_lut_2bpp[ ( data >> 2 ) & 0x03 ]]; p++;
		*p = palette[m_video.palette_lut_2bpp[   data        & 0x03 ]]; p++;
	}
}

/***************************************************************************
  Draw graphics mode with 640x200 pixels (default).
  The cell size is 1x1 (1 scanline is the real default)
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
***************************************************************************/

POISK1_UPDATE_ROW( p1_state::cga_gfx_1bpp_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(ra);
	UINT8   fg = 15, bg = BG_COLOR(m_video.color_select_68);
	UINT16  odd, offset;
	int i;

	if ( ra == 0 ) DBG_LOG(1,"cga_gfx_1bpp_update_row",("bg %d\n", bg));
	odd = ( ra & 1 ) << 13;
	offset = ( ma & 0x1fff ) | odd;
	for ( i = 0; i < stride; i++ )
	{
		UINT8 data = videoram[ offset++ ];

		*p = palette[( data & 0x80 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg ]; p++;
	}
}

/***************************************************************************
  Draw graphics mode with 640x200 pixels + extra highlight color for text
  mode emulation
  Even scanlines are from CGA_base + 0x0000, odd from CGA_base + 0x2000
***************************************************************************/

POISK1_UPDATE_ROW( p1_state::poisk1_gfx_1bpp_update_row )
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	UINT32  *p = &bitmap.pix32(ra);
	UINT8   fg, bg = BG_COLOR(m_video.color_select_68);
	UINT16  odd, offset;
	int i;

	if ( ra == 0 ) DBG_LOG(1,"poisk1_gfx_1bpp_update_row",("bg %d\n", bg));
	odd = ( ra & 1 ) << 13;
	offset = ( ma & 0x1fff ) | odd;
	for ( i = 0; i < stride; i++ )
	{
		UINT8 data = videoram[ offset++ ];

		fg = (data & 0x80) ? ( (m_video.color_select_68 & 0x20) ? 10 : 11 ) : 15; // XXX
		*p = palette[bg]; p++;
		*p = palette[( data & 0x40 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x20 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x10 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x08 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x04 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x02 ) ? fg : bg ]; p++;
		*p = palette[( data & 0x01 ) ? fg : bg ]; p++;
	}
}

/* Initialise the cga palette */
PALETTE_INIT_MEMBER(p1_state, p1)
{
	int i;

	DBG_LOG(0,"init",("palette_init()\n"));

	for ( i = 0; i < CGA_PALETTE_SETS * 16; i++ )
	{
		palette.set_pen_color(i, cga_palette[i][0], cga_palette[i][1], cga_palette[i][2] );
	}
}

void p1_state::video_start()
{
	address_space &space = m_maincpu->space( AS_PROGRAM );

	DBG_LOG(0,"init",("video_start()\n"));

	memset(&m_video, 0, sizeof(m_video));
	m_video.videoram_base = std::make_unique<UINT8[]>(0x8000);
	m_video.videoram = m_video.videoram_base.get();
	m_video.stride = 80;

	space.install_readwrite_bank(0xb8000, 0xbffff, "bank11" );
	machine().root_device().membank("bank11")->set_base(m_video.videoram);
}

UINT32 p1_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT16 ra, ma = 0;

	if (!m_video.stride || !m_video.videoram) return 0;

	// bit 6 of 6Ah disables color burst -- not implemented
	for (ra = cliprect.min_y; ra <= cliprect.max_y; ra++)
	{
		if (BIT(m_video.color_select_68, 7)) {
			if (BIT(m_video.mode_control_6a, 7)) {
				cga_gfx_1bpp_update_row(bitmap, cliprect, m_video.videoram, ma, ra, m_video.stride);
			} else {
				poisk1_gfx_1bpp_update_row(bitmap, cliprect, m_video.videoram, ma, ra, m_video.stride);
			}
		} else {
			cga_gfx_2bpp_update_row(bitmap, cliprect, m_video.videoram, ma, ra, m_video.stride);
		}
		if (ra & 1) ma += m_video.stride;
	}

	return 0;
}
