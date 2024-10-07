// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/*******************************************************************************************************

    MCM/70

    TODO:
    - at power on need to press START to use (need schematic to know what's happening)
    - cassette (x2) handling, digital with pre-recorded clock track.
    - MCP-132 printer/plotter device (Diablo HyType I).
    - serial communications interface unit (terminal and modem ports).
    - artwork to show keyboard layout.

******************************************************************************************************/

#include "emu.h"
#include "cpu/i8008/i8008.h"

#include "emupal.h"
#include "screen.h"

#define LOG_DEVICE (1U << 1)

#define VERBOSE (LOG_DEVICE)
#include "logmacro.h"


namespace {

class mcm70_state : public driver_device
{
public:
	mcm70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
		, m_ram(*this, "ram")
		, m_kbd(*this, "COL%u", 0U)
		, m_kbd_mod(*this, "MODIFIERS")
		, m_palette(*this, "palette")
		, m_device_select(0)
	{ }

	void mcm70(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(start);

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	uint8_t status_r(offs_t offset);
	void devsel_w(offs_t offset, uint8_t data);
	void device_w(offs_t offset, uint8_t data);
	uint8_t device_r(offs_t offset);

	uint8_t keyboard_r(offs_t offset);

	void io_map(address_map &map) ATTR_COLD;
	void mem_map(address_map &map) ATTR_COLD;

	void mcm70_palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_shared_ptr<uint8_t> m_ram;
	required_ioport_array<8> m_kbd;
	required_ioport m_kbd_mod;
	required_device<palette_device> m_palette;

	uint8_t m_device_select;
};


void mcm70_state::mem_map(address_map &map)
{
	map(0x0000, 0x17ff).rom().region("maincpu", 0);
	map(0x1800, 0x1fff).bankr("rombank");
	map(0x2000, 0x3fff).ram().share("ram");
}

void mcm70_state::io_map(address_map &map)
{
	map(0x00, 0x00).select(0xffe0).r(FUNC(mcm70_state::keyboard_r));
	map(0x01, 0x01).select(0xffe0).r(FUNC(mcm70_state::status_r));
	map(0x02, 0x03).mirror(0xffe0).r(FUNC(mcm70_state::device_r));
	map(0x08, 0x08).mirror(0xffe0).lw8(NAME([this](uint8_t data) { m_rombank->set_entry(data >> 4); }));
	map(0x09, 0x09).select(0xffe0).w(FUNC(mcm70_state::devsel_w));
	map(0x0a, 0x0b).mirror(0xffe0).w(FUNC(mcm70_state::device_w));
	map(0x1f, 0x1f).mirror(0xffe0).nopw(); // output single column from display
}


uint8_t mcm70_state::status_r(offs_t offset)
{
	return 0x00;
}


void mcm70_state::devsel_w(offs_t offset, uint8_t data)
{
	m_device_select = offset >> 8;
	std::string device_selected;

	switch (m_device_select)
	{
	case 0x00: // None
		device_selected = "None";
		break;

	case 0x01: // Printer (MCP-132)
		device_selected = "Printer";
		break;

	case 0xc8: // Cassette 1
		device_selected = "Cassette 1";
		break;

	case 0xc9: // Cassette 2
		device_selected = "Cassette 2";
		break;

	default:
		device_selected = "Unknown";
		break;
	}

	LOGMASKED(LOG_DEVICE, "devsel_w: %02x %s\n", data, device_selected);
}

uint8_t mcm70_state::device_r(offs_t offset)
{
	uint8_t data = 0x00;

	switch (m_device_select)
	{
	case 0x01: // Printer
		switch (offset & 1)
		{
		case 0: // status
			// bit 0 Printer powered and ready
			// bit 1 Check condition (printer crashed)
			// bit 2 Not used
			// bit 3 Not used
			// bit 4 Ribbon up
			// bit 5 Character print ready
			// bit 6 Carriage ready
			// bit 7 Paper feed ready
			break;

		case 1: // data
			break;
		}
		break;

	case 0xc8: // Cassette 1
		break;

	case 0xc9: // Cassette 2
		break;
	}

	LOGMASKED(LOG_DEVICE, "device_r: %s %02x\n", (offset & 1) ? "data" : "status", data);

	return data;
}

void mcm70_state::device_w(offs_t offset, uint8_t data)
{
	switch (m_device_select)
	{
	case 0x01: // Printer
		break;

	case 0xc8: // Cassette 1
		break;

	case 0xc9: // Cassette 2
		break;
	}

	LOGMASKED(LOG_DEVICE, "device_w: %s %02x\n", (offset & 1) ? "data" : "control", data);
}


static INPUT_PORTS_START( mcm70 )
	PORT_START("COL0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)          PORT_CHAR('+') PORT_CHAR('-')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)      PORT_CHAR(8)   PORT_NAME("BACK SPACE")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)              PORT_CHAR('9') PORT_CHAR(0x2228)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)              PORT_CHAR('7') PORT_CHAR('>')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)              PORT_CHAR('5') PORT_CHAR('=')
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)              PORT_CHAR('3') PORT_CHAR('<')

	PORT_START("COL1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)          PORT_CHAR(13)  PORT_CHAR(13) PORT_NAME("RETURN")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)              PORT_CHAR('P') PORT_CHAR('*')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)              PORT_CHAR('I') PORT_CHAR(0x2373)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)              PORT_CHAR('Y') PORT_CHAR(0x2191)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)              PORT_CHAR('R') PORT_CHAR(0x2374)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)              PORT_CHAR('W') PORT_CHAR(0x2375)

	PORT_START("COL2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)          PORT_CHAR(']') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)              PORT_CHAR('L') PORT_CHAR(0x2395)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)              PORT_CHAR('J') PORT_CHAR(0x2218)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)              PORT_CHAR('G') PORT_CHAR(0x2207)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)              PORT_CHAR('D') PORT_CHAR(0x230a)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)              PORT_CHAR('A') PORT_CHAR(0x237a)

	PORT_START("COL3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)          PORT_CHAR('/') PORT_CHAR('\\')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)          PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)              PORT_CHAR('N') PORT_CHAR(0x22a4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)              PORT_CHAR('V') PORT_CHAR(0x222a)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)              PORT_CHAR('X') PORT_CHAR(0x2283)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)          PORT_CHAR(' ') PORT_NAME("SPACE")

	PORT_START("COL4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)          PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)             PORT_CHAR('M') PORT_CHAR('|')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)             PORT_CHAR('B') PORT_CHAR(0x22a5)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)             PORT_CHAR('C') PORT_CHAR(0x2229)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)             PORT_CHAR('Z') PORT_CHAR(0x2282)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)             PORT_CHAR('Q') PORT_CHAR('?')

	PORT_START("COL5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)         PORT_CHAR('[') PORT_CHAR('(')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)             PORT_CHAR('K') PORT_CHAR('\'')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)             PORT_CHAR('H') PORT_CHAR(0x2206)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)             PORT_CHAR('F') PORT_CHAR('_')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)             PORT_CHAR('S') PORT_CHAR(0x2308)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)             PORT_CHAR('2') PORT_CHAR(0x203e)

	PORT_START("COL6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)     PORT_CHAR(0x2190) PORT_CHAR(0x2192)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)             PORT_CHAR('O') PORT_CHAR(0x25cb)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)             PORT_CHAR('U') PORT_CHAR(0x2193)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)             PORT_CHAR('T') PORT_CHAR('~')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)             PORT_CHAR('E') PORT_CHAR(0x2208)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)             PORT_CHAR('1') PORT_CHAR(0xa8)

	PORT_START("COL7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)        PORT_CHAR('x') PORT_CHAR(0xf7)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)             PORT_CHAR('0') PORT_CHAR(0x2227)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)             PORT_CHAR('8') PORT_CHAR(0x2260)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)             PORT_CHAR('6') PORT_CHAR(0x2265)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)             PORT_CHAR('4') PORT_CHAR(0x2264)

	PORT_START("MODIFIERS")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("CTRL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)   PORT_CODE(KEYCODE_RSHIFT)   PORT_NAME("SHIFT")

	PORT_START("START")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_NAME("START") PORT_CHANGED_MEMBER(DEVICE_SELF, mcm70_state, start, 0)
INPUT_PORTS_END


INPUT_CHANGED_MEMBER(mcm70_state::start)
{
}


uint8_t mcm70_state::keyboard_r(offs_t offset)
{
	uint8_t data = m_kbd_mod->read();

	switch (offset >> 8)
	{
	case 0x01: data |= m_kbd[0]->read(); break;
	case 0x02: data |= m_kbd[1]->read(); break;
	case 0x04: data |= m_kbd[2]->read(); break;
	case 0x08: data |= m_kbd[3]->read(); break;
	case 0x10: data |= m_kbd[4]->read(); break;
	case 0x20: data |= m_kbd[5]->read(); break;
	case 0x40: data |= m_kbd[6]->read(); break;
	case 0x80: data |= m_kbd[7]->read(); break;
	}

	return data;
}


void mcm70_state::machine_start()
{
	m_rombank->configure_entries(0, 16, memregion("banked")->base(), 0x800);
	m_rombank->set_entry(0);

	save_item(NAME(m_device_select));
}

void mcm70_state::machine_reset()
{
	m_device_select = 0x00;
}


uint32_t mcm70_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	// Burroughs Self-Scan Panel Display C4047
	// The panel displays 222 columns of 7 dots, 32 characters of 5x7 with a blank column between each.
	// Each column of the display is controlled by a byte of video RAM

	pen_t const *const pen = m_palette->pens();

	for (int x = 0; x < 222; x++)
	{
		uint8_t data = m_ram[0x0021 + x];

		for (int y = 0; y < 7; y++)
		{
			int32_t xl = x * 4;
			int32_t yl = y * 4;

			// pixel
			if (BIT(data, y)) // on
			{
				bitmap.pix(yl,     xl)     = pen[1];
				bitmap.pix(yl,     xl + 1) = pen[2];
				bitmap.pix(yl,     xl + 2) = pen[1];
				bitmap.pix(yl + 1, xl)     = pen[2];
				bitmap.pix(yl + 1, xl + 1) = pen[3];
				bitmap.pix(yl + 1, xl + 2) = pen[2];
				bitmap.pix(yl + 2, xl)     = pen[1];
				bitmap.pix(yl + 2, xl + 1) = pen[2];
				bitmap.pix(yl + 2, xl + 2) = pen[1];
			}
			else // off
			{
				bitmap.pix(yl,     xl)     = pen[4];
				bitmap.pix(yl,     xl + 1) = pen[4];
				bitmap.pix(yl,     xl + 2) = pen[4];
				bitmap.pix(yl + 1, xl)     = pen[4];
				bitmap.pix(yl + 1, xl + 1) = pen[4];
				bitmap.pix(yl + 1, xl + 2) = pen[4];
				bitmap.pix(yl + 2, xl)     = pen[4];
				bitmap.pix(yl + 2, xl + 1) = pen[4];
				bitmap.pix(yl + 2, xl + 2) = pen[4];
			}

			// gap around pixel
			if (x < 222)
			{
				bitmap.pix(yl,     xl + 3) = pen[0];
				bitmap.pix(yl + 1, xl + 3) = pen[0];
				bitmap.pix(yl + 2, xl + 3) = pen[0];
			}
			if (y < 7)
			{
				bitmap.pix(yl + 3, xl    ) = pen[0];
				bitmap.pix(yl + 3, xl + 1) = pen[0];
				bitmap.pix(yl + 3, xl + 2) = pen[0];
			}
			if (x < 222 && y < 7)
			{
				bitmap.pix(yl + 3, xl + 3) = pen[0];
			}
		}
	}

	return 0;
}

void mcm70_state::mcm70_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(41, 24, 26));    // background
	palette.set_pen_color(1, rgb_t(174, 35, 35));   // cell on outer corner
	palette.set_pen_color(2, rgb_t(237, 79, 80));   // cell on outer center
	palette.set_pen_color(3, rgb_t(255, 228, 238)); // cell on center
	palette.set_pen_color(4, rgb_t(79, 44, 40));    // cell off
}


void mcm70_state::mcm70(machine_config &config)
{
	I8008(config, m_maincpu, 400000); // TODO: verify clock
	m_maincpu->set_addrmap(AS_PROGRAM, &mcm70_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mcm70_state::io_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(mcm70_state::screen_update));
	screen.set_size(887, 27);
	screen.set_visarea(0, 886, 0, 26);
	PALETTE(config, m_palette, FUNC(mcm70_state::mcm70_palette), 5);
}


ROM_START( mcm70 )
	ROM_REGION(0x1800, "maincpu", 0)
	ROM_LOAD( "mcm70_m1.bin", 0x0000, 0x0800, CRC(e16ea421) SHA1(c3b8608100689fca56dfc43703b6f2e1c5dc5425))
	ROM_LOAD( "mcm70_m2.bin", 0x0800, 0x0800, CRC(dde53a75) SHA1(f9d0946bea7dc2f6c6697636061a6308236e0436))
	ROM_LOAD( "mcm70_m3.bin", 0x1000, 0x0800, CRC(319516f4) SHA1(d63f23173bc86a872d9b8a908e91f73e61b51ed8))

	ROM_REGION(0x8000, "banked", ROMREGION_ERASEFF)
	ROM_LOAD( "mcm70_b0.bin", 0x0000, 0x0800, CRC(71f0f656) SHA1(48d444b389135cb63c71b8f8f3d35433fa2d4424))
	ROM_LOAD( "mcm70_b1.bin", 0x0800, 0x0800, CRC(9167509e) SHA1(ece999709f6493c23e79f080f565fb89b87f71aa))
	ROM_LOAD( "mcm70_b2.bin", 0x1000, 0x0800, CRC(b9b60da0) SHA1(862f8c9f48f80bfade1be0adf8e53f109cb0b5b3))
	ROM_LOAD( "mcm70_b3.bin", 0x1800, 0x0800, CRC(cfa0cffc) SHA1(dbeda9396422122b47321723a6425815dd8d5ac5))
	ROM_LOAD( "mcm70_b4.bin", 0x2000, 0x0800, CRC(9f67282c) SHA1(23707721e330d10f3c157bb95fb6329fb857b05c))
	ROM_LOAD( "mcm70_b5.bin", 0x2800, 0x0800, CRC(766c3646) SHA1(b96819eff1b711a677a97f650f12467e23107406))
	ROM_LOAD( "mcm70_b6.bin", 0x3000, 0x0800, CRC(c8f1eb05) SHA1(92aaca4f15ce971e5b45dd5fa3ff60206dc59015))
	ROM_LOAD( "mcm70_b7.bin", 0x3800, 0x0800, CRC(520e8238) SHA1(132b5ae1666da7cad7af386e3971c2dcb92a5316))
	ROM_LOAD( "mcm70_b8.bin", 0x4000, 0x0800, CRC(1128f1ed) SHA1(4d667cd28d5a4a424b827fc7f75b8bfe62f1c162))
	ROM_LOAD( "mcm70_b9.bin", 0x4800, 0x0800, CRC(11ac4225) SHA1(b37610479b48027c9bdf9e5e72556404a28c2987))
	ROM_LOAD( "mcm70_ba.bin", 0x5000, 0x0800, CRC(14eb2a1f) SHA1(9d7959b918f148b70f4fcb74222a4287a8721c63))
	ROM_LOAD( "mcm70_bb.bin", 0x5800, 0x0800, CRC(0685f8b0) SHA1(4fd6abc6f9ff4e02690e58a349a398a20534c788))
	ROM_LOAD( "mcm70_bc.bin", 0x6000, 0x0800, CRC(1ae19741) SHA1(c22dd49180999d70e6fd0c7d89a7ff6cc5eed974))
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS        INIT        COMPANY                    FULLNAME   FLAGS
COMP( 1974, mcm70,  0,      0,      mcm70,   mcm70,  mcm70_state, empty_init, "Micro Computer Machines", "MCM/70",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
