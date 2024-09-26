// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion HC Series

    TODO:
    - battery backed RAM

******************************************************************************/

#include "emu.h"
#include "cpu/nec/nec.h"
#include "machine/nvram.h"
#include "machine/psion_asic1.h"
#include "machine/psion_asic2.h"
#include "machine/psion_asic3.h"
#include "machine/psion_ssd.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"
#include "bus/psion/module/slot.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"
#include "utf8.h"


namespace {

class psionhc_state : public driver_device
{
public:
	psionhc_state(const machine_config &mconfig, device_type type, const char *tag)
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
		, m_exp(*this, "exp%u", 0U)
	{ }

	void psionhc100(machine_config &config);
	void psionhc110(machine_config &config);
	void psionhc120(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_on);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<psion_asic1_device> m_asic1;
	required_device<psion_asic2_device> m_asic2;
	required_device<psion_asic3_device> m_asic3;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<palette_device> m_palette;
	required_ioport_array<8> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_device_array<psion_ssd_device, 2> m_ssd;
	required_device_array<psion_module_slot_device, 2> m_exp;

	void palette_init(palette_device &palette);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void asic1_map(address_map &map) ATTR_COLD;

	uint8_t port_data_r();
	void port_data_w(uint8_t data);

	int m_dr = 0;
};


void psionhc_state::machine_start()
{
	m_asic1->space(0).install_ram(0, m_ram->mask(), m_ram->pointer());
	m_nvram->set_base(m_ram->pointer(), m_ram->size());
}

void psionhc_state::machine_reset()
{
}


void psionhc_state::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(m_asic1, FUNC(psion_asic1_device::mem_r), FUNC(psion_asic1_device::mem_w));
}

void psionhc_state::io_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_asic1, FUNC(psion_asic1_device::io_r), FUNC(psion_asic1_device::io_w));
	map(0x0080, 0x008f).rw(m_asic2, FUNC(psion_asic2_device::io_r), FUNC(psion_asic2_device::io_w)).umask16(0x00ff);
	map(0x0100, 0x01ff).rw(m_exp[0], FUNC(psion_module_slot_device::io_r), FUNC(psion_module_slot_device::io_w));
	map(0x0200, 0x02ff).rw(m_exp[1], FUNC(psion_module_slot_device::io_r), FUNC(psion_module_slot_device::io_w));
}

void psionhc_state::asic1_map(address_map &map)
{
	map(0x00000, 0x7ffff).noprw();
	map(0x80000, 0xfffff).rom().region("flash", 0);
}


static INPUT_PORTS_START( psionhc_uk )
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME(UTF8_RIGHT" Info")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME(UTF8_LEFT" Task")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))      PORT_NAME(UTF8_DOWN" PG Dn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME(UTF8_UP" Pg Up")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Menu")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_NAME("Esc")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')  PORT_CHAR('=')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Off")

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f')  PORT_CHAR('F')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e')  PORT_CHAR('E')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d')  PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c')  PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b')  PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a')  PORT_CHAR('A')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l')  PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k')  PORT_CHAR('K')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j')  PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i')  PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h')  PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g')  PORT_CHAR('G')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')  PORT_CHAR(',')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r')  PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')  PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p')  PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o')  PORT_CHAR('O')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')      PORT_NAME("n N No")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m')  PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')                      PORT_NAME("Space")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x')  PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w')  PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v')  PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u')  PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t')  PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s')  PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Shift")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')      PORT_NAME("y Y Yes")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')  PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')  PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')  PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')  PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                     PORT_NAME("Tab")
	PORT_BIT(0xc0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z')  PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_CHAR('*')  PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')  PORT_CHAR(':')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')  PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')  PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                     PORT_NAME("Del")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Backlight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                       PORT_NAME("Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')  PORT_CHAR('\\')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')  PORT_CHAR(0xa3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')  PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')  PORT_CHAR('!')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))      PORT_NAME("Psion Lock")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Contrast")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("ON_OFF")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_NAME("On Off") PORT_CHANGED_MEMBER(DEVICE_SELF, psionhc_state, key_on, 0)
INPUT_PORTS_END

//static INPUT_PORTS_START( psionhc_num )
//  PORT_START("COL0")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n')  PORT_CHAR('N')      PORT_NAME("No")
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME(UTF8_UP)
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))      PORT_NAME(UTF8_DOWN)
//  PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                     PORT_NAME("Del")
//  PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_NAME("C")
//  PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('+')
//  PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Off")
//
//  PORT_START("COL1")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.')
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_CHAR('*')
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_CHAR('%')
//  PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Info")
//  PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("COL2")
//  PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("COL3")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))        PORT_NAME("F1")
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))        PORT_NAME("F2")
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))        PORT_NAME("F3")
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))        PORT_NAME("F4")
//  PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("COL4")
//  PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Shift")
//  PORT_BIT(0xbf, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("COL5")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-')
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))      PORT_NAME(UTF8_LEFT)
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9')
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8')
//  PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7')
//  PORT_BIT(0xe0, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("COL6")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)//
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME(UTF8_RIGHT)
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6')
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5')
//  PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4')
//  PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/')
//  PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Backlight")
//  PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("COL7")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                       PORT_NAME("Enter")
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')  PORT_CHAR('Y')      PORT_NAME("Yes")
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3')
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2')
//  PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1')
//  PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
//  PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("Contrast")
//  PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
//
//  PORT_START("ON_OFF")
//  PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_NAME("On Off") PORT_CHANGED_MEMBER(DEVICE_SELF, psionhc_state, key_on, 0)
//INPUT_PORTS_END

INPUT_CHANGED_MEMBER(psionhc_state::key_on)
{
	if (newval)
	{
		m_asic2->on_clr_w(newval);
	}
}


uint8_t psionhc_state::port_data_r()
{
	// b0 NC
	// b1 NC
	// b2 LED ?
	// b3 VSLED ?
	// b4 SCK6
	// b5 SD6
	// b6 ?
	return 0x00;
}

void psionhc_state::port_data_w(uint8_t data)
{
	// b7 BackLight ?
}


void psionhc_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}


void psionhc_state::psionhc100(machine_config &config)
{
	V30(config, m_maincpu, 7.68_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &psionhc_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &psionhc_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_asic1, FUNC(psion_asic1_device::inta_cb));

	RAM(config, m_ram).set_default_size("128K");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(160, 80);
	screen.set_visarea_full();
	screen.set_refresh_hz(66);
	screen.set_screen_update(m_asic1, FUNC(psion_asic1_device::screen_update_single));
	screen.set_palette(m_palette);
	PALETTE(config, "palette", FUNC(psionhc_state::palette_init), 2);

	PSION_ASIC1(config, m_asic1, 7.68_MHz_XTAL);
	m_asic1->set_screen("screen");
	m_asic1->set_laptop_mode(false);
	m_asic1->set_addrmap(0, &psionhc_state::asic1_map);
	m_asic1->int_cb().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_asic1->nmi_cb().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_asic1->frcovl_cb().set(m_asic2, FUNC(psion_asic2_device::frcovl_w));

	PSION_ASIC2(config, m_asic2, 7.68_MHz_XTAL);
	m_asic2->int_cb().set(m_asic1, FUNC(psion_asic1_device::eint3_w));
	m_asic2->nmi_cb().set(m_asic1, FUNC(psion_asic1_device::enmi_w));
	m_asic2->cbusy_cb().set_inputline(m_maincpu, NEC_INPUT_LINE_POLL);
	m_asic2->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	m_asic2->buzvol_cb().set([this](int state) { m_speaker->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.25); });
	m_asic2->dr_cb().set([this](int state) { m_dr = state; });
	m_asic2->col_cb().set([this](uint8_t data) { return m_keyboard[data & 7]->read(); });
	m_asic2->read_pd_cb().set(FUNC(psionhc_state::port_data_r));
	m_asic2->write_pd_cb().set(FUNC(psionhc_state::port_data_w));
	m_asic2->data_r<0>().set(m_asic3, FUNC(psion_asic3_device::data_r));        // Power supply (ASIC3)
	m_asic2->data_w<0>().set(m_asic3, FUNC(psion_asic3_device::data_w));
	m_asic2->data_r<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_r));         // SSD
	m_asic2->data_w<1>().set(m_ssd[0], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<2>().set(m_ssd[1], FUNC(psion_ssd_device::data_r));         // SSD
	m_asic2->data_w<2>().set(m_ssd[1], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<5>().set(m_exp[1], FUNC(psion_module_slot_device::data_r)); // Expansion port B
	m_asic2->data_w<5>().set(m_exp[1], FUNC(psion_module_slot_device::data_w));
	m_asic2->data_r<6>().set(m_exp[0], FUNC(psion_module_slot_device::data_r)); // Expansion port A
	m_asic2->data_w<6>().set(m_exp[0], FUNC(psion_module_slot_device::data_w));
	//m_asic2->data_r<7>().set(m_exp[2], FUNC(psion_module_slot_device::data_r));
	//m_asic2->data_w<7>().set(m_exp[2], FUNC(psion_module_slot_device::data_w));

	PSION_PSU_ASIC3(config, m_asic3);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00); // Piezo buzzer

	PSION_SSD(config, m_ssd[0]);
	m_ssd[0]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[1]);
	m_ssd[1]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));

	PSION_MODULE_SLOT(config, m_exp[0], psion_hcmodule_devices, nullptr); // RS232/Parallel
	m_exp[0]->intr_cb().set(m_asic1, FUNC(psion_asic1_device::eint2_w));
	PSION_MODULE_SLOT(config, m_exp[1], psion_hcmodule_devices, nullptr);
	m_exp[1]->intr_cb().set(m_asic1, FUNC(psion_asic1_device::eint1_w));

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("HC");
}

void psionhc_state::psionhc110(machine_config &config)
{
	psionhc100(config);

	m_ram->set_default_size("256K");
}

void psionhc_state::psionhc120(machine_config &config)
{
	psionhc100(config);

	m_ram->set_default_size("512K");
}


ROM_START( psionhc120 )
	ROM_REGION16_LE(0x80000, "flash", ROMREGION_ERASEFF)
	// 2 x 28F010 128k flash chips, V1.51F 050592, V1.62F, V1.64F, V1.71F also known to exist
	ROM_SYSTEM_BIOS(0, "172f", "V1.72F")
	ROMX_LOAD("v172f_1.bin", 0x00000, 0x20000, CRC(5ba21d09) SHA1(a9348eeb223cdc767e434ec34beae546defab108), ROM_BIOS(0))
	ROM_RELOAD(0x20000, 0x20000)
	ROMX_LOAD("v172f_2.bin", 0x40000, 0x20000, CRC(4436f332) SHA1(c6154cd948260729e079c47dea47def5cdc99a36), ROM_BIOS(0))
	ROM_RELOAD(0x60000, 0x20000)
	ROM_SYSTEM_BIOS(1, "170f", "V1.70F")
	ROMX_LOAD("v170f_1.bin", 0x00000, 0x20000, CRC(f733a7ab) SHA1(e49273a3d97d5ffb9f9b5958ad031aeedb40af01), ROM_BIOS(1))
	ROM_RELOAD(0x20000, 0x20000)
	ROMX_LOAD("v170f_2.bin", 0x40000, 0x20000, CRC(edd6a78d) SHA1(61344cd38928183b0e2c58ebfe92984518d8e731), ROM_BIOS(1))
	ROM_RELOAD(0x60000, 0x20000)
ROM_END

#define rom_psionhc100 rom_psionhc120
#define rom_psionhc110 rom_psionhc120

} // anonymous namespace


//    YEAR  NAME         PARENT       COMPAT  MACHINE       INPUT        CLASS           INIT         COMPANY   FULLNAME    FLAGS
COMP( 1991, psionhc100,  psionhc120,  0,      psionhc100,   psionhc_uk,  psionhc_state,  empty_init,  "Psion",  "HC 100",   MACHINE_SUPPORTS_SAVE )
COMP( 1991, psionhc110,  psionhc120,  0,      psionhc110,   psionhc_uk,  psionhc_state,  empty_init,  "Psion",  "HC 110",   MACHINE_SUPPORTS_SAVE )
COMP( 1991, psionhc120,  0,           0,      psionhc120,   psionhc_uk,  psionhc_state,  empty_init,  "Psion",  "HC 120",   MACHINE_SUPPORTS_SAVE )
