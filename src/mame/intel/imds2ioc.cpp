// license:BSD-3-Clause
// copyright-holders:F. Ulivi
// I/O controller for Intel Intellec MDS series-II
//
// TODO:
// - Adjust speed of processors. Wait states are not accounted for yet.

#include "emu.h"
#include "imds2ioc.h"
#include "screen.h"
#include "speaker.h"

// Main oscillator of IOC board: 22.032 MHz
#define IOC_XTAL_Y2     22.032_MHz_XTAL

// FDC oscillator of IOC board: 8 MHz
#define IOC_XTAL_Y1     8_MHz_XTAL

// PIO oscillator: 6 MHz
#define IOC_XTAL_Y3     6_MHz_XTAL

// Frequency of beeper
#define IOC_BEEP_FREQ   3300

// device type definition
DEFINE_DEVICE_TYPE(IMDS2IOC, imds2ioc_device, "imds2ioc", "Intellec Series II Input/Output Controller")

void imds2ioc_device::mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x1fff).rom();
	map(0x4000, 0x5fff).ram();
}

void imds2ioc_device::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x0f).w(FUNC(imds2ioc_device::ioc_dbbout_w));
	map(0x20, 0x2f).w(FUNC(imds2ioc_device::ioc_f0_w));
	map(0x30, 0x3f).w(FUNC(imds2ioc_device::ioc_set_f1_w));
	map(0x40, 0x4f).w(FUNC(imds2ioc_device::ioc_reset_f1_w));
	map(0x50, 0x5f).w(FUNC(imds2ioc_device::start_timer_w));
	map(0x60, 0x6f).w(FUNC(imds2ioc_device::miscout_w));
	map(0x80, 0x8f).r(FUNC(imds2ioc_device::miscin_r));
	map(0x90, 0x9f).r(FUNC(imds2ioc_device::kb_read));
	map(0xa0, 0xaf).r(FUNC(imds2ioc_device::ioc_status_r));
	map(0xb0, 0xbf).r(FUNC(imds2ioc_device::ioc_dbbin_r));
	map(0xc0, 0xcf).m(m_iocfdc, FUNC(i8271_device::map));
	map(0xd0, 0xdf).rw(m_ioccrtc, FUNC(i8275_device::read), FUNC(i8275_device::write));
	map(0xe0, 0xef).rw(m_ioctimer, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
// DMA controller range doesn't extend to 0xff because register 0xfd needs to be read as 0xff
// This register is used by IOC firmware to detect DMA controller model (either 8237 or 8257)
	map(0xf0, 0xf8).rw(m_iocdma, FUNC(i8257_device::read), FUNC(i8257_device::write));
}

imds2ioc_device::imds2ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, IMDS2IOC, tag, owner, clock),
	m_ioccpu(*this, "ioccpu"),
	m_iocdma(*this, "iocdma"),
	m_ioccrtc(*this, "ioccrtc"),
	m_iocbeep(*this, "iocbeep"),
	m_ioctimer(*this, "ioctimer"),
	m_iocfdc(*this, "iocfdc"),
	m_flop0(*this, "iocfdc:0"),
	m_iocpio(*this, "iocpio"),
	m_kbcpu(*this, "kbcpu"),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_centronics(*this, "centronics"),
	m_io_key(*this, "KEY%u", 0U),
	m_ioc_options(*this, "IOC_OPTS"),
	m_master_intr_cb(*this),
	m_parallel_int_cb(*this),
	m_chargen(*this, "gfx1"),
	m_device_status_byte(0xff)
{
}


void imds2ioc_device::miscout_w(uint8_t data)
{
	m_miscout = data;
	update_beeper();
	// Send INTR to IPC
	m_master_intr_cb(BIT(m_miscout, 1));
}

uint8_t imds2ioc_device::miscin_r()
{
	uint8_t res = m_ioc_options->read();
	return res | ((m_beeper_timer == 0) << 2);
}

WRITE_LINE_MEMBER(imds2ioc_device::beep_timer_w)
{
	m_beeper_timer = state;
	update_beeper();
}

void imds2ioc_device::start_timer_w(uint8_t data)
{
	// Trigger timer 2 of ioctimer
	m_ioctimer->write_gate2(0);
	m_ioctimer->write_gate2(1);
}

uint8_t imds2ioc_device::kb_read(offs_t offset)
{
	return m_kbcpu->upi41_master_r((offset & 2) >> 1);
}

uint8_t imds2ioc_device::kb_port_p2_r()
{
	if ((m_kb_p1 & 3) == 0) {
		// Row selected
		// Row number is encoded on bits P15..P12, they are "backwards" (P15 is LSB) and keyboard rows are encoded starting with value 2 on these bits (see A4, pg 56 of schematic)
		unsigned row = (m_kb_p1 >> 2) & 0x0f;
		ioport_value data;

		switch (row) {
		case 4:
			// Row 0
			data = m_io_key[0]->read();
			break;

		case 12:
			// Row 1
			data = m_io_key[1]->read();
			break;

		case 2:
			// Row 2
			data = m_io_key[2]->read();
			break;

		case 10:
			// Row 3
			data = m_io_key[3]->read();
			break;

		case 6:
			// Row 4
			data = m_io_key[4]->read();
			break;

		case 14:
			// Row 5
			data = m_io_key[5]->read();
			break;

		case 1:
			// Row 6
			data = m_io_key[6]->read();
			break;

		case 9:
			// Row 7
			data = m_io_key[7]->read();
			break;

		default:
			data = 0xff;
			break;
		}
		return data & 0xff;
	} else {
		// No row selected
		return 0xff;
	}
}

void imds2ioc_device::kb_port_p1_w(uint8_t data)
{
	m_kb_p1 = data;
}

READ_LINE_MEMBER(imds2ioc_device::kb_port_t0_r)
{
	// T0 tied low
	// It appears to be some kind of strapping option on kb hw
	return 0;
}

READ_LINE_MEMBER(imds2ioc_device::kb_port_t1_r)
{
	// T1 tied low
	// It appears to be some kind of strapping option on kb hw
	return 0;
}

void imds2ioc_device::ioc_dbbout_w(offs_t offset, uint8_t data)
{
	m_ioc_obf = ~data;
	// Set/reset OBF flag (b0)
	m_ipc_ioc_status = ((offset & 1) == 0) | (m_ipc_ioc_status & ~0x01);
}

void imds2ioc_device::ioc_f0_w(offs_t offset, uint8_t data)
{
	// Set/reset F0 flag (b2)
	m_ipc_ioc_status = ((offset & 1) << 2) | (m_ipc_ioc_status & ~0x04);
}

void imds2ioc_device::ioc_set_f1_w(uint8_t data)
{
	// Set F1 flag (b3)
	m_ipc_ioc_status |= 0x08;
}

void imds2ioc_device::ioc_reset_f1_w(uint8_t data)
{
	// Reset F1 flag (b3)
	m_ipc_ioc_status &= ~0x08;
}

uint8_t imds2ioc_device::ioc_status_r()
{
	return ~m_ipc_ioc_status;
}

uint8_t imds2ioc_device::ioc_dbbin_r()
{
	// Reset IBF flag (b1)
	if (!machine().side_effects_disabled())
		m_ipc_ioc_status &= ~0x02;
	return ~m_ioc_ibf;
}

uint8_t imds2ioc_device::dbb_master_r(offs_t offset)
{
	// Read status register
	if (BIT(offset, 0))
		return m_ipc_ioc_status;
	else
	{
		// Reset OBF flag (b0)
		if (!machine().side_effects_disabled())
			m_ipc_ioc_status &= ~0x01;

		// Read output buffer
		return m_ioc_obf;
	}
}

void imds2ioc_device::dbb_master_w(offs_t offset, uint8_t data)
{
	// Set IBF flag (b1)
	m_ipc_ioc_status |= 0x02;

	// Command/Data = Master A0
	if (BIT(offset, 0))
	{
		// Set F1 flag (b3)
		m_ipc_ioc_status |= 0x08;
	}
	else
	{
		// Reset F1 flag (b3)
		m_ipc_ioc_status &= ~0x08;
	}

	// Set input buffer
	m_ioc_ibf = data;
}

WRITE_LINE_MEMBER(imds2ioc_device::hrq_w)
{
	// Should be propagated to HOLD input of IOC CPU
	m_iocdma->hlda_w(state);
}

uint8_t imds2ioc_device::ioc_mem_r(offs_t offset)
{
	address_space& prog_space = m_ioccpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void imds2ioc_device::ioc_mem_w(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_ioccpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

uint8_t imds2ioc_device::pio_port_p1_r()
{
	// If STATUS ENABLE/ == 0 return inverted device status byte, else return 0xff
	// STATUS ENABLE/ == 0 when P23-P20 == 12 & P24 == 0 & P25 = 1 & P26 = 1
	if ((m_pio_port2 & 0x7f) == 0x6c) {
		return ~m_device_status_byte;
	} else {
	return 0xff;
}
}

void imds2ioc_device::pio_port_p1_w(uint8_t data)
{
	m_pio_port1 = data;
	update_printer();
}

uint8_t imds2ioc_device::pio_port_p2_r()
{
	return m_pio_port2;
}

void imds2ioc_device::pio_port_p2_w(uint8_t data)
{
	m_pio_port2 = data;
	update_printer();
	// Send INTR to IPC
	m_parallel_int_cb(BIT(data, 7));
}

WRITE_LINE_MEMBER(imds2ioc_device::pio_lpt_ack_w)
{
	if (state) {
		m_device_status_byte |= 0x20;
	} else {
		m_device_status_byte &= ~0x20;
	}
}

WRITE_LINE_MEMBER(imds2ioc_device::pio_lpt_busy_w)
{
	if (state) {
		m_device_status_byte |= 0x10;
	} else {
		m_device_status_byte &= ~0x10;
	}
}

I8275_DRAW_CHARACTER_MEMBER(imds2ioc_device::crtc_display_pixels)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	uint8_t const chargen_byte = m_chargen[ (linecount & 7) | ((unsigned)charcode << 3) ];
	uint16_t pixels;

	if (lten) {
		pixels = ~0;
	} else if (vsp != 0 || (linecount & 8) != 0) {
		pixels = 0; // VSP is gated with LC3
	} else {
		// See hardware ref. manual, pg 58 for the very peculiar way of generating character images
		// Here each half-pixel is translated into a full pixel
		uint16_t exp_pix_l;
		uint16_t exp_pix_r;

		exp_pix_l = (uint16_t)chargen_byte;
		exp_pix_l = ((exp_pix_l & 0x80) << 5) |
			((exp_pix_l & 0x40) << 4) |
			((exp_pix_l & 0x20) << 3) |
			((exp_pix_l & 0x10) << 2) |
			((exp_pix_l & 0x08) << 1) |
			(exp_pix_l & 0x04);
		exp_pix_l |= (exp_pix_l << 1);
		exp_pix_r = exp_pix_l;

		// Layout of exp_pix_l/r:
		// Bit #              : F  E  D  C  B  A  9  8  7  6  5  4  3  2  1  0
		// Bit of chargen_byte: 0  0  b7 b7 b6 b6 b5 b5 b4 b4 b3 b3 b2 b2 0  0
		if ((chargen_byte & 2) == 0) {
			exp_pix_l >>= 1;
		}
		exp_pix_l &= 0x3fc0;

		if ((chargen_byte & 1) == 0) {
			exp_pix_r >>= 1;
		}
		exp_pix_r &= 0x003e;

		pixels = exp_pix_l | exp_pix_r;
	}

	if (rvv) {
		pixels = ~pixels;
	}

	for (unsigned i = 0; i < 14; i++) {
		bitmap.pix(y, x + i) = palette[ (pixels & (1U << (13 - i))) != 0 ];
	}
}

uint8_t imds2ioc_device::pio_master_r(offs_t offset)
{
	return m_iocpio->upi41_master_r(offset);
}

void imds2ioc_device::pio_master_w(offs_t offset, uint8_t data)
{
	m_iocpio->upi41_master_w(offset, data);
}

void imds2ioc_device::device_resolve_objects()
{
	m_master_intr_cb.resolve_safe();
	m_parallel_int_cb.resolve_safe();
}

void imds2ioc_device::device_start()
{
	m_iocfdc->set_ready_line_connected(true);
}

void imds2ioc_device::device_reset()
{
	m_ipc_ioc_status = 0x0f;

	m_iocfdc->set_rate(500000); // The IMD images show a rate of 500kbps
}

void imds2ioc_device::update_beeper()
{
	m_iocbeep->set_state(m_beeper_timer == 0 && BIT(m_miscout, 0) == 0);
}

void imds2ioc_device::update_printer()
{
	// Data to printer is ~P1 when STATUS ENABLE/==1, else 0xff (assuming pull-ups on printer)
	uint8_t printer_data;
	if ((m_pio_port2 & 0x7f) == 0x6c) {
		printer_data = 0xff;
	} else {
		printer_data = ~m_pio_port1;
	}
	m_centronics->write_data0(BIT(printer_data, 0));
	m_centronics->write_data1(BIT(printer_data, 1));
	m_centronics->write_data2(BIT(printer_data, 2));
	m_centronics->write_data3(BIT(printer_data, 3));
	m_centronics->write_data4(BIT(printer_data, 4));
	m_centronics->write_data5(BIT(printer_data, 5));
	m_centronics->write_data6(BIT(printer_data, 6));
	m_centronics->write_data7(BIT(printer_data, 7));

	// LPT DATA STROBE/ == 0 when P23-P20 == 9 & P24 == 0
	m_centronics->write_strobe((m_pio_port2 & 0x1f) != 0x09);
}

static INPUT_PORTS_START(imds2ioc)
	// See schematic, pg 56 for key matrix layout
	// See schematic, pg 57 for keyboard layout
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)           PORT_CHAR('\t')                     // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR('@') PORT_CHAR('`')       // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)         PORT_CHAR(',') PORT_CHAR('<')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)         PORT_CHAR(13)                       // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)         PORT_CHAR(' ')                      // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)     PORT_CHAR(':') PORT_CHAR('*')       // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR('>')       // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)         PORT_CHAR('/') PORT_CHAR('?')       // OK

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('z') PORT_CHAR('Z')       // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)             PORT_CHAR('x') PORT_CHAR('X')       // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('m') PORT_CHAR('M')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)             PORT_CHAR('v') PORT_CHAR('V')       // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('c') PORT_CHAR('C')       // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)             PORT_CHAR('n') PORT_CHAR('N')       // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('b') PORT_CHAR('B')       // OK

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR('~')       // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR('[') PORT_CHAR('{')       // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('o') PORT_CHAR('O')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)             PORT_CHAR('l') PORT_CHAR('L')       // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)             PORT_CHAR('9') PORT_CHAR(')')       // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)         PORT_CHAR('-') PORT_CHAR('=')       // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)             PORT_CHAR('p') PORT_CHAR('P')       // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)         PORT_CHAR(';') PORT_CHAR('+')       // OK

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('s') PORT_CHAR('S')       // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)             PORT_CHAR('d') PORT_CHAR('D')       // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('k') PORT_CHAR('K')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)             PORT_CHAR('g') PORT_CHAR('G')       // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)             PORT_CHAR('a') PORT_CHAR('A')       // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('f') PORT_CHAR('F')       // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)             PORT_CHAR('j') PORT_CHAR('J')       // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('h') PORT_CHAR('H')       // OK

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)             PORT_CHAR('w') PORT_CHAR('W')       // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('e') PORT_CHAR('E')       // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)             PORT_CHAR('i') PORT_CHAR('I')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('t') PORT_CHAR('T')       // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('q') PORT_CHAR('Q')       // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)             PORT_CHAR('r') PORT_CHAR('R')       // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('u') PORT_CHAR('U')       // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)             PORT_CHAR('y') PORT_CHAR('Y')       // OK

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR('"')       // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)             PORT_CHAR('3') PORT_CHAR('#')       // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR('(')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)             PORT_CHAR('5') PORT_CHAR('%')       // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR('!')       // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR('$')       // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)             PORT_CHAR('7') PORT_CHAR('\'')      // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR('&')       // OK

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)     PORT_CHAR(8)                        // BS
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)          PORT_CHAR(UCHAR_MAMEKEY(HOME))      // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)         PORT_CHAR('\\') PORT_CHAR('|')      // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)         PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)      PORT_CHAR(UCHAR_SHIFT_2)            // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)          PORT_CHAR(UCHAR_MAMEKEY(LEFT))      // OK
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)    PORT_CHAR(']') PORT_CHAR('}')       // OK
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)            PORT_CHAR(UCHAR_MAMEKEY(UP))        // OK

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)           PORT_CHAR(UCHAR_MAMEKEY(ESC))       // OK
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)          PORT_CHAR(UCHAR_MAMEKEY(DOWN))      // OK
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('_') PORT_CHAR('^')       // OK
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)        PORT_CHAR(UCHAR_SHIFT_1)            // OK
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)          PORT_CHAR(UCHAR_MAMEKEY(LALT))      // OK
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)      PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	// Options on IOC: see schematic, pg 40
	PORT_START("IOC_OPTS")
	PORT_DIPNAME(0x80, 0x80, "Keyboard present")
	PORT_DIPSETTING(0x80, DEF_STR(Yes))
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPNAME(0x28, 0x00, "IOC mode")
	PORT_DIPSETTING(0x00, "On line")
	PORT_DIPSETTING(0x08, "Local")
	PORT_DIPSETTING(0x20, "Diagnostic")
	PORT_DIPNAME(0x02, 0x00, "Floppy present")
	PORT_DIPSETTING(0x02, DEF_STR(Yes))
	PORT_DIPSETTING(0x00, DEF_STR(No))
	PORT_DIPNAME(0x01, 0x01, "CRT frame frequency")
	PORT_DIPSETTING(0x01, "50 Hz")
	PORT_DIPSETTING(0x00, "60 Hz")
INPUT_PORTS_END

static GFXLAYOUT_RAW(imds2_charlayout, 8, 8, 8, 64)

static GFXDECODE_START(gfx_imds2)
	GFXDECODE_ENTRY("gfx1", 0x0000, imds2_charlayout, 0, 1)
GFXDECODE_END

static void imds2_floppies(device_slot_interface &device)
{
	device.option_add("8sssd", FLOPPY_8_SSSD);
}

void imds2ioc_device::device_add_mconfig(machine_config &config)
{
	I8080A(config, m_ioccpu, IOC_XTAL_Y2 / 18);     // 2.448 MHz but running at 50% (due to wait states & DMA usage of bus)
	m_ioccpu->set_addrmap(AS_PROGRAM, &imds2ioc_device::mem_map);
	m_ioccpu->set_addrmap(AS_IO, &imds2ioc_device::io_map);
	config.set_maximum_quantum(attotime::from_hz(100));

	// The IOC CRT hw is a bit complex, as the character clock (CCLK) to i8275
	// is varied according to the part of the video frame being scanned and according to
	// the 50/60 Hz option jumper (W8).
	// The basic clock (BCLK) runs at 22.032 MHz.
	// CCLK = BCLK / 14 when in the active region of video
	// CCLK = BCLK / 12 when in horizontal retrace (HRTC=1)
	// CCLK = BCLK / 10 when in horizontal retrace of "short scan lines" (50 Hz only)
	//
	// ***** 50 Hz timings *****
	// 80 chars/row, 26 chars/h. retrace, 11 scan lines/row, 25 active rows, 3 vertical retrace rows
	// Scan line: 80 chars * 14 BCLK + 26 chars * 12 BCLK = 1432 BCLK (64.996 usec)
	// Scan row: 11 * scan lines = 15752 BCLK (714.960 usec)
	// "Short" scan line: 80 chars * 14 BCLK + 26 chars * 10 BCLK = 1380 BCLK (62.636 usec)
	// Frame: 28 scan rows (8 scan lines of 27th row are short): 27 * scan row + 3 * scan line + 8 * short scan line: 440640 BCLK (20 msec)
	//
	// ***** 60 Hz timings *****
	// 80 chars/row, 20 chars/h. retrace, 10 scan lines/row, 25 active rows, 2 vertical retrace rows
	// Scan line: 80 chars * 14 BCLK + 20 chars * 12 BCLK = 1360 BCLK (61.728 usec)
	// Scan row: 10 * scan lines = 13600 BCLK (617.284 usec)
	// Frame: 27 scan rows : 367200 BCLK (16.667 msec)
	//
	// Clock here is semi-bogus: it gives the correct frame frequency at 50 Hz (with the incorrect
	// assumption that CCLK is fixed at BCLK / 14)
	I8275(config, m_ioccrtc, 22853600 / 14);
	m_ioccrtc->set_character_width(14);
	m_ioccrtc->set_refresh_hack(true);
	m_ioccrtc->set_display_callback(FUNC(imds2ioc_device::crtc_display_pixels));
	m_ioccrtc->drq_wr_callback().set(m_iocdma, FUNC(i8257_device::dreq2_w));
	m_ioccrtc->irq_wr_callback().set_inputline(m_ioccpu, I8085_INTR_LINE);
	m_ioccrtc->set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_screen_update("ioccrtc", FUNC(i8275_device::screen_update));
	screen.set_refresh_hz(50);
	GFXDECODE(config, m_gfxdecode, m_palette, gfx_imds2);
	PALETTE(config, m_palette, palette_device::MONOCHROME);

	SPEAKER(config, "mono").front_center();
	BEEP(config, m_iocbeep, IOC_BEEP_FREQ).add_route(ALL_OUTPUTS, "mono", 1.00);

	I8257(config, m_iocdma, IOC_XTAL_Y2 / 9);
	m_iocdma->out_hrq_cb().set(FUNC(imds2ioc_device::hrq_w));
	m_iocdma->in_memr_cb().set(FUNC(imds2ioc_device::ioc_mem_r));
	m_iocdma->out_memw_cb().set(FUNC(imds2ioc_device::ioc_mem_w));
	m_iocdma->in_ior_cb<1>().set("iocfdc", FUNC(i8271_device::data_r));
	m_iocdma->out_iow_cb<1>().set("iocfdc", FUNC(i8271_device::data_w));
	m_iocdma->out_iow_cb<2>().set(m_ioccrtc, FUNC(i8275_device::dack_w));

	PIT8253(config, m_ioctimer, 0);
	m_ioctimer->set_clk<0>(IOC_XTAL_Y1 / 4);
	m_ioctimer->out_handler<0>().set(m_ioctimer, FUNC(pit8253_device::write_clk2));
	m_ioctimer->out_handler<2>().set(FUNC(imds2ioc_device::beep_timer_w));

	I8271(config, m_iocfdc, IOC_XTAL_Y1 / 2);
	m_iocfdc->drq_wr_callback().set(m_iocdma, FUNC(i8257_device::dreq1_w));
	FLOPPY_CONNECTOR(config, "iocfdc:0", imds2_floppies, "8sssd", floppy_image_device::default_mfm_floppy_formats, true);

	I8041A(config, m_iocpio, IOC_XTAL_Y3);
	m_iocpio->p1_in_cb().set(FUNC(imds2ioc_device::pio_port_p1_r));
	m_iocpio->p1_out_cb().set(FUNC(imds2ioc_device::pio_port_p1_w));
	m_iocpio->p2_in_cb().set(FUNC(imds2ioc_device::pio_port_p2_r));
	m_iocpio->p2_out_cb().set(FUNC(imds2ioc_device::pio_port_p2_w));

	I8741A(config, m_kbcpu, 3.579545_MHz_XTAL);
	m_kbcpu->p1_out_cb().set(FUNC(imds2ioc_device::kb_port_p1_w));
	m_kbcpu->p2_in_cb().set(FUNC(imds2ioc_device::kb_port_p2_r));
	m_kbcpu->t0_in_cb().set(FUNC(imds2ioc_device::kb_port_t0_r));
	m_kbcpu->t1_in_cb().set(FUNC(imds2ioc_device::kb_port_t1_r));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(FUNC(imds2ioc_device::pio_lpt_ack_w));
	m_centronics->busy_handler().set(FUNC(imds2ioc_device::pio_lpt_busy_w));
}

ROM_START(imds2ioc)
	// ROM definition of IOC cpu (8080A)
	ROM_REGION(0x2000, "ioccpu", 0)
	ROM_DEFAULT_BIOS("v15")
	ROM_SYSTEM_BIOS(0, "v15", "Series II IOC Diagnostic v1.5") // "CORP INTEL corp.1981-83"
	ROMX_LOAD("ioc_a50_104692-001.bin", 0x0000, 0x0800, CRC(d9f926a1) SHA1(bd9d0f7458acc2806120a6dbaab9c48be315b060), ROM_BIOS(0))
	ROMX_LOAD("ioc_a51_104692-002.bin", 0x0800, 0x0800, CRC(6aa2f86c) SHA1(d3a5314d86e3366545b4c97b29e323dfab383d5f), ROM_BIOS(0))
	ROMX_LOAD("ioc_a52_104692-003.bin", 0x1000, 0x0800, CRC(b88a38d5) SHA1(934716a1daec852f4d1f846510f42408df0c9584), ROM_BIOS(0))
	ROMX_LOAD("ioc_a53_104692-004.bin", 0x1800, 0x0800, CRC(c8df4bb9) SHA1(2dfb921e94ae7033a7182457b2f00657674d1b77), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v14", "Series II IOC Diagnostic v1.4") // "(C) Intel Corp. 1981,1982"
	ROMX_LOAD("a62_2716.bin", 0x0000, 0x0800, CRC(86a55b2f) SHA1(21033f7abb2c3e08028613e0c35ffecb703ff4f1), ROM_BIOS(1))
	ROMX_LOAD("a51_2716.bin", 0x0800, 0x0800, CRC(ee55c448) SHA1(16c2f7e3b5baeb398adcc59603943910813e6a9b), ROM_BIOS(1))
	ROMX_LOAD("a52_2716.bin", 0x1000, 0x0800, CRC(8db1b33e) SHA1(6fc5e438009636dd6d7185071b152b0491d3baeb), ROM_BIOS(1))
	ROMX_LOAD("a53_2716.bin", 0x1800, 0x0800, CRC(01690f4f) SHA1(eadef30a3797f41e08d28e7691f8de44c0f3b8ea), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v13", "Series II IOC Diagnostic v1.3") // no copyright string; accompanying installation instructions © 1980 Intel Corporation
	ROMX_LOAD("104593-001.a50", 0x0000, 0x0800, CRC(801de7ad) SHA1(291b7db8a163b970e9082bdb637d7dee436c7a62), ROM_BIOS(2))
	ROMX_LOAD("104593-002.a51", 0x0800, 0x0800, CRC(6c8e21b6) SHA1(b3e63b2846a79866eb74ab354001996424ec22a0), ROM_BIOS(2))
	ROMX_LOAD("104593-003.a52", 0x1000, 0x0800, CRC(134c8115) SHA1(cd8c566344ee7a9cda78eea5fd2efdaca5a2e540), ROM_BIOS(2))
	ROMX_LOAD("104593-004.a53", 0x1800, 0x0800, CRC(04407e2a) SHA1(ecd65e4337d2bb7f5f6fdaae95696c9207ba57ee), ROM_BIOS(2))

	// ROM definition of PIO controller (8041A)
	ROM_REGION(0x400, "iocpio", 0)
	ROM_LOAD("104566-0012.bin", 0, 0x400, CRC(f999e5da) SHA1(77ac1fab5443a16f906b25926ab3ba3ae42db6bb))

	// ROM definition of keyboard controller (8741)
	ROM_REGION(0x400, "kbcpu", 0)
	ROM_LOAD("kbd_a3_104675-001.bin", 0, 0x400, CRC(ba7c4303) SHA1(19899af732d0ae1247bfc79979b1ee5f339ee5cf))

	// ROM definition of character generator (2708, A19 on IOC)
	ROM_REGION(0x400, "gfx1", 0)
	ROM_LOAD("ioc_a19_101539-001.bin", 0x0000, 0x0400, CRC(47487d0f) SHA1(0ed98f9f06622949ee3cc2ffc572fb9702db0f81))
ROM_END

ioport_constructor imds2ioc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(imds2ioc);
}

const tiny_rom_entry *imds2ioc_device::device_rom_region() const
{
	return ROM_NAME(imds2ioc);
}
