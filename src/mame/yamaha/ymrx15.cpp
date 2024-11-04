// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Yamaha RX15 drum machine.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6801.h"
#include "sound/ym2154.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

namespace {

class rx15_state : public driver_device
{
public:
	rx15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ryp4(*this, "ryp4")
	{
	}

	void rx15(machine_config &config);

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<hd6303x_cpu_device> m_maincpu;
	required_device<ym2154_device> m_ryp4;
};


HD44780_PIXEL_UPDATE(rx15_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 8)
		bitmap.pix(y, (line * 8 + pos) * 6 + x) = state;
}

void rx15_state::mem_map(address_map &map)
{
	map(0x1000, 0x1000).nopr();
	map(0x2000, 0x207f).rw(m_ryp4, FUNC(ym2154_device::read), FUNC(ym2154_device::write));
	map(0x3000, 0x3001).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write));
	map(0x7800, 0x7fff).ram(); // NVRAM #1?
	map(0x8000, 0x87ff).ram(); // NVRAM #2?
	map(0xc000, 0xffff).rom().region("firmware", 0);
}


static INPUT_PORTS_START(rx15)
INPUT_PORTS_END

void rx15_state::rx15(machine_config &config)
{
	HD6303X(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &rx15_state::mem_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // M5M5118P-15L (one or both battery-backed?)

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*1);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 8);
	lcdc.set_pixel_update_cb(FUNC(rx15_state::pixel_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2154(config, m_ryp4, 2.7_MHz_XTAL);
	m_ryp4->add_route(0, "lspeaker", 0.50);
	m_ryp4->add_route(1, "rspeaker", 0.50);
}

ROM_START(rx15)
	ROM_REGION(0x4000, "firmware", 0)
	ROM_LOAD("yamaharx15_v1-0_hn27128ag.bin", 0x0000, 0x4000, CRC(0ca8e3d4) SHA1(d09cdb3686b15603660abec49848ef451005be62))

	ROM_REGION(0x8000, "ryp4:group0", 0)
	ROM_LOAD("ym21901.bin", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "ryp4:group1", 0)
	ROM_LOAD("ym21902.bin", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "ryp4:group2", 0)
	ROM_LOAD("ym21903.bin", 0x0000, 0x8000, NO_DUMP)

	ROM_REGION(0x8000, "ryp4:group3", 0)
	ROM_LOAD("ym21907.bin", 0x0000, 0x8000, NO_DUMP)
ROM_END

} // anonymous namespace

SYST(1984, rx15, 0, 0, rx15, rx15, rx15_state, empty_init, "Yamaha", "RX15 Digital Rhythm Programmer", MACHINE_IS_SKELETON)
