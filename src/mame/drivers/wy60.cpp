// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Wyse WY-60 terminal.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/i2cmem.h"
#include "machine/mc2661.h"
#include "screen.h"

class wy60_state : public driver_device
{
public:
	wy60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
		, m_sio(*this, "sio")
	{
	}

	void wy60(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void driver_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<screen_device> m_screen;
	required_device<mc2661_device> m_sio;
};


void wy60_state::machine_start()
{
}

u32 wy60_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void wy60_state::prog_map(address_map &map)
{
	// TODO: EA banking?
	map(0x0000, 0xffff).rom().region("wyseprog", 0);
}

void wy60_state::ext_map(address_map &map)
{
}


static INPUT_PORTS_START(wy60)
INPUT_PORTS_END

void wy60_state::wy60(machine_config &config)
{
	i8051_device &maincpu(I8051(config, "maincpu", 11_MHz_XTAL)); // P8051AN-40196
	maincpu.set_addrmap(AS_PROGRAM, &wy60_state::prog_map);
	maincpu.set_addrmap(AS_IO, &wy60_state::ext_map);

	//X2404(config, m_eeprom);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(26.58_MHz_XTAL, 1000, 0, 800, 443, 0, 416); // 26.580 kHz horizontal
	//m_screen->set_raw(39.71_MHz_XTAL, 1494, 0, 1188, 443, 0, 416);
	m_screen->set_screen_update(FUNC(wy60_state::screen_update));

	MC2661(config, m_sio, 4.9152_MHz_XTAL); // SCN2661B
}

// CPU:   8051(202008-03)
// EPROM: 27512(193003-01)
// Video: 211003-02/205001-02
// RAM:   2064 (2064/2016/2016/2064)
// NVRAM: X2404
// UART:  2661
// XTALs: 39.710, 26.580, 11.000, 4.9152

ROM_START(wy60)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("202008-03.bin", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x10000, "wyseprog", 0)
	ROM_LOAD("193003-01.u9", 0x00000, 0x10000, CRC(26de0ea4) SHA1(91409f98a3990b514fbcb7de2eb45944bf5b95bc))
ROM_END

ROM_START(wy60a)
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("202008-03.bin", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x10000, "wyseprog", 0)
	ROM_LOAD("wy-60_4k.u6", 0x00000, 0x10000, CRC(6daf2824) SHA1(23cd039ec7ae71b0742e8eebf75be8cd5992e3fd))
ROM_END

void wy60_state::driver_start()
{
	uint8_t *rom = memregion("wyseprog")->base();
	for (offs_t base = 0x00000; base < 0x10000; base += 0x2000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x2000]);

		for (offs_t offset = 0; offset < 0x2000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<13>(offset, 0, 6, 9, 4, 2, 1, 3, 5, 7, 8, 10, 11, 12)], 6, 0, 5, 1, 4, 2, 3, 7);
	}

	// FIXME: remove once internal ROM is dumped
	std::copy_n(rom, 0x1000, memregion("maincpu")->base());
}

COMP(1986, wy60,  0,    0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (set 1)", MACHINE_IS_SKELETON)
COMP(1986, wy60a, wy60, 0, wy60, wy60, wy60_state, empty_init, "Wyse Technology", "WY-60 (set 2)", MACHINE_IS_SKELETON)
