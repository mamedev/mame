// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Wyse WY-55 and related terminals.

    The WY-55's custom video gate array is numbered 211019-05. The WY-185 is believed to run on similar hardware.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/ins8250.h"
//#include "machine/nvram.h"
#include "screen.h"

class wy55_state : public driver_device
{
public:
	wy55_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{
	}

	void wy55(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void driver_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};


void wy55_state::machine_start()
{
}

u32 wy55_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void wy55_state::prog_map(address_map &map)
{
	// TODO: banking (probably selected by a port pin)
	map(0x0000, 0xffff).rom().region("program", 0);
}

void wy55_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x8000, 0x97ff).ram();
	//map(0xf028, 0xf037).rw("uart", FUNC(pc16552_device::read), FUNC(pc16552_device::write));
}


static INPUT_PORTS_START(wy55)
INPUT_PORTS_END

void wy55_state::wy55(machine_config &config)
{
	I8032(config, m_maincpu, 14.7456_MHz_XTAL); // exact type and clock divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &wy55_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &wy55_state::ext_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 8K SRAM + battery?

	//PC16552D(config, "uart", 14.7456_MHz_XTAL / 2); // 16C452 (divider not verified)

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(49.4235_MHz_XTAL, 1530, 0, 1200, 369, 0, 338);
	//m_screen->set_raw(49.4235_MHz_XTAL * 2 / 3, 1050, 0, 800, 523, 0, 338);
	m_screen->set_screen_update(FUNC(wy55_state::screen_update));
}


ROM_START(wy55)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("251352-12.bin", 0x0000, 0x20000, CRC(efe41862) SHA1(52ee76d636b166fa10a37356aef81011a9b079cc)) // v2.1
ROM_END

void wy55_state::driver_start()
{
	uint8_t *rom = memregion("program")->base();
	for (offs_t base = 0x00000; base < 0x20000; base += 0x4000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x4000]);

		for (offs_t offset = 0; offset < 0x4000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<14>(offset, 3, 8, 2, 0, 7, 4, 9, 10, 11, 12, 13, 5, 1, 6)], 3, 4, 5, 2, 6, 1, 7, 0);
	}
}

COMP(1993, wy55, 0, 0, wy55, wy55, wy55_state, empty_init, "Wyse Technology", "WY-55 (v2.1)", MACHINE_IS_SKELETON)
