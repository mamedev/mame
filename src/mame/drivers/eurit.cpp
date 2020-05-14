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

class eurit_state : public driver_device
{
public:
	eurit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void eurit30(machine_config &mconfig);

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void mem_map(address_map &map);

	void palette_init(palette_device &palette);

	required_device<m37730s2_device> m_maincpu;
};

HD44780_PIXEL_UPDATE(eurit_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 20)
		bitmap.pix16(line * 8 + y, pos * 6 + x) = state;
}


void eurit_state::mem_map(address_map &map)
{
	map(0x000480, 0x007fff).ram(); // probably NVRAM (0x000480 counts seconds for software RTC)
	map(0x008000, 0x00ffff).rom().region("firmware", 0x8000);
	map(0x040000, 0x05ffff).rom().region("firmware", 0);
	map(0x0c0000, 0x0c0007).rw("dsc", FUNC(am79c30a_device::read), FUNC(am79c30a_device::write));
}


static INPUT_PORTS_START(eurit30)
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
	m_maincpu->p4_out_cb().set("lcdc", FUNC(hd44780_device::db_w));
	m_maincpu->p6_out_cb().set("lcdc", FUNC(hd44780_device::e_w)).bit(6);
	m_maincpu->p6_out_cb().append("lcdc", FUNC(hd44780_device::rw_w)).bit(5);
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

	hd44780_device &lcdc(HD44780(config, "lcdc", 0));
	lcdc.set_lcd_size(2, 20);
	lcdc.set_pixel_update_cb(FUNC(eurit_state::lcd_pixel_update));
	lcdc.set_busy_factor(0.01);
}


ROM_START(eurit30)
	ROM_REGION16_LE(0x20000, "firmware", 0) // Firmware 2.210 deutsch
	ROM_LOAD("d_2.210", 0x00000, 0x20000, CRC(c77be0ac) SHA1(1eaba66dcb4f64cc33565ca85de25341572ddb2e))
ROM_END


SYST(1996, eurit30, 0, 0, eurit30, eurit30, eurit_state, empty_init, "Ascom", "Eurit 30", MACHINE_IS_SKELETON)
