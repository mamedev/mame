// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Ascom Eurit Euro-ISDN Telefon.

****************************************************************************/

#include "emu.h"

#include "cpu/m37710/m37710.h"
#include "machine/am79c30.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"


namespace {

class eurit_state : public driver_device
{
public:
	eurit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_keys(*this, "KEY%c", 'A')
		, m_key_scan(0x1f)
	{
	}

	void eurit30(machine_config &mconfig);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void key_scan_w(u8 data);
	u8 key_matrix_r();

	void mem_map(address_map &map) ATTR_COLD;

	void palette_init(palette_device &palette);

	required_device<m37730s2_device> m_maincpu;
	required_ioport_array<5> m_keys;

	u8 m_key_scan;
};

void eurit_state::machine_start()
{
	save_item(NAME(m_key_scan));
}

HD44780_PIXEL_UPDATE(eurit_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 20)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


void eurit_state::key_scan_w(u8 data)
{
	m_key_scan = data >> 3;
}

u8 eurit_state::key_matrix_r()
{
	u8 ret = 0xff;
	for (int i = 0; i < 5; i++)
		if (!BIT(m_key_scan, i))
			ret &= m_keys[i]->read();

	return ret;
}


void eurit_state::mem_map(address_map &map)
{
	map(0x000480, 0x007fff).ram(); // probably NVRAM (0x000480 counts seconds for software RTC)
	map(0x008000, 0x00ffff).rom().region("firmware", 0x8000);
	map(0x040000, 0x05ffff).rom().region("firmware", 0);
	map(0x0c0000, 0x0c0007).rw("dsc", FUNC(am79c30a_device::read), FUNC(am79c30a_device::write));
}


static INPUT_PORTS_START(eurit30)
	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key A0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key A1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("#") PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("0  0...9") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("*") PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("ESC") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("R") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key A7") // phone off hook?

	PORT_START("KEYB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z2") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Menu") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("9  YZ") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("8  VWX") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("7  STU") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key B5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("MIC") PORT_CODE(KEYCODE_M)

	PORT_START("KEYC")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z6") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z3") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("6  PQR") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("5  MNO") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("4  JKL") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key C5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("+") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key C7")

	PORT_START("KEYD")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z4") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z5") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("3  GHI") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("2  DEF") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("1  ABC") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Unknown Key D5")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME(u8"\u2192") PORT_CODE(KEYCODE_RIGHT) // →
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME(u8"\u2190") PORT_CODE(KEYCODE_LEFT) // ←

	PORT_START("KEYE")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fox Key Center Right") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fox Key Next to Left") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("-") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fox Key Next to Right") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fox Key Right") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fox Key Center Left") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fox Key Left") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Z1") PORT_CODE(KEYCODE_Z)
INPUT_PORTS_END

void eurit_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void eurit_state::eurit30(machine_config &config)
{
	M37730S2(config, m_maincpu, 4'096'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &eurit_state::mem_map);
	m_maincpu->p4_in_cb().set("lcdc", FUNC(hd44780_device::db_r)); // not actually used for input?
	m_maincpu->p4_out_cb().set("lcdc", FUNC(hd44780_device::db_w));
	m_maincpu->p4_out_cb().append(FUNC(eurit_state::key_scan_w));
	m_maincpu->p5_in_cb().set(FUNC(eurit_state::key_matrix_r));
	m_maincpu->p6_out_cb().set("lcdc", FUNC(hd44780_device::e_w)).bit(6);
	m_maincpu->p6_out_cb().append("lcdc", FUNC(hd44780_device::rw_w)).bit(5); // not actually used for read mode?
	m_maincpu->p6_out_cb().append("lcdc", FUNC(hd44780_device::rs_w)).bit(4);

	am79c30a_device &dsc(AM79C30A(config, "dsc", 12'288'000));
	dsc.int_callback().set_inputline(m_maincpu, M37710_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*20, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(eurit_state::palette_init), 2);

	hd44780_device &lcdc(SED1278(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_default_bios_tag("0b");
	lcdc.set_lcd_size(2, 20);
	lcdc.set_pixel_update_cb(FUNC(eurit_state::lcd_pixel_update));
}


ROM_START(eurit30)
	ROM_REGION16_LE(0x20000, "firmware", 0) // Firmware 2.210 deutsch
	ROM_LOAD("d_2.210", 0x00000, 0x20000, CRC(c77be0ac) SHA1(1eaba66dcb4f64cc33565ca85de25341572ddb2e))
ROM_END

} // anonymous namespace


SYST(1996, eurit30, 0, 0, eurit30, eurit30, eurit_state, empty_init, "Ascom", "Eurit 30", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
