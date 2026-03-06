// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*******************************************************************************

    Wren Executive

    TODO:
    - interrupt priorities for ADC and parallel port
    - layout with POWER and CAPS LOCK LED's, also floppy drive LED's (to indicate current drive)
    - Winchester (unknown hardware)

*******************************************************************************/

#include "emu.h"

#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/floppy.h"
#include "machine/adc0804.h"
#include "machine/bankdev.h"
#include "machine/i8279.h"
#include "machine/mc146818.h"
#include "machine/mc68681.h"
#include "machine/ram.h"
#include "machine/wd_fdc.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#include "formats/hxchfe_dsk.h"
#include "formats/wren_dsk.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class wren_state : public driver_device
{
public:
	wren_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_bankdev(*this, "bankdev")
		, m_ram(*this, "ram")
		, m_col_ram(*this, "col_ram")
		, m_scr_ram(*this, "scr_ram")
		, m_chr_ram(*this, "chr_ram")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:%u", 0U)
		, m_centronics(*this, "centronics")
		, m_cent_data_out(*this, "cent_data_out")
		, m_cent_status_in(*this, "cent_status_in")
		, m_kbd_row(*this, "ROW%u", 0U)
		, m_channel(*this, "CHANNEL%u", 0U)
		, m_power_led(*this, "power_led")
		, m_caps_led(*this, "caps_led")
	{ }

	void wren(machine_config &config);

	void init_wren();

	DECLARE_INPUT_CHANGED_MEMBER(reset_palette);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	static void floppy_formats(format_registration &fr);

	void bank_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	MC6845_UPDATE_ROW(crtc_update_row);

	template<int Line> void int_w(int state);
	TIMER_CALLBACK_MEMBER(update_interrupts);
	IRQ_CALLBACK_MEMBER(restart_cb);

	void update_palette(int monitor);

	void bank_select_w(uint8_t data);
	void video_control_w(uint8_t data);
	void relay_control_w(uint8_t data);
	uint8_t parallel_r();
	void parallel_w(uint8_t data);
	void fdc_control_w(uint8_t data);
	void fdc_dma(int state);

	required_device<z80_device> m_maincpu;
	required_device<address_map_bank_device> m_bankdev;
	required_device<ram_device> m_ram;
	required_shared_ptr<uint8_t> m_col_ram;
	required_shared_ptr<uint8_t> m_scr_ram;
	required_shared_ptr<uint8_t> m_chr_ram;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<wd2791_device> m_fdc;
	required_device_array<floppy_connector, 2> m_floppy;
	required_device<centronics_device> m_centronics;
	required_device<output_latch_device> m_cent_data_out;
	required_device<input_buffer_device> m_cent_status_in;
	required_ioport_array<8> m_kbd_row;
	required_ioport_array<8> m_channel;
	output_finder<> m_power_led;
	output_finder<> m_caps_led;

	uint8_t m_kbd_col;
	uint8_t m_bank_select;
	uint8_t m_video_control;
	uint8_t m_relay_control;
	uint8_t m_adc_channel;
	uint8_t m_fdc_control;
	uint16_t m_fdc_dma_addr;

	uint8_t m_interrupts;
};


void wren_state::bank_map(address_map &map)
{
	map(0x00000, 0x3ffff).rw(m_ram, FUNC(ram_device::read), FUNC(ram_device::write));
	map(0x40000, 0x43fff).rom().region("maincpu", 0x4000);
	map(0x80000, 0x83fff).mirror(0x70000).rom().region("maincpu", 0x0000);
	map(0x84000, 0x87fff).mirror(0x70000).ram().share("col_ram");
	map(0x88000, 0x8bfff).mirror(0x70000).ram().share("scr_ram");
	map(0x8c000, 0x8cfff).mirror(0x70000).ram().share("chr_ram");
	//   $xF000 - $xFFFF is common RAM, always select bank 0
	map(0x0f000, 0x0ffff).mirror(0xf0000).lr8(NAME([this] (offs_t offset) { return m_ram->pointer()[offset + 0xf000]; }));
	map(0x0f000, 0x0ffff).mirror(0xf0000).lw8(NAME([this] (offs_t offset, uint8_t data) { m_ram->pointer()[offset + 0xf000] = data; }));
}

void wren_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).m(m_bankdev, FUNC(address_map_bank_device::amap8));
}

void wren_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(wren_state::bank_select_w));
	map(0x01, 0x01).w(FUNC(wren_state::video_control_w));
	//map(0x02, 0x02).rw() wini data
	//map(0x03, 0x03).w()  1 = wini reset, 0 = wini run
	map(0x04, 0x04).w(FUNC(wren_state::relay_control_w));
	map(0x05, 0x05).w(FUNC(wren_state::fdc_control_w));
	map(0x06, 0x06).lw8(NAME([this](uint8_t data) { m_fdc_dma_addr &= 0x00ff; m_fdc_dma_addr |= data << 8; }));
	map(0x07, 0x07).lw8(NAME([this](uint8_t data) { m_fdc_dma_addr &= 0xff00; m_fdc_dma_addr |= data; }));
	map(0x08, 0x09).rw(FUNC(wren_state::parallel_r), FUNC(wren_state::parallel_w));
	map(0x18, 0x19).rw("adc", FUNC(adc0804_device::read), FUNC(adc0804_device::write));
	map(0x1a, 0x1a).rw("i8279", FUNC(i8279_device::data_r), FUNC(i8279_device::data_w));
	map(0x1b, 0x1b).rw("i8279", FUNC(i8279_device::status_r), FUNC(i8279_device::cmd_w));
	map(0x1c, 0x1c).rw("rtc", FUNC(mc146818_device::data_r), FUNC(mc146818_device::data_w));
	map(0x1d, 0x1d).w("rtc", FUNC(mc146818_device::address_w));
	map(0x1e, 0x1e).rw(m_crtc, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x1f, 0x1f).rw(m_crtc, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x20, 0x20).mirror(0x06).rw("psg", FUNC(ay8912_device::data_r), FUNC(ay8912_device::data_w));
	map(0x21, 0x21).mirror(0x06).w("psg", FUNC(ay8912_device::address_w));
	map(0x28, 0x2b).rw(m_fdc, FUNC(wd2791_device::read), FUNC(wd2791_device::write));
	map(0x30, 0x3f).rw("uart", FUNC(scn2681_device::read), FUNC(scn2681_device::write));
}


void wren_state::machine_start()
{
	m_power_led.resolve();
	m_caps_led.resolve();

	m_kbd_col       = 0x00;
	m_video_control = 0x00;
	m_relay_control = 0x00;
	m_adc_channel   = 0x00;
	m_fdc_control   = 0x00;
	m_fdc_dma_addr  = 0x00;

	save_item(NAME(m_kbd_col));
	save_item(NAME(m_bank_select));
	save_item(NAME(m_video_control));
	save_item(NAME(m_relay_control));
	save_item(NAME(m_adc_channel));
	save_item(NAME(m_fdc_control));
	save_item(NAME(m_fdc_dma_addr));
	save_item(NAME(m_interrupts));
}

void wren_state::machine_reset()
{
	// select bank 8 on reset
	bank_select_w(8);

	// clear pending interrupts
	m_interrupts = 0;

	// initialise FDC drive control
	fdc_control_w(0);

	// tied low to select mini-floppy in double density
	m_fdc->dden_w(0);
	m_fdc->enmf_w(0);
}


void wren_state::init_wren()
{
	// initialise palette for amber monitor
	update_palette(0);
}


template <int Line>
void wren_state::int_w(int state)
{
	if (BIT(m_interrupts, Line) == state)
		return;

	if (state)
		m_interrupts |= 0x01 << Line;
	else
		m_interrupts &= ~(0x01 << Line);

	machine().scheduler().synchronize(timer_expired_delegate(FUNC(wren_state::update_interrupts), this));
}

TIMER_CALLBACK_MEMBER(wren_state::update_interrupts)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_interrupts != 0 ? ASSERT_LINE : CLEAR_LINE);
}

IRQ_CALLBACK_MEMBER(wren_state::restart_cb)
{
	// IM 2 vector is produced through a 74LS148 priority encoder plus some buffers and a latch
	uint8_t vector = 0xf0;
	uint8_t active = m_interrupts;

	while (vector < 0xfe && !BIT(active, 0))
	{
		active >>= 1;
		vector += 0x02;
	}
	return vector;
}


void wren_state::bank_select_w(uint8_t data)
{
	// Bank select
	// bits 0 to 3 = bank select (A16 to A19)
	m_bank_select = data & 0x0f;

	m_bankdev->set_bank(m_bank_select);
}


void wren_state::video_control_w(uint8_t data)
{
	// Video control port
	// 0 = flash enable       1 = hide enable
	// 2 = 40 characters      3 = bit map
	// 4 = NOT double height  5 = display enable
	// 6 = NOT POWER ON LED   7 = NOT CAPS LOCK LED
	m_video_control = data;

	if (BIT(data, 2))
		m_crtc->set_unscaled_clock(12_MHz_XTAL / 8 / 2);
	else
		m_crtc->set_unscaled_clock(12_MHz_XTAL / 8);

	m_power_led = BIT(data, 6);
	m_caps_led  = BIT(data, 7);
}


void wren_state::relay_control_w(uint8_t data)
{
	// Relay control port
	// 0 = relay 1 ON         1 = relay 2 ON
	// 2 = relay 3 ON         3 = relay 4 ON
	// 4 = spare              5 = Modem TRS
	// 6 = Modem TXR2         7 = Modem TXR1
	m_relay_control = data;
}


uint8_t wren_state::parallel_r()
{
	m_centronics->write_strobe(1);

	return m_cent_status_in->read();
}

void wren_state::parallel_w(uint8_t data)
{
	m_cent_data_out->write(data);

	m_centronics->write_strobe(0);
}


void wren_state::fdc_control_w(uint8_t data)
{
	// FDC drive control & DMA bank address
	// bits 0 to 3 are bank select
	// 4 = second head        5 = drives disable
	// 6 = select drive B     7 = write to FDC
	m_fdc_control = data;

	floppy_image_device *floppy = m_floppy[BIT(data, 6)]->get_device();

	m_fdc->set_floppy(floppy);

	floppy->ss_w(!BIT(data, 4));

	if (BIT(data, 5))
	{
		// disable drives
		m_floppy[0]->get_device()->mon_w(1);
		m_floppy[1]->get_device()->mon_w(1);
	}
	else
	{
		// enable selected drive
		m_floppy[0]->get_device()->mon_w(BIT(data, 6));
		m_floppy[1]->get_device()->mon_w(!BIT(data, 6));
	}
}


void wren_state::fdc_dma(int state)
{
	if (state)
	{
		// select DMA bank
		m_bankdev->set_bank(m_fdc_control & 0x0f);

		if (BIT(m_fdc_control, 7))
			m_fdc->data_w(m_bankdev->read8(m_fdc_dma_addr++));
		else
			m_bankdev->write8(m_fdc_dma_addr++, m_fdc->data_r());

		// restore CPU bank
		m_bankdev->set_bank(m_bank_select);
	}
}


INPUT_PORTS_START(wren)
	PORT_START("ROW0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('@')  PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)        PORT_CHAR(UCHAR_MAMEKEY(F11))       PORT_NAME("*") // not verified
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))       PORT_NAME("#") // not verified
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                     PORT_NAME("DEL")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME(u8"\u2192") // U+2192 = →
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(0x0d)                     PORT_NAME("RETURN")

	PORT_START("ROW1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(':')  PORT_CHAR('*')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')

	PORT_START("ROW2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))      PORT_NAME("HOME")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')

	PORT_START("ROW3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('=')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')

	PORT_START("ROW4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))      PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                     PORT_NAME("TAB")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')                      PORT_NAME("SPACE")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  PORT_NAME("CAPS LOCK")

	PORT_START("ROW5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))        PORT_NAME("F1")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))        PORT_NAME("F4")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))        PORT_NAME("F5")

	PORT_START("ROW6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')

	PORT_START("ROW7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))        PORT_NAME("F2")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_NAME("ESC")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))        PORT_NAME("F3")

	PORT_START("SHIFT")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("SHIFT")

	PORT_START("CTRL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))  PORT_NAME("CTRL")


	PORT_START("CHANNEL0")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_NAME("Left Paddle X")

	PORT_START("CHANNEL1")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_NAME("Left Paddle Y")

	PORT_START("CHANNEL2")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_NAME("Right Paddle X")

	PORT_START("CHANNEL3")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_NAME("Right Paddle Y")

	PORT_START("CHANNEL4")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Z) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_NAME("Left Paddle Z")

	PORT_START("CHANNEL5")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Z) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(20) PORT_NAME("Right Paddle Z")

	PORT_START("CHANNEL6")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CHANNEL7")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)


	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(wren_state::reset_palette), 0)
	PORT_CONFSETTING(    0x00, "Internal (Amber)")
	PORT_CONFSETTING(    0x01, "External (Colour)")
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(wren_state::reset_palette)
{
	update_palette(newval);
}


void wren_state::update_palette(int monitor)
{
	for (int i = 0; i < m_palette->entries(); i++)
	{
		const rgb_t pen = rgb_t(pal1bit(i >> 0), pal1bit(i >> 1), pal1bit(i >> 2));
		const float luma = float(pen.r()) * 0.299 + float(pen.g()) * 0.587 + float(pen.b()) * 0.114;
		switch (monitor)
		{
		case 0: // internal (amber)
			m_palette->set_pen_color(i, rgb_t(1.0 * luma, 0.8 * luma, 0.1 * luma));
			break;
		case 1: // external (colour)
			m_palette->set_pen_color(i, pen);
			break;
		}
	}
}


MC6845_UPDATE_ROW(wren_state::crtc_update_row)
{
	if (BIT(m_video_control, 5)) // display enable
	{
		const rgb_t *palette = m_palette->palette()->entry_list_raw();

		uint32_t *p = &bitmap.pix(y);

		uint8_t colour = 0x00;
		uint8_t data   = 0x00;
		uint8_t dbl_ht = 0x00; // reset at end of each line with display enable

		for (int x_pos = 0; x_pos < x_count; x_pos++)
		{
			uint16_t addr = ma + x_pos;

			if (BIT(m_video_control, 3)) // bitmap
			{
				addr += ra << 10;
				colour = m_col_ram[addr];
				data   = m_scr_ram[addr];
			}
			else
			{
				if (!BIT(m_video_control, 4) && (m_col_ram[addr] == 0xff)) // double height control code
				{
					dbl_ht = m_scr_ram[addr];

					// clear last character
					if (BIT(dbl_ht, 5))
						data = 0x00;
				}
				else
				{
					colour = m_col_ram[addr];

					uint8_t line;

					// adjust line counter for double height
					if (BIT(dbl_ht, 4))
						line = (dbl_ht & 0x0f) + (ra >> 1);
					else
						line = ra & 0x0f;

					// character data
					data = m_chr_ram[(m_scr_ram[addr] << 4) | line];
				}

				if (x_pos == cursor_x)
				{
					// cursor on
					data = 0xff;
				}
				else if ((BIT(m_video_control, 0) && BIT(colour, 3)) || (BIT(m_video_control, 1) && BIT(colour, 7)))
				{
					// flash enable & flash on OR hide enable & hide on
					data = 0x00;
				}
			}

			for (int i = 0; i < 8; i++)
			{
				*p = BIT(data, 7) ? palette[BIT(colour, 4, 3)] : palette[BIT(colour, 0, 3)]; p++;
				data <<= 1;
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
}


//-------------------------------------------------
//  FLOPPY_FORMATS( floppy_formats )
//-------------------------------------------------

void wren_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_HFE_FORMAT);
	fr.add(FLOPPY_WREN_FORMAT);
}


void wren_state::wren(machine_config &config)
{
	Z80(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &wren_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &wren_state::io_map);
	m_maincpu->busack_cb().set(FUNC(wren_state::fdc_dma));
	m_maincpu->set_irq_acknowledge_callback(FUNC(wren_state::restart_cb));

	ADDRESS_MAP_BANK(config, m_bankdev).set_map(&wren_state::bank_map).set_options(ENDIANNESS_LITTLE, 8, 20, 0x10000);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(12_MHz_XTAL, 768, 0, 512, 312, 0, 256);
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette).set_entries(8);

	MC6845(config, m_crtc, 12_MHz_XTAL / 8);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(wren_state::crtc_update_row));

	RAM(config, m_ram).set_default_size("64K").set_extra_options("256K");

	i8279_device &i8279(I8279(config, "i8279", 12_MHz_XTAL / 16));
	i8279.out_sl_callback().set([this](uint8_t data) { m_kbd_col = data & 7; });
	i8279.in_rl_callback().set([this]() { return m_kbd_row[m_kbd_col]->read(); });
	i8279.in_shift_callback().set_ioport("SHIFT");
	i8279.in_ctrl_callback().set_ioport("CTRL");
	i8279.out_irq_callback().set(FUNC(wren_state::int_w<1>));

	SPEAKER(config, "mono").front_center();
	ay8912_device &psg(AY8912(config, "psg", 12_MHz_XTAL / 8));
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);

	mc146818_device &rtc(MC146818(config, "rtc", 4.194304_MHz_XTAL));
	rtc.set_epoch(1940);
	rtc.irq().set(FUNC(wren_state::int_w<4>));

	scn2681_device &uart(SCN2681(config, "uart", 3.6864_MHz_XTAL));
	uart.irq_cb().set(FUNC(wren_state::int_w<6>));
	uart.a_tx_cb().set("rs232", FUNC(rs232_port_device::write_txd));
	uart.b_tx_cb().set("modem", FUNC(rs232_port_device::write_txd));
	uart.outport_cb().set([this](uint8_t data) { m_adc_channel = (data & 0x0e) >> 5; });
	uart.outport_cb().append("rs232", FUNC(rs232_port_device::write_rts)).bit(0);
	//uart.outport_cb().append("rs232", FUNC(rs232_port_device::)).bit(2); // Ch.A Tx clk
	uart.outport_cb().append("rs232", FUNC(rs232_port_device::write_dtr)).bit(4);

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set("uart", FUNC(scn2681_device::rx_a_w));
	rs232.cts_handler().set("uart", FUNC(scn2681_device::ip0_w));
	rs232.dsr_handler().set("uart", FUNC(scn2681_device::ip3_w));
	rs232.dcd_handler().set("uart", FUNC(scn2681_device::ip4_w));

	rs232_port_device &modem(RS232_PORT(config, "modem", default_rs232_devices, nullptr)); // TCM3103
	modem.rxd_handler().set("uart", FUNC(scn2681_device::rx_b_w));
	modem.dcd_handler().set("uart", FUNC(scn2681_device::ip1_w));

	CENTRONICS(config, m_centronics, centronics_devices, nullptr);
	m_centronics->busy_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit0));
	m_centronics->perror_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit1));
	m_centronics->select_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit2));
	m_centronics->ack_handler().set(m_cent_status_in, FUNC(input_buffer_device::write_bit3));

	OUTPUT_LATCH(config, m_cent_data_out);
	m_centronics->set_output_latch(*m_cent_data_out);

	INPUT_BUFFER(config, m_cent_status_in);

	adc0804_device &adc(ADC0804(config, "adc", 12_MHz_XTAL / 16));
	adc.vin_callback().set([this]() { return m_channel[m_adc_channel]->read(); });
	//adc.intr_callback().set(FUNC(wren_state::int_w<?>));

	WD2791(config, m_fdc, 12_MHz_XTAL / 6);
	m_fdc->set_force_ready(true);
	m_fdc->intrq_wr_callback().set(FUNC(wren_state::int_w<3>));
	m_fdc->drq_wr_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSREQ);

	FLOPPY_CONNECTOR(config, "fdc:0", "525dd", FLOPPY_525_DD, true, wren_state::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, "fdc:1", "525dd", FLOPPY_525_DD, true, wren_state::floppy_formats).enable_sound(true);

	SOFTWARE_LIST(config, "flop_list").set_original("wren_flop");
}


ROM_START(wren)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASE00)
	ROM_LOAD("wren.u105", 0x0000, 0x2000, CRC(8b4e4fe4) SHA1(3412cd8b3a03f68d564a2da04ec3fc7d68cb6b99))
	ROM_LOAD("wren.u120", 0x4000, 0x2000, CRC(dd7ed73e) SHA1(d979c33c1607fbf7048f46ef22e2030935f9eb59))
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT         COMPANY         FULLNAME           FLAGS
COMP( 1984, wren,  0,      0,      wren,    wren,  wren_state,  init_wren,   "Thorn EMI",    "Wren Executive",  0 )
