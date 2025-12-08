// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion MC600 Series

    TODO:
    - dump HD6305V0 internal ROM.
    - On/Off key doesn't work, use PSETUP to disable Standby.
    - add CGA output.

******************************************************************************/

#include "emu.h"

#include "bus/psion/module/slot.h"
#include "bus/rs232/rs232.h"
#include "cpu/i86/i86.h"
#include "cpu/m6805/hd6305.h"
#include "imagedev/floppy.h"
#include "machine/82c100.h"
#include "machine/82c606.h"
#include "machine/nvram.h"
#include "machine/psion_asic2.h"
#include "machine/psion_asic3.h"
#include "machine/psion_asic7.h"
#include "machine/psion_ssd.h"
#include "machine/ram.h"
#include "machine/upd765.h"
#include "sound/spkrdev.h"
#include "video/82c425.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"


namespace {

class mc600_state : public driver_device
{
public:
	mc600_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_kbdmcu(*this, "kbdmcu")
		, m_82c100(*this, "82c100")
		, m_82c425(*this, "82c425")
		, m_82c606(*this, "82c606")
		, m_asic2(*this, "asic2")
		, m_asic3(*this, "asic3")
		, m_asic5(*this, "asic5")
		, m_asic7(*this, "asic7")
		, m_ram(*this, "ram")
		, m_rambank(*this, "rambank")
		, m_nvram(*this, "nvram")
		, m_ramdisk(*this, "ramdisk")
		, m_pakram(*this, "pakram", 0x100000, ENDIANNESS_LITTLE)
		, m_pakflash(*this, "pakflash")
		, m_keyboard(*this, "COL%u", 0U)
		, m_speaker(*this, "speaker")
		, m_fdc(*this, "fdc")
		, m_ssd(*this, "ssd%u", 1U)
		, m_exp(*this, "exp")
		, m_caps_lock(*this, "caps_lock")
		, m_num_lock(*this, "num_lock")
		, m_scroll_lock(*this, "scroll_lock")
		, m_low_battery(*this, "low_battery")
		, m_standby(*this, "standby")
	{ }

	void mc600(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(key_on);
	//DECLARE_INPUT_CHANGED_MEMBER(reset);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void palette_init(palette_device &palette);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t pak_data_r(uint8_t cs, offs_t addr);
	void pak_data_w(uint8_t cs, offs_t addr, uint8_t data);

	uint8_t memr_r(offs_t offset);
	void memw_w(offs_t offset, uint8_t data);

	uint8_t porta_r();
	void portb_w(offs_t offset, uint8_t data, uint8_t mask);
	uint8_t portc_r();
	void portc_w(offs_t offset, uint8_t data, uint8_t mask);
	uint8_t portd_r();
	void portd_w(offs_t offset, uint8_t data, uint8_t mask);

	required_device<cpu_device> m_maincpu;
	required_device<hd6305v0_device> m_kbdmcu;
	required_device<f82c100_device> m_82c100;
	required_device<f82c425_device> m_82c425;
	required_device<p82c606_device> m_82c606;
	required_device<psion_asic2_device> m_asic2;
	required_device<psion_asic3_device> m_asic3;
	required_device<psion_asic5_device> m_asic5;
	required_device<psion_asic7_device> m_asic7;
	required_device<ram_device> m_ram;
	memory_bank_creator m_rambank;
	required_device<nvram_device> m_nvram;
	required_device<nvram_device> m_ramdisk;
	memory_share_creator<uint8_t> m_pakram;
	required_region_ptr<uint8_t> m_pakflash;
	required_ioport_array<13> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	required_device<tc8566af_device> m_fdc;
	required_device_array<psion_ssd_device, 4> m_ssd;
	required_device<psion_module_slot_device> m_exp;

	output_finder<> m_caps_lock;
	output_finder<> m_num_lock;
	output_finder<> m_scroll_lock;
	output_finder<> m_low_battery;
	output_finder<> m_standby;

	uint16_t m_keycol;
	uint32_t m_pak_addr;
	uint8_t m_pak_cs;

	int m_rtc32 = 0;
};


void mc600_state::machine_start()
{
	m_caps_lock.resolve();
	m_num_lock.resolve();
	m_scroll_lock.resolve();
	m_low_battery.resolve();
	m_standby.resolve();

	m_rambank->configure_entries(0, 2, m_ram->pointer() + 0x80000, 0x20000);
	m_rambank->set_entry(0);

	m_nvram->set_base(m_ram->pointer(), m_ram->size());
	m_ramdisk->set_base(m_pakram, 0x100000);

	m_pak_cs = 0;
}


void mc600_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram().share("ram");
	map(0x80000, 0x9ffff).bankrw(m_rambank);
	map(0xb8000, 0xbbfff).rw(m_82c425, FUNC(f82c425_device::mem_r), FUNC(f82c425_device::mem_w));
	map(0xc0000, 0xdffff).noprw();
	map(0xe0000, 0xfffff).rom().region("flash", 0);
}

void mc600_state::io_map(address_map &map)
{
	map(0x0000, 0x00ff).m(m_82c100, FUNC(f82c100_device::map));
	map(0x00f0, 0x00ff).rw(m_asic7, FUNC(psion_asic7_device::io_r), FUNC(psion_asic7_device::io_w));
	map(0x0100, 0x01ff).rw(m_exp, FUNC(psion_module_slot_device::io_r), FUNC(psion_module_slot_device::io_w));
	map(0x03d0, 0x03df).m(m_82c425, FUNC(f82c425_device::io_map));
	map(0x03f0, 0x03f5).m(m_fdc, FUNC(tc8566af_device::map));
	map(0x03f7, 0x03f7).rw(m_fdc, FUNC(tc8566af_device::dir_r), FUNC(tc8566af_device::ccr_w));
}


uint8_t mc600_state::pak_data_r(uint8_t cs, offs_t addr)
{
	uint8_t data = 0x00;

	switch (cs)
	{
	case 0: data = m_pakram[addr]; break;
	case 1: data = m_pakram[addr + 0x80000]; break;
	case 2: data = m_pakflash[addr]; break;
	case 3: data = m_pakflash[addr + 0x20000]; break;
	}

	return data;
}

void mc600_state::pak_data_w(uint8_t cs, offs_t addr, uint8_t data)
{
	switch (cs)
	{
	case 0: m_pakram[addr] = data; break;
	case 1: m_pakram[addr + 0x80000] = data; break;
	}
}


static INPUT_PORTS_START( mc600 )
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))  PORT_NAME("Caps Lock")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Shift (L)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))      PORT_NAME("Alt")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))  PORT_NAME("Control")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))        PORT_NAME(u8"\u2191") // U+2191 = ↑
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
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('`')  PORT_CHAR('~')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)            PORT_NAME("Shift (R)")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)                       PORT_NAME("Enter")

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(0x08)                     PORT_NAME("Backspace")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(0x09)                     PORT_NAME("Tab")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')                      PORT_NAME("Space")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))     PORT_NAME(u8"\u2192") // U+2192 = →

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))    PORT_NAME("Insert")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))      PORT_NAME("Home")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)       PORT_CHAR(UCHAR_MAMEKEY(PGUP))      PORT_NAME("Page Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)                                                                   PORT_NAME("LCD")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_NUMLOCK)    PORT_CHAR(UCHAR_MAMEKEY(NUMLOCK))   PORT_NAME("Num Lock")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_NAME("-")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))        PORT_NAME("F3")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))        PORT_NAME("F7")

	PORT_START("COL9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(DEL))       PORT_NAME("Delete")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))       PORT_NAME("End")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))      PORT_NAME("Page Down")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))  PORT_NAME("Palette")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SCRLOCK)    PORT_CHAR(UCHAR_MAMEKEY(SCRLOCK))   PORT_NAME("Scroll Lock")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))  PORT_NAME("+")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))        PORT_NAME("F4")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))        PORT_NAME("F8")

	PORT_START("COL10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(PAUSE))     PORT_NAME("Pause/Break")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))        PORT_NAME("F1")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))        PORT_NAME("F5")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))        PORT_NAME("F9")

	PORT_START("COL11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PRTSCR)     PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))    PORT_NAME("PrtScr")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))        PORT_NAME("F2")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))        PORT_NAME("F6")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))       PORT_NAME("F10")

	PORT_START("COL12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_ESC)        PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_NAME("Esc")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_NAME("On Off") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mc600_state::key_on), 0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	//PORT_START("RESET")
	//PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11)) PORT_NAME("Reset") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mc600_state::reset), 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(mc600_state::key_on)
{
	if (newval)
	{
		m_asic2->on_clr_w(newval);
	}
}

//INPUT_CHANGED_MEMBER(mc600_state::reset)
//{
//  if (newval)
//  {
//      m_asic2->reset_w(0);
//  }
//}


uint8_t mc600_state::porta_r()
{
	uint8_t data = 0x00;

	for (int col = 0; col < 12; col++)
	{
		if (BIT(m_keycol, col))
		{
			data |= m_keyboard[col]->read();
		}
	}

	return data;
}

void mc600_state::portb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	data &= mem_mask;
	m_keycol = (m_keycol & 0x0f00) | bitswap<8>(data, 0, 1, 2, 3, 4, 5, 6, 7);
}

uint8_t mc600_state::portc_r()
{
	uint8_t data = 0x00;

	data |= m_keyboard[12]->read() & 1; // Esc
	//data |= m_asic7->keycmd_r() << 1;
	data |= m_keyboard[12]->read() & 4; // On/Off
	data |= m_fdc->c6_r() << 3;

	return data;
}

void mc600_state::portc_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	data &= mem_mask;
	m_keycol = (m_keycol & 0x00ff) | ((data & 0xf0) << 4);
}

uint8_t mc600_state::portd_r()
{
	uint8_t data = 0x00;

	data |= m_82c100->kbdata_r() << 3;
	data |= m_82c100->kbclk_r() << 5;

	return data;
}

void mc600_state::portd_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (BIT(mem_mask, 3))
		m_82c100->kbdata_w(BIT(data, 3));

	if (BIT(mem_mask, 5))
		m_82c100->kbclk_w(BIT(data, 5));
}


void mc600_state::palette_init(palette_device &palette)
{
	// estimated LCD palette
	palette.set_pen_color(0, rgb_t(0x69, 0x70, 0x69));
	palette.set_pen_color(1, rgb_t(0x5f, 0x65, 0x5f));
	palette.set_pen_color(2, rgb_t(0x54, 0x5a, 0x54));
	palette.set_pen_color(3, rgb_t(0x4a, 0x4e, 0x4a));
	palette.set_pen_color(4, rgb_t(0x3f, 0x43, 0x3f));
	palette.set_pen_color(5, rgb_t(0x35, 0x38, 0x35));
	palette.set_pen_color(6, rgb_t(0x2a, 0x2d, 0x2a));
	palette.set_pen_color(7, rgb_t(0x1f, 0x22, 0x1f));
}


uint8_t mc600_state::memr_r(offs_t offset)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	return program.read_byte(offset);
}

void mc600_state::memw_w(offs_t offset, uint8_t data)
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	program.write_byte(offset, data);
}


void mc600_state::mc600(machine_config &config)
{
	I8086(config, m_maincpu, 14.318181_MHz_XTAL / 3); // M80C86A
	m_maincpu->set_addrmap(AS_PROGRAM, &mc600_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mc600_state::io_map);
	m_maincpu->set_irq_acknowledge_callback(m_82c100, FUNC(f82c100_device::inta_cb));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_size(640, 200);
	screen.set_visarea_full();
	screen.set_refresh_hz(56);
	screen.set_physical_aspect(640, 400);
	screen.set_screen_update(m_82c425, FUNC(f82c425_device::screen_update));

	PALETTE(config, "palette", FUNC(mc600_state::palette_init), 8);

	F82C425(config, m_82c425, 14.318181_MHz_XTAL);
	m_82c425->set_screen("screen");
	//m_82c425->crt_lcd_callback(); TODO: reconfigure screen
	m_82c425->set_lcd_palette("palette");

	F82C100(config, m_82c100, 14.318181_MHz_XTAL, 24_MHz_XTAL);
	m_82c100->set_cpu_tag(m_maincpu);
	m_82c100->intr().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_82c100->intr().append(m_asic7, FUNC(psion_asic7_device::intr_w));
	m_82c100->nmi().set_inputline(m_maincpu, INPUT_LINE_NMI);
	m_82c100->nmi().append(m_asic7, FUNC(psion_asic7_device::nmi_w));
	m_82c100->tc().set(m_fdc, FUNC(tc8566af_device::tc_line_w));
	m_82c100->memr().set(FUNC(mc600_state::memr_r));
	m_82c100->memw().set(FUNC(mc600_state::memw_w));
	m_82c100->ior<2>().set(m_fdc, FUNC(tc8566af_device::dma_r));
	m_82c100->iow<2>().set(m_fdc, FUNC(tc8566af_device::dma_w));
	m_82c100->spkdata().set(m_speaker, FUNC(speaker_sound_device::level_w));

	PSION_ASIC7(config, m_asic7, 14.318181_MHz_XTAL);
	m_asic7->as2rd().set(m_asic2, FUNC(psion_asic2_device::io_r));
	m_asic7->as2wr().set(m_asic2, FUNC(psion_asic2_device::io_w));
	m_asic7->pgsel().set([this](int state) { m_rambank->set_entry(state); });
	m_asic7->caps().set([this](int state) { m_caps_lock   = state; logerror("Caps Lock %s\n",   state ? "ON" : "OFF"); });
	m_asic7->numl().set([this](int state) { m_num_lock    = state; logerror("Num Lock %s\n",    state ? "ON" : "OFF"); });
	m_asic7->scrl().set([this](int state) { m_scroll_lock = state; logerror("Scroll Lock %s\n", state ? "ON" : "OFF"); });
	m_asic7->batt().set([this](int state) { m_low_battery = state; logerror("Low Battery %s\n", state ? "ON" : "OFF"); });
	m_asic7->stby().set([this](int state) { m_standby     = state; logerror("Standby %s\n",     state ? "ON" : "OFF"); });

	PSION_ASIC2(config, m_asic2, 14.318181_MHz_XTAL);
	m_asic2->int_cb().set(m_82c100, FUNC(f82c100_device::irq2_w));
	m_asic2->nmi_cb().set(m_82c100, FUNC(f82c100_device::pwrnmi_w));
	m_asic2->cbusy_cb().set_inputline(m_maincpu, INPUT_LINE_TEST);
	//m_asic2->buz_cb().set(m_speaker, FUNC(speaker_sound_device::level_w));
	//m_asic2->buzvol_cb().set([this](int state) { m_speaker->set_output_gain(ALL_OUTPUTS, state ? 1.0 : 0.25); });
	m_asic2->read_pd_cb().set([this]() { m_rtc32 ^= 1;  return m_rtc32 << 6; }); // TODO: implement RTC32 output from ASIC3
	m_asic2->data_r<0>().set(m_asic3, FUNC(psion_asic3_device::data_r));        // Power supply (ASIC3)
	m_asic2->data_w<0>().set(m_asic3, FUNC(psion_asic3_device::data_w));
	m_asic2->data_r<1>().set(m_ssd[1], FUNC(psion_ssd_device::data_r));         // SSD Pack 1
	m_asic2->data_w<1>().set(m_ssd[1], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<2>().set(m_ssd[0], FUNC(psion_ssd_device::data_r));         // SSD Pack 2
	m_asic2->data_w<2>().set(m_ssd[0], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<3>().set(m_ssd[3], FUNC(psion_ssd_device::data_r));         // SSD Pack 3
	m_asic2->data_w<3>().set(m_ssd[3], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<4>().set(m_ssd[2], FUNC(psion_ssd_device::data_r));         // SSD Pack 4
	m_asic2->data_w<4>().set(m_ssd[2], FUNC(psion_ssd_device::data_w));
	m_asic2->data_r<5>().set(m_asic5, FUNC(psion_asic5_device::data_r));        // Internal Pack
	m_asic2->data_w<5>().set(m_asic5, FUNC(psion_asic5_device::data_w));
	m_asic2->data_r<6>().set(m_exp, FUNC(psion_module_slot_device::data_r));    // Expansion port A
	m_asic2->data_w<6>().set(m_exp, FUNC(psion_module_slot_device::data_w));
	//m_asic2->data_r<7>().set(m_asic5, FUNC(psion_asic5_device::data_r));        // High speed link
	//m_asic2->data_w<7>().set(m_asic5, FUNC(psion_asic5_device::data_w));

	PSION_PSU_ASIC3(config, m_asic3);

	PSION_ASIC5(config, m_asic5).set_mode(psion_asic5_device::PACK_MODE);
	m_asic5->set_info_byte(0x70);
	m_asic5->readpa_handler().set([this]() { return pak_data_r(m_pak_cs, m_pak_addr & 0x7ffff); });
	m_asic5->writepa_handler().set([this](uint8_t data) { pak_data_w(m_pak_cs, m_pak_addr & 0x7ffff, data); });
	m_asic5->writepb_handler().set([this](uint8_t data) { m_pak_addr = (m_pak_addr & 0xffff00) | (data << 0); });
	m_asic5->writepd_handler().set([this](uint8_t data) { m_pak_addr = (m_pak_addr & 0xff00ff) | (data << 8); });
	m_asic5->writepc_handler().set([this](uint8_t data) { m_pak_addr = (m_pak_addr & 0x00ffff) | (data << 16); m_pak_cs = data >> 6; });

	HD6305V0(config, m_kbdmcu, 24_MHz_XTAL / 3); // TODO: verify clock (divided by ASIC7 KEYOSC)
	m_kbdmcu->read_porta().set(FUNC(mc600_state::porta_r));
	m_kbdmcu->write_portb().set(FUNC(mc600_state::portb_w));
	m_kbdmcu->read_portc().set(FUNC(mc600_state::portc_r));
	m_kbdmcu->write_portc().set(FUNC(mc600_state::portc_w));
	m_kbdmcu->read_portd().set(FUNC(mc600_state::portd_r));
	m_kbdmcu->write_portd().set(FUNC(mc600_state::portd_w));

	RAM(config, m_ram).set_default_size("768K");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);
	NVRAM(config, "ramdisk", nvram_device::DEFAULT_NONE);

	P82C606(config, m_82c606, 1.8432_MHz_XTAL);
	m_82c606->set_cpu_tag(m_maincpu);
	m_82c606->irq3().set(m_82c100, FUNC(f82c100_device::irq3_w));
	m_82c606->irq4().set(m_82c100, FUNC(f82c100_device::irq4_w));
	m_82c606->irq5().set(m_82c100, FUNC(f82c100_device::rtcnmi_w));
	m_82c606->irq7().set(m_82c100, FUNC(f82c100_device::irq7_w));
	m_82c606->txd1().set("rs232", FUNC(rs232_port_device::write_txd));
	m_82c606->rts1().set("rs232", FUNC(rs232_port_device::write_rts));
	m_82c606->dtr1().set("rs232", FUNC(rs232_port_device::write_dtr));

	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	rs232.rxd_handler().set(m_82c606, FUNC(p82c606_device::write_rxd1));
	rs232.dcd_handler().set(m_82c606, FUNC(p82c606_device::write_dcd1));
	rs232.cts_handler().set(m_82c606, FUNC(p82c606_device::write_cts1));
	rs232.dsr_handler().set(m_82c606, FUNC(p82c606_device::write_dsr1));
	rs232.ri_handler().set(m_82c606, FUNC(p82c606_device::write_ri1));

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 1.00);

	TC8566AF(config, m_fdc, 16_MHz_XTAL);
	m_fdc->set_ready_line_connected(false);
	m_fdc->intrq_wr_callback().set(m_82c100, FUNC(f82c100_device::irq6_w));
	m_fdc->drq_wr_callback().set(m_82c100, FUNC(f82c100_device::drq2_w));

	FLOPPY_CONNECTOR(config, "fdc:0", "35hd", FLOPPY_35_HD, true, floppy_image_device::default_pc_floppy_formats).enable_sound(true);

	PSION_SSD(config, m_ssd[0]);
	m_ssd[0]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[1]);
	m_ssd[1]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[2]);
	m_ssd[2]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));
	PSION_SSD(config, m_ssd[3]);
	m_ssd[3]->door_cb().set(m_asic2, FUNC(psion_asic2_device::dnmi_w));

	PSION_MODULE_SLOT(config, m_exp, psion_mcmodule_devices, nullptr);
	m_exp->intr_cb().set(m_82c100, FUNC(f82c100_device::irq5_w));

	SOFTWARE_LIST(config, "ssd_list").set_original("psion_ssd").set_filter("MC");
	SOFTWARE_LIST(config, "disk_list").set_original("psion_flop").set_filter("DOS");
	SOFTWARE_LIST(config, "pc_disk_list").set_compatible("ibm5150");
	SOFTWARE_LIST(config, "at_disk_list").set_compatible("ibm5170"); // should ideally be an ibm5160 xt_disk_list
}


ROM_START( mc600 )
	ROM_REGION16_LE(0x20000, "flash", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "111f", "V1.11F/ENG") // 3504-5003v1.11f 3504-5002v1.11f 3504-5001v1.11f 29/10/92
	ROMX_LOAD("mc600_v1.11f.bin", 0x00000, 0x20000, CRC(5f756f42) SHA1(6ba043ee07ac045d5dc32c2ccaf7f8f004d7ec7c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "108f", "V1.08F/ENG")
	ROMX_LOAD("mc600_v1.08f.bin", 0x00000, 0x20000, CRC(5ebd3c81) SHA1(c6d4f9eb81db76dde72be75b7e56813b8073088c), ROM_BIOS(1))

	ROM_REGION(0x40000, "pakflash", 0)
	ROM_LOAD("intpak_v111f.bin", 0x0000, 0x40000, BAD_DUMP CRC(57fcad77) SHA1(8dd7d23c5bbe97408d40b25df705f27081f237fb)) // recreated from individual files

	ROM_REGION(0x1000, "kbdmcu", 0)
	ROM_LOAD("hd6305v0.bin", 0x0000, 0x1000, BAD_DUMP CRC(641c5dba) SHA1(7717fa82fbbe3bd65b94aaa8c464fa9e65b61006)) // not actual dump, built from source, maybe pre-release.
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT     COMPAT  MACHINE  INPUT      CLASS         INIT         COMPANY   FULLNAME       FLAGS
COMP( 1990, mc600,    0,         0,      mc600,   mc600,     mc600_state,  empty_init,  "Psion",  "MC 600",      MACHINE_IMPERFECT_GRAPHICS )
