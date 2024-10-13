// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Preliminary driver for Yeno Primus Expert mit Stimme.

    This learning computer is likely a German-localized version of some
    VTech product (possibly code-named "Mouse Toy" if the program ROM can
    be trusted), though the keyboard layout suggests that a French version
    might also have been contemplated.

    It includes a mouse port (not yet emulated) and a cartridge slot (no
    carts have been dumped, though at least one titled "SprachenExperte"
    was released; pinout may be identical to Yeno's PC-Logomax 2).

***************************************************************************/

#include "emu.h"

//#include "bus/generic/slot.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "video/hd61202.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

namespace {

class primusex_state : public driver_device
{
public:
	primusex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "hd61202")
		, m_rombank(*this, "rombank")
		, m_keys(*this, "KEY%u", 0U)
		, m_cursor(*this, "CURSOR")
	{ }

	void primusex(machine_config &config);

	ioport_value encoder_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void ppi1_pa_w(u8 data);
	void ppi1_pb_w(u8 data);
	u8 ppi1_pc_r();
	void ppi2_pa_w(u8 data);
	u8 ppi2_pb_r();
	void ppi2_pc_w(u8 data);
	void key_scan_w(u8 data);
	u8 keyboard_r();

	HD61202_UPDATE_CB(hd61202_update);

	INTERRUPT_GEN_MEMBER(periodic_int);

	void primusex_mem(address_map &map);
	void primusex_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<hd61202_device> m_lcdc;
	required_memory_bank m_rombank;
	required_ioport_array<8> m_keys;
	required_ioport m_cursor;

	bool m_nmi_enable = false;
	u8 m_key_scan = 0;
};

 
HD61202_UPDATE_CB(primusex_state::hd61202_update)
{
	if (lcd_on)
	{
		for (int y = 0; y < 6; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				int addr = y * 64 + x;
				for (int yi = 0; yi < 8; yi++)
				{
					int px = x;
					int py = y * 8 + yi;

					if (cliprect.contains(px, py))
						bitmap.pix(py, px) = BIT(ddr[addr & 0x1ff], yi);
				}
			}

			for (int x = 0; x < 16; x++)
			{
				int px = 64 + x;
				for (int yi = 0; yi < 8; yi++)
				{
					int addr = (6 + (x >> 3)) * 64 + y * 8 + yi;
					int py = y * 8 + yi;

					if (cliprect.contains(px, py))
						bitmap.pix(py, px) = BIT(ddr[addr & 0x1ff], x & 7);
				}
			}
		}
	}
	else
		bitmap.fill(0, cliprect);

	return 0;
}


void primusex_state::ppi1_pa_w(u8 data)
{
	// TODO
	logerror("%s: PPI1 port A = %02Xh\n", machine().describe_context(), data);
}

void primusex_state::ppi1_pb_w(u8 data)
{
	// TODO: bits 5, 7 also used
	m_rombank->set_entry(data & 0x1f);
	m_nmi_enable = BIT(data, 6);
}

void primusex_state::ppi2_pa_w(u8 data)
{
	// TODO
	logerror("%s: PPI2 port A = %02Xh\n", machine().describe_context(), data);
}

u8 primusex_state::ppi2_pb_r()
{
	// TODO
	return 0;
}

void primusex_state::ppi2_pc_w(u8 data)
{
	// TODO
	//logerror("%s: PPI2 port C = %02Xh\n", machine().describe_context(), data);
}

void primusex_state::key_scan_w(u8 data)
{
	m_key_scan = data; // active low
}

u8 primusex_state::keyboard_r()
{
	u8 data = 0xff;
	for (int i = 0; i < 8; i++)
		if (!BIT(m_key_scan, i))
			data &= m_keys[i]->read();
	return data;
}

void primusex_state::primusex_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).bankr("rombank");
	map(0x8000, 0x9fff).ram();
	map(0xc000, 0xc000).rw(m_lcdc, FUNC(hd61202_device::status_r), FUNC(hd61202_device::control_w));
	map(0xc001, 0xc001).rw(m_lcdc, FUNC(hd61202_device::data_r), FUNC(hd61202_device::data_w));
}

void primusex_state::primusex_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x20).w(FUNC(primusex_state::key_scan_w));
	map(0x40, 0x40).r(FUNC(primusex_state::keyboard_r));
	map(0x60, 0x63).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


ioport_value primusex_state::encoder_r()
{
	ioport_value cursor_keys = m_cursor->read() ^ 0xff;
	if (cursor_keys == 0)
		return 0xf;
	else
		return 31 - count_leading_zeros_32(cursor_keys);
}

INPUT_PORTS_START(primusex)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u2190  Demo") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR(0x00a8) // dead key
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('$') PORT_CHAR('0')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR(';') PORT_CHAR('8')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('\'') PORT_CHAR('6')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR(0x00e7) PORT_CHAR('4')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR(0x00e9) PORT_CHAR('2')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("Unknown 07")

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ñ")  PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(0x00d1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_MINUS) PORT_CHAR('<') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_9) PORT_CHAR(',') PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_7) PORT_CHAR('"') PORT_CHAR('7')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_5) PORT_CHAR(0x00df) PORT_CHAR('5')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_3) PORT_CHAR(0x00e8) PORT_CHAR('3')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_1) PORT_CHAR(0x00e0) PORT_CHAR('1')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2) PORT_NAME("Unknown 17")

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Neuer Name  Entfernen") PORT_CODE(KEYCODE_END)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ü")  PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR(0x00fc) PORT_CHAR(0x00dc)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("O")    PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("U")    PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("T")    PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("E")    PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Q")    PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Antwort") PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("Multiple Choice")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                 PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(0x00b4) PORT_CHAR('`') // dead key
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("P")  PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("I")  PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Z")  PORT_CODE(KEYCODE_Y) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("R")  PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("W")  PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("A")  PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ä")  PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00e4) PORT_CHAR(0x00c4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("L")    PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("J")    PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G")    PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("D")    PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("X")    PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("S")    PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("Unknown 47")

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21b5") PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(0x0d)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"Ö")  PORT_CODE(KEYCODE_COLON) PORT_CHAR(0x00f6) PORT_CHAR(0x00d6)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                   PORT_CODE(KEYCODE_STOP) PORT_CHAR('-') PORT_CHAR('/')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("K")    PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("H")    PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F")    PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("C")    PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("Unknown 57")

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7) PORT_NAME("Unknown 60")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                 PORT_CODE(KEYCODE_SLASH) PORT_CHAR('*') PORT_CHAR('=')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                 PORT_CODE(KEYCODE_COMMA) PORT_CHAR('+') PORT_CHAR('.')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("M")  PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("N")  PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("B")  PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("V")  PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Y")  PORT_CODE(KEYCODE_Z) PORT_CHAR('y') PORT_CHAR('Y')

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8) PORT_NAME("Unknown 70")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9) PORT_NAME("Unknown 71")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Unknown 72")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Unknown 73")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("Hilfe")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Unknown 75")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Space")  PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10) PORT_NAME("Unknown 77")

	PORT_START("SPECIAL")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Unknown 0") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift")     PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Unknown 2") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Unknown 3") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CUSTOM_MEMBER(primusex_state, encoder_r)

	PORT_START("CURSOR")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d2") PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d7") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d1") PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d6") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d0") PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d9") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d3") PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\u21d8") PORT_CODE(KEYCODE_3_PAD)
INPUT_PORTS_END


void primusex_state::machine_start()
{
	u8 *mainrom = memregion("program")->base();
	m_rombank->configure_entries(0, 0x10, mainrom, 0x8000);
	m_rombank->configure_entries(0x10, 0x10, mainrom, 0x8000); // or from cartridge?

	m_rombank->set_entry(0);

	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_key_scan));
}

INTERRUPT_GEN_MEMBER(primusex_state::periodic_int)
{
	if (m_nmi_enable)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

void primusex_state::primusex(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &primusex_state::primusex_mem);
	m_maincpu->set_addrmap(AS_IO, &primusex_state::primusex_io);
	m_maincpu->set_periodic_int(FUNC(primusex_state::periodic_int), attotime::from_hz(50));

	i8255_device &ppi1(I8255(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(primusex_state::ppi1_pa_w));
	ppi1.out_pb_callback().set(FUNC(primusex_state::ppi1_pb_w));
	ppi1.tri_pb_callback().set_constant(0);
	ppi1.in_pc_callback().set_ioport("SPECIAL");

	i8255_device &ppi2(I8255(config, "ppi2"));
	ppi2.out_pa_callback().set(FUNC(primusex_state::ppi2_pa_w));
	ppi2.in_pb_callback().set(FUNC(primusex_state::ppi2_pb_r));
	ppi2.out_pc_callback().set(FUNC(primusex_state::ppi2_pc_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(50);
	screen.set_size(80, 48);
	screen.set_visarea_full();
	screen.set_screen_update(m_lcdc, FUNC(hd61202_device::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);
	HD61202(config, m_lcdc).set_screen_update_cb(FUNC(primusex_state::hd61202_update));

	SOFTWARE_LIST(config, "cart_list").set_original("primusex");
}

ROM_START(primusex) // Z84C0006PEC + 2x CP82C55A + V05040INS (?) with 80x48 pixel LCD
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("mtrom.u11", 0x00000, 0x80000, CRC(b35f8e80) SHA1(aa79175f0e590201b62ba1c19492833064e69f71))
ROM_END

} // anonymous namespace

COMP(1994, primusex, 0, 0, primusex, primusex, primusex_state, empty_init, "Yeno", "Primus Expert mit Stimme", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
