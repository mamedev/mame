// license: GPL-2.0+
// copyright-holders: Dirk Best
// thanks-to: Tomas Karlsson, Sean Young
/***************************************************************************

    Spectravideo SVI-318/328

***************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "machine/bankdev.h"
#include "machine/i8255.h"
#include "machine/ram.h"
#include "sound/ay8910.h"
#include "sound/spkrdev.h"
#include "video/tms9928a.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "bus/svi3x8/expander/expander.h"

#include "softlist_dev.h"
#include "speaker.h"

#include "formats/svi_cas.h"

#include "utf8.h"


namespace {

//**************************************************************************
//  CONSTANTS & MACROS
//**************************************************************************

#define IS_SVI328  (m_ram->size() == 64 * 1024)

#define CCS1       (m_cart == 0 && offset < 0x4000)
#define CCS2       (m_cart == 0 && offset >= 0x4000 && offset < 0x8000)
#define CCS3       (m_cart == 0 && m_rom2 == 0 && offset >= 0x8000 && offset < 0xc000)
#define CCS4       (m_cart == 0 && m_rom3 == 0 && offset >= 0xc000)
#define ROMCS      (m_romdis == 1 && offset < 0x8000)
#define RAMCS      (m_ramdis == 1 && offset >= 0x8000)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class svi3x8_state : public driver_device
{
public:
	svi3x8_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_io(*this, "io"),
		m_basic(*this, "basic"),
		m_vdp(*this, "vdp"),
		m_speaker(*this, "speaker"),
		m_cassette(*this, "cassette"),
		m_cart_rom(*this, "cartslot"),
		m_expander(*this, "exp"),
		m_keyboard(*this, "KEY.%u", 0),
		m_buttons(*this, "BUTTONS"),
		m_led_caps_lock(*this, "led_caps_lock"),
		m_intvdp(0), m_intexp(0),
		m_romdis(1), m_ramdis(1),
		m_cart(1), m_bk21(1),
		m_rom2(1), m_rom3(1),
		m_ctrl1(-1),
		m_keyboard_row(0)
	{}

	void svi318(machine_config &config);
	void svi328n(machine_config &config);
	void svi318p(machine_config &config);
	void svi318n(machine_config &config);
	void svi328p(machine_config &config);

private:
	uint8_t ppi_port_a_r();
	uint8_t ppi_port_b_r();
	void ppi_port_c_w(uint8_t data);
	void bank_w(uint8_t data);
	void intvdp_w(int state);

	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);

	// from expander bus
	void intexp_w(int state);
	void romdis_w(int state);
	void ramdis_w(int state);
	void ctrl1_w(int state);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);

	void svi3x8_io(address_map &map) ATTR_COLD;
	void svi3x8_io_bank(address_map &map) ATTR_COLD;
	void svi3x8_mem(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<address_map_bank_device> m_io;
	required_memory_region m_basic;
	required_device<tms9928a_device> m_vdp;
	required_device<speaker_sound_device> m_speaker;
	required_device<cassette_image_device> m_cassette;
	required_device<generic_slot_device> m_cart_rom;
	required_device<svi_expander_device> m_expander;
	required_ioport_array<16> m_keyboard;
	required_ioport m_buttons;
	output_finder<> m_led_caps_lock;

	int m_intvdp;
	int m_intexp;
	int m_romdis;
	int m_ramdis;
	int m_cart;
	int m_bk21;
	int m_rom2;
	int m_rom3;
	int m_ctrl1;

	uint8_t m_keyboard_row;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void svi3x8_state::svi3x8_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).rw(FUNC(svi3x8_state::mreq_r), FUNC(svi3x8_state::mreq_w));
}

void svi3x8_state::svi3x8_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0xff).m(m_io, FUNC(address_map_bank_device::amap8));
}

void svi3x8_state::svi3x8_io_bank(address_map &map)
{
	map(0x000, 0x0ff).rw(m_expander, FUNC(svi_expander_device::iorq_r), FUNC(svi_expander_device::iorq_w));
	map(0x100, 0x17f).rw(m_expander, FUNC(svi_expander_device::iorq_r), FUNC(svi_expander_device::iorq_w));
	map(0x180, 0x181).mirror(0x22).w(m_vdp, FUNC(tms9928a_device::write));
	map(0x184, 0x185).mirror(0x22).r(m_vdp, FUNC(tms9928a_device::read));
	map(0x188, 0x188).mirror(0x23).w("psg", FUNC(ay8910_device::address_w));
	map(0x18c, 0x18c).mirror(0x23).w("psg", FUNC(ay8910_device::data_w));
	map(0x190, 0x190).mirror(0x23).r("psg", FUNC(ay8910_device::data_r));
	map(0x194, 0x197).w("ppi", FUNC(i8255_device::write));
	map(0x198, 0x19a).r("ppi", FUNC(i8255_device::read));
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( svi318 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('&')

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(':') PORT_CHAR(';')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')

	PORT_START("KEY.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR('\\') PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)   PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)          PORT_CHAR(UCHAR_MAMEKEY(UP))   PORT_NAME(UTF8_UP)

	PORT_START("KEY.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)      PORT_CODE(KEYCODE_RSHIFT)      PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)    PORT_CHAR(UCHAR_SHIFT_2)       PORT_NAME("Ctrl")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)        PORT_CHAR(UCHAR_MAMEKEY(PGUP)) PORT_NAME("Left Grph")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RALT)        PORT_CHAR(UCHAR_MAMEKEY(PGDN)) PORT_NAME("Right Grph")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)         PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)         PORT_CHAR(UCHAR_MAMEKEY(END))  PORT_NAME("Stop")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)       PORT_CHAR(13)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)        PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(UTF8_LEFT)

	PORT_START("KEY.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)          PORT_CHAR(UCHAR_MAMEKEY(F1))     PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)          PORT_CHAR(UCHAR_MAMEKEY(F2))     PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)          PORT_CHAR(UCHAR_MAMEKEY(F3))     PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)          PORT_CHAR(UCHAR_MAMEKEY(F4))     PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)          PORT_CHAR(UCHAR_MAMEKEY(F5))     PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)        PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_NAME("CLS/HM  Copy")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)      PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("Ins  Paste")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)        PORT_CHAR(UCHAR_MAMEKEY(DOWN))   PORT_NAME(UTF8_DOWN)

	PORT_START("KEY.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)       PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)         PORT_CHAR('\t')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)         PORT_CHAR(UCHAR_MAMEKEY(DEL))      PORT_NAME("Del  Cut")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)    PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_NAME("Caps Lock")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)       PORT_CHAR(UCHAR_MAMEKEY(F11))      PORT_NAME("Select")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)      PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))   PORT_NAME("Print")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)       PORT_CHAR(UCHAR_MAMEKEY(RIGHT))    PORT_NAME(UTF8_RIGHT)

	PORT_START("KEY.9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.12")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.13")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.14")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY.15")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("JOY")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP)    PORT_PLAYER(2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN)  PORT_PLAYER(2)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT)  PORT_PLAYER(2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)

	PORT_START("BUTTONS")
	PORT_BIT(0x0f, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0xc0, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

static INPUT_PORTS_START( svi328 )
	PORT_INCLUDE(svi318)

	PORT_MODIFY("KEY.9")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_MODIFY("KEY.10")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(COMMA_PAD))
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void svi3x8_state::intvdp_w(int state)
{
	m_intvdp = state;

	if (m_ctrl1 == 0)
		m_maincpu->set_input_line(INPUT_LINE_NMI, m_intvdp ? ASSERT_LINE : CLEAR_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_intvdp || m_intexp) ? ASSERT_LINE : CLEAR_LINE);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void svi3x8_state::machine_start()
{
	m_led_caps_lock.resolve();

	// register for save states
	save_item(NAME(m_intvdp));
	save_item(NAME(m_intexp));
	save_item(NAME(m_romdis));
	save_item(NAME(m_ramdis));
	save_item(NAME(m_cart));
	save_item(NAME(m_bk21));
	save_item(NAME(m_rom2));
	save_item(NAME(m_rom3));
	save_item(NAME(m_ctrl1));
	save_item(NAME(m_keyboard_row));
}

void svi3x8_state::machine_reset()
{
	m_intvdp = 0;
	m_intexp = 0;
	m_romdis = 1;
	m_ramdis = 1;
	m_cart = 1;
	m_bk21 = 1;
	m_rom2 = 1;
	m_rom3 = 1;
	m_keyboard_row = 0;

	if (m_ctrl1 == -1)
		ctrl1_w(1);
}

uint8_t svi3x8_state::mreq_r(offs_t offset)
{
	// ctrl1 inverts a15
	if (m_ctrl1 == 0)
		offset ^= 0x8000;

	if (CCS1 || CCS2 || CCS3 || CCS4)
		return m_cart_rom->read_rom(offset);

	uint8_t data = m_expander->mreq_r(offset);

	if (ROMCS)
		data = m_basic->as_u8(offset);

	if (m_bk21 == 0 && IS_SVI328 && offset < 0x8000)
		data = m_ram->read(offset);

	if (RAMCS && (IS_SVI328 || offset >= 0xc000))
		data = m_ram->read(IS_SVI328 ? offset : offset - 0xc000);

	return data;
}

void svi3x8_state::mreq_w(offs_t offset, uint8_t data)
{
	// ctrl1 inverts a15
	if (m_ctrl1 == 0)
		offset ^= 0x8000;

	if (CCS1 || CCS2 || CCS3 || CCS4)
		return;

	m_expander->mreq_w(offset, data);

	if (m_bk21 == 0 && IS_SVI328 && offset < 0x8000)
		m_ram->write(offset, data);

	if (RAMCS && (IS_SVI328 || offset >= 0xc000))
		m_ram->write(IS_SVI328 ? offset : offset - 0xc000, data);
}

void svi3x8_state::bank_w(uint8_t data)
{
	logerror("bank_w: %02x\n", data);

	m_cart = BIT(data, 0);
	m_bk21 = BIT(data, 1);

	m_expander->bk21_w(BIT(data, 1));
	m_expander->bk22_w(BIT(data, 2));
	m_expander->bk31_w(BIT(data, 3));
	m_expander->bk32_w(BIT(data, 4));

	m_rom2 = BIT(data, 6);
	m_rom3 = BIT(data, 7);

	m_led_caps_lock = BIT(data, 5);
}

uint8_t svi3x8_state::ppi_port_a_r()
{
	uint8_t data = 0x3f;

	// bit 0-3, paddle or tablet input

	// bit 4-5, joystick buttons
	data &= m_buttons->read();

	// bit 6-7, cassette
	data |= m_cassette->exists() ? 0x00 : 0x40;
	data |= (m_cassette->input() > 0.0038) ? 0x80 : 0x00;

	return data;
}

uint8_t svi3x8_state::ppi_port_b_r()
{
	// bit 0-7, keyboard data
	return m_keyboard[m_keyboard_row]->read();
}

void svi3x8_state::ppi_port_c_w(uint8_t data)
{
	// bit 0-3, keyboard row
	m_keyboard_row = data & 0x0f;

	// bit 4-6, cassette
	m_cassette->change_state(BIT(data, 4) ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
	m_cassette->output(BIT(data, 5) ? -1.0 : +1.0);
	//m_cassette->change_state(BIT(data, 6) ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER);

	// bit 7, mix psg sound (keyboard click)
	m_speaker->level_w(BIT(data, 7));
}

void svi3x8_state::intexp_w(int state)
{
	m_intexp = state;

	if (m_ctrl1 == 0)
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_intexp ? ASSERT_LINE : CLEAR_LINE);
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ0, (m_intvdp || m_intexp) ? ASSERT_LINE : CLEAR_LINE);
}

void svi3x8_state::romdis_w(int state)
{
	m_romdis = state;
}

void svi3x8_state::ramdis_w(int state)
{
	m_ramdis = state;
}

void svi3x8_state::ctrl1_w(int state)
{
	m_ctrl1 = state;

	// ctrl1 disables internal io address decoding
	m_io->set_bank(m_ctrl1);
}


//**************************************************************************
//  CARTRIDGE
//**************************************************************************

DEVICE_IMAGE_LOAD_MEMBER( svi3x8_state::cart_load )
{
	uint32_t const size = m_cart_rom->common_get_size("rom");

	m_cart_rom->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart_rom->common_load_rom(m_cart_rom->get_rom_base(), size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void svi3x8_state::svi318(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(10'738'635) / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &svi3x8_state::svi3x8_mem);
	m_maincpu->set_addrmap(AS_IO, &svi3x8_state::svi3x8_io);

	RAM(config, m_ram).set_default_size("16K");

	ADDRESS_MAP_BANK(config, "io").set_map(&svi3x8_state::svi3x8_io_bank).set_data_width(8).set_addr_width(9).set_stride(0x100);

	i8255_device &ppi(I8255(config, "ppi"));
	ppi.in_pa_callback().set(FUNC(svi3x8_state::ppi_port_a_r));
	ppi.in_pb_callback().set(FUNC(svi3x8_state::ppi_port_b_r));
	ppi.out_pc_callback().set(FUNC(svi3x8_state::ppi_port_c_w));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.50);
	ay8910_device &psg(AY8910(config, "psg", XTAL(10'738'635) / 6));
	psg.port_a_read_callback().set_ioport("JOY");
	psg.port_b_write_callback().set(FUNC(svi3x8_state::bank_w));
	psg.add_route(ALL_OUTPUTS, "mono", 0.75);

	// cassette
	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);
	m_cassette->set_formats(svi_cassette_formats);
	m_cassette->set_interface("svi318_cass");
	SOFTWARE_LIST(config, "cass_list").set_original("svi318_cass");

	// cartridge slot
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "svi318_cart", "bin,rom").set_device_load(FUNC(svi3x8_state::cart_load));
	SOFTWARE_LIST(config, "cart_list").set_original("svi318_cart");

	// expander bus
	SVI_EXPANDER(config, m_expander, svi_expander_modules);
	m_expander->int_handler().set(FUNC(svi3x8_state::intexp_w));
	m_expander->romdis_handler().set(FUNC(svi3x8_state::romdis_w));
	m_expander->ramdis_handler().set(FUNC(svi3x8_state::ramdis_w));
	m_expander->ctrl1_handler().set(FUNC(svi3x8_state::ctrl1_w));
	m_expander->excsr_handler().set(m_vdp, FUNC(tms9928a_device::read));
	m_expander->excsw_handler().set(m_vdp, FUNC(tms9928a_device::write));
	m_expander->add_route(ALL_OUTPUTS, "mono", 1.00);
}

void svi3x8_state::svi318p(machine_config &config)
{
	svi318(config);

	// video hardware
	TMS9929A(config, m_vdp, XTAL(10'738'635)).set_screen("screen");
	m_vdp->set_vram_size(0x4000);
	m_vdp->int_callback().set(FUNC(svi3x8_state::intvdp_w));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

void svi3x8_state::svi318n(machine_config &config)
{
	svi318(config);
	TMS9928A(config, m_vdp, XTAL(10'738'635)).set_screen("screen");
	m_vdp->set_vram_size(0x4000);
	m_vdp->int_callback().set(FUNC(svi3x8_state::intvdp_w));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);
}

void svi3x8_state::svi328p(machine_config &config)
{
	svi318p(config);
	m_ram->set_default_size("64K");
}

void svi3x8_state::svi328n(machine_config &config)
{
	svi318n(config);
	m_ram->set_default_size("64K");
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

// note: should be split into two 16k chips, document differences
ROM_START( svi318 )
	ROM_REGION(0x8000, "basic", 0)
	ROM_SYSTEM_BIOS(0, "v111", "SV BASIC v1.11")
	ROMX_LOAD("svi111.rom", 0x0000, 0x8000, CRC(bc433df6) SHA1(10349ce675f6d6d47f0976e39cb7188eba858d89), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v11", "SV BASIC v1.1")
	ROMX_LOAD("svi110.rom", 0x0000, 0x8000, CRC(709904e9) SHA1(7d8daf52f78371ca2c9443e04827c8e1f98c8f2c), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v10", "SV BASIC v1.0")
	ROMX_LOAD("svi100.rom", 0x0000, 0x8000, CRC(98d48655) SHA1(07758272df475e5e06187aa3574241df1b14035b), ROM_BIOS(2))
ROM_END

#define rom_svi318n rom_svi318
#define rom_svi328  rom_svi318
#define rom_svi328n rom_svi318

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY         FULLNAME          FLAGS
COMP( 1983, svi318,  0,      0,      svi318p, svi318, svi3x8_state, empty_init, "Spectravideo", "SVI-318 (PAL)",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, svi318n, svi318, 0,      svi318n, svi318, svi3x8_state, empty_init, "Spectravideo", "SVI-318 (NTSC)", MACHINE_SUPPORTS_SAVE )
COMP( 1983, svi328,  0,      0,      svi328p, svi328, svi3x8_state, empty_init, "Spectravideo", "SVI-328 (PAL)",  MACHINE_SUPPORTS_SAVE )
COMP( 1983, svi328n, svi328, 0,      svi328n, svi328, svi3x8_state, empty_init, "Spectravideo", "SVI-328 (NTSC)", MACHINE_SUPPORTS_SAVE )
