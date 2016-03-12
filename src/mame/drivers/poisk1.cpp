// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    drivers/poisk1.c

    Driver file for Poisk-1

***************************************************************************/

#include "emu.h"

#include "machine/kb_poisk1.h"
#include "video/cgapal.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "bus/isa/isa.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/ram.h"
#include "bus/isa/xsu_cards.h"
#include "sound/speaker.h"

#include "cpu/i86/i86.h"

#define CGA_PALETTE_SETS 83
/* one for colour, one for mono, 81 for colour composite */

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

#define POISK1_UPDATE_ROW(name) \
	void name(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 *videoram, UINT16 ma, UINT8 ra, UINT8 stride)

class p1_state : public driver_device
{
public:
	p1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pic8259(*this, "pic8259"),
		m_pit8253(*this, "pit8253"),
		m_ppi8255n1(*this, "ppi8255n1"),
		m_ppi8255n2(*this, "ppi8255n2"),
		m_isabus(*this, "isa"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_palette(*this, "palette") { }

	required_device<cpu_device>  m_maincpu;
	required_device<pic8259_device>  m_pic8259;
	required_device<pit8253_device>  m_pit8253;
	required_device<i8255_device>  m_ppi8255n1;
	required_device<i8255_device>  m_ppi8255n2;
	required_device<isa8_device>  m_isabus;
	required_device<speaker_sound_device>  m_speaker;
	required_device<cassette_image_device>  m_cassette;
	required_device<ram_device> m_ram;
	required_device<palette_device> m_palette;

	DECLARE_DRIVER_INIT(poisk1);
	DECLARE_MACHINE_START(poisk1);
	DECLARE_MACHINE_RESET(poisk1);

	DECLARE_PALETTE_INIT(p1);
	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void set_palette_luts();
	POISK1_UPDATE_ROW(cga_gfx_2bpp_update_row);
	POISK1_UPDATE_ROW(cga_gfx_1bpp_update_row);
	POISK1_UPDATE_ROW(poisk1_gfx_1bpp_update_row);

	DECLARE_WRITE_LINE_MEMBER(p1_pit8253_out2_changed);
	DECLARE_WRITE_LINE_MEMBER(p1_speaker_set_spkrdata);
	UINT8 m_p1_spkrdata;
	UINT8 m_p1_input;

	UINT8 m_kbpoll_mask;

	struct
	{
		UINT8 trap[4];
		std::unique_ptr<UINT8[]> videoram_base;
		UINT8 *videoram;
		UINT8 mode_control_6a;
		UINT8 color_select_68;
		UINT8 palette_lut_2bpp[4];
		int stride;
		void *update_row(bitmap_rgb32 &bitmap, const rectangle &cliprect, UINT8 *videoram, UINT16 ma, UINT8 ra, UINT8 stride);
	} m_video;

	DECLARE_READ8_MEMBER(p1_trap_r);
	DECLARE_WRITE8_MEMBER(p1_trap_w);
	DECLARE_READ8_MEMBER(p1_cga_r);
	DECLARE_WRITE8_MEMBER(p1_cga_w);
	DECLARE_WRITE8_MEMBER(p1_vram_w);

	DECLARE_READ8_MEMBER(p1_ppi_r);
	DECLARE_WRITE8_MEMBER(p1_ppi_w);
	DECLARE_WRITE8_MEMBER(p1_ppi_porta_w);
	DECLARE_READ8_MEMBER(p1_ppi_porta_r);
	DECLARE_READ8_MEMBER(p1_ppi_portb_r);
	DECLARE_READ8_MEMBER(p1_ppi_portc_r);
	DECLARE_WRITE8_MEMBER(p1_ppi_portc_w);
	DECLARE_WRITE8_MEMBER(p1_ppi2_porta_w);
	DECLARE_WRITE8_MEMBER(p1_ppi2_portb_w);
	DECLARE_READ8_MEMBER(p1_ppi2_portc_r);
	const char *m_cputag;
};

/*
 * onboard devices:
 */

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

// Timer.  Poisk-1 uses single XTAL for everything? -- check

WRITE_LINE_MEMBER( p1_state::p1_speaker_set_spkrdata )
{
	m_p1_spkrdata = state ? 1 : 0;
	m_speaker->level_w(m_p1_spkrdata & m_p1_input);
}

WRITE_LINE_MEMBER( p1_state::p1_pit8253_out2_changed )
{
	m_p1_input = state ? 1 : 0;
	m_speaker->level_w(m_p1_spkrdata & m_p1_input);
}

// Keyboard (via PPI)

WRITE8_MEMBER(p1_state::p1_ppi_porta_w)
{
	m_kbpoll_mask = data;
	DBG_LOG(2,"p1_ppi_porta_w",("( %02X -> %02X )\n", data, m_kbpoll_mask));
}

READ8_MEMBER(p1_state::p1_ppi_porta_r)
{
	UINT8 ret;

	ret = m_kbpoll_mask;
	DBG_LOG(1,"p1_ppi_porta_r",("= %02X\n", ret));
	return ret;
}

READ8_MEMBER(p1_state::p1_ppi_portb_r)
{
	UINT16 key = 0xffff;
	UINT8 ret = 0;

	if (m_kbpoll_mask & 0x01) { key &= ioport("Y1")->read(); }
	if (m_kbpoll_mask & 0x02) { key &= ioport("Y2")->read(); }
	if (m_kbpoll_mask & 0x04) { key &= ioport("Y3")->read(); }
	if (m_kbpoll_mask & 0x08) { key &= ioport("Y4")->read(); }
	if (m_kbpoll_mask & 0x10) { key &= ioport("Y5")->read(); }
	if (m_kbpoll_mask & 0x20) { key &= ioport("Y6")->read(); }
	if (m_kbpoll_mask & 0x40) { key &= ioport("Y7")->read(); }
	if (m_kbpoll_mask & 0x80) { key &= ioport("Y8")->read(); }
	ret = key & 0xff;
//  DBG_LOG(1,"p1_ppi_portb_r",("= %02X\n", ret));
	return ret;
}

READ8_MEMBER(p1_state::p1_ppi_portc_r)
{
	UINT16 key = 0xffff;
	UINT8 ret = 0;

	if (m_kbpoll_mask & 0x01) { key &= ioport("Y1")->read(); }
	if (m_kbpoll_mask & 0x02) { key &= ioport("Y2")->read(); }
	if (m_kbpoll_mask & 0x04) { key &= ioport("Y3")->read(); }
	if (m_kbpoll_mask & 0x08) { key &= ioport("Y4")->read(); }
	if (m_kbpoll_mask & 0x10) { key &= ioport("Y5")->read(); }
	if (m_kbpoll_mask & 0x20) { key &= ioport("Y6")->read(); }
	if (m_kbpoll_mask & 0x40) { key &= ioport("Y7")->read(); }
	if (m_kbpoll_mask & 0x80) { key &= ioport("Y8")->read(); }
	ret = (key >> 8) & 0xff;
	DBG_LOG(2,"p1_ppi_portc_r",("= %02X\n", ret));
	return ret;
}

// XXX

READ8_MEMBER(p1_state::p1_ppi2_portc_r)
{
	int data = 0xff;
	double tap_val = m_cassette->input();

	data = ( data & ~0x10 ) | ( tap_val < 0 ? 0x10 : 0x00 );

	DBG_LOG(2,"p1_ppi_portc_r",("= %02X (tap_val %f) at %s\n",
		data, tap_val, machine().describe_context()));
	return data;
}

WRITE8_MEMBER(p1_state::p1_ppi2_portb_w)
{
	m_pit8253->write_gate2(BIT(data, 0));
	p1_speaker_set_spkrdata( data & 0x02 );
}

READ8_MEMBER(p1_state::p1_ppi_r)
{
//  DBG_LOG(1,"p1ppi",("R %.2x\n", 0x60+offset));
	switch (offset) {
		case 0:
			return m_ppi8255n1->read(space, 0);
		case 9:
		case 10:
		case 11:
			return m_ppi8255n1->read(space, offset - 8);
		case 8:
			return m_ppi8255n2->read(space, 0);
		case 1:
		case 2:
		case 3:
			return m_ppi8255n2->read(space, offset);
		default:
			DBG_LOG(1,"p1ppi",("R %.2x (unimp)\n", 0x60+offset));
			return 0xff;
	}
}

WRITE8_MEMBER(p1_state::p1_ppi_w)
{
//  DBG_LOG(1,"p1ppi",("W %.2x $%02x\n", 0x60+offset, data));
	switch (offset) {
		case 0:
			return m_ppi8255n1->write(space, 0, data);
		case 9:
		case 10:
		case 11:
			return m_ppi8255n1->write(space, offset - 8, data);
		case 8:
			return m_ppi8255n2->write(space, 0, data);
		case 1:
		case 2:
		case 3:
			return m_ppi8255n2->write(space, offset, data);
		default:
			DBG_LOG(1,"p1ppi",("W %.2x $%02x (unimp)\n", 0x60+offset, data));
			return;
	}
}

/**********************************************************
 *
 * NMI handling
 *
 **********************************************************/

DRIVER_INIT_MEMBER( p1_state, poisk1 )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	DBG_LOG(0,"init",("driver_init()\n"));

	program.install_readwrite_bank(0, m_ram->size()-1, "bank10");
	membank( "bank10" )->set_base( m_ram->pointer() );
}

MACHINE_START_MEMBER( p1_state, poisk1 )
{
	DBG_LOG(0,"init",("machine_start()\n"));
}

MACHINE_RESET_MEMBER( p1_state, poisk1 )
{
	DBG_LOG(0,"init",("machine_reset()\n"));

	m_kbpoll_mask = 0;
}

/*
 * macros
 */

static ADDRESS_MAP_START( poisk1_map, AS_PROGRAM, 8, p1_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("bios", 0xc000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( poisk1_io, AS_IO, 8, p1_state )
	AM_RANGE(0x0020, 0x0021) AM_DEVREADWRITE("pic8259", pic8259_device, read, write)
	AM_RANGE(0x0028, 0x002B) AM_READWRITE(p1_trap_r, p1_trap_w)
	AM_RANGE(0x0040, 0x0043) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	// can't use regular AM_DEVREADWRITE, because THIS IS SPARTA!
	// 1st PPI occupies ports 60, 69, 6A and 6B; 2nd PPI -- 68, 61, 62 and 63.
	AM_RANGE(0x0060, 0x006F) AM_READWRITE(p1_ppi_r, p1_ppi_w)
	AM_RANGE(0x03D0, 0x03DF) AM_READWRITE(p1_cga_r, p1_cga_w)
ADDRESS_MAP_END

static INPUT_PORTS_START( poisk1 )
	PORT_INCLUDE( poisk1_keyboard_v91 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( poisk1, p1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8088, 5000000)
	MCFG_CPU_PROGRAM_MAP(poisk1_map)
	MCFG_CPU_IO_MAP(poisk1_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259", pic8259_device, inta_cb)

	MCFG_MACHINE_START_OVERRIDE( p1_state, poisk1 )
	MCFG_MACHINE_RESET_OVERRIDE( p1_state, poisk1 )

	MCFG_DEVICE_ADD( "pit8253", PIT8253 ,0)
	MCFG_PIT8253_CLK0(XTAL_15MHz/12) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_15MHz/12) /* keyboard poll -- XXX edge or level triggered? */
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("pic8259", pic8259_device, ir6_w))
	MCFG_PIT8253_CLK2(XTAL_15MHz/12) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(p1_state, p1_pit8253_out2_changed))

	MCFG_PIC8259_ADD( "pic8259", INPUTLINE("maincpu", 0), VCC, NULL )

	MCFG_DEVICE_ADD("ppi8255n1", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(p1_state, p1_ppi_porta_r)) /*60H*/
	MCFG_I8255_OUT_PORTA_CB(WRITE8(p1_state, p1_ppi_porta_w))
	MCFG_I8255_IN_PORTB_CB(READ8(p1_state, p1_ppi_portb_r)) /*69H*/
	MCFG_I8255_IN_PORTC_CB(READ8(p1_state, p1_ppi_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(p1_state, p1_ppi_portc_w))   /*6AH*/

	MCFG_DEVICE_ADD("ppi8255n2", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(p1_state, p1_ppi2_porta_w))  /*68H*/
	MCFG_I8255_OUT_PORTB_CB(WRITE8(p1_state, p1_ppi2_portb_w))  /*61H*/
	MCFG_I8255_IN_PORTC_CB(READ8(p1_state, p1_ppi2_portc_r))    /*62H*/

	MCFG_DEVICE_ADD("isa", ISA8, 0)
	MCFG_ISA8_CPU(":maincpu")
	MCFG_ISA_OUT_IRQ2_CB(DEVWRITELINE("pic8259", pic8259_device, ir2_w))
	MCFG_ISA_OUT_IRQ3_CB(DEVWRITELINE("pic8259", pic8259_device, ir3_w))
	MCFG_ISA_OUT_IRQ4_CB(DEVWRITELINE("pic8259", pic8259_device, ir4_w))
	MCFG_ISA_OUT_IRQ5_CB(DEVWRITELINE("pic8259", pic8259_device, ir5_w))
	MCFG_ISA_OUT_IRQ7_CB(DEVWRITELINE("pic8259", pic8259_device, ir7_w))
	MCFG_ISA8_SLOT_ADD("isa", "isa1", p1_isa8_cards, "fdc", false)
	MCFG_ISA8_SLOT_ADD("isa", "isa2", p1_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("isa", "isa3", p1_isa8_cards, nullptr, false)
	MCFG_ISA8_SLOT_ADD("isa", "isa4", p1_isa8_cards, nullptr, false)

	MCFG_CASSETTE_ADD( "cassette" )
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_PLAY | CASSETTE_MOTOR_DISABLED | CASSETTE_SPEAKER_ENABLED)

	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "speaker", SPEAKER_SOUND, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	MCFG_SCREEN_ADD( "screen", RASTER )
	MCFG_SCREEN_RAW_PARAMS( XTAL_15MHz, 912,0,640, 262,0,200 )
	MCFG_SCREEN_UPDATE_DRIVER( p1_state, screen_update )

	/* XXX verify palette */
	MCFG_PALETTE_ADD("palette",  CGA_PALETTE_SETS * 16 )
	MCFG_PALETTE_INIT_OWNER(p1_state, p1)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
MACHINE_CONFIG_END

ROM_START( poisk1 )
	ROM_REGION16_LE(0x10000,"bios", 0)

	ROM_DEFAULT_BIOS("v91")
	ROM_SYSTEM_BIOS(0, "v89", "1989")
	ROMX_LOAD( "biosp1s.rf4", 0xe000, 0x2000, CRC(1a85f671) SHA1(f0e59b2c4d92164abca55a96a58071ce869ff988), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v91", "1991")
	ROMX_LOAD( "poisk_1991.bin", 0xe000, 0x2000, CRC(d61c56fd) SHA1(de202e1f7422d585a1385a002a4fcf9d756236e5), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(2, "v91r2", "1991r2")
	ROMX_LOAD( "p_bios_nm.bin", 0xe000, 0x2000, CRC(84430b4f) SHA1(3e477962be3cea09662cb2e3ad9966ad01c7455d), ROM_BIOS(3))

	// 0xc0000, sets 80x25 text and loops asking for 'Boot from hard disk (Y or N)?'
	ROM_LOAD( "boot_net.rf4", 0x00000, 0x2000, CRC(316c2030) SHA1(d043325596455772252e465b85321f1b5c529d0b)) // NET BIOS
	// 0xc0000, accesses ports 0x90..0x97
	ROM_LOAD( "pois_net.bin", 0x00000, 0x2000, CRC(cf9dd80a) SHA1(566bcb40c0cb2c8bfd5b485f0db689fdeaca3e86)) // ??? BIOS

	ROM_REGION(0x2000,"gfx1", ROMREGION_ERASE00)
	ROM_LOAD( "poisk.cga", 0x0000, 0x0800, CRC(f6eb39f0) SHA1(0b788d8d7a8e92cc612d044abcb2523ad964c200))
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*     YEAR     NAME        PARENT      COMPAT  MACHINE     INPUT       INIT                COMPANY       FULLNAME */
COMP ( 1989,    poisk1,    ibm5150,    0,      poisk1,    poisk1,    p1_state, poisk1,   "Electronmash",  "Poisk-1", 0)
