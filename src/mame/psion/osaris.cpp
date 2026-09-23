// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Oregon Scientific Osaris (EPOC R4)

    TODO:
    - UART support
    - Compact Flash (CLPS6700)
    - Bezel artwork

****************************************************************************/

#include "emu.h"

#include "bus/pccard/ataflash.h"
#include "cpu/arm7/arm7.h"
#include "machine/adc1213x.h"
#include "machine/clps6700.h"
#include "machine/clps7110.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class osaris_state : public driver_device
{
public:
	osaris_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soc(*this, "soc")
		, m_clps6700(*this, "clps6700")
		, m_pccard(*this, "pccard")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_eeprom(*this, "eeprom")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_buzzer(*this, "buzzer")
		, m_touchx(*this, "TOUCHX")
		, m_touchy(*this, "TOUCHY")
		, m_touch(*this, "TOUCH")
		, m_kbd_cols(*this, "COL%u", 0U)
	{
	}

	void osaris(machine_config &config) ATTR_COLD;

	void init_osaris() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(touch_down);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void palette_init(palette_device &palette) const ATTR_COLD;

	void osaris_map(address_map &map) ATTR_COLD;

	uint8_t keyb_r();
	uint8_t keym_r();
	uint16_t adc12138_r(offs_t offset);

	required_device<arm710a_cpu_device> m_maincpu;
	required_device<clps711x_device> m_soc;
	required_device<clps6700_device> m_clps6700;
	required_device<pccard_slot_device> m_pccard;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_buzzer;
	required_ioport m_touchx;
	required_ioport m_touchy;
	required_ioport m_touch;
	required_ioport_array<9> m_kbd_cols;

	uint8_t m_kbd_scan = 0;
};


void osaris_state::init_osaris()
{
	uint8_t *eeprom = memregion("eeprom")->base();

	// defaults expected by the touchscreen code
	eeprom[0x0a] = 20;
	eeprom[0x0b] = 20;
	eeprom[0x0c] = 20;
	eeprom[0x0d] = 30;

	// machine unique ID
	eeprom[0x1b] = 0xde;
	eeprom[0x1a] = 0xad;
	eeprom[0x19] = 0xbe;
	eeprom[0x18] = 0xef;

	// calculate the checksum
	uint8_t chksum = 0;
	for (int i = 1; i < 0x20; i++)
		chksum ^= eeprom[i];

	// EPOC is expecting 0x42
	eeprom[0x00] = chksum ^ 0x42;
}


void osaris_state::machine_start()
{
	// install RAM
	switch (m_ram->size())
	{
	case 0x1000000:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc07fffff, 0x0f800000, m_ram->pointer());
		m_maincpu->space(AS_PROGRAM).install_ram(0xd0000000, 0xd07fffff, 0x0f800000, m_ram->pointer() + 0x0800000);
		break;

	case 0x0800000:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc03fffff, 0x0fc00000, m_ram->pointer());
		m_maincpu->space(AS_PROGRAM).install_ram(0xd0000000, 0xd03fffff, 0x0fc00000, m_ram->pointer() + 0x0400000);
		break;

	case 0x0400000:
		m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc03fffff, 0x0fc00000, m_ram->pointer());
		break;
	}

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	save_item(NAME(m_kbd_scan));
}

void osaris_state::machine_reset()
{
	m_kbd_scan = 0;
}


uint8_t osaris_state::keyb_r()
{
	uint8_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbd_scan, i))
			data |= m_kbd_cols[i]->read();
	}

	return data;
}

uint8_t osaris_state::keym_r()
{
	return (m_kbd_cols[8]->read() ^ 0xff) << 4;
}


uint16_t osaris_state::adc12138_r(offs_t offset)
{
	uint16_t data = 0xffff;

	// TODO: Proper SSP support with ADC12138 device
	switch (offset & 0xff)
	{
	case 0xc1: // Digitiser X
		data = (uint16_t)(m_touchx->read() * 8) + 305;
		break;
	case 0x81: // Digitiser Y
		data = (uint16_t)(m_touchy->read() * 13.53) + 680;
		break;
	case 0x91: // Main battery
		data = 3000;
		break;
	case 0xd1: // Backup Battery
		data = 3100;
		break;
	case 0xa1: // Reference
		data = 1000;
		break;
	}

	return data;
}


void osaris_state::osaris_map(address_map &map)
{
	map(0x00000000, 0x007fffff).mirror(0x0f000000).rom().region("maincpu", 0);
	map(0x40000000, 0x4fffffff).m(m_clps6700, FUNC(clps6700_device::map));
	map(0x80000000, 0x80001fff).rw(m_soc, FUNC(clps7111_device::periphs_r), FUNC(clps7111_device::periphs_w));
}


void osaris_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		const int r = (0x99 * i) / 15;
		const int g = (0xaa * i) / 15;
		const int b = (0x88 * i) / 15;
		m_palette->set_pen_color(15 - i, rgb_t(r, g, b));
	}
}


INPUT_CHANGED_MEMBER(osaris_state::touch_down)
{
	m_soc->eint2_w(newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_PORTS_START( osaris )
	PORT_START("TOUCHX")
	PORT_BIT(0x3ff, 362, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,439) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT(0x1ff, 125, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,199) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCH")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(osaris_state::touch_down), 0)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('{')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR('\\')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)       PORT_CHAR(27)  PORT_NAME("Esc")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9)   PORT_NAME("Tab  Caps")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') PORT_NAME("Space")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('@') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('/') PORT_NAME("Help")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR(U'×')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)       PORT_NAME("Menu")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%')  PORT_CHAR('[')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR('\'') PORT_CHAR('~') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')  PORT_CHAR(U'÷')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Home") // U+2190 = ←
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR(']')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)  PORT_NAME("Enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(':')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME(u8"\u2193 PgDn") // U+2193 = ↓
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR(U'£')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)   PORT_NAME("Del <-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))   PORT_NAME(u8"\u2191 PgUp") // U+2191 = ↑
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192 End") // U+2192 = →
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)           PORT_NAME("Shift (L)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)                                       PORT_NAME("Shift (R)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)      PORT_CHAR(UCHAR_SHIFT_2)           PORT_NAME("Fn")
INPUT_PORTS_END

INPUT_PORTS_START( osaris_fr )
	PORT_INCLUDE(osaris)

	PORT_MODIFY("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('&')  PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('_')  PORT_CHAR('\\')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)       PORT_CHAR(27)  PORT_NAME("Esc")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')  PORT_CHAR(U'ù')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9)   PORT_NAME("Tab  Caps")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ') PORT_NAME("Espace")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR(U'é') PORT_CHAR('%')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(U'ç') PORT_CHAR('^')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')  PORT_CHAR('+')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('q') PORT_CHAR('Q')  PORT_CHAR('{')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(':') PORT_CHAR('/')  PORT_NAME("Aide")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR('"')  PORT_CHAR('#')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(U'à') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I')  PORT_CHAR('*')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')  PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')  PORT_CHAR(U'÷')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)       PORT_NAME("Menu")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('\'') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P')  PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')  PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')  PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('(')  PORT_CHAR(U'º')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR(';') PORT_CHAR('.')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O')  PORT_CHAR(U'µ')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')  PORT_CHAR('>')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_NAME(u8"\u2190 Début") // U+2190 = ←
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR(')')  PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)  PORT_NAME("Entrée")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L')  PORT_CHAR(U'×')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')  PORT_CHAR('[')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME(u8"\u2193 PgBs") // U+2193 = ↓
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR(U'è') PORT_CHAR(U'£')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)   PORT_NAME("Eff <-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')  PORT_CHAR('+')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))   PORT_NAME(u8"\u2191 PgHt") // U+2191 = ↑
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')  PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192 Fin") // U+2192 = →
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL7")
	PORT_BIT(0xff, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_MODIFY("COL8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)           PORT_NAME("Maj (L)")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)                                       PORT_NAME("Maj (R)")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)      PORT_CHAR(UCHAR_SHIFT_2)           PORT_NAME("Fn")
INPUT_PORTS_END


static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("cf", ATA_FLASH_PCCARD);
}


void osaris_state::osaris(machine_config &config)
{
	ARM710A(config, m_maincpu, 3.6864_MHz_XTAL * 5);
	m_maincpu->set_addrmap(AS_PROGRAM, &osaris_state::osaris_map);

	CLPS7111(config, m_soc, 3.6864_MHz_XTAL, m_maincpu);
	m_soc->lcd_dma_cb().set(m_ram, FUNC(ram_device::read));
	m_soc->porta_r().set(FUNC(osaris_state::keyb_r)).mask(0x7f);
	m_soc->porta_r().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	m_soc->portb_r().set(FUNC(osaris_state::keym_r)).mask(0xf0);
	m_soc->portb_r().append(m_clps6700, FUNC(clps6700_device::pcm_rdy_r)).bit(0);
	m_soc->portb_w().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write)).bit(3);
	m_soc->portb_w().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write)).bit(2);
	m_soc->portd_w().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	m_soc->buz_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));
	m_soc->col_cb().set([this](uint8_t data) { m_kbd_scan = data; });
	m_soc->adc_r().set(FUNC(osaris_state::adc12138_r)); // TODO: verify ADC type
	m_soc->set_screen_origin(60, 0);
	m_soc->set_screen("screen");

	RAM(config, m_ram).set_default_size("8M").set_extra_options("4M");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(58);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(m_soc, FUNC(clps7111_device::screen_update));
	m_screen->set_size(440, 200); // PEN 440x200 LCD 320x200
	m_screen->set_visarea_full();
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(osaris_state::palette_init), 16);

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer).add_route(ALL_OUTPUTS, "mono", 1.0);

	EEPROM_93C06_16BIT(config, m_eeprom); // 93S06

	ADC12138(config, "adc");

	CLPS6700(config, m_clps6700).set_pccard(m_pccard);
	//m_clps6700->pirq_handler().set(m_soc, FUNC(clps7111_device::eint2_w));

	PCCARD_SLOT(config, m_pccard, pcmcia_devices, "cf").set_fixed(true);
	m_pccard->cd1().set(m_clps6700, FUNC(clps6700_device::write_pcm_cd1));
	m_pccard->cd2().set(m_clps6700, FUNC(clps6700_device::write_pcm_cd2));
	m_pccard->bvd1().set(m_clps6700, FUNC(clps6700_device::write_pcm_bvd1));
	m_pccard->bvd2().set(m_clps6700, FUNC(clps6700_device::write_pcm_bvd2));
	m_pccard->wp().set(m_clps6700, FUNC(clps6700_device::write_pcm_wp));
}


ROM_START( osaris )
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "209", "V1.02(209)")
	ROMX_LOAD("osaris_uk_v209.rom", 0x000000, 0x800000, CRC(2ea9ff1e) SHA1(8a346f8279b0aef50bd5f2c62a71fa59e53b8318), ROM_BIOS(0))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( osaris_fr )
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "209", "V1.02(209)")
	ROMX_LOAD("osaris_fr_209.rom", 0x000000, 0x800000, CRC(9cbe31d0) SHA1(5258bc5b4138ff530be31a76796913d9ae1e414c), ROM_BIOS(0))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

} // anonymous namespace


//    YEAR  NAME        PARENT     COMPAT  MACHINE    INPUT      CLASS          INIT          COMPANY               FULLNAME              FLAGS
COMP( 1999, osaris,     0,         0,      osaris,    osaris,    osaris_state,  init_osaris,  "Oregon Scientific",  "Osaris",             MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2000, osaris_fr,  osaris,    0,      osaris,    osaris_fr, osaris_state,  init_osaris,  "Oregon Scientific",  "Osaris (French)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
