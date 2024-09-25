// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Acorn System 2, 3, 4, and 5

    Acorn System 2: 6502, VDU 40x25, 4K RAM, 4K ROM (BASIC), Cassette
    http://chrisacorns.computinghistory.org.uk/Computers/System2.html

    Acorn System 3: 6502, VDU 40x25, 16K RAM, 4K ROM (BASIC), FDC (1 disc)
    http://chrisacorns.computinghistory.org.uk/Computers/System3.html

    Acorn System 3: 6809, VDU 40x25, 16K RAM, FDC (1 disc)
    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_6809_CPU.html

    Acorn System 4: 6502, VDU 40x25, 16K RAM, FDC (2 discs)
    http://chrisacorns.computinghistory.org.uk/Computers/System4.html

    Acorn System 5: 6502A, VDU 80x25, 32K RAM, FDC
    http://chrisacorns.computinghistory.org.uk/Computers/System5.html

  TODO:
    - AY-3-4572 Keyboard Encoder for full keyboard support
    - 4K BASIC ROM is undumped for System 2/3


  6809 Monitor Commands:-
    Modify memory:
    M  - Modify starting at specified address
    MR - Modify registers
    MG - Modify from Go address
    MV - Modify from breakpoint address
    MP - Modify from saved Program counter

    Execute programs:
    G - Go to specified address
    P - Proceed from saved Program counter past specified number of breakpoints

    Debugging aids:
    R - Display registers
    V - Insert/delete breakpoint
    T - Trace one, or more, instructions
    . - Do trace, displaying register contents at each step

    Cassette interface:
    S - Save memory to named file
    L - Load named file, with optional offset
    F - Finish loading - no name search

    Printer interface:
    C - Copy to parallel printer

    Disk interface:
    D - Disk bootstrap

****************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6809/m6809.h"
#include "machine/ins8154.h"
#include "machine/6522via.h"
#include "machine/keyboard.h"
#include "machine/input_merger.h"
#include "bus/centronics/ctronics.h"
#include "bus/acorn/bus.h"
#include "softlist_dev.h"
#include "utf8.h"

namespace {

class acrnsys_state : public driver_device
{
public:
	acrnsys_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_irqs(*this, "irqs")
		, m_bus(*this, "bus")
		, m_ins8154(*this, "ins8154")
		, m_via6522(*this, "via6522")
		, m_centronics(*this, "centronics")
		, m_kbd_data(0)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

	void a6502(machine_config &config);
	void a6809(machine_config &config);
	void a6502a(machine_config &config);

	void acrnsys2(machine_config &config);
	void acrnsys3(machine_config &config);
	void acrnsys3_6809(machine_config &config);
	void acrnsys4(machine_config &config);
	void acrnsys5(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void kbd_put(u8 data);
	void kbd_put_pb(u8 data);

	uint8_t kbd_r();
	void bus_nmi_w(int state);

	void a6502_mem(address_map &map) ATTR_COLD;
	void a6809_mem(address_map &map) ATTR_COLD;
	void a6502a_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<input_merger_device> m_irqs;
	required_device<acorn_bus_device> m_bus;
	optional_device<ins8154_device> m_ins8154;
	optional_device<via6522_device> m_via6522;
	optional_device<centronics_device> m_centronics;

	uint8_t m_kbd_data = 0U;
};


void acrnsys_state::a6502_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x03ff).ram();
	map(0x0e00, 0x0e7f).mirror(0x100).rw(m_ins8154, FUNC(ins8154_device::read_io), FUNC(ins8154_device::write_io));
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void acrnsys_state::a6809_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x03ff).ram();
	map(0x0980, 0x098f).mirror(0x70).m(m_via6522, FUNC(via6522_device::map));
	map(0xf000, 0xffff).rom().region("maincpu", 0);
}

void acrnsys_state::a6502a_mem(address_map &map)
{
	map.unmap_value_low();
	map(0x0000, 0x07ff).ram();
	map(0x0e00, 0x0e0f).mirror(0x1f0).m(m_via6522, FUNC(via6522_device::map));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


void acrnsys_state::machine_start()
{
	save_item(NAME(m_kbd_data));
}

void acrnsys_state::machine_reset()
{
	m_kbd_data = 0x7f;
}


void acrnsys_state::bus_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state);
}


INPUT_CHANGED_MEMBER(acrnsys_state::trigger_reset)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);

	if (newval)
	{
		machine().schedule_soft_reset();
	}
}

/***************************************************************************
    Acorn Keyboard - Part No. 400,013
***************************************************************************/

static INPUT_PORTS_START( acrnsys )
//PORT_START("X0")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L SHIFT")      PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R SHIFT")      PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SHIFT LOCK")   PORT_CODE(KEYCODE_LALT)
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL")         PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
//
//PORT_START("X1")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("ESC")          PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(UCHAR_MAMEKEY(ESC))
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('\"')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
//
//PORT_START("X2")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
//
//PORT_START("X3")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
//
//PORT_START("X4")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SPACE")        PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(32)
//
//PORT_START("X5")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')

//PORT_START("X6")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
//
//PORT_START("X7")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
//
//PORT_START("X8")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
//
//PORT_START("X9")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR(':') PORT_CHAR('*')
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("COPY")         PORT_CODE(KEYCODE_TAB)        PORT_CHAR(UCHAR_MAMEKEY(TAB))
//
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LINE FEED")    PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(10)
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('[')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("RETURN")       PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR(']')
//
//PORT_START("X10")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE")       PORT_CODE(KEYCODE_DEL)        PORT_CHAR(8)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('@')
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR('^')
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('\\')
//
//PORT_START("X11")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
//
//PORT_START("X12")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
//PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                           PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
//
//PORT_START("CAPS")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("LOCK")         PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_TOGGLE
//
//PORT_START("RPT")
//PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("REPT")         PORT_CODE(KEYCODE_RCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))

	PORT_START("BRK")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("BREAK") PORT_CODE(KEYCODE_F12) PORT_CHANGED_MEMBER(DEVICE_SELF, acrnsys_state, trigger_reset, 0)
INPUT_PORTS_END


void acrnsys_state::kbd_put(uint8_t data)
{
	data &= 0x7f;
	/* allow backspace to work */
	if (data == 8) data = 0x7f;
	/* assert strobe */
	m_kbd_data = data | 0x80;
}

uint8_t acrnsys_state::kbd_r()
{
	uint8_t data = m_kbd_data;

	/* clear strobe */
	m_kbd_data &= 0x7f;

	return data;
}

void acrnsys_state::kbd_put_pb(uint8_t data)
{
	data &= 0x7f;
	/* allow backspace to work */
	if (data == 8) data = 0x7f;

	m_via6522->write_pb0(BIT(data, 0));
	m_via6522->write_pb1(BIT(data, 1));
	m_via6522->write_pb2(BIT(data, 2));
	m_via6522->write_pb3(BIT(data, 3));
	m_via6522->write_pb4(BIT(data, 4));
	m_via6522->write_pb5(BIT(data, 5));
	m_via6522->write_pb6(BIT(data, 6));
	/* assert strobe */
	m_via6522->write_cb1(1);
	/* clear strobe */
	m_via6522->write_cb1(0);
}


/***************************************************************************
    6502 CPU Board - Part No. 200,000
***************************************************************************/

void acrnsys_state::a6502(machine_config &config)
{
	M6502(config, m_maincpu, 24_MHz_XTAL / 12); /* 1MHz 6502 CPU */
	m_maincpu->set_addrmap(AS_PROGRAM, &acrnsys_state::a6502_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	INS8154(config, m_ins8154);
	//m_ins8154->in_a().set(FUNC(acrnsys_state::ins8154_pa_r));
	//m_ins8154->out_a().set(FUNC(acrnsys_state::ins8154_pa_w));
	m_ins8154->in_b().set(FUNC(acrnsys_state::kbd_r));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(acrnsys_state::kbd_put));
}


/***************************************************************************
    6502A CPU Board - Part No. 200,005
***************************************************************************/

void acrnsys_state::a6502a(machine_config &config)
{
	M6502(config, m_maincpu, 24_MHz_XTAL / 12); /* 2MHz 6502A CPU */
	m_maincpu->set_addrmap(AS_PROGRAM, &acrnsys_state::a6502a_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	MOS6522(config, m_via6522, 4_MHz_XTAL / 4);
	m_via6522->readpa_handler().set(FUNC(acrnsys_state::kbd_r));
	//m_via6522->cb2_handler().set(FUNC(acrnsys_state::cass_w));
	m_via6522->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(acrnsys_state::kbd_put));
}


/***************************************************************************
    6809 CPU Board - Part No. 200,012
***************************************************************************/

void acrnsys_state::a6809(machine_config &config)
{
	MC6809(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &acrnsys_state::a6809_mem);

	INPUT_MERGER_ANY_HIGH(config, m_irqs).output_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	MOS6522(config, m_via6522, 4_MHz_XTAL / 4);
	m_via6522->writepa_handler().set("cent_data_out", FUNC(output_latch_device::write));
	m_via6522->ca2_handler().set("centronics", FUNC(centronics_device::write_strobe));
	//m_via6522->cb2_handler().set(FUNC(acrnsys_state::cass_w));
	m_via6522->irq_handler().set(m_irqs, FUNC(input_merger_device::in_w<0>));

	CENTRONICS(config, m_centronics, centronics_devices, "printer");
	m_centronics->ack_handler().set(m_via6522, FUNC(via6522_device::write_ca1));
	m_centronics->busy_handler().set(m_via6522, FUNC(via6522_device::write_pa7));
	output_latch_device &latch(OUTPUT_LATCH(config, "cent_data_out"));
	m_centronics->set_output_latch(latch);

	generic_keyboard_device &keyboard(GENERIC_KEYBOARD(config, "keyboard", 0));
	keyboard.set_keyboard_callback(FUNC(acrnsys_state::kbd_put_pb));
}

/***************************************************************************
    DEFAULT CARD SETTINGS
***************************************************************************/

static DEVICE_INPUT_DEFAULTS_START(8k_def_ram0000)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x00)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(8k_def_ram2000)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x01)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(8k_def_ramc000)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x07, 0x06)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(32k_def_ram32k)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x01, 0x00)
DEVICE_INPUT_DEFAULTS_END

static DEVICE_INPUT_DEFAULTS_START(32k_def_ram16k)
	DEVICE_INPUT_DEFAULTS("LINKS", 0x01, 0x01)
DEVICE_INPUT_DEFAULTS_END

/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

void acrnsys_state::acrnsys2(machine_config &config)
{
	/* 6502 CPU Board */
	a6502(config);

	/* Acorn Bus - 8 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->set_space(m_maincpu, AS_PROGRAM);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set(FUNC(acrnsys_state::bus_nmi_w));
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "cass");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


void acrnsys_state::acrnsys3(machine_config &config)
{
	/* 6502 CPU Board */
	a6502(config);

	/* Acorn Bus - 8 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->set_space(m_maincpu, AS_PROGRAM);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set(FUNC(acrnsys_state::bus_nmi_w));
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ramc000)); // 0xc000-0xdfff
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


void acrnsys_state::acrnsys3_6809(machine_config &config)
{
	/* 6809 CPU Board */
	a6809(config);

	/* Acorn Bus - 8 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->set_space(m_maincpu, AS_PROGRAM);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set(FUNC(acrnsys_state::bus_nmi_w));
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ram0000)); // 0x0000-0x1fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ramc000)); // 0xc000-0xdfff
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, "cass");
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
}


void acrnsys_state::acrnsys4(machine_config &config)
{
	/* 6502 CPU Board */
	a6502(config);

	/* Acorn Bus - 14 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->set_space(m_maincpu, AS_PROGRAM);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set(FUNC(acrnsys_state::bus_nmi_w));
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ram2000)); // 0x2000-0x3fff
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "8k").set_option_device_input_defaults("8k", DEVICE_INPUT_DEFAULTS_NAME(8k_def_ramc000)); // 0xc000-0xdfff
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "vdu40");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus7", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus8", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus9", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus10", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus11", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus12", m_bus, acorn_bus_devices, nullptr);
	ACORN_BUS_SLOT(config, "bus13", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


void acrnsys_state::acrnsys5(machine_config &config)
{
	/* 6502A CPU Board */
	a6502a(config);

	/* Acorn Bus - 7 Slot Backplane */
	ACORN_BUS(config, m_bus, 0);
	m_bus->set_space(m_maincpu, AS_PROGRAM);
	m_bus->out_irq_callback().set(m_irqs, FUNC(input_merger_device::in_w<1>));
	m_bus->out_nmi_callback().set(FUNC(acrnsys_state::bus_nmi_w));
	ACORN_BUS_SLOT(config, "bus1", m_bus, acorn_bus_devices, "32k").set_option_device_input_defaults("32k", DEVICE_INPUT_DEFAULTS_NAME(32k_def_ram32k)); // 32K
	ACORN_BUS_SLOT(config, "bus2", m_bus, acorn_bus_devices, "32k").set_option_device_input_defaults("32k", DEVICE_INPUT_DEFAULTS_NAME(32k_def_ram16k)); // 16K
	ACORN_BUS_SLOT(config, "bus3", m_bus, acorn_bus_devices, "vdu80");
	ACORN_BUS_SLOT(config, "bus4", m_bus, acorn_bus_devices, "fdc");
	ACORN_BUS_SLOT(config, "bus5", m_bus, acorn_bus_devices, "econet");
	ACORN_BUS_SLOT(config, "bus6", m_bus, acorn_bus_devices, nullptr);

	/* Software lists */
	SOFTWARE_LIST(config, "flop_list").set_original("acrnsys_flop");
	SOFTWARE_LIST(config, "rom_list").set_original("acrnsys_rom");
}


/***************************************************************************
    ROM definitions
***************************************************************************/

ROM_START( acrnsys2 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("syscos40.ic7", 0x0800, 0x0800, BAD_DUMP CRC(af457ccc) SHA1(c8c1fbfb8d2e8aa0fdf8e3dc80993ad404cc943d)) // Acorn COS (no known dump, this is re-created from source)
ROM_END

ROM_START( acrnsys3 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("tosdos-s3.ic7", 0x0000, 0x1000, CRC(9b1fbec4) SHA1(4cb322dadcfba9c452797d6cc2096f0c92e8792c)) // Acorn DOS
ROM_END

ROM_START( acrnsys3_6809 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("acorn6809.ic4", 0x0800, 0x0800, CRC(5fa5b632) SHA1(b14a884bf82a7a8c23bc03c2e112728dd1a74896))

	ROM_REGION(0x100, "proms", 0)
	ROM_LOAD("acorn6809.ic11", 0x0000, 0x0100, CRC(7908317d) SHA1(e0f1e5bd3a8598d3b62bc432dd1f3892ed7e66d8)) // address decoder
ROM_END

#define rom_acrnsys4 rom_acrnsys3

ROM_START( acrnsys5 )
	/* 6502A CPU board can take 4K, 8K, 16K ROMs */
	ROM_REGION(0x2000, "maincpu", 0)
	/* References suggest models 5A-5E also exist, this is 5F */
	ROM_LOAD("sys5f_iss1.ic11", 0x0000, 0x2000, CRC(cd80418d) SHA1(e588298239b5360b5d1e15d5cd9f7fe2b1693e5d)) // 201,625
ROM_END

} // Anonymous namespace

/*    YEAR  NAME           PARENT    COMPAT  MACHINE        INPUT    CLASS          INIT        COMPANY            FULLNAME                     FLAGS */
COMP( 1980, acrnsys2,      acrnsys3, 0,      acrnsys2,      acrnsys, acrnsys_state, empty_init, "Acorn Computers", "Acorn System 2",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, acrnsys3,      0,        0,      acrnsys3,      acrnsys, acrnsys_state, empty_init, "Acorn Computers", "Acorn System 3 (6502 CPU)", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, acrnsys3_6809, acrnsys3, 0,      acrnsys3_6809, acrnsys, acrnsys_state, empty_init, "Acorn Computers", "Acorn System 3 (6809 CPU)", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1980, acrnsys4,      acrnsys3, 0,      acrnsys4,      acrnsys, acrnsys_state, empty_init, "Acorn Computers", "Acorn System 4",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
COMP( 1982, acrnsys5,      0,        0,      acrnsys5,      acrnsys, acrnsys_state, empty_init, "Acorn Computers", "Acorn System 5",            MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
