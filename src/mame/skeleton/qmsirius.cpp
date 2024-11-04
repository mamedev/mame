// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Quasimidi Sirius keyboard and Rave-O-Lution module.

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class qmsirius_state : public driver_device
{
public:
	qmsirius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void qmsirius(machine_config &config);

private:
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

HD44780_PIXEL_UPDATE(qmsirius_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void qmsirius_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void qmsirius_state::ext_map(address_map &map)
{
	map(0x8000, 0x8001).w("lcdc", FUNC(hd44780_device::write));
	map(0x8002, 0x8003).r("lcdc", FUNC(hd44780_device::read));
}

static INPUT_PORTS_START(qmsirius)
INPUT_PORTS_END

void qmsirius_state::qmsirius(machine_config &config)
{
	P80C552(config, m_maincpu, 12_MHz_XTAL); // PCB80C552-5 16WP
	m_maincpu->set_addrmap(AS_PROGRAM, &qmsirius_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &qmsirius_state::ext_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // HC16202NY-LY; TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 16);
	lcdc.set_pixel_update_cb(FUNC(qmsirius_state::lcd_pixel_update));
}

ROM_START(qmsirius)
	ROM_REGION(0x80000, "program", 0)
	ROM_DEFAULT_BIOS("v205a")
	ROM_SYSTEM_BIOS(0, "v205a", "Version No. 2.05a")
	ROMX_LOAD("siriusv205a.bin", 0x00000, 0x80000, CRC(3e974cec) SHA1(d44dca58717f89c6eabcf329a39014df1dea215c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v203", "Version No. 2.03")
	ROMX_LOAD("siriusv203.bin", 0x00000, 0x80000, CRC(34e08d49) SHA1(fc50b9f89d66eddd0aac4ffc2447ee4be7cc41f6), ROM_BIOS(1)) // TMS27C040-12
ROM_END

ROM_START(qmrave)
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("quasimidi raveolution 309 os v2.0e.bin", 0x00000, 0x80000, CRC(b0872f0b) SHA1(db71b3654981ef82eeaa2c999453824ea7d2676e))
ROM_END

} // anonymous namespace

SYST(1998, qmsirius, 0, 0, qmsirius, qmsirius, qmsirius_state, empty_init, "Quasimidi Musikelektronik GmbH", "Quasimidi Sirius",  MACHINE_IS_SKELETON)
SYST(1996, qmrave,   0, 0, qmsirius, qmsirius, qmsirius_state, empty_init, "Quasimidi Musikelektronik GmbH", "Rave-O-Lution 309", MACHINE_IS_SKELETON)
