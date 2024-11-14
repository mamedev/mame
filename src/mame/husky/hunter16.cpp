// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/**********************************************************************

    Husky Hunter 16, Hunter 2/16, Hunter 16/80

    Known RAM configurations:
    Hunter 16 - 1M, 2M, 4M
    Hunter 2/16 - 4M
    Hunter 16/80 - 2M

**********************************************************************/


#include "emu.h"

#include "cpu/nec/v25.h"
#include "video/hd61830.h"
#include "video/mc6845.h"

#include "screen.h"
#include "emupal.h"


namespace {

class hunter16_state : public driver_device
{
public:
	hunter16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_lcdc(*this, "lcdc")
		, m_cga(*this, "cga")
	{ }

	void hunter16(machine_config &config);
	void hunter1680(machine_config &config);

protected:
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map) ATTR_COLD;
	void io_16_map(address_map &map) ATTR_COLD;
	void io_1680_map(address_map &map) ATTR_COLD;

	required_device<v25_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	optional_device<hd61830_device> m_lcdc;
	optional_device<mc6845_device> m_cga;

private:
	void palette_init_hunter16(palette_device &palette);

	MC6845_UPDATE_ROW(crtc_update_row);

	void keyboard_w(uint8_t data);

	uint8_t m_keydata = 0;
	uint8_t pt_r();
};


void hunter16_state::mem_map(address_map &map)
{
	map(0x00000, 0x3ffff).ram();
	map(0xb8000, 0xbffff).ram().share("videoram");
	map(0xf0000, 0xfffff).rom().region("bios",0);
}

void hunter16_state::io_16_map(address_map &map)
{
	map.unmap_value_high();
	map(0x405, 0x405).w(FUNC(hunter16_state::keyboard_w));
	map(0x41b, 0x41b).r(m_lcdc, FUNC(hd61830_device::data_r));
	map(0x430, 0x430).w(m_lcdc, FUNC(hd61830_device::data_w));
	map(0x431, 0x431).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w));
}

void hunter16_state::io_1680_map(address_map &map)
{
	map.unmap_value_high();
	map(0x3d4, 0x3d4).rw(m_cga, FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x3d5, 0x3d5).rw(m_cga, FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
}

static INPUT_PORTS_START( hunter2 )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc/Brk") PORT_CODE(KEYCODE_ESC) PORT_CHAR(27)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Lbl/Ins") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR(']')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctl/Fn")  PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
INPUT_PORTS_END

static INPUT_PORTS_START( hunter16 )
INPUT_PORTS_END

void hunter16_state::palette_init_hunter16(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

MC6845_UPDATE_ROW(hunter16_state::crtc_update_row)
{
	uint32_t  *p = &bitmap.pix(y);
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();

	for (int i = 0; i < x_count; i++)
	{
		uint16_t const offset = (((ma + i) << 1) & 0x1fff) | ((ra & 1) << 13);
		uint8_t data = m_videoram[offset];

		for (int bit = 7; bit >= 0; bit--)
		{
			*p = palette[BIT(data, bit)]; p++;
		}

		data = m_videoram[offset + 1];

		for (int bit = 7; bit >= 0; bit--)
		{
			*p = palette[BIT(data, bit)]; p++;
		}
	}
}

void hunter16_state::keyboard_w(uint8_t data)
{
	m_keydata = data;
}

uint8_t hunter16_state::pt_r()
{
	uint8_t data = 0xff;

	//if (BIT(m_p0, 0) == 0) data &= m_keys[0]->read();
	//if (BIT(m_p0, 1) == 0) data &= m_keys[1]->read();
	//if (BIT(m_p0, 2) == 0) data &= m_keys[2]->read();
	//if (BIT(m_p0, 3) == 0) data &= m_keys[3]->read();

	return data;
}

void hunter16_state::hunter16(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &hunter16_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hunter16_state::io_16_map);
	m_maincpu->pt_in_cb().set(FUNC(hunter16_state::pt_r));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(240, 128);
	m_screen->set_visarea(0, 239, 0, 63);
	m_screen->set_palette(m_palette);
	m_screen->set_screen_update("lcdc", FUNC(hd61830_device::screen_update));

	PALETTE(config, m_palette, FUNC(hunter16_state::palette_init_hunter16), 2);

	HD61830(config, m_lcdc, 16_MHz_XTAL / 16);  // unknown divider
	m_lcdc->set_screen("screen");
}

void hunter16_state::hunter1680(machine_config &config)
{
	V25(config, m_maincpu, 14.318181_MHz_XTAL / 2); // 14.31818
	m_maincpu->set_addrmap(AS_PROGRAM, &hunter16_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &hunter16_state::io_1680_map);

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_raw(14.318181_MHz_XTAL, 912, 0, 640, 296, 0, 200);
	m_screen->set_screen_update("cga", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(hunter16_state::palette_init_hunter16), 2);

	MC6845(config, m_cga, 14.318181_MHz_XTAL / 16); // Chips 82C426 CGA, exact clock unknown
	m_cga->set_screen("screen");
	m_cga->set_show_border_area(false);
	m_cga->set_char_width(8);
	m_cga->set_update_row_callback(FUNC(hunter16_state::crtc_update_row));
}

ROM_START( hunter16 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_DEFAULT_BIOS("304")
	ROM_SYSTEM_BIOS(0, "304", "v3.04")
	ROMX_LOAD( "v3.04.bin", 0x0000, 0x20000, CRC(e6e28685) SHA1(826f3898ee34bd19066daf770167635f9970f8a7), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS(1, "303", "v3.03")
	ROMX_LOAD( "v3.03i.bin", 0x0000, 0x20000, CRC(8b3ee7fd) SHA1(8bf962358a316c78ba91658c21e45e1e6cda9605), ROM_BIOS(1) )
	// v3.1 also know to exist
ROM_END

ROM_START( hunter216 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "v1.0.bin", 0x0000, 0x20000, CRC(3810fad6) SHA1(efbf15726c68df5a5339f26331b674c7505d5598) )
ROM_END

ROM_START( hunter1680 )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD( "v4.05.bin", 0x0000, 0x20000, CRC(e0cfabf4) SHA1(183d5bf7553404302697ac89eed25f2a8bb7695c) )
ROM_END

} // anonymous namespace


/*    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT     CLASS           INIT        COMPANY                FULLNAME              FLAGS */
COMP( 1989, hunter16,   0,        0,      hunter16,   hunter16, hunter16_state, empty_init, "Husky Computers Ltd", "Husky Hunter 16",    MACHINE_IS_SKELETON )
COMP( 1990, hunter216,  hunter16, 0,      hunter16,   hunter2,  hunter16_state, empty_init, "Husky Computers Ltd", "Husky Hunter 2/16",  MACHINE_IS_SKELETON )
COMP( 1989, hunter1680, hunter16, 0,      hunter1680, hunter16, hunter16_state, empty_init, "Husky Computers Ltd", "Husky Hunter 16/80", MACHINE_IS_SKELETON )
