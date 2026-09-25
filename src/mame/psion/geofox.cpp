// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/***************************************************************************

    Geofox-One

    TODO:
    - need board photos to identify devices
    - fix CODEC for audio recording/playback
    - improve EEPROM implementation
    - Compact Flash (unknown interface)

****************************************************************************/

#include "emu.h"
#include "codec.h"
//#include "etna.h"

#include "bus/pccard/ataflash.h"
#include "cpu/arm7/arm7.h"
#include "machine/adc1213x.h"
#include "machine/clps7110.h"
#include "machine/eepromser.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class geofox_state : public driver_device
{
public:
	geofox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_soc(*this, "soc")
		//, m_etna(*this, "etna")
		, m_pccard(*this, "pccard")
		, m_ram(*this, "ram")
		, m_nvram(*this, "nvram")
		, m_eeprom(*this, "eeprom")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_buzzer(*this, "buzzer")
		, m_codec(*this, "codec")
		, m_mic(*this, "mic")
		, m_mouse_x(*this, "MOUSEX")
		, m_mouse_y(*this, "MOUSEY")
		, m_mouse_b(*this, "BUTTON")
		, m_kbd_cols(*this, "COL%u", 0U)
	{
	}

	void geofox(machine_config &config) ATTR_COLD;

	void init_geofox() ATTR_COLD;

	DECLARE_INPUT_CHANGED_MEMBER(mousepad_changed);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void palette_init(palette_device &palette) const ATTR_COLD;

	void geofox_map(address_map &map) ATTR_COLD;

	template <int N> uint8_t keyboard_r();
	uint16_t adc_r(offs_t offset);
	void mousepad_packet();
	//void update_amp();

	required_device<arm710a_cpu_device> m_maincpu;
	required_device<clps7110_device> m_soc;
	//required_device<etna_device> m_etna;
	required_device<pccard_slot_device> m_pccard;
	required_device<ram_device> m_ram;
	required_device<nvram_device> m_nvram;
	//required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_region_ptr<uint16_t> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<speaker_sound_device> m_buzzer;
	required_device<psion_codec_device> m_codec;
	required_device<microphone_device> m_mic;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_mouse_b;
	required_ioport_array<8> m_kbd_cols;

	uint8_t m_kbd_scan = 0;
	//uint8_t m_volume = 0;
	//bool m_amp_enable = true;

	uint8_t  m_mouse_last_x = 0;
	uint8_t  m_mouse_last_y = 0;
	uint32_t m_mouse_data = 0;
	uint8_t  m_mouse_pkt = 0;
};


void geofox_state::init_geofox()
{
	uint8_t *eeprom = memregion("eeprom")->base();
	const std::string &name = machine().basename();

	if (name == "geofox_us")
	{
		// language and keyboard for USA
		eeprom[0x04] = 01;
		eeprom[0x06] = 01;
	}

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


void geofox_state::machine_start()
{
	// install RAM
	m_maincpu->space(AS_PROGRAM).install_ram(0xc0000000, 0xc0000000 + m_ram->mask(), 0x0fffffff ^ m_ram->mask(), m_ram->pointer());

	m_nvram->set_base(m_ram->pointer(), m_ram->size());

	save_item(NAME(m_kbd_scan));
	//save_item(NAME(m_volume));
	//save_item(NAME(m_amp_enable));
	save_item(NAME(m_mouse_last_x));
	save_item(NAME(m_mouse_last_y));
	save_item(NAME(m_mouse_data));
	save_item(NAME(m_mouse_pkt));
}

void geofox_state::machine_reset()
{
	m_kbd_scan = 0;

	m_mouse_last_x = 0;
	m_mouse_last_y = 0;
	m_mouse_data = 0;
	m_mouse_pkt = 0;
}


template <int N> uint8_t geofox_state::keyboard_r()
{
	uint16_t data = 0x00;

	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_kbd_scan, i))
			data |= m_kbd_cols[i]->read();
	}

	return uint8_t(data >> (N * 8));
}

uint16_t geofox_state::adc_r(offs_t offset)
{
	uint16_t data = 0xffff;

	// TODO: Proper SSP support
	switch (offset & 0xf0)
	{
	case 0x80: // Mouse Pad
		if (m_mouse_pkt == 0)
		{
			// generate mouse packet data
			mousepad_packet();
		}
		data = (m_mouse_data >> (16 * m_mouse_pkt)) & 0xffff;
		m_mouse_pkt ^= 1;
		break;
	case 0xc0: // EEPROM
		data = m_eeprom[offset & 0x0f];
		break;
	}

	return data;
}

void geofox_state::mousepad_packet()
{
	uint8_t status = m_mouse_b->read();
	uint8_t mousex = m_mouse_x->read();
	uint8_t mousey = m_mouse_y->read();

	int16_t dx = mousex - m_mouse_last_x;
	int16_t dy = mousey - m_mouse_last_y;

	if (dx > 0x80)
		dx -= 0x100;
	else if (dx < -0x80)
		dx += 0x100;

	if (dy > 0x80)
		dy -= 0x100;
	else if (dy < -0x80)
		dy += 0x100;

	if (dx < 0) status |= 0x10;
	if (dy < 0) status |= 0x20;

	m_mouse_data = ((dy & 0xff) << 24) | (status << 8) | (dx & 0xff);

	m_mouse_last_x = mousex;
	m_mouse_last_y = mousey;
}

//void geofox_state::update_amp()
//{
//	// TODO: MSC1192 speaker amplifier, can be put into standby to mute audio.
//	static const float codec_volume[4] = { 1.0f, 0.75f, 0.5f, 0.25f };
//
//	if (m_amp_enable)
//		m_codec->set_output_gain(ALL_OUTPUTS, codec_volume[m_volume]); // VOL
//	else
//		m_codec->set_output_gain(ALL_OUTPUTS, 0.0);
//}


void geofox_state::geofox_map(address_map &map)
{
	map(0x00000000, 0x003fffff).mirror(0x0fc00000).rom().region("maincpu", 0);
	map(0x10000000, 0x103fffff).mirror(0x0fc00000).rom().region("maincpu", 0x400000);
	map(0x30000000, 0x30000000).lr8(NAME([]() { return 0x06; })); // power status (unknown device)
	//map(0x40000000, 0x40000fff).rw(m_etna, FUNC(etna_device::regs_r), FUNC(etna_device::regs_w)); // not confirmed
	map(0x80000000, 0x80001fff).rw(m_soc, FUNC(clps7110_device::periphs_r), FUNC(clps7110_device::periphs_w));
}


void geofox_state::palette_init(palette_device &palette) const
{
	for (int i = 0; i < 16; i++)
	{
		const int r = (0x99 * i) / 15;
		const int g = (0xaa * i) / 15;
		const int b = (0x88 * i) / 15;
		m_palette->set_pen_color(15 - i, rgb_t(r, g, b));
	}
}

INPUT_CHANGED_MEMBER(geofox_state::mousepad_changed)
{
	m_soc->eint2_w(newval ? ASSERT_LINE : CLEAR_LINE);
}

INPUT_PORTS_START( geofox )
	PORT_START("MOUSEX")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_X) PORT_NAME("Mouse Pad X") PORT_SENSITIVITY(100) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geofox_state::mousepad_changed), 0)

	PORT_START("MOUSEY")
	PORT_BIT(0xff, 0x00, IPT_MOUSE_Y) PORT_NAME("Mouse Pad Y") PORT_SENSITIVITY(100) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geofox_state::mousepad_changed), 0) PORT_REVERSE

	PORT_START("BUTTON")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Touch") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geofox_state::mousepad_changed), 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Menu")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(geofox_state::mousepad_changed), 0)
	PORT_BIT(0xfc, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("COL0")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_CHAR(27)            PORT_NAME("Esc")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_SHIFT_2) PORT_NAME("Fn")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ') PORT_NAME("Space")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))  PORT_NAME(u8"\u2190") // U+2190 = ←
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))  PORT_NAME(u8"\u2193") // U+2193 = ↓
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_NAME(u8"\u2192") // U+2192 = →
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))    PORT_NAME(u8"\u2191") // U+2191 = ↑

	PORT_START("COL1")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1) PORT_NAME("Left Shift")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('@')

	PORT_START("COL2")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR('\t') PORT_NAME("Tab")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')

	PORT_START("COL3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('#') PORT_CHAR('~')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("COL4")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR(U'£')
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)   PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_NAME("Right Shift")

	PORT_START("COL5")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_NAME("Backlight")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_NAME("Calendar")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_NAME("Word")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_NAME("Menu")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_PGDN)       PORT_NAME("Zoom")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_NAME("Connect  Hang Up")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_NAME("System  App")
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)  PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13) PORT_NAME("Enter")

	PORT_START("COL6")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_NAME("IrDA")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_NAME("Time")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_NAME("Spreadsheet")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_NAME("WWW")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD))
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)   PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')

	PORT_START("COL7")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_NAME("Mail")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_NAME("Calc")
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_NAME("Filer")
	PORT_BIT(0x008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_NAME("Extras  Help")
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_NAME("Calc C")
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD)                               PORT_NAME("Calc AC")
	PORT_BIT(0x040, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT(0x080, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))
	PORT_BIT(0x100, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x200, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x400, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)  PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(8) PORT_NAME("<- Del")
INPUT_PORTS_END

INPUT_PORTS_START( geofox_us )
	PORT_INCLUDE(geofox)

	PORT_MODIFY("COL1")
	PORT_BIT(0x800, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR('\'') PORT_CHAR('"')

	PORT_MODIFY("COL3")
	PORT_BIT(0x001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('`') PORT_CHAR('~')

	PORT_MODIFY("COL4")
	PORT_BIT(0x002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')
INPUT_PORTS_END


static void pcmcia_devices(device_slot_interface &device)
{
	device.option_add("cf", ATA_FLASH_PCCARD);
}


void geofox_state::geofox(machine_config &config)
{
	ARM710A(config, m_maincpu, 3.6864_MHz_XTAL * 5);
	m_maincpu->set_addrmap(AS_PROGRAM, &geofox_state::geofox_map);

	CLPS7110(config, m_soc, 3.6864_MHz_XTAL, m_maincpu);
	m_soc->lcd_dma_cb().set(m_ram, FUNC(ram_device::read));
	m_soc->porta_r().set(FUNC(geofox_state::keyboard_r<0>));
	m_soc->portb_r().set(FUNC(geofox_state::keyboard_r<1>)).mask(0x0f);
	//m_soc->portc_r().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	//m_soc->portc_w().set(m_eeprom, FUNC(eeprom_serial_93cxx_device::cs_write)).bit(3);
	//m_soc->portc_w().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::clk_write)).bit(2);
	//m_soc->portc_w().append(m_eeprom, FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	m_soc->pcm_in().set(m_codec, FUNC(psion_codec_device::pcm_out));
	m_soc->pcm_out().set(m_codec, FUNC(psion_codec_device::pcm_in));
	m_soc->buz_cb().set(m_buzzer, FUNC(speaker_sound_device::level_w));
	m_soc->col_cb().set([this](uint8_t data) { m_kbd_scan = data; });
	m_soc->adc_r().set(FUNC(geofox_state::adc_r));
	m_soc->set_screen("screen");

	//ADC12138(config, "adc"); // TODO: verify device

	RAM(config, m_ram).set_default_size("16M").set_extra_options("4M");
	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	//ETNA(config, m_etna);
	//m_etna->porta_r().set([this] () { logerror("%s: porta_r\n", machine().describe_context()); return 0; });

	PCCARD_SLOT(config, m_pccard, pcmcia_devices, nullptr);
	//m_pccard->cd1().set(m_etna, FUNC(etna_device::write_pc1_cd1));
	//m_pccard->cd2().set(m_etna, FUNC(etna_device::write_pc1_cd2));
	//m_pccard->bvd1().set(m_etna, FUNC(etna_device::write_pc1_bvd1));
	//m_pccard->bvd2().set(m_etna, FUNC(etna_device::write_pc1_bvd2));
	//m_pccard->wp().set(m_etna, FUNC(etna_device::write_pc1_wp));

	SCREEN(config, m_screen).set_lcd();
	m_screen->set_refresh_hz(58);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_screen_update(m_soc, FUNC(clps7110_device::screen_update));
	m_screen->set_size(640, 320); // PEN 640x320 LCD 640x320
	m_screen->set_visarea_full();
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(geofox_state::palette_init), 16);

	//EEPROM_93C06_16BIT(config, m_eeprom); // TODO: verify device

	PSION_CODEC(config, m_codec, 8000).add_route(ALL_OUTPUTS, "mono", 1.0); // TODO: verify device

	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_buzzer).add_route(ALL_OUTPUTS, "mono", 1.0);

	MICROPHONE(config, m_mic, 1).front_center();
	m_mic->add_route(0, m_codec, 1.0);
}


ROM_START( geofox )
	ROM_REGION32_LE(0x800000, "maincpu", ROMREGION_ERASEFF)
	ROM_SYSTEM_BIOS(0, "146", "V1.01(146)")
	ROMX_LOAD("geofox_ukus_v146.rom", 0x000000, 0x800000, CRC(b27d8bb1) SHA1(7729676aaa3b05ddff70ee2d0e9700a22092ec8d), ROM_BIOS(0))

	ROM_REGION16_LE(0x20, "eeprom", ROMREGION_ERASE00)
ROM_END

#define rom_geofox_us rom_geofox

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT       CLASS          INIT          COMPANY      FULLNAME            FLAGS
COMP( 1997, geofox,    0,      0,      geofox,    geofox,     geofox_state,  init_geofox,  "Geofox",    "Geofox-One",       MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
COMP( 1997, geofox_us, geofox, 0,      geofox,    geofox_us,  geofox_state,  init_geofox,  "Geofox",    "Geofox-One (US)",  MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
