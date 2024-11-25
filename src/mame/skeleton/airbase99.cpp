// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for AirBase 99 drum machine by JoMoX GmbH.

****************************************************************************/

#include "emu.h"
#include "cpu/pic17/pic17c4x.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"

namespace {

class airbase99_state : public driver_device
{
public:
	airbase99_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_slavecpu(*this, "slavecpu")
	{
	}

	void airbase99(machine_config &config);

private:
	HD44780_PIXEL_UPDATE(pixel_update);

	void mem_map(address_map &map) ATTR_COLD;

	required_device<pic17c4x_device> m_maincpu;
	required_device<pic17c4x_device> m_slavecpu;
};


HD44780_PIXEL_UPDATE(airbase99_state::pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

void airbase99_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("firmware", 0);
	map(0x8000, 0x8008).nopw();
	map(0x8008, 0x8008).nopr();
	map(0x8010, 0x8011).rw("lcdc", FUNC(hd44780_device::control_r), FUNC(hd44780_device::write)).umask16(0x00ff);
}

static INPUT_PORTS_START(airbase99)
INPUT_PORTS_END

void airbase99_state::airbase99(machine_config &config)
{
	PIC17C43(config, m_maincpu, 16'000'000); // PIC17C43-16/P (XTAL unreadable)
	m_maincpu->set_mode(pic17c43_device::mode::MICROPROCESSOR);
	m_maincpu->set_addrmap(AS_PROGRAM, &airbase99_state::mem_map);

	PIC17C43(config, m_slavecpu, 16'000'000); // PIC17C43-33/P (XTAL unreadable)
	m_slavecpu->set_mode(pic17c43_device::mode::MICROCONTROLLER);
	m_slavecpu->set_disable();

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 20);
	lcdc.set_pixel_update_cb(FUNC(airbase99_state::pixel_update));
}

ROM_START(airbase99)
	ROM_REGION16_LE(0x20000, "firmware", 0)
	ROM_LOAD16_BYTE("airbase99 r1_09l.bin", 0x00000, 0x10000, CRC(218ec7cc) SHA1(8eeffa22f927d5e17937f9c04dbebf5367f520df)) // second half blank
	ROM_LOAD16_BYTE("airbase99 r1_09h.bin", 0x00001, 0x10000, CRC(f3e7a25f) SHA1(8978326615661238aa07304bd4aaf8471b7bf449)) // second half blank

	ROM_REGION(0x2000, "slavecpu", 0)
	ROM_LOAD("airbase_slave.bin", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x80000, "samples", 0)
	ROM_LOAD("rom0.bin", 0x00000, 0x80000, NO_DUMP) // M27C4001
ROM_END

} // anonymous namespace

SYST(1998, airbase99, 0, 0, airbase99, airbase99, airbase99_state, empty_init, "JoMoX", "AiRBase 99", MACHINE_IS_SKELETON)
