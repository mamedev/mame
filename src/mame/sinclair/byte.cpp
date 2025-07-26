// license:BSD-3-Clause
// copyright-holders:Andrei I. Holub
/**********************************************************************
    PEVM Byte

Refs:
    https://web.archive.org/web/20241003085933/https://zxbyte.ru/index_en.htm

TODO:
- Kempston joystick
- Byte-01?

**********************************************************************/

#include "emu.h"

#include "spectrum.h"

#include "machine/pit8253.h"

namespace {

#define DD66_ROM_REGION  "dd66_rom"
#define DD71_ROM_REGION  "dd71_rom"

class byte_state : public spectrum_state
{
public:
	byte_state(const machine_config &mconfig, device_type type, const char *tag)
		: spectrum_state(mconfig, type, tag)
		, m_pit(*this, "pit")
		, m_dd66_rom(*this, DD66_ROM_REGION)
		, m_dd71_rom(*this, DD71_ROM_REGION)
		, m_io_comp(*this, "COMP")
		, m_io_line(*this, "IO_LINE%u", 0U)
	{ }

	void byte(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void map_io(address_map &map) ATTR_COLD;

	virtual u8 spectrum_rom_r(offs_t offset) override;
	u8 kbd_fe_r(offs_t offset);

private:
	required_device<pit8253_device> m_pit;
	required_region_ptr<u8> m_dd66_rom;
	required_region_ptr<u8> m_dd71_rom;

	required_ioport m_io_comp;
	required_ioport_array<8> m_io_line;

	bool m_1f_gate;
};

u8 byte_state::spectrum_rom_r(offs_t offset)
{
	if (m_1f_gate)
	{
		const u16 adr66 = ((offset >> 7) & 0xff) | ((m_io_comp->read() & 1) << 8);
		const u8 dat66 = m_dd66_rom[adr66];
		if (~dat66 & 0x10)
		{
			u16 adr71 = ((dat66 & 0x0f) << 7) | (offset & 0x7f);
			return m_dd71_rom[adr71];
		}
	}

	return spectrum_state::spectrum_rom_r(offset);
}

void byte_state::map_io(address_map &map)
{
	spectrum_state::spectrum_clone_io(map);
	map(0x0000, 0x0000).select(0xfffe).rw(FUNC(byte_state::kbd_fe_r), FUNC(byte_state::spectrum_ula_w));

	// #8e-ch0, #ae-ch1, #ce-ch2, #ee-ctrl
	map(0x0004, 0x0004).select(0xffea).lw8(NAME([this](offs_t offset, u8 data) { m_pit->write(BIT(offset, 5, 2), data); } ));
	map(0x0015, 0x0015).mirror(0xff8a).lr8(NAME([this]() { m_1f_gate = true; return 0xff; })); // #1f
}

u8 byte_state::kbd_fe_r(offs_t offset)
{
	if (!machine().side_effects_disabled())
	{
		if (is_contended(offset)) content_early();
		content_early(1);
	}

	u8 lines = offset >> 8;
	u8 data = 0xff;
	bool cs_on = false;
	bool ss_on = false;

	for (auto i = 0; i < 8; i++)
	{
		u32 tmp = m_io_line[i]->read();
		if (~lines & 1)
			data &= tmp;

		tmp >>= 8;
		cs_on |= (tmp & 0xff) != 0xff;
		if (~lines & 1)
			data &= tmp;

		tmp >>= 8;
		ss_on |= (tmp & 0xff) != 0xff;
		if (~lines & 1)
			data &= tmp;

		lines >>= 1;
	}

	if (cs_on && (~offset & 0x0100))
		data &= ~0x01; // CS

	if (ss_on && (~offset & 0x8000))
		data &= ~0x02; // SS

	data = data | 0x40;
	if (m_cassette->input() > 0.0038 )
	{
		data &= ~0x40;
	}

	return data;
}

INPUT_PORTS_START(byte)

	PORT_START("IO_LINE0") /* 0xFEFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ПРОПИСНЫЕ")       PORT_CODE(KEYCODE_LSHIFT)   PORT_CHAR(UCHAR_SHIFT_1)
																				   PORT_CODE(KEYCODE_RSHIFT)   PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Я  Z  :")         PORT_CODE(KEYCODE_Z)        PORT_CHAR('z') PORT_CHAR('Z') PORT_CHAR(':')
	PORT_BIT(0x020000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SS+Z) :")          PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ч  X  $")         PORT_CODE(KEYCODE_X)        PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR('$')
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"С  C  ?")         PORT_CODE(KEYCODE_C)        PORT_CHAR('c') PORT_CHAR('C') PORT_CHAR('?')
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"М  V  /")         PORT_CODE(KEYCODE_V)        PORT_CHAR('v') PORT_CHAR('V') PORT_CHAR('/')
	PORT_BIT(0x000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ю  ПФ6")          PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0xfdff60, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE1") /* 0xFDFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ф  A  ~")         PORT_CODE(KEYCODE_A)        PORT_CHAR('a') PORT_CHAR('A') PORT_CHAR('~')
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ы  S  |")         PORT_CODE(KEYCODE_S)        PORT_CHAR('s') PORT_CHAR('S') PORT_CHAR('|')
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"В  D  \\")        PORT_CODE(KEYCODE_D)        PORT_CHAR('d') PORT_CHAR('D') PORT_CHAR('\\')
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"А  F  {")         PORT_CODE(KEYCODE_F)        PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"П  G  }")         PORT_CODE(KEYCODE_G)        PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
	PORT_BIT(0x000020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ЛАТ")             PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Э  ПФ4")          PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0xffff40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE2") /* 0xFBFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Й  Q  <=")        PORT_CODE(KEYCODE_Q)        PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ц  W  <>")        PORT_CODE(KEYCODE_W)        PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"У  E  >=")        PORT_CODE(KEYCODE_E)        PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"К  R  <")         PORT_CODE(KEYCODE_R)        PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('<')
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Е  T  >")         PORT_CODE(KEYCODE_T)        PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR('>')
	PORT_BIT(0xffffe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE3") /* 0xF7FE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1  !")              PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x000100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+1) РЕД")      PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  @")              PORT_CODE(KEYCODE_2)        PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x000200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+2) ПР/СТ")    PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3  #")              PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x000400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+3) ПОЗИТ")    PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $")              PORT_CODE(KEYCODE_4)        PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x000800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+4) НЕГАТ")    PORT_CODE(KEYCODE_END)
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %")              PORT_CODE(KEYCODE_5)        PORT_CHAR('5') PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CHAR('%')
	PORT_BIT(0x001000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+5) \u2190")   PORT_CODE(KEYCODE_LEFT)  // ←
	PORT_BIT(0xffe0e0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE4") /* 0xEFFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0  _")              PORT_CODE(KEYCODE_0)        PORT_CHAR('0') PORT_CHAR(8) PORT_CHAR('_')
	PORT_BIT(0x000100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+0) УДАЛЕНИЕ") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9  )")              PORT_CODE(KEYCODE_9)        PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x000200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+9) ГРАФ")     PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8  (")              PORT_CODE(KEYCODE_8)        PORT_CHAR('8') PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CHAR('(')
	PORT_BIT(0x000400, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+8) \u2192")   PORT_CODE(KEYCODE_RIGHT) // →
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7  '")              PORT_CODE(KEYCODE_7)        PORT_CHAR('7') PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CHAR('\'')
	PORT_BIT(0x000800, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+7) \u2191")   PORT_CODE(KEYCODE_UP)    // ↑
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6  &")              PORT_CODE(KEYCODE_6)        PORT_CHAR('6') PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CHAR('&')
	PORT_BIT(0x001000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"(CS+6) \u2193")   PORT_CODE(KEYCODE_DOWN)  // ↓
	PORT_BIT(0x000020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"РУС")             PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ъ  ПФ2")          PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0xffe040, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE5") /* 0xDFFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"З  P  \"  ©")     PORT_CODE(KEYCODE_P)        PORT_CHAR('p') PORT_CHAR('P') PORT_CHAR('"')
	PORT_BIT(0x010000, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("(SS+P) \"")         PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Щ  O  ;")         PORT_CODE(KEYCODE_O)        PORT_CHAR('o') PORT_CHAR('O') PORT_CHAR(';')
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ш  I")            PORT_CODE(KEYCODE_I)        PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Г  U  ]")         PORT_CODE(KEYCODE_U)        PORT_CHAR('u') PORT_CHAR('U') PORT_CHAR(']')
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Н  Y  [")         PORT_CODE(KEYCODE_Y)        PORT_CHAR('y') PORT_CHAR('Y') PORT_CHAR('[')
	PORT_BIT(0x000020, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ЛАТ/РУС")         PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Х  ПФ1")          PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0xfeff40, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE6") /* 0xBFFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ВВОД")            PORT_CODE(KEYCODE_ENTER)    PORT_CHAR(13)
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Д  L  =")         PORT_CODE(KEYCODE_L)        PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('=')
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Л  K  +")         PORT_CODE(KEYCODE_K)        PORT_CHAR('k') PORT_CHAR('K') PORT_CHAR('+')
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"О  J  -")         PORT_CODE(KEYCODE_J)        PORT_CHAR('j') PORT_CHAR('J') PORT_CHAR('-')
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Р  H  ^")         PORT_CODE(KEYCODE_H)        PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^')
	PORT_BIT(0x000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ж  ПФ3")          PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0xffff60, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("IO_LINE7") /* 0x7FFE */
	PORT_BIT(0x000001, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ПРОБЕЛ")          PORT_CODE(KEYCODE_SPACE)    PORT_CHAR(' ')
	PORT_BIT(0x000100, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ОСТАНОВ")         PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x000002, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"РЕГ1")            PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
																				   PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))
	PORT_BIT(0x000200, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"РЕГ2")            PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x000004, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ь  M  .")         PORT_CODE(KEYCODE_M)        PORT_CHAR('m') PORT_CHAR('M') PORT_CHAR('.')
	PORT_BIT(0x000008, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Т  N  ,")         PORT_CODE(KEYCODE_N)        PORT_CHAR('n') PORT_CHAR('N') PORT_CHAR(',')
	PORT_BIT(0x000010, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"И  B  *")         PORT_CODE(KEYCODE_B)        PORT_CHAR('b') PORT_CHAR('B') PORT_CHAR('*')
	PORT_BIT(0x000080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Б  ПФ5")          PORT_CODE(KEYCODE_COMMA)
	PORT_BIT(0xfffc60, IP_ACTIVE_LOW, IPT_UNUSED)


	PORT_START("COMP")
	PORT_CONFNAME( 0x01, 0x01, "Compatibility")
	PORT_CONFSETTING(    0x00, "On (Sinclair)")
	PORT_CONFSETTING(    0x01, "Off (Byte)")
INPUT_PORTS_END

void byte_state::machine_start()
{
	spectrum_state::machine_start();

	// Save
	save_item(NAME(m_1f_gate));
}

void byte_state::machine_reset()
{
	spectrum_state::machine_reset();

	m_1f_gate = false;
}


void byte_state::byte(machine_config &config)
{
	spectrum_state::spectrum_clone(config);

	PIT8253(config, m_pit, 0); // КР580ВИ53
	m_pit->set_clk<0>(20_MHz_XTAL / 10);
	m_pit->out_handler<0>().set([this](int state) { m_speaker->level_w(state); });
	m_pit->set_clk<1>(20_MHz_XTAL / 10);
	m_pit->out_handler<1>().set([this](int state) { m_speaker->level_w(state); });
	m_pit->set_clk<2>((20_MHz_XTAL / 10));
	m_pit->out_handler<2>().set([this](int state) { m_speaker->level_w(state); });

	m_maincpu->set_io_map(&byte_state::map_io);
	m_exp->fb_r_handler().set([]() { return 0xff; });
}

ROM_START(byte)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_DEFAULT_BIOS("prusak")

	ROM_SYSTEM_BIOS(0, "v1", "V1")
	ROMX_LOAD("byte.rom", 0x0000, 0x4000, CRC(c13ba473) SHA1(99f40727185abbb2413f218d69df021ae2e99e45), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "prusak", "Prusak's")
	ROMX_LOAD("dd72.bin", 0x0000, 0x2000, CRC(2464d537) SHA1(e8b4a468e6f254f090fc5b2c59ea573c2a4f0455), ROM_BIOS(1))
	ROMX_LOAD("dd73.bin", 0x2000, 0x2000, CRC(bd430288) SHA1(b4c67a6213b1ecfa37cc476bc483e8a8deef6149), ROM_BIOS(1))

	ROM_REGION(0x800, DD71_ROM_REGION, ROMREGION_ERASEFF)
	ROM_LOAD("dd71_rt7.bin", 0x000, 0x800, CRC(c91b07c2) SHA1(2365d45b028b1e91dffb5bcdc87bd26ca9a7c26f))

	ROM_REGION(0x200, DD66_ROM_REGION, ROMREGION_ERASEFF)
	ROM_LOAD("dd66_rt5.bin", 0x000, 0x200, CRC(f8f9766a) SHA1(3f5345763a30e5370199c454301de655e7f1a1da))

	ROM_REGION(0x600, "tbd", ROMREGION_ERASEFF)
	ROM_LOAD("dd10_rt5.reva.bin", 0x000, 0x200, CRC(aae13e3e) SHA1(46f0ca97ceee0c591277aaac8b0cecc445927690)) // SN 1..7599 - 1989..1990
	ROM_LOAD("dd10_rt5.revb.bin", 0x200, 0x200, CRC(b649b5d1) SHA1(2d067962b08aee8cdf1bc4f5ce337815dd9d6c66)) // SN 7600..  - 1991..1996
	ROM_LOAD("dd11_rt5.bin", 0x400, 0x200, CRC(0f32b304) SHA1(d7adf9861c332510ff3682a1b06e6d9898343b6d))
ROM_END

} // Anonymous namespace

//    YEAR  NAME      PARENT    COMPAT  MACHINE INPUT CLASS       INIT           COMPANY FULLNAME     FLAGS
COMP( 1990, byte,     spectrum, 0,      byte,   byte, byte_state, init_spectrum, "BEMZ", "PEVM Byte", 0 )
//COMP( 1993, byte01,   ...
