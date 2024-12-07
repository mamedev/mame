// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion MC400/MC200 Series

******************************************************************************/

#include "emu.h"

#include "bus/psion/module/slot.h"
#include "cpu/i86/i86.h"
#include "machine/nvram.h"
#include "machine/psion_asic1.h"
#include "machine/psion_asic2.h"
#include "machine/psion_asic3.h"
#include "machine/psion_ssd.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class psionmc_state : public driver_device
{
public:
	psionmc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_asic1(*this, "asic1")
		, m_asic2(*this, "asic2")
		, m_asic3(*this, "asic3")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_palette(*this, "palette")
		, m_keyboard(*this, "COL%u", 0U)
		, m_speaker(*this, "speaker")
		, m_ssd(*this, "ssd%u", 1U)
		, m_exp(*this, "exp%u", 1U)
		, m_penup_timer(nullptr)
	{ }

	void psionmc(machine_config &config);
	void mc200(machine_config &config);
	void mc400(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_on);
	//DECLARE_INPUT_CHANGED_MEMBER(reset);
	DECLARE_INPUT_CHANGED_MEMBER(digitiser_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<psion_asic1_device> m_asic1;
	required_device<psion_asic2_device> m_asic2;
	required_device<psion_asic3_device> m_asic3;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<palette_device> m_palette;
	required_ioport_array<10> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_device_array<psion_ssd_device, 4> m_ssd;
	required_device_array<psion_module_slot_device, 2> m_exp;

	emu_timer *m_penup_timer;

	TIMER_CALLBACK_MEMBER(digitiser_penup);

	void palette_init(palette_device &palette);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void asic1_map(address_map &map) ATTR_COLD;

	int m_dr = 0;
	uint16_t m_digitiser[2] = { 0, 0 };
};


void psionmc_state::machine_start()
{
	m_asic1->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	m_penup_timer = timer_alloc(FUNC(psionmc_state::digitiser_penup), this);
	m_penup_timer->adjust(attotime::never);
}


void psionmc_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(m_asic1, FUNC(psion_asic1_device::mem_r), FUNC(psion_asic1_device::mem_w));
}

void psionmc_state::io_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_asic1, FUNC(psion_asic1_device::io_r), FUNC(psion_asic1_device::io_w));
	map(0x0080, 0x008f).rw(m_asic2, FUNC(psion_asic2_device::io_r), FUNC(psion_asic2_device::io_w)).umask16(0x00ff);
	map(0x0100, 0x01ff).rw(m_exp[0], FUNC(psion_module_slot_device::io_r), FUNC(psion_module_slot_device::io_w));
	map(0x0200, 0x02ff).rw(m_exp[1], FUNC(psion_module_slot_device::io_r), FUNC(psion_module_slot_device::io_w));
}

void psionmc_state::asic1_map(address_map &map)
{
	map(0x00000, 0xb7fff).noprw();
	map(0xb8000, 0xbffff).ram(); // 32K video RAM
	map(0xc0000, 0xfffff).rom().region("flash", 0);
}


static INPUT_PORTS_START( psionmc )
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))  PORT_NAME("Control")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Shift (L)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))      PORT_NAME("Psion")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  PORT_NAME("Caps Lock")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Shift (R)")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))      PORT_NAME(u8"\u2193") // U+2193 = ↓

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa3)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR('<')

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';')  PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR('>')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('?')

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('_')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=')  PORT_CHAR('+')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[')  PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('@')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('#')  PORT_CHAR('~')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                    PORT_NAME("Enter")

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                  PORT_NAME("Backspace")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                  PORT_NAME("Tab")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')                   PORT_NAME("Space")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))  PORT_NAME(u8"\u2192") // U+2192 = →

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT)) PORT_NAME("Task")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))   PORT_NAME("Home")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))   PORT_NAME("Page Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                PORT_NAME("LCD")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1)  PORT_CODE(MOUSECODE_BUTTON1)                                   PORT_NAME("Touchpad Button")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))    PORT_NAME("Delete")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))    PORT_NAME("End")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))   PORT_NAME("Page Down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                PORT_NAME("Record")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ESC")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC)) PORT_NAME("On Esc") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psionmc_state::key_on), 0)

	PORT_START("DIGIT0")
	PORT_BIT(0x7ff, 0x300, IPT_AD_STICK_X) PORT_NAME("Touchpad X-Axis") PORT_MINMAX(0x0c0, 0x7c0) PORT_SENSITIVITY(20) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psionmc_state::digitiser_changed), 0)

	PORT_START("DIGIT1")
	PORT_BIT(0x7ff, 0x300, IPT_AD_STICK_Y) PORT_NAME("Touchpad Y-Axis") PORT_MINMAX(0x100, 0x640) PORT_SENSITIVITY(20) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psionmc_state::digitiser_changed), 1) PORT_REVERSE

	//PORT_START("RESET")
	//PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psionmc_state::reset), 0)
INPUT_PORTS_END

static INPUT_PORTS_START(psionmc_de)
	PORT_INCLUDE(psionmc)

	PORT_MODIFY("COL1")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')  PORT_CHAR('@')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('y')  PORT_CHAR('Y')

	PORT_MODIFY("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa7)

	PORT_MODIFY("COL3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('z')  PORT_CHAR('Z')

	PORT_MODIFY("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('/')  PORT_CHAR('{')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')  PORT_CHAR('[')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',')  PORT_CHAR(';')

	PORT_MODIFY("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')  PORT_CHAR(']')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR('=')  PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0xf6) PORT_CHAR(0xd6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')  PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-')  PORT_CHAR('_')

	PORT_MODIFY("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(0xdf) PORT_CHAR('?')  PORT_CHAR('\\')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('\'') PORT_CHAR('`')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0xfc) PORT_CHAR(0xdc)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+')  PORT_CHAR('*')  PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(0xe4) PORT_CHAR(0xc4)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('<')  PORT_CHAR('>')  PORT_CHAR('|')

	PORT_MODIFY("COL7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('#')  PORT_CHAR('^')
INPUT_PORTS_END

INPUT_CHANGED_MEMBER(psionmc_state::key_on)
{
	if (newval)
	{
		m_asic2->on_clr_w(newval);
	}
}

//INPUT_CHANGED_MEMBER(psionmc_state::reset)
//{
//	if (newval)
//	{
//		m_asic2->reset_w(0);
//	}
//}

INPUT_CHANGED_MEMBER(psionmc_state::digitiser_changed)
{
	m_digitiser[param] = newval;

	m_penup_timer->adjust(attotime::from_msec(200), 0);
}

TIMER_CALLBACK_MEMBER(psionmc_state::digitiser_penup)
{
	m_digitiser[0] |= 0x800;
	m_digitiser[1] |= 0x800;
}


void psionmc_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


void psionmc_state::psionmc(machine_config &config)
{
	I8086(config, m_maincpu, 15.36_MHz_XTAL / 2); // M80C86A
	m_maincpu->set_addrmap(AS_PROGRAM, &psionmc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &psionmc_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_asic1, FUNC(psion_asic1_device::inta_cb));

	PSION_ASIC1(config, m_asic1, 15.36_MHz_XTAL);
	m_asic1->set_screen("screen");
	m_asic1->set_laptop_mode(true);
	m_asic1->set_addrmap(0, &psionmc_state::asic1_map);
	m_asic1->int_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_asic1->nmi_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_asic1->frcovl_cb().set(m_asic2, FUNC(psion_asic2_device::frcovl_w));

	PSION_ASIC2(config, m_asic2, 15.36_MHz_XTAL);
	m_asic2->int_cb().set(m_asic1, FUNC(psion_asic1_device::eint3_w));
	m_asic2->nmi_cb().set(m_asic1, FUNC(psion_asic1_device::enmi_w));
	m_asic2->cbusy_cb().set_inputline(m_maincpu, INPUT_LINE_TEST);
	m_asic2->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_asic2->buzvol_cb().set([this](int state) { m_speaker->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.25); });
	m_asic2->dr_cb().set([this](int state) { m_dr = state; });
	m_asic2->col_cb().set([this](uint8_t data) { return m_keyboard[data]->read(); });
	m_asic2->data_r<0>().set(m_asic3, FUNC(psion_asic3_device::data_r));        // Power supply (ASIC3)
	m_asic2->data_w<0>().set(m_asic3, FUNC(psion_asic3_device::data_w));
	m_asic2->data_r<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_r));         // SSD Pack 1
	m_asic2->data_w<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<2>().set(m_ssd[1], FUNC(psion_ssd_device::data_r));         // SSD Pack 2
	m_asic2->data_w<2>().set(m_ssd[1], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<3>().set(m_ssd[2], FUNC(psion_ssd_device::data_r));         // SSD Pack 3
	m_asic2->data_w<3>().set(m_ssd[2], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<4>().set(m_ssd[3], FUNC(psion_ssd_device::data_r));         // SSD Pack 4
	m_asic2->data_w<4>().set(m_ssd[3], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<5>().set(m_exp[1], FUNC(psion_module_slot_device::data_r)); // Expansion port B
	m_asic2->data_w<5>().set(m_exp[1], FUNC(psion_module_slot_device::data_w));
	m_asic2->data_r<6>().set(m_exp[0], FUNC(psion_module_slot_device::data_r)); // Expansion port A
	m_asic2->data_w<6>().set(m_exp[0], FUNC(psion_module_slot_device::data_w));
	//m_asic2->data_r<7>().set(m_asic5, FUNC(psion_asic5_device::data_r));        // High speed link
	//m_asic2->data_w<7>().set(m_asic5, FUNC(psion_asic5_device::data_w));

	PSION_PSU_ASIC5(config, m_asic3);
	m_asic3->adin_cb().set([this]() { return m_digitiser[m_dr]; });

	RAM(config, m_ram);
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	PALETTE(config, "palette", FUNC(psionmc_state::palette_init), 2);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00); // Piezo buzzer

	PSION_SSD(config, m_ssd[0]);
	m_ssd[0]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[1]);
	m_ssd[1]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[2]);
	m_ssd[2]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[3]);
	m_ssd[3]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));

	PSION_MODULE_SLOT(config, m_exp[0], psion_mcmodule_devices, "serpar"); // RS232/Parallel
	m_exp[0]->intr_cb().set(m_asic1, FUNC(psion_asic1_device::eint2_w));
	PSION_MODULE_SLOT(config, m_exp[1], psion_mcmodule_devices, nullptr);
	m_exp[1]->intr_cb().set(m_asic1, FUNC(psion_asic1_device::eint1_w));

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("MC");
}

void psionmc_state::mc200(machine_config &config)
{
	psionmc(config);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic1, FUNC(psion_asic1_device::screen_update_single));
	screen.set_palette(m_palette);

	m_ram->set_default_size("128K");
}

void psionmc_state::mc400(machine_config &config)
{
	psionmc(config);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(640, 400);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic1, FUNC(psion_asic1_device::screen_update_dual));
	screen.set_palette(m_palette);

	m_ram->set_default_size("256K");
}


ROM_START( mc200 )
	ROM_REGION16_LE(0x40000, "flash", ROMREGION_ERASEFF)
	// 2 x 28F010 128k flash chips, V2.20F also known to exist
	ROM_SYSTEM_BIOS(0, "212f", "V2.12F 081090")
	ROMX_LOAD("v212f_0.bin", 0x00000, 0x20000, CRC(ff346271) SHA1(50a01bbd653bbdffb59e37753a3ddd8c1f677ca5), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("v212f_1.bin", 0x00001, 0x20000, CRC(4f266410) SHA1(3d31b36daa239b0b80ec74855ecfa59fb05c9aaa), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_REGION(0x20000, "ssd3", 0)
	ROM_LOAD("mc200_system_disk.bin", 0x00000, 0x20000, CRC(8d7dab78) SHA1(a4ae14bfb30525033e681558e0aa5e4c5b21c62e))
ROM_END

ROM_START( mc400 )
	ROM_REGION16_LE(0x40000, "flash", ROMREGION_ERASEFF)
	// 2 x 28F010 128k flash chips
	ROM_SYSTEM_BIOS(0, "212f", "V2.12F 081090")
	ROMX_LOAD("v212f_0.bin", 0x00000, 0x20000, CRC(ff346271) SHA1(50a01bbd653bbdffb59e37753a3ddd8c1f677ca5), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("v212f_1.bin", 0x00001, 0x20000, CRC(4f266410) SHA1(3d31b36daa239b0b80ec74855ecfa59fb05c9aaa), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "126f", "V1.26F 020290")
	ROMX_LOAD("v126f_0.bin", 0x00000, 0x20000, CRC(2c569929) SHA1(8a89a3aa1811e3e41da0e3849c9d98ad61362b33), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("v126f_1.bin", 0x00001, 0x20000, CRC(e5a63fae) SHA1(dd630d5905601217dfbae183d3df621f98a82fd3), ROM_BIOS(1) | ROM_SKIP(1))

	// Psion test ROM master images
	ROM_SYSTEM_BIOS(2, "110ftst", "V1.10F Serial/Parallel Test")
	ROMX_LOAD("v110ftst.bin", 0x00000, 0x40000, CRC(178e8a7e) SHA1(a6e3e14e63c83473292eea87526c66873930b275), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "101ftst", "V1.01F Serial/Parallel Test")
	ROMX_LOAD("v101ftst.bin", 0x00000, 0x40000, CRC(8b8ec6b2) SHA1(fc9f914322bd1b1dbe60e84f317549fda9b80aad), ROM_BIOS(3))
	ROM_SYSTEM_BIOS(4, "062atst", "V0.62A/1 MC Final Test")
	ROMX_LOAD("v062atst.bin", 0x00000, 0x40000, CRC(72e026a5) SHA1(e7cbdecdad0574bd4804393189c8c768e934b6f5), ROM_BIOS(4))

	ROM_REGION(0x20000, "ssd3", 0)
	ROM_LOAD("mc400_system_disk.bin", 0x00000, 0x20000, CRC(2ee09e83) SHA1(2ba5dcdb07986f53ac97ca2e397ded0fc839a70a))
ROM_END

ROM_START( mcword )
	ROM_REGION16_LE(0x40000, "flash", ROMREGION_ERASEFF)
	// 2 x 28F010 128k flash chips
	ROM_SYSTEM_BIOS(0, "260f", "V2.60F/ENG 060892")
	ROMX_LOAD("v260f_0.bin", 0x00000, 0x20000, CRC(b4f2e560) SHA1(18bed2f8dfde458953eca2d0c21c4a94ba83bdf1), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("v260f_1.bin", 0x00001, 0x20000, CRC(abe679da) SHA1(751d9efd44c4bedaad77ef2ef8a16605eb88787c), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_REGION(0x40000, "ssd3", 0)
	ROM_LOAD("mcword_system_disk.bin", 0, 0x40000, CRC(c7f8ad21) SHA1(5bc99c5430ad631dd094a0f99c2963a8c32b0556))
ROM_END

ROM_START( mcword_de )
	ROM_REGION16_LE(0x40000, "flash", ROMREGION_ERASEFF)
	// 2 x 28F010 128k flash chips
	ROM_SYSTEM_BIOS(0, "260f", "V2.60F/ENG 260892")
	ROMX_LOAD("v260f_0_de.bin", 0x00000, 0x20000, CRC(5a92ee9a) SHA1(b84de4c30c78aa6cb9f453ebbe54bf53ae00a842), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("v260f_1_de.bin", 0x00001, 0x20000, CRC(088bc86d) SHA1(9aa665e4e6bc111ffb4bd24c0eeb7da91962e902), ROM_BIOS(0) | ROM_SKIP(1))

	ROM_REGION(0x40000, "ssd3", 0)
	ROM_LOAD("mcword_system_disk.bin", 0x00000, 0x40000, CRC(c7f8ad21) SHA1(5bc99c5430ad631dd094a0f99c2963a8c32b0556))
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT     COMPAT  MACHINE    INPUT        CLASS           INIT         COMPANY   FULLNAME             FLAGS
COMP( 1989, mc200,      mc400,     0,      mc200,     psionmc,     psionmc_state,  empty_init,  "Psion",  "MC 200",            MACHINE_SUPPORTS_SAVE )
COMP( 1989, mc400,      0,         0,      mc400,     psionmc,     psionmc_state,  empty_init,  "Psion",  "MC 400",            MACHINE_SUPPORTS_SAVE )
COMP( 1992, mcword,     mc400,     0,      mc400,     psionmc,     psionmc_state,  empty_init,  "Psion",  "MC Word",           MACHINE_SUPPORTS_SAVE )
COMP( 1992, mcword_de,  mc400,     0,      mc400,     psionmc_de,  psionmc_state,  empty_init,  "Psion",  "MC Word (German)",  MACHINE_SUPPORTS_SAVE )
