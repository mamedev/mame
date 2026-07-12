// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, Nigel Barnes
/***************************************************************************

        Psion Series 5mx (EPOC R5)

        Driver by Ryan Holtz, ported from work by Ash Wolf

        More info: https://github.com/Treeki/WindEmu

        TODO:
        - Audio (MSM7717)
        - UART support (Windermere)
        - Compact Flash (Etna)
        - Bezel artwork

****************************************************************************/

#include "emu.h"
#include "codec.h"
#include "etna.h"
#include "windermere.h"

#include "bus/pccard/ataflash.h"
#include "cpu/arm7/arm7.h"
#include "imagedev/snapquik.h"
//#include "machine/bq2018.h"
#include "machine/eepromser.h"
#include "machine/intelfsh.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class psion5mx_state : public driver_device
{
public:
	psion5mx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_windermere(*this, "windermere")
		, m_etna(*this, "etna")
		, m_pccard(*this, "pccard")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_eeprom(*this, "eeprom")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_buzzer(*this, "buzzer")
		, m_codec(*this, "codec")
		, m_mic(*this, "mic")
		, m_touchx(*this, "TOUCHX")
		, m_touchy(*this, "TOUCHY")
		, m_touch(*this, "TOUCH")
		, m_kbd_cols(*this, "COL%u", 0U)
	{
	}

	void windermere(machine_config &config) ATTR_COLD;
	void psion5mx(machine_config &config) ATTR_COLD;
	void psion5mxp(machine_config &config) ATTR_COLD;
	void revo(machine_config &config) ATTR_COLD;
	void revoplus(machine_config &config) ATTR_COLD;

	void init_s5mx() ATTR_COLD;
	void init_mc218() ATTR_COLD;
	void init_revo() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(touch_down);

	DECLARE_QUICKLOAD_LOAD_MEMBER(quickload_cb);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void palette_init(palette_device &palette) const ATTR_COLD;

	void s5mx_map(address_map &map) ATTR_COLD;
	void s5mxp_map(address_map &map) ATTR_COLD;
	void revo_map(address_map &map) ATTR_COLD;

	void init_eeprom(std::string type, uint8_t locale = 0xff, uint8_t lang = 0xff);

	uint8_t keyboard_r();
	void portb_w(uint8_t data);
	void portd_w(uint8_t data);
	uint16_t ads7843_r(offs_t offset);
	uint8_t etna_porta_r();
	void etna_porta_w(uint8_t data);
	void update_amp();

	required_device<arm710t_cpu_device> m_maincpu;
	required_device<windermere_device> m_windermere;
	optional_device<etna_device> m_etna;
	optional_device<pccard_slot_device> m_pccard;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_buzzer;
	required_device<psion_codec_device> m_codec;
	optional_device<microphone_device> m_mic;
	required_ioport m_touchx;
	required_ioport m_touchy;
	required_ioport m_touch;
	required_ioport_array<8> m_kbd_cols;

	uint8_t m_kbd_scan = 0;
	uint8_t m_volume = 0;
	bool m_amp_enable = true;

	uint16_t m_main_battery   = 3100;
	uint16_t m_backup_battery = 3100;
};


void psion5mx_state::init_eeprom(std::string machine, uint8_t locale, uint8_t lang)
{
	uint8_t *eeprom = memregion("eeprom")->base();

	// defaults expected by the touchscreen code
	eeprom[0x0a] = 20;
	eeprom[0x0b] = 20;
	eeprom[0x0c] = 20;
	eeprom[0x0d] = 30;

	// machine serial
	eeprom[0x18] = 0xef;
	eeprom[0x19] = 0xbe;
	eeprom[0x1a] = 0xad;
	eeprom[0x1b] = 0xde;

	// machine type
	eeprom[0x28] = machine.length();
	if (machine.length() > 0)
	{
		const char *key = "PSIONPSIONPSION";
		for (int i = 0; i < 16; i++)
			eeprom[0x29 + i] = (i < machine.length()) ? machine[i] ^ key[i] : key[i];
	}

	// language (5mx Pro only)
	if (lang != 0xff)
	{
		eeprom[0x38] = lang;
		eeprom[0x39] = lang;
	}

	// localisation (Revo only)
	if (locale != 0xff)
	{
		eeprom[0x3a] = locale;
		eeprom[0x3b] = locale;
	}

	// calculate the checksum
	uint8_t chksum = 0;
	for (int i = 1; i < 0x80; i++)
		chksum ^= eeprom[i];

	// EPOC is expecting 0x42
	eeprom[0x00] = chksum ^ 0x42;
}

void psion5mx_state::init_s5mx()
{
	const std::string &name = machine().basename();
	uint8_t lang = 0x00;

	if (name == "psion5mxp_de") lang = 0x03; // German bootloader (other languages are supported)

	init_eeprom("SERIES5mx", 0xff, lang);
}

void psion5mx_state::init_mc218()
{
	const std::string &name = machine().basename();
	uint8_t lang = 0x00;

	if (name == "mc218") lang = 0x01; // not sure why UK version sets this

	init_eeprom("Ericsson MC 218", 0xff, lang);
}

void psion5mx_state::init_revo()
{
	const std::string &name = machine().basename();
	uint8_t locale = 0xf0; // default locale

	if (name == "mako") locale = 0xf2; // set locale to USA

	init_eeprom("", locale); // machine 'REVO' is read from ROM

	m_main_battery   = 4000;
	m_backup_battery = 2000;
}


void psion5mx_state::machine_start()
{
	// install ROM (mirroring is essential for size detection)
	if (!memregion("flash")) // not PRO
	{
		uint32_t *rom = &memregion("maincpu")->as_u32();
		switch (memregion("maincpu")->bytes())
		{
		case 0x1000000: // 16MB region (Series 5mx / MC218)
			// install CS0
			m_maincpu->space(AS_PROGRAM).install_rom(0x00000000, 0x007fffff, 0x0f800000, &rom[0]);

			// install CS1
			if (rom[0x3fffff] == 0xffffffff)      // 16MB ROM (8MB + 8MB MaskROM)
				m_maincpu->space(AS_PROGRAM).install_rom(0x10000000, 0x107fffff, 0x0f800000, &rom[0x200000]);
			else if (rom[0x2fffff] == 0xffffffff) // 12MB ROM (8MB + 4MB MaskROM)
				m_maincpu->space(AS_PROGRAM).install_rom(0x10000000, 0x103fffff, 0x0fc00000, &rom[0x200000]);
			else if (rom[0x27ffff] == 0xffffffff) // 10MB ROM (8MB MaskROM + 2MB Flash)
				m_maincpu->space(AS_PROGRAM).install_rom(0x10000000, 0x101fffff, 0x0fe00000, &rom[0x200000]);
			break;

		case 0x0800000: // 8MB region (Revo)
			// install CS0
			m_maincpu->space(AS_PROGRAM).install_rom(0x00000000, 0x003fffff, 0x0fc00000, &rom[0]);

			// install CS1
			m_maincpu->space(AS_PROGRAM).install_rom(0x10000000, 0x103fffff, 0x0fc00000, &rom[0x100000]);
			break;
		}
	}

	// install RAM
	m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc0000000 + m_ram->mask(), 0x0fffffff ^ m_ram->mask(), m_ram->pointer());

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	save_item(NAME(m_kbd_scan));
	save_item(NAME(m_volume));
	save_item(NAME(m_amp_enable));
}

void psion5mx_state::machine_reset()
{
	m_kbd_scan = 0;
}


uint8_t psion5mx_state::keyboard_r()
{
	uint8_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbd_scan, i))
			data |= m_kbd_cols[i]->read();
	}

	return data;
}


void psion5mx_state::portb_w(uint8_t data)
{
	m_eeprom->cs_write(BIT(data, 0));
	m_eeprom->clk_write(BIT(data, 1));
}

void psion5mx_state::portd_w(uint8_t data)
{
	m_codec->pdn_w(BIT(data, 0)); // CODEN

	m_amp_enable = BIT(data, 1); // AMPEN
	update_amp();
}


uint16_t psion5mx_state::ads7843_r(offs_t offset)
{
	uint16_t data = 0;

	// TODO: Proper SSP support with ADS7843 device
	switch (offset)
	{
	case 0xd0d3: // Touch X
		data = 50 + (uint16_t)(m_touchx->read() * 5.7);
		break;
	case 0x9093: // Touch Y
		data = 3834 - (uint16_t)(m_touchy->read() * 13.225);
		break;
	case 0xa4a4: // Main Battery
		data = m_main_battery;
		break;
	case 0xe4e4: // Backup Battery
		data = m_backup_battery;
		break;
	}

	return data;
}


uint8_t psion5mx_state::etna_porta_r()
{
	uint8_t data = 0x00;

	data |= m_eeprom->do_read() << 3;

	return data;
}

void psion5mx_state::etna_porta_w(uint8_t data)
{
	m_volume = data & 3;
	update_amp();

	m_eeprom->di_write(BIT(data, 2));
}


void psion5mx_state::update_amp()
{
	// TODO: MSC1192 speaker amplifier, can be put into standby to mute audio.
	static const float codec_volume[4] = { 1.0f, 0.75f, 0.5f, 0.25f };

	if (m_amp_enable)
		m_codec->set_output_gain(ALL_OUTPUTS, codec_volume[m_volume]); // VOL
	else
		m_codec->set_output_gain(ALL_OUTPUTS, 0.0);
}


void psion5mx_state::s5mx_map(address_map &map)
{
	map(0x20000000, 0x20000fff).rw(m_etna, FUNC(etna_device::regs_r), FUNC(etna_device::regs_w));
	map(0x40000000, 0x43ffffff).rw(m_pccard, FUNC(pccard_slot_device::read_reg), FUNC(pccard_slot_device::write_reg));
	map(0x44000000, 0x47ffffff).rw(m_pccard, FUNC(pccard_slot_device::read_memory), FUNC(pccard_slot_device::write_memory));
	map(0x80000000, 0x80000fff).rw(m_windermere, FUNC(windermere_device::periphs_r), FUNC(windermere_device::periphs_w));
}

void psion5mx_state::s5mxp_map(address_map &map)
{
	map(0x00000000, 0x0001ffff).rw("flash", FUNC(intelfsh8_device::read), FUNC(intelfsh8_device::write));
	map(0x20000000, 0x20000fff).rw(m_etna, FUNC(etna_device::regs_r), FUNC(etna_device::regs_w));
	map(0x40000000, 0x43ffffff).rw(m_pccard, FUNC(pccard_slot_device::read_reg), FUNC(pccard_slot_device::write_reg));
	map(0x44000000, 0x47ffffff).rw(m_pccard, FUNC(pccard_slot_device::read_memory), FUNC(pccard_slot_device::write_memory));
	map(0x80000000, 0x80000fff).rw(m_windermere, FUNC(windermere_device::periphs_r), FUNC(windermere_device::periphs_w));
}

void psion5mx_state::revo_map(address_map &map)
{
	map(0x80000000, 0x80000fff).rw(m_windermere, FUNC(windermere_device::periphs_r), FUNC(windermere_device::periphs_w));
}


void psion5mx_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		const int r = (0x99 * i) / 15;
		const int g = (0xaa * i) / 15;
		const int b = (0x88 * i) / 15;
		m_palette->set_pen_color(15 - i, rgb_t(r, g, b));
	}
}


INPUT_CHANGED_MEMBER(psion5mx_state::touch_down)
{
	m_windermere->eint3_w(newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_PORTS_START( psion5mx )
	PORT_START("TOUCHX")
	PORT_BIT(0x3ff, 362, IPT_LIGHTGUN_X) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0,694) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCHY")
	PORT_BIT(0x1ff, 125, IPT_LIGHTGUN_Y) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_MINMAX(0,279) PORT_SENSITIVITY(25) PORT_KEYDELTA(13)

	PORT_START("TOUCH")
	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(psion5mx_state::touch_down), 0)
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('^') PORT_CHAR('>')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)         PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR('<')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)         PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)         PORT_CHAR('3') PORT_CHAR(163) PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)         PORT_CHAR('2') PORT_CHAR('"') PORT_CHAR('#')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)         PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)    PORT_NAME("Dictaphone Record")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)     PORT_CHAR('\'') PORT_CHAR('~') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)   PORT_NAME("Del<-")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR(')') PORT_CHAR('}')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR('(') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('*') PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('&') PORT_CHAR('[')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)      PORT_NAME("Dictaphone Play")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)         PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)         PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)         PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)         PORT_CHAR('e') PORT_CHAR('E') PORT_CHAR(128)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)         PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)         PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)       PORT_CHAR(27)  PORT_NAME("Esc")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)  PORT_NAME("Enter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)         PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)         PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)         PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR('-')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)         PORT_CHAR('i') PORT_CHAR('I') PORT_CHAR('+')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)         PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)       PORT_NAME("Menu")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)         PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)         PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)         PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)         PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)         PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)       PORT_CHAR(9)   PORT_NAME("Tab  Caps")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)  PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)      PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)      PORT_CHAR('.') PORT_CHAR('?')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)         PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)         PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)         PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)         PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)      PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Fn")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)         PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)         PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)         PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)         PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)         PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)         PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)    PORT_NAME("Right Shift")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)     PORT_CHAR(UCHAR_MAMEKEY(RIGHT))  PORT_NAME(u8"\u2192") // U+2192 = →
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)      PORT_CHAR(UCHAR_MAMEKEY(LEFT))   PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)     PORT_CHAR(',') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)        PORT_CHAR(UCHAR_MAMEKEY(UP))     PORT_NAME(u8"\u2191") // U+2191 = ↑
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)     PORT_CHAR(' ')                   PORT_NAME("Space")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGUP)                                       PORT_NAME("Dictaphone Stop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)    PORT_CHAR(UCHAR_SHIFT_1)         PORT_NAME("Left Shift")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

INPUT_PORTS_START( psion5mx_de )
	PORT_INCLUDE(psion5mx)

	PORT_MODIFY("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('z') PORT_CHAR('Z')

	PORT_MODIFY("COL6")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')
INPUT_PORTS_END

INPUT_PORTS_START( psion5mx_fr )
	PORT_INCLUDE(psion5mx)

	PORT_MODIFY("COL2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('a') PORT_CHAR('A')

	PORT_MODIFY("COL4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('q') PORT_CHAR('Q')

	PORT_MODIFY("COL6")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('w') PORT_CHAR('W')
INPUT_PORTS_END

//INPUT_PORTS_START( psion5mx_us )
//  PORT_INCLUDE(psion5mx)
//
//  PORT_MODIFY("COL0")
//  PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR('/')
//  PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#') PORT_CHAR('\\')
//  PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@') PORT_CHAR('~')
//INPUT_PORTS_END

INPUT_PORTS_START( revo )
	PORT_INCLUDE(psion5mx)

	PORT_MODIFY("COL0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4')  PORT_CHAR('$') PORT_CHAR('~')

	PORT_MODIFY("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR('\'') PORT_CHAR('@') PORT_CHAR(':')
INPUT_PORTS_END

INPUT_PORTS_START( revo_us )
	PORT_INCLUDE(psion5mx)

	PORT_MODIFY("COL0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)     PORT_CHAR('4')  PORT_CHAR('$') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)     PORT_CHAR('3')  PORT_CHAR('#') PORT_CHAR('\\')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)     PORT_CHAR('2')  PORT_CHAR('@') PORT_CHAR('~')

	PORT_MODIFY("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(':') PORT_CHAR('"') PORT_CHAR(';')
INPUT_PORTS_END

INPUT_PORTS_START( psion618c )
	PORT_INCLUDE(revo_us)

	// TODO: Traditional Chinese keycaps
INPUT_PORTS_END


QUICKLOAD_LOAD_MEMBER(psion5mx_state::quickload_cb)
{
	// load the OS image as a ROM (this would normally be loaded from CF by the bootloader)
	uint8_t *rom = memregion("maincpu")->base();
	image.fread(rom, image.length());
	m_maincpu->space(AS_PROGRAM).install_rom(0x00000000, 0x00ffffff, 0x0f000000, rom);

	machine().schedule_soft_reset();

	return std::make_pair(std::error_condition(), std::string());
}


static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("cf", ATA_FLASH_PCCARD);
}


void psion5mx_state::windermere(machine_config &config)
{
	ARM710T(config, m_maincpu, 3.6864_MHz_XTAL * 10);
	m_maincpu->set_addrmap(AS_PROGRAM, &psion5mx_state::s5mx_map);

	WINDERMERE(config, m_windermere, 3.6864_MHz_XTAL, m_maincpu);
	m_windermere->lcd_dma_cb().set(m_ram, FUNC(ram_device::read));
	m_windermere->porta_r().set(FUNC(psion5mx_state::keyboard_r));
	m_windermere->portb_w().set(FUNC(psion5mx_state::portb_w));
	//m_windermere->portc_w().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::pre_write)).bit(7);
	m_windermere->portd_w().set(FUNC(psion5mx_state::portd_w));
	m_windermere->pcm_in().set(m_codec, FUNC(psion_codec_device::pcm_out));
	m_windermere->pcm_out().set(m_codec, FUNC(psion_codec_device::pcm_in));
	m_windermere->buz_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));
	m_windermere->col_cb().set([this](uint8_t data) { m_kbd_scan = data; });
	m_windermere->ssp_r().set(FUNC(psion5mx_state::ads7843_r));
	m_windermere->set_screen_origin(45, 5);
	m_windermere->set_screen("screen");

	//ADS7843(config, "adc");

	RAM(config, m_ram).set_default_size("16M");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(m_windermere, FUNC(windermere_device::screen_update));
	m_screen->set_size(695, 280); // PEN 695x280 LCD 640x240
	m_screen->set_visarea_full();
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(psion5mx_state::palette_init), 16);

	PSION_CODEC(config, m_codec, 8000).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: MSM7717

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer).add_route(ALL_OUTPUTS, "mono", 1.0);

	EEPROM_93C46_16BIT(config, m_eeprom); // 93CS46
}


void psion5mx_state::psion5mx(machine_config& config)
{
	windermere(config);

	ETNA(config, m_etna);
	m_etna->porta_r().set(FUNC(psion5mx_state::etna_porta_r));
	m_etna->porta_w().set(FUNC(psion5mx_state::etna_porta_w));

	PCCARD_SLOT(config, m_pccard, pcmcia_devices, "cf").set_fixed(true);
	//m_pccard->cd1().set(m_etna, FUNC(etna_device::write_pc1_cd1));
	//m_pccard->cd2().set(m_etna, FUNC(etna_device::write_pc1_cd2));
	//m_pccard->bvd1().set(m_etna, FUNC(etna_device::write_pc1_bvd1));
	//m_pccard->bvd2().set(m_etna, FUNC(etna_device::write_pc1_bvd2));
	//m_pccard->wp().set(m_etna, FUNC(etna_device::write_pc1_wp));

	MICROPHONE(config, m_mic, 1).front_center();
	m_mic->add_route(0, m_codec, 1.0);
}


void psion5mx_state::psion5mxp(machine_config &config)
{
	psion5mx(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion5mx_state::s5mxp_map);

	m_ram->set_default_size("32M");

	ATMEL_29C010(config, "flash"); // 29LV010

	quickload_image_device &quickload(QUICKLOAD(config, "quickload", "bin"));
	quickload.set_load_callback(FUNC(psion5mx_state::quickload_cb));
	quickload.set_interface("psion_quik");

	SOFTWARE_LIST(config, "quik_ls").set_original("psion_quik").set_filter("PRO");
}


void psion5mx_state::revo(machine_config &config)
{
	windermere(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &psion5mx_state::revo_map);

	m_windermere->set_screen_origin(37, 5);
	//m_windermere->portd_r().set("pm", FUNC(bq2018_device::hdq_r)).lshift(7);
	//m_windermere->portd_w().set("pm", FUNC(bq2018_device::hdq_w)).bit(7);
	m_windermere->porte_r().set(FUNC(psion5mx_state::etna_porta_r));
	m_windermere->porte_w().set(FUNC(psion5mx_state::etna_porta_w));

	m_ram->set_default_size("8M").set_extra_options("16M"); // Revo = 8MB, Revo Plus = 16MB

	m_screen->set_size(527, 208); // PEN 527x208 LCD 480x160
	m_screen->set_visarea_full();

	//BQ2018(config, "pm");
}


void psion5mx_state::revoplus(machine_config &config)
{
	revo(config);

	m_ram->set_default_size("16M");
}


ROM_START( psion5mx )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "260", "V1.05(260) 16M")
	ROMX_LOAD("s5mx_uk16_v260.rom", 0x0000000, 0x1000000, CRC(3b25a6a4) SHA1(8d67b5ed347bd6e202d6dda26c5752c87dc805e5), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "255", "V1.05(255) 16M")
	ROMX_LOAD("s5mx_uk16_v255.rom", 0x0000000, 0x1000000, CRC(c3a7486c) SHA1(9120d8f209d8763adfbab89a39c02cfd4ebf5c26), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "255a", "V1.05(255)")
	ROMX_LOAD("s5mx_uk10_v255.rom", 0x0000000, 0x0a00000, CRC(a1e2d038) SHA1(4c082321264e1ae7fe77699e59b8960460690fa6), ROM_BIOS(2))
	ROM_SYSTEM_BIOS(3, "250", "V1.05(250)")
	ROMX_LOAD("s5mx_uk10_v250.rom", 0x0000000, 0x0a00000, CRC(31f02326) SHA1(0ea2e3131f5cb2c105b4d5ec3bfafbf5e9e31a89), ROM_BIOS(3))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( psion5mx_fr )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "315", "V1.05(315) 16M")
	ROMX_LOAD("s5mx_fr16_v315.rom", 0x0000000, 0x1000000, CRC(8473d78c) SHA1(8e4284da7a2060d60c8dfc42357a730849f459cb), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "292", "V1.05(292)")
	ROMX_LOAD("s5mx_fr10_v292.rom", 0x0000000, 0x0a00000, CRC(36798a40) SHA1(2e84c690e462858c571f5651fc8e258043b4fe8b), ROM_BIOS(1))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( psion5mxp )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION(0x20000, "flash", 0)
	ROM_LOAD("s5mx_blge_v109.bin", 0x0000, 0x20000, BAD_DUMP CRC(95b6d2b0) SHA1(cd65a0dd904954759cf900b67f4e4edf59496c0a)) // UK version not verified as being same as German

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( psion5mxp_de )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)

	ROM_REGION(0x20000, "flash", 0)
	ROM_SYSTEM_BIOS(0, "109", "Bootloader V1.09")
	ROMX_LOAD("s5mx_blge_v109.bin", 0x0000, 0x20000, CRC(95b6d2b0) SHA1(cd65a0dd904954759cf900b67f4e4edf59496c0a), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "108", "Bootloader V1.08")
	ROMX_LOAD("s5mx_blge_v108.bin", 0x0000, 0x20000, CRC(18a7be84) SHA1(689dcaf69acdd2caead2d8d6c1f239a7444fdbf0), ROM_BIOS(1))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( mc218 )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)
	// Known missing versions: V1.05(257)
	ROM_SYSTEM_BIOS(0, "259", "V1.05(259)")
	ROMX_LOAD("mc218_uk12_v259.rom", 0x0000000, 0x0c00000, CRC(92f353b5) SHA1(f6ff73bdd59457e449f1faf95fe73878b3a94d8c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "256", "V1.05(256)")
	ROMX_LOAD("mc218_uk12_v256.rom", 0x0000000, 0x0c00000, CRC(616d0b54) SHA1(77dcd44dc6a26654f52c0ced07db0f505c896b48), ROM_BIOS(1))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( mc218_de )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "260", "V1.05(260)")
	ROMX_LOAD("mc218_de12_v260.rom", 0x0000000, 0x0c00000, CRC(6d4835ff) SHA1(917071175c6632f3ff7061ff93d30b681b58f3d6), ROM_BIOS(0))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( mc218_fr )
	ROM_REGION32_LE(0x1000000, "maincpu", ROMREGION_ERASE00)
	ROM_SYSTEM_BIOS(0, "262", "V1.05(262)")
	ROMX_LOAD("mc218_fr12_v262.rom", 0x0000000, 0x0c00000, CRC(e00f8b67) SHA1(5de1896615e916bb5ddf84820678c317fcbfb820), ROM_BIOS(0))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASE00)
ROM_END

ROM_START( revo )
	ROM_REGION32_LE(0x800000, "maincpu", 0)
	// Known missing versions: V1.06(320), V1.06(353)
	ROM_SYSTEM_BIOS(0, "390", "V1.06(390)")
	ROMX_LOAD("revo_ukus8_v390.rom", 0x000000, 0x800000, CRC(846c8176) SHA1(297c18621ea6c9440e74c71cc1cb58f21fe46796), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "361", "V1.06(361)")
	ROMX_LOAD("revo_ukus8_v361.rom", 0x000000, 0x800000, CRC(2418a352) SHA1(dc03eec968b1d503fd389999db3ef2070793b507), ROM_BIOS(1))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASEFF)
ROM_END

ROM_START( revo_de )
	ROM_REGION32_LE(0x800000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "391", "V1.06(391)")
	ROMX_LOAD("revo_de8_v391.rom", 0x000000, 0x800000, CRC(e627747f) SHA1(3f451b7b0e738ee581bf73e1db310605d407e218), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "369", "V1.06(369)")
	ROMX_LOAD("revo_de8_v369.rom", 0x000000, 0x800000, CRC(1935958f) SHA1(bfe02a1817fb98ef24176b09e1c67b81c7235473), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "352", "V1.06(352)")
	ROMX_LOAD("revo_de8_v352.rom", 0x000000, 0x800000, CRC(100f6567) SHA1(1b79f66b2cf8bc87304377112d9bcad4dd59891c), ROM_BIOS(2))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASEFF)
ROM_END

ROM_START( revo_fr )
	ROM_REGION32_LE(0x800000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "392", "V1.06(392)")
	ROMX_LOAD("revo_fr8_v392.rom", 0x000000, 0x800000, CRC(dc2806ee) SHA1(587ef391f45bc1de8ad141d8d20958029355ad2c), ROM_BIOS(0))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASEFF)
ROM_END

#define rom_mako rom_revo

ROM_START( psion618c )
	ROM_REGION32_LE(0x1000000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "14", "V1.08(14)")
	ROMX_LOAD("psion618c_v14.rom", 0x000000, 0x1000000, CRC(4691779d) SHA1(4653e7b1b126c45178e23153bc9e897587f0b8e0), ROM_BIOS(0))

	ROM_REGION16_LE(0x80, "eeprom", ROMREGION_ERASEFF)
ROM_END

} // anonymous namespace


//    YEAR  NAME           PARENT     COMPAT  MACHINE    INPUT         CLASS           INIT          COMPANY       FULLNAME                      FLAGS
COMP( 1999, psion5mx,      0,         0,      psion5mx,  psion5mx,     psion5mx_state, init_s5mx,    "Psion",      "Series 5mx",                 MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1999, psion5mx_fr,   psion5mx,  0,      psion5mx,  psion5mx_fr,  psion5mx_state, init_s5mx,    "Psion",      "Series 5mx (French)",        MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1999, psion5mxp,     0,         0,      psion5mxp, psion5mx,     psion5mx_state, init_s5mx,    "Psion",      "Series 5mx PRO",             MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1999, psion5mxp_de,  psion5mxp, 0,      psion5mxp, psion5mx_de,  psion5mx_state, init_s5mx,    "Psion",      "Series 5mx PRO (German)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1999, revo,          0,         0,      revo,      revo,         psion5mx_state, init_revo,    "Psion",      "Revo",                       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1999, revo_de,       revo,      0,      revo,      psion5mx_de,  psion5mx_state, init_revo,    "Psion",      "Revo (German)",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1999, revo_fr,       revo,      0,      revo,      psion5mx_fr,  psion5mx_state, init_revo,    "Psion",      "Revo (French)",              MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2000, mako,          revo,      0,      revoplus,  revo_us,      psion5mx_state, init_revo,    "SONICblue",  "Diamond Mako",               MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2000, mc218,         0,         0,      psion5mx,  psion5mx,     psion5mx_state, init_mc218,   "Ericsson",   "MC 218",                     MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2000, mc218_de,      mc218,     0,      psion5mx,  psion5mx_de,  psion5mx_state, init_mc218,   "Ericsson",   "MC 218 (German)",            MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2000, mc218_fr,      mc218,     0,      psion5mx,  psion5mx_fr,  psion5mx_state, init_mc218,   "Ericsson",   "MC 218 (French)",            MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 2001, psion618c,     revo,      0,      revo,      psion618c,    psion5mx_state, init_revo,    "Psion",      "Psion 618C",                 MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
