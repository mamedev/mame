// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie EG2000

    TODO:
    - System is too fast, there should be one wait cycle every five cycles?
    - Adjust visible area so that the borders aren't that large (needs
      MC6845 changes)
    - Verify BASIC and character set versions

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "imagedev/cassette.h"
#include "formats/cgen_cas.h"
#include "bus/rs232/rs232.h"
#include "bus/cgenie/expansion/expansion.h"
#include "bus/cgenie/parallel/parallel.h"
#include "softlist.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cgenie_state : public driver_device
{
public:
	cgenie_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_cassette(*this, "cassette"),
		m_ram(*this, RAM_TAG),
		m_crtc(*this, "crtc"),
		m_rs232(*this, "rs232"),
		m_exp(*this, "exp"),
		m_char_rom(*this, "gfx1"),
		m_color_ram(*this, "colorram"),
		m_font_ram(*this, "fontram"),
		m_keyboard(*this, "KEY"),
		m_palette(nullptr),
		m_control(0xff),
		m_rs232_rx(1),
		m_rs232_dcd(1)
	{}

	DECLARE_DRIVER_INIT(cgenie_eu);
	DECLARE_DRIVER_INIT(cgenie_nz);

	MC6845_BEGIN_UPDATE(crtc_begin_update);
	MC6845_UPDATE_ROW(crtc_update_row);

	// 4-bit color ram
	DECLARE_READ8_MEMBER(colorram_r);
	DECLARE_WRITE8_MEMBER(colorram_w);

	// control port
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(control_r);

	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_INPUT_CHANGED_MEMBER(rst_callback);

	DECLARE_WRITE_LINE_MEMBER(rs232_rx_w);
	DECLARE_WRITE_LINE_MEMBER(rs232_dcd_w);

protected:
	virtual void machine_start();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cassette_image_device> m_cassette;
	required_device<ram_device> m_ram;
	required_device<hd6845_device> m_crtc;
	required_device<rs232_port_device> m_rs232;
	required_device<expansion_slot_device> m_exp;
	required_memory_region m_char_rom;
	required_shared_ptr<UINT8> m_color_ram;
	required_shared_ptr<UINT8> m_font_ram;
	required_ioport_array<8> m_keyboard;

	static const rgb_t m_palette_bg[];
	static const rgb_t m_palette_eu[];
	static const rgb_t m_palette_nz[];

	const rgb_t *m_palette;
	rgb_t m_background_color;

	UINT8 m_control;

	int m_rs232_rx;
	int m_rs232_dcd;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( cgenie_mem, AS_PROGRAM, 8, cgenie_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x3fff) AM_ROM
//  AM_RANGE(0x4000, 0xbfff) AM_RAM // set up in machine_start
	AM_RANGE(0xc000, 0xefff) AM_NOP // cartridge space
	AM_RANGE(0xf000, 0xf3ff) AM_READWRITE(colorram_r, colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xf400, 0xf7ff) AM_RAM AM_SHARE("fontram")
	AM_RANGE(0xf800, 0xf8ff) AM_MIRROR(0x300) AM_READ(keyboard_r)
	AM_RANGE(0xfc00, 0xffff) AM_NOP // cartridge space
ADDRESS_MAP_END

static ADDRESS_MAP_START( cgenie_io, AS_IO, 8, cgenie_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf8, 0xf8) AM_DEVWRITE("ay8910", ay8910_device, address_w)
	AM_RANGE(0xf9, 0xf9) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, data_w)
	AM_RANGE(0xfa, 0xfa) AM_DEVWRITE("crtc", hd6845_device, address_w)
	AM_RANGE(0xfb, 0xfb) AM_DEVREADWRITE("crtc", hd6845_device, register_r, register_w)
	AM_RANGE(0xff, 0xff) AM_READWRITE(control_r, control_w)
ADDRESS_MAP_END


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( cgenie )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)   // produces [ and { when pressed, not on keyboard
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)            PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)            PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CHAR(UCHAR_MAMEKEY(F8))

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)         PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Break") PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)            PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)          PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)          PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)         PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Mod Sel") PORT_CODE(KEYCODE_LALT)  PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Rpt") PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("L.P.") // marked as "L.P." in the manual, lightpen?

	PORT_START("RST")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_CODE(KEYCODE_F5) PORT_NAME("Rst") PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CHANGED_MEMBER(DEVICE_SELF, cgenie_state, rst_callback, NULL)
INPUT_PORTS_END


//**************************************************************************
//  KEYBOARD
//**************************************************************************

READ8_MEMBER( cgenie_state::keyboard_r )
{
	UINT8 data = 0;

	for (int i = 0; i < 8; i++)
		if (BIT(offset, i))
			data |= m_keyboard[i]->read();

	return data;
}

INPUT_CHANGED_MEMBER( cgenie_state::rst_callback )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval);
}


//**************************************************************************
//  CONTROL PORT & RS232
//**************************************************************************

WRITE8_MEMBER( cgenie_state::control_w )
{
	// cassette output
	m_cassette->output(BIT(data, 0) ? -1.0 : 1.0);

	// serial output
	m_rs232->write_txd(BIT(data, 1));

	// background color selection
	if (BIT(data, 2))
		m_background_color = m_palette_bg[4];
	else
		m_background_color = m_palette_bg[data >> 6 & 0x03];

	// graphics/text mode switch
	m_crtc->set_hpixels_per_column(BIT(data, 5) ? 4 : 8);

	m_control = data;
}

READ8_MEMBER( cgenie_state::control_r )
{
	UINT8 data = 0;

	data |= m_cassette->input() > 0 ? 1 : 0;
	data |= m_rs232_rx << 1;
	data |= m_rs232_dcd << 2;

	return data;
}

WRITE_LINE_MEMBER( cgenie_state::rs232_rx_w )
{
	m_rs232_rx = state;
}

WRITE_LINE_MEMBER( cgenie_state::rs232_dcd_w )
{
	m_rs232_dcd = state;
}


//**************************************************************************
//  DRIVER INIT
//**************************************************************************

DRIVER_INIT_MEMBER( cgenie_state, cgenie_eu )
{
	m_palette = &m_palette_eu[0];
}

DRIVER_INIT_MEMBER( cgenie_state, cgenie_nz )
{
	m_palette = &m_palette_nz[0];
}

void cgenie_state::machine_start()
{
	// setup ram
	m_maincpu->space(AS_PROGRAM).install_ram(0x4000, 0x4000 + m_ram->size() - 1, m_ram->pointer());

	// setup expansion bus
	m_exp->set_program_space(&m_maincpu->space(AS_PROGRAM));
	m_exp->set_io_space(&m_maincpu->space(AS_IO));
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

READ8_MEMBER( cgenie_state::colorram_r )
{
	return m_color_ram[offset] | 0xf0;
}

WRITE8_MEMBER( cgenie_state::colorram_w )
{
	m_color_ram[offset] = data & 0x0f;
}

MC6845_BEGIN_UPDATE( cgenie_state::crtc_begin_update )
{
	bitmap.fill(m_background_color, cliprect);
}

MC6845_UPDATE_ROW( cgenie_state::crtc_update_row )
{
	// don't need to do anything in vblank
	if (!de)
		return;

	for (int column = 0; column < x_count; column++)
	{
		UINT8 code = m_ram->pointer()[ma + column];
		UINT8 color = m_color_ram[(ma & 0xbff) + column];

		// gfx mode?
		if (BIT(m_control, 5))
		{
			const rgb_t map[] = { m_background_color, m_palette[8], m_palette[6], m_palette[5] };

			bitmap.pix32(y + vbp, column * 4 + hbp + 0) = map[code >> 6 & 0x03];
			bitmap.pix32(y + vbp, column * 4 + hbp + 1) = map[code >> 4 & 0x03];
			bitmap.pix32(y + vbp, column * 4 + hbp + 2) = map[code >> 2 & 0x03];
			bitmap.pix32(y + vbp, column * 4 + hbp + 3) = map[code >> 0 & 0x03];
		}
		else
		{
			UINT8 gfx = 0;

			// cursor visible?
			if (cursor_x == column)
				gfx = 0xff;

			// or use character rom?
			else if ((code < 128) || (code < 192 && BIT(m_control, 4)) || (code > 192 && BIT(m_control, 3)))
				gfx = m_char_rom->base()[(code << 3) | ra];

			// or the programmable characters?
			else
				gfx = m_font_ram[((code << 3) | ra) & 0x3ff];

			// 8 pixel chars
			for (int p = 0; p < 8; p++)
				bitmap.pix32(y + vbp, column * 8 + hbp + p) = BIT(gfx, 7 - p) ? m_palette[color] : m_background_color;
		}
	}
}


//**************************************************************************
//  PALETTE
//**************************************************************************

// how accurate are these colors?
const rgb_t cgenie_state::m_palette_bg[] =
{
	rgb_t::black,
	rgb_t(0x70, 0x28, 0x20), // dark orange
	rgb_t(0x28, 0x70, 0x20), // dark green
	rgb_t(0x48, 0x48, 0x48), // dark gray
	rgb_t(0x70, 0x00, 0x70)  // dark purple
};

// european palette
const rgb_t cgenie_state::m_palette_eu[] =
{
	rgb_t(0x5e, 0x5e, 0x5e), // gray
	rgb_t(0x11, 0xff, 0xea), // cyan
	rgb_t(0xff, 0x00, 0x5e), // red
	rgb_t(0xea, 0xea, 0xea), // white
	rgb_t(0xff, 0xf9, 0x00), // yellow
	rgb_t(0x6e, 0xff, 0x00), // green
	rgb_t(0xff, 0x52, 0x00), // orange
	rgb_t(0xea, 0xff, 0x00), // light yellow
	rgb_t(0x02, 0x48, 0xff), // blue
	rgb_t(0x8e, 0xd4, 0xff), // light blue
	rgb_t(0xff, 0x12, 0xff), // pink
	rgb_t(0x88, 0x43, 0xff), // purple
	rgb_t(0x8c, 0x8c, 0x8c), // light gray
	rgb_t(0x00, 0xfb, 0x8c), // turquoise
	rgb_t(0xd2, 0x00, 0xff), // magenta
	rgb_t::white             // bright white
};

// new zealand palette
const rgb_t cgenie_state::m_palette_nz[] =
{
	rgb_t::white,
	rgb_t(0x12, 0xff, 0xff),
	rgb_t(0xff, 0x6f, 0xff),
	rgb_t(0x31, 0x77, 0xff),
	rgb_t(0xff, 0xcb, 0x00),
	rgb_t(0x6e, 0xff, 0x00),
	rgb_t(0xff, 0x52, 0x00),
	rgb_t(0x5e, 0x5e, 0x5e),
	rgb_t(0xea, 0xea, 0xea),
	rgb_t(0x00, 0xff, 0xdd),
	rgb_t(0xd2, 0x00, 0xff),
	rgb_t(0x02, 0x48, 0xff),
	rgb_t(0xff, 0xf9, 0x00),
	rgb_t(0x00, 0xda, 0x00),
	rgb_t(0xff, 0x22, 0x00),
	rgb_t::black
};


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static MACHINE_CONFIG_START( cgenie, cgenie_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", Z80, XTAL_17_73447MHz / 8)  // 2.2168 MHz
	MCFG_CPU_PROGRAM_MAP(cgenie_mem)
	MCFG_CPU_IO_MAP(cgenie_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_17_73447MHz / 2, 568, 32, 416, 312, 28, 284)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", hd6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", HD6845, "screen", XTAL_17_73447MHz / 16)
	MCFG_MC6845_SHOW_BORDER_AREA(true)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_BEGIN_UPDATE_CB(cgenie_state, crtc_begin_update)
	MCFG_MC6845_UPDATE_ROW_CB(cgenie_state, crtc_update_row)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, XTAL_17_73447MHz / 8)
	MCFG_AY8910_PORT_A_READ_CB(DEVREAD8("par", parallel_slot_device, pa_r))
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("par", parallel_slot_device, pa_w))
	MCFG_AY8910_PORT_B_READ_CB(DEVREAD8("par", parallel_slot_device, pb_r))
	MCFG_AY8910_PORT_B_WRITE_CB(DEVWRITE8("par", parallel_slot_device, pb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.75)

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(cgenie_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED)
	MCFG_CASSETTE_INTERFACE("cgenie_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list", "cgenie_cass")

	// serial port
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(cgenie_state, rs232_rx_w))
	MCFG_RS232_DCD_HANDLER(WRITELINE(cgenie_state, rs232_dcd_w))

	// cartridge expansion slot
	MCFG_EXPANSION_SLOT_ADD("exp")
	MCFG_EXPANSION_SLOT_INT_HANDLER(INPUTLINE("maincpu", INPUT_LINE_IRQ0))

	// parallel slot
	MCFG_PARALLEL_SLOT_ADD("par")

	// internal ram
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K")
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( cgenie )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("cg_rom1.z1", 0x0000, 0x1000, CRC(d3369420) SHA1(fe7f06f8e2b648d695d4787d00a374d3e4ac5d39))
	ROM_LOAD("cg_rom2.z2", 0x1000, 0x1000, CRC(73d2c9ea) SHA1(343d595b4eeaea627f9c36d5cef3827c79593425))
	ROM_LOAD("cg_rom3.z3", 0x2000, 0x1000, CRC(3f358811) SHA1(6ba151759fd8fd367806cf2dd5f1dfc33ee9521f))
	ROM_LOAD("cg_rom4.z4", 0x3000, 0x1000, CRC(be235782) SHA1(d7d61208a9855ffd09ecb051618f5ed6a8816f3f))

	ROM_REGION(0x0800, "gfx1", 0)
	// this is a "german" character set with umlauts, is this official?
	ROM_LOAD("cgenieg.fnt", 0x0000, 0x0800, CRC(c3e60d57) SHA1(fb96f608fdb47391145fdcd614a9c7a79756e6a4))
	// default character set
	ROM_LOAD("cgenie1.fnt", 0x0000, 0x0800, CRC(4fed774a) SHA1(d53df8212b521892cc56be690db0bb474627d2ff))
ROM_END

ROM_START( cgenienz )
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "old", "Old ROM")
	ROMX_LOAD("cg-basic-rom-v1-pal-en.rom", 0x0000, 0x4000, CRC(844aaedd) SHA1(b7f984bc5cd979c7ad11ff909e8134f694aea7aa), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "new", "New ROM")
	ROMX_LOAD("cgromv2.rom", 0x0000, 0x4000, CRC(cfb84e09) SHA1(e199e4429bab6f9fca2bb05e71324538928a693a), ROM_BIOS(2))

	ROM_REGION(0x0800, "gfx1", 0)
	ROM_LOAD("cgenie1.fnt", 0x0000, 0x0800, CRC(4fed774a) SHA1(d53df8212b521892cc56be690db0bb474627d2ff))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT    COMPAT  MACHINE INPUT   CLASS         INIT       COMPANY FULLNAME                             FLAGS
COMP( 1982, cgenie,   0,        0,      cgenie, cgenie, cgenie_state, cgenie_eu, "EACA", "Colour Genie EG2000",               0)
COMP( 1982, cgenienz, cgenie,   0,      cgenie, cgenie, cgenie_state, cgenie_nz, "EACA", "Colour Genie EG2000 (New Zealand)", 0)
