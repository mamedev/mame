// license: GPL-2.0+
// copyright-holders: Kevin Thacker, Dirk Best, Phill Harvey-Smith
// thanks-to: Chris Coxall, Andrew Dunipace
/******************************************************************************

    Tatung Einstein

    TODO:
    - Einstein 256 support (need bios dump)

 ******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80daisy.h"
#include "machine/z80daisy_generic.h"
#include "bus/centronics/ctronics.h"
#include "bus/einstein/pipe/pipe.h"
#include "bus/einstein/userport/userport.h"
#include "bus/rs232/rs232.h"
#include "machine/adc0844.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "machine/ram.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "machine/wd_fdc.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "video/tms9928a.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "softlist.h"
#include "speaker.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define VERBOSE_KEYBOARD    0
#define VERBOSE_DISK        0

#define XTAL_X001  XTAL(10'738'635)
#define XTAL_X002  XTAL(8'000'000)

#define IC_I001  "i001"  /* Z8400A */
#define IC_I030  "i030"  /* AY-3-8910 */
#define IC_I038  "i038"  /* TMM9129 */
#define IC_I042  "i042"  /* WD1770-PH */
#define IC_I050  "i050"  /* ADC0844CCN */
#define IC_I058  "i058"  /* Z8430A */
#define IC_I060  "i060"  /* uPD8251A */
#define IC_I063  "i063"  /* Z8420A */


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class einstein_state : public driver_device
{
public:
	einstein_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, IC_I001),
		m_keyboard_daisy(*this, "keyboard_daisy"),
		m_adc_daisy(*this, "adc_daisy"),
		m_fire_daisy(*this, "fire_daisy"),
		m_pipe(*this, "pipe"),
		m_fdc(*this, IC_I042),
		m_ram(*this, RAM_TAG),
		m_psg(*this, IC_I030),
		m_centronics(*this, "centronics"),
		m_strobe_timer(*this, "strobe"),
		m_bios(*this, "bios"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2"),
		m_bank3(*this, "bank3"),
		m_floppy{ { *this, IC_I042 ":0" }, { *this, IC_I042 ":1" }, { *this, IC_I042 ":2" }, { *this, IC_I042 ":3" } },
		m_line(*this, "LINE%u", 0),
		m_extra(*this, "EXTRA"),
		m_buttons(*this, "BUTTONS"),
		m_rom_enabled(0),
		m_keyboard_line(0), m_keyboard_data(0xff),
		m_centronics_busy(0), m_centronics_perror(0), m_centronics_fault(0), m_strobe(-1),
		m_int(0)
	{}

	DECLARE_INPUT_CHANGED_MEMBER(joystick_button);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_timer_callback);
	DECLARE_WRITE8_MEMBER(keyboard_line_write);
	DECLARE_READ8_MEMBER(keyboard_data_read);
	DECLARE_READ8_MEMBER(reset_r);
	DECLARE_WRITE8_MEMBER(reset_w);
	DECLARE_READ8_MEMBER(rom_r);
	DECLARE_WRITE8_MEMBER(rom_w);
	template <int src> DECLARE_WRITE_LINE_MEMBER(int_w);
	DECLARE_READ8_MEMBER(kybint_msk_r);
	DECLARE_WRITE8_MEMBER(kybint_msk_w);
	DECLARE_WRITE8_MEMBER(adcint_msk_w);
	DECLARE_WRITE8_MEMBER(fireint_msk_w);
	DECLARE_WRITE8_MEMBER(drsel_w);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_busy);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_perror);
	DECLARE_WRITE_LINE_MEMBER(write_centronics_fault);
	DECLARE_WRITE_LINE_MEMBER(ardy_w);
	TIMER_DEVICE_CALLBACK_MEMBER(strobe_callback);

	void einstein(machine_config &config);
	void einstein_io(address_map &map);
	void einstein_mem(address_map &map);
protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void einstein_scan_keyboard();

	required_device<cpu_device> m_maincpu;
	required_device<z80daisy_generic_device> m_keyboard_daisy;
	required_device<z80daisy_generic_device> m_adc_daisy;
	required_device<z80daisy_generic_device> m_fire_daisy;
	required_device<tatung_pipe_device> m_pipe;
	required_device<wd1770_device> m_fdc;
	required_device<ram_device> m_ram;
	required_device<ay8910_device> m_psg;
	required_device<centronics_device> m_centronics;
	required_device<timer_device> m_strobe_timer;
	required_memory_region m_bios;
	required_memory_bank m_bank1;
	required_memory_bank m_bank2;
	required_memory_bank m_bank3;
	required_device<floppy_connector> m_floppy[4];
	required_ioport_array<8> m_line;
	required_ioport m_extra;
	required_ioport m_buttons;

	int m_rom_enabled;

	uint8_t m_keyboard_line;
	uint8_t m_keyboard_data;

	int m_centronics_busy;
	int m_centronics_perror;
	int m_centronics_fault;
	int m_strobe;

	int m_int;
};


/***************************************************************************
    KEYBOARD
***************************************************************************/

INPUT_CHANGED_MEMBER( einstein_state::joystick_button )
{
	int button_down = (m_buttons->read() & 0x03) != 0x03;
	m_fire_daisy->int_w(button_down ? ASSERT_LINE : CLEAR_LINE);
}

/* refresh keyboard data. It is refreshed when the keyboard line is written */
void einstein_state::einstein_scan_keyboard()
{
	uint8_t data = 0xff;

	if (!BIT(m_keyboard_line, 0)) data &= m_line[0]->read();
	if (!BIT(m_keyboard_line, 1)) data &= m_line[1]->read();
	if (!BIT(m_keyboard_line, 2)) data &= m_line[2]->read();
	if (!BIT(m_keyboard_line, 3)) data &= m_line[3]->read();
	if (!BIT(m_keyboard_line, 4)) data &= m_line[4]->read();
	if (!BIT(m_keyboard_line, 5)) data &= m_line[5]->read();
	if (!BIT(m_keyboard_line, 6)) data &= m_line[6]->read();
	if (!BIT(m_keyboard_line, 7)) data &= m_line[7]->read();

	m_keyboard_data = data;
}

TIMER_DEVICE_CALLBACK_MEMBER( einstein_state::keyboard_timer_callback )
{
	/* re-scan keyboard */
	einstein_scan_keyboard();

	if (m_keyboard_data != 0xff)
		m_keyboard_daisy->int_w(ASSERT_LINE);
}

WRITE8_MEMBER( einstein_state::keyboard_line_write )
{
	if (VERBOSE_KEYBOARD)
		logerror("einstein_keyboard_line_write: %02x\n", data);

	m_keyboard_line = data;

	/* re-scan the keyboard */
	einstein_scan_keyboard();
}

READ8_MEMBER( einstein_state::keyboard_data_read )
{
	/* re-scan the keyboard */
	einstein_scan_keyboard();

	if (VERBOSE_KEYBOARD)
		logerror("einstein_keyboard_data_read: %02x\n", m_keyboard_data);

	return m_keyboard_data;
}


/***************************************************************************
    FLOPPY DRIVES
***************************************************************************/

WRITE8_MEMBER(einstein_state::drsel_w)
{
	if (VERBOSE_DISK)
		logerror("%s: drsel_w %02x\n", machine().describe_context(), data);

	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy[0]->get_device();
	if (BIT(data, 1)) floppy = m_floppy[1]->get_device();
	if (BIT(data, 2)) floppy = m_floppy[2]->get_device();
	if (BIT(data, 3)) floppy = m_floppy[3]->get_device();

	if (floppy)
		floppy->ss_w(BIT(data, 4));

	m_fdc->set_floppy(floppy);
}


/***************************************************************************
    CENTRONICS
***************************************************************************/

WRITE_LINE_MEMBER( einstein_state::write_centronics_busy )
{
	m_centronics_busy = state;
}

WRITE_LINE_MEMBER( einstein_state::write_centronics_perror )
{
	m_centronics_perror = state;
}

WRITE_LINE_MEMBER( einstein_state::write_centronics_fault )
{
	m_centronics_fault = state;
}

WRITE_LINE_MEMBER( einstein_state::ardy_w )
{
	if (m_strobe == 0 && state == 1)
	{
		m_centronics->write_strobe(1);
		m_strobe_timer->adjust(attotime::from_double(TIME_OF_74LS123(RES_K(10), CAP_N(1))));
	}

	m_strobe = state;
}

TIMER_DEVICE_CALLBACK_MEMBER( einstein_state::strobe_callback )
{
	m_centronics->write_strobe(0);
}


/***************************************************************************
    INTERRUPTS
***************************************************************************/

static const z80_daisy_config einstein_daisy_chain[] =
{
	{ "keyboard_daisy" },
	{ IC_I058 },
	{ "adc_daisy" },
	{ IC_I063 },
	{ "fire_daisy" },
	{ nullptr }
};

template <int src> WRITE_LINE_MEMBER( einstein_state::int_w )
{
	int old = m_int;

	if (state)
	{
		m_int |= (1 << src);
		if (!old)
		{
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
			m_pipe->host_int_w(ASSERT_LINE);
		}
	}
	else
	{
		m_int &= ~(1 << src);
		if (old && !m_int)
		{
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
			m_pipe->host_int_w(CLEAR_LINE);
		}
	}
}

READ8_MEMBER( einstein_state::kybint_msk_r )
{
	uint8_t data = 0;

	// reading this port clears the keyboard interrupt
	m_keyboard_daisy->int_w(CLEAR_LINE);

	/* bit 0 and 1: fire buttons on the joysticks */
	data |= m_buttons->read() & 0x03;

	/* bit 2 to 4: printer status */
	data |= m_centronics_busy << 2;
	data |= m_centronics_perror << 3;
	data |= m_centronics_fault << 4;

	/* bit 5 to 7: graph, control and shift key */
	data |= m_extra->read();

	if(VERBOSE_KEYBOARD)
		logerror("%s: kybint_msk_r %02x\n", machine().describe_context(), data);

	return data;
}

WRITE8_MEMBER( einstein_state::kybint_msk_w )
{
	logerror("KEY interrupt %s\n", BIT(data, 0) ? "disabled" : "enabled");
	m_keyboard_daisy->mask_w(BIT(data, 0));
}

WRITE8_MEMBER( einstein_state::adcint_msk_w )
{
	logerror("ADC interrupt %s\n", BIT(data, 0) ? "disabled" : "enabled");
	m_adc_daisy->mask_w(BIT(data, 0));
}

WRITE8_MEMBER( einstein_state::fireint_msk_w )
{
	logerror("FIRE interrupt %s\n", BIT(data, 0) ? "disabled" : "enabled");
	m_fire_daisy->mask_w(BIT(data, 0));
}


/***************************************************************************
    MACHINE EMULATION
***************************************************************************/

READ8_MEMBER( einstein_state::rom_r )
{
	m_rom_enabled ^= 1;
	m_bank1->set_entry(m_rom_enabled);

	return 0xff;
}

WRITE8_MEMBER( einstein_state::rom_w )
{
	m_rom_enabled ^= 1;
	m_bank1->set_entry(m_rom_enabled);
}

READ8_MEMBER( einstein_state::reset_r )
{
	m_psg->reset();
	m_fdc->reset();

	return 0xff;
}

WRITE8_MEMBER( einstein_state::reset_w )
{
	m_psg->reset();
	m_fdc->reset();
}

void einstein_state::machine_start()
{
	// initialize memory mapping
	m_bank1->configure_entry(0, m_ram->pointer());
	m_bank1->configure_entry(1, m_bios->base());
	m_bank2->set_base(m_ram->pointer());
	m_bank3->set_base(m_ram->pointer() + 0x8000);

	// setup expansion slot
	m_pipe->set_program_space(&m_maincpu->space(AS_PROGRAM));
	m_pipe->set_io_space(&m_maincpu->space(AS_IO));
}

void einstein_state::machine_reset()
{
	// rom enabled on reset
	m_rom_enabled = 1;
	m_bank1->set_entry(m_rom_enabled);

	// interrupt mask enabled
	m_keyboard_daisy->mask_w(1);
	m_adc_daisy->mask_w(1);
	m_fire_daisy->mask_w(1);

	m_strobe = -1;
}


/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

void einstein_state::einstein_mem(address_map &map)
{
	map(0x0000, 0x07fff).bankr("bank1").bankw("bank2");
	map(0x8000, 0x0ffff).bankrw("bank3");
}

// I/O ports are decoded into 8 blocks using address lines A3 to A7
void einstein_state::einstein_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x00).mirror(0xff04).rw(this, FUNC(einstein_state::reset_r), FUNC(einstein_state::reset_w));
	map(0x02, 0x02).mirror(0xff04).rw(m_psg, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x03, 0x03).mirror(0xff04).w(m_psg, FUNC(ay8910_device::data_w));
	map(0x08, 0x08).mirror(0xff06).rw("vdp", FUNC(tms9129_device::vram_read), FUNC(tms9129_device::vram_write));
	map(0x09, 0x09).mirror(0xff06).rw("vdp", FUNC(tms9129_device::register_read), FUNC(tms9129_device::register_write));
	map(0x10, 0x10).mirror(0xff06).rw(IC_I060, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x11, 0x11).mirror(0xff06).rw(IC_I060, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x18, 0x1b).mirror(0xff04).rw(m_fdc, FUNC(wd1770_device::read), FUNC(wd1770_device::write));
	map(0x20, 0x20).mirror(0xff00).rw(this, FUNC(einstein_state::kybint_msk_r), FUNC(einstein_state::kybint_msk_w));
	map(0x21, 0x21).mirror(0xff00).w(this, FUNC(einstein_state::adcint_msk_w));
	map(0x23, 0x23).mirror(0xff00).w(this, FUNC(einstein_state::drsel_w));
	map(0x24, 0x24).mirror(0xff00).rw(this, FUNC(einstein_state::rom_r), FUNC(einstein_state::rom_w));
	map(0x25, 0x25).mirror(0xff00).w(this, FUNC(einstein_state::fireint_msk_w));
	map(0x28, 0x2b).mirror(0xff04).rw(IC_I058, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x30, 0x33).mirror(0xff04).rw(IC_I063, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x38, 0x38).mirror(0xff07).rw("adc", FUNC(adc0844_device::read), FUNC(adc0844_device::write));
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( einstein )
	PORT_START("LINE0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("?") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CAPS LOCK") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("LINE1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR(0xA3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0xBA) PORT_CHAR(0xBD)    // is \xBA correct for double vertical bar || ?
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('@')

	PORT_START("LINE2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F5") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F5))

	PORT_START("LINE3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('-')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("UP") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F4") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F4))

	PORT_START("LINE4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F3") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("LINE5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F2") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F2))

	PORT_START("LINE6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F1") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F6") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("EXTRA")
	PORT_BIT(0x1f, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("GRPH")    PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CONTROL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT")   PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)

	// fire buttons for analogue joysticks
	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 1 Button 1") PORT_PLAYER(1) PORT_CHANGED_MEMBER(DEVICE_SELF, einstein_state, joystick_button, nullptr)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Joystick 2 Button 1") PORT_PLAYER(2) PORT_CHANGED_MEMBER(DEVICE_SELF, einstein_state, joystick_button, nullptr)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("analogue_1_x")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_CENTERDELTA(100) PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_4_PAD)         PORT_CODE_INC(KEYCODE_6_PAD)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("analogue_1_y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_CENTERDELTA(100) PORT_MINMAX(0,0xff) PORT_PLAYER(1)
	PORT_CODE_DEC(KEYCODE_8_PAD)         PORT_CODE_INC(KEYCODE_2_PAD)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)
	PORT_REVERSE

	PORT_START("analogue_2_x")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_CENTERDELTA(100) PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_X_LEFT_SWITCH) PORT_CODE_INC(JOYCODE_X_RIGHT_SWITCH)

	PORT_START("analogue_2_y")
	PORT_BIT(0xff, 0x80, IPT_AD_STICK_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(100) PORT_CENTERDELTA(100) PORT_MINMAX(0,0xff) PORT_PLAYER(2)
	PORT_CODE_DEC(JOYCODE_Y_UP_SWITCH)   PORT_CODE_INC(JOYCODE_Y_DOWN_SWITCH)
	PORT_REVERSE
INPUT_PORTS_END


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

static void einstein_floppies(device_slot_interface &device)
{
	device.option_add("3ss", TEAC_FD_30A);
	device.option_add("3ds", FLOPPY_3_DSDD);
	device.option_add("525ssqd", FLOPPY_525_SSQD);
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("35ssdd", FLOPPY_35_SSDD);
	device.option_add("35dd", FLOPPY_35_DD);
}

MACHINE_CONFIG_START(einstein_state::einstein)
	/* basic machine hardware */
	MCFG_DEVICE_ADD(IC_I001, Z80, XTAL_X002 / 2)
	MCFG_DEVICE_PROGRAM_MAP(einstein_mem)
	MCFG_DEVICE_IO_MAP(einstein_io)
	MCFG_Z80_DAISY_CHAIN(einstein_daisy_chain)

	/* this is actually clocked at the system clock 4 MHz, but this would be too fast for our
	driver. So we update at 50Hz and hope this is good enough. */
	MCFG_TIMER_DRIVER_ADD_PERIODIC("keyboard", einstein_state, keyboard_timer_callback, attotime::from_hz(50))

	MCFG_DEVICE_ADD(IC_I063, Z80PIO, XTAL_X002 / 2)
	MCFG_Z80PIO_OUT_INT_CB(WRITELINE(*this, einstein_state, int_w<0>))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8("cent_data_out", output_latch_device, write))
	MCFG_Z80PIO_OUT_ARDY_CB(WRITELINE(*this, einstein_state, ardy_w))
	MCFG_Z80PIO_IN_PB_CB(READ8("user", einstein_userport_device, read))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8("user", einstein_userport_device, write))
	MCFG_Z80PIO_OUT_BRDY_CB(WRITELINE("user", einstein_userport_device, brdy_w))

	MCFG_DEVICE_ADD(IC_I058, Z80CTC, XTAL_X002 / 2)
	MCFG_Z80CTC_INTR_CB(WRITELINE(*this, einstein_state, int_w<1>))
	MCFG_Z80CTC_ZC0_CB(WRITELINE(IC_I060, i8251_device, write_txc))
	MCFG_Z80CTC_ZC1_CB(WRITELINE(IC_I060, i8251_device, write_rxc))
	MCFG_Z80CTC_ZC2_CB(WRITELINE(IC_I058, z80ctc_device, trg3))

	MCFG_CLOCK_ADD("ctc_trigger", XTAL_X002 / 4)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(IC_I058, z80ctc_device, trg0))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE(IC_I058, z80ctc_device, trg1))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE(IC_I058, z80ctc_device, trg2))

	/* Einstein daisy chain support for non-Z80 devices */
	MCFG_Z80DAISY_GENERIC_ADD("keyboard_daisy", 0xf7)
	MCFG_Z80DAISY_GENERIC_INT_CB(WRITELINE(*this, einstein_state, int_w<2>))
	MCFG_Z80DAISY_GENERIC_ADD("adc_daisy", 0xfb)
	MCFG_Z80DAISY_GENERIC_INT_CB(WRITELINE(*this, einstein_state, int_w<3>))
	MCFG_Z80DAISY_GENERIC_ADD("fire_daisy", 0xfd)
	MCFG_Z80DAISY_GENERIC_INT_CB(WRITELINE(*this, einstein_state, int_w<4>))

	/* video hardware */
	MCFG_DEVICE_ADD("vdp", TMS9129, XTAL(10'738'635) / 2)
	MCFG_TMS9928A_VRAM_SIZE(0x4000) // 16k RAM, provided by IC i040 and i041
	MCFG_TMS9928A_SET_SCREEN("screen")
	MCFG_TMS9928A_SCREEN_ADD_PAL("screen")
	MCFG_SCREEN_UPDATE_DEVICE("vdp", tms9129_device, screen_update)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD(IC_I030, AY8910, XTAL_X002 / 4)
	MCFG_AY8910_PORT_B_READ_CB(READ8(*this, einstein_state, keyboard_data_read))
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(*this, einstein_state, keyboard_line_write))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.20)

	MCFG_ADC0844_ADD("adc")
	MCFG_ADC0844_INTR_CB(WRITELINE("adc_daisy", z80daisy_generic_device, int_w))
	MCFG_ADC0844_CH1_CB(IOPORT("analogue_1_x"))
	MCFG_ADC0844_CH2_CB(IOPORT("analogue_1_y"))
	MCFG_ADC0844_CH3_CB(IOPORT("analogue_2_x"))
	MCFG_ADC0844_CH4_CB(IOPORT("analogue_2_y"))

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE(IC_I063, z80pio_device, strobe_a))
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(*this, einstein_state, write_centronics_busy))
	MCFG_CENTRONICS_PERROR_HANDLER(WRITELINE(*this, einstein_state, write_centronics_perror))
	MCFG_CENTRONICS_FAULT_HANDLER(WRITELINE(*this, einstein_state, write_centronics_fault))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")

	MCFG_TIMER_DRIVER_ADD("strobe", einstein_state, strobe_callback)

	// uart
	MCFG_DEVICE_ADD(IC_I060, I8251, XTAL_X002 / 4)
	MCFG_I8251_TXD_HANDLER(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_RTS_HANDLER(WRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_I8251_DTR_HANDLER(WRITELINE("rs232", rs232_port_device, write_dtr))

	// rs232 port
	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE(IC_I060, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(WRITELINE(IC_I060, i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(WRITELINE(IC_I060, i8251_device, write_cts))

	// floppy
	MCFG_WD1770_ADD(IC_I042, XTAL_X002)

	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":0", einstein_floppies, "3ss", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":1", einstein_floppies, "3ss", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":2", einstein_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(IC_I042 ":3", einstein_floppies, "525qd", floppy_image_device::default_floppy_formats)

	/* software lists */
	MCFG_SOFTWARE_LIST_ADD("disk_list","einstein")

	/* RAM is provided by 8k DRAM ICs i009, i010, i011, i012, i013, i014, i015 and i016 */
	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")

	// tatung pipe connector
	MCFG_TATUNG_PIPE_ADD("pipe")
	MCFG_TATUNG_PIPE_NMI_HANDLER(INPUTLINE(IC_I001, INPUT_LINE_NMI))

	// user port
	MCFG_EINSTEIN_USERPORT_ADD("user")
	MCFG_EINSTEIN_USERPORT_BSTB_HANDLER(WRITELINE(IC_I063, z80pio_device, strobe_b))
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

/* There are two sockets, i023 and i024, each either a 2764 or 27128
 * only i023 is used by default and fitted with the 8k bios (called MOS).
 *
 * We are missing dumps of version MOS 1.1, possibly of 1.0 if it exists.
 */
ROM_START( einstein )
	ROM_REGION(0x8000, "bios", 0)
	/* i023 */
	ROM_SYSTEM_BIOS(0,  "mos12",  "MOS 1.2")
	ROMX_LOAD("mos12.i023", 0, 0x2000, CRC(ec134953) SHA1(a02125d8ebcda48aa784adbb42a8b2d7ef3a4b77), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1,  "mos121",  "MOS 1.21")
	ROMX_LOAD("mos121.i023", 0, 0x2000, CRC(a746eeb6) SHA1(f75aaaa777d0fd92225acba291f6bf428b341d3e), ROM_BIOS(2))
	ROM_RELOAD(0x2000, 0x2000)
	/* i024 */
	ROM_FILL(0x4000, 0x4000, 0xff)
ROM_END

ROM_START( einst256 )
	ROM_REGION(0x8000, "bios", 0)
	ROM_LOAD("tc256.rom", 0x0000, 0x4000, BAD_DUMP CRC(ef8dad88) SHA1(eb2102d3bef572db7161c26a7c68a5fcf457b4d0) )
ROM_END


/***************************************************************************
    GAME DRIVERS
***************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY   FULLNAME          FLAGS
COMP( 1984, einstein, 0,      0,      einstein, einstein, einstein_state, empty_init, "Tatung", "Einstein TC-01", 0 )
COMP( 1986, einst256, 0,      0,      einstein, einstein, einstein_state, empty_init, "Tatung", "Einstein 256",   MACHINE_NOT_WORKING )
