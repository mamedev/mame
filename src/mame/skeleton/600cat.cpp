// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***********************************************************************************************************************************

    Skeleton driver for Wavetek 600 Cellular Activation Tester.

************************************************************************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/timekpr.h"
#include "video/hd44780.h"

#include "emupal.h"
#include "screen.h"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

class _600cat_state : public driver_device
{
public:
	_600cat_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc(*this, "lcdc")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_rtc(*this, "rtc")
	{ }

	void _600cat(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void lcd_palette(palette_device &palette) const;

	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);
	void p4_w(u8 data);
	void sc2_w(int state);
	void ser_tx_w(int state);

	required_device<hd6303r_cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ds1643_device> m_rtc;
};

HD44780_PIXEL_UPDATE(_600cat_state::lcd_pixel_update)
{
	// char size is 5x8
	if (x > 4 || y > 7)
		return;

	if (pos < 40)
	{
		if (pos >= 20)
		{
			line += 2;
			pos -= 20;
		}

		if (line < 4)
			bitmap.pix(line * (8 + 1) + y, pos * 6 + x) = state ? 1 : 2;
	}
}

void _600cat_state::lcd_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148)); // background
	palette.set_pen_color(1, rgb_t( 92,  83,  88)); // lcd pixel on
	palette.set_pen_color(2, rgb_t(131, 136, 139)); // lcd pixel off
}

void _600cat_state::p1_w(u8 data)
{
	LOG("p1_w: %02x\n", data);
}

void _600cat_state::p2_w(u8 data)
{
	LOG("p2_w: %02x\n", data);
}

void _600cat_state::p3_w(u8 data)
{
	LOG("p3_w: %02x\n", data);
}

void _600cat_state::p4_w(u8 data)
{
	LOG("p4_w: %02x\n", data);
}

void _600cat_state::sc2_w(int state)
{
	LOG("sc2_w: %d\n", state);
}

void _600cat_state::ser_tx_w(int state)
{
	LOG("ser_tx_w: %d\n", state);
}

void _600cat_state::mem_map(address_map &map)
{
	map(0x0380, 0x0381).rw(m_lcdc, FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x0400, 0x1fff).lrw8(NAME([this](offs_t offset) { return m_rtc->read(offset + 0x400); }), NAME([this](offs_t offset, u8 data) { m_rtc->write(offset + 0x400, data); }));
	map(0x2000, 0xffff).rom().region("maincpu", 0x2000);
}

static INPUT_PORTS_START(_600cat)
INPUT_PORTS_END

void _600cat_state::_600cat(machine_config &config)
{
	HD6303R(config, m_maincpu, 4'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &_600cat_state::mem_map);
	m_maincpu->out_p1_cb().set(FUNC(_600cat_state::p1_w));
	m_maincpu->out_p2_cb().set(FUNC(_600cat_state::p2_w));
	m_maincpu->out_p3_cb().set(FUNC(_600cat_state::p3_w));
	m_maincpu->out_p4_cb().set(FUNC(_600cat_state::p4_w));
	m_maincpu->out_sc2_cb().set(FUNC(_600cat_state::sc2_w));
	m_maincpu->out_ser_tx_cb().set(FUNC(_600cat_state::ser_tx_w));

	SCREEN(config, m_screen, SCREEN_TYPE_LCD);
	m_screen->set_refresh_hz(50);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_size(120, 36);
	m_screen->set_visarea_full();
	m_screen->set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(_600cat_state::lcd_palette), 3);

	HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
	m_lcdc->set_lcd_size(4, 20);
	m_lcdc->set_pixel_update_cb(FUNC(_600cat_state::lcd_pixel_update));

	DS1643(config, m_rtc);
}

ROM_START( 600cat )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "600cat.bin", 0x00000, 0x10000, CRC(7cd375f2) SHA1(611a3b8f9d2b4d54f60db40556847b229a34a309) )
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY    FULLNAME                              FLAGS
COMP( 199?, 600cat, 0,      0,      _600cat, _600cat, _600cat_state, empty_init, "Wavetek", "600 Cellular Activation Tester", MACHINE_IS_SKELETON )
