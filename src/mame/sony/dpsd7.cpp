// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Sony DPS-D7 digital delay processor.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8532.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class dpsd7_state : public driver_device
{
public:
	dpsd7_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void dpsd7(machine_config &config) ATTR_COLD;

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8500_device> m_maincpu;
};

HD44780_PIXEL_UPDATE(dpsd7_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 40)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void dpsd7_state::mem_map(address_map &map)
{
	map(0x00000, 0x0faff).rom().region("eprom", 0);
	map(0x0fb40, 0x0fb41).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x10000, 0x17fff).ram().share("nvram");
}

static INPUT_PORTS_START(dpsd7)
INPUT_PORTS_END

void dpsd7_state::dpsd7(machine_config &config)
{
	HD6435328(config, m_maincpu, 20'000'000).set_mode(3); // H8/532(?), internal ROM disabled
	m_maincpu->set_addrmap(AS_PROGRAM, &dpsd7_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*40, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	// TODO: needs custom CGROM?
	lcdc.set_lcd_size(2, 40);
	lcdc.set_pixel_update_cb(FUNC(dpsd7_state::pixel_update));
}

ROM_START(dpsd7)
	ROM_SYSTEM_BIOS(0, "v150", "Ver1.50")
	ROM_SYSTEM_BIOS(1, "v140", "Ver1.40")

	ROM_REGION(0x10000, "eprom", 0)
	ROMX_LOAD("sony_dps-d7b_v150.bin", 0x00000, 0x10000, CRC(e7f90056) SHA1(1f83f42bd93ef4612e5521aeebc45e72be1ba5db), ROM_BIOS(0)) // M5M27C512AK-12
	ROMX_LOAD("sony_dps-d7_v140.bin", 0x00000, 0x10000, CRC(378b1da2) SHA1(b0ae29cbe9d4be8f206304e567d7df8e4200358f), ROM_BIOS(1))
ROM_END

} // anonymous namespace

SYST(1991, dpsd7, 0, 0, dpsd7, dpsd7, dpsd7_state, empty_init, "Sony", "DPS-D7 Digital Delay Unit", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS)
