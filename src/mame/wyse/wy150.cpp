// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for Wyse WY-150 and related terminals.

    The WY-150, originally introduced in Nov. 1988, was the first of the "terminals of the future" that Wyse sold well into the
    1990s. The WY-160 is a graphical terminal that runs on different but clearly related hardware.

    All video functions in these terminals are integrated into ASICs (e.g. 211009-01, 211009-02). The 8032 generates all active
    signals for both the main serial port and the serial keyboard. Three 8Kx8 SRAMs are used to store characters, attributes and
    font data; the second is also battery-backed.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "screen.h"

class wy150_state : public driver_device
{
public:
	wy150_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
	{
	}

	void wy150(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void driver_start() override;

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<screen_device> m_screen;
};


void wy150_state::machine_start()
{
}

u32 wy150_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void wy150_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void wy150_state::ext_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x5fff).ram();
}


static INPUT_PORTS_START(wy150)
INPUT_PORTS_END


void wy150_state::wy150(machine_config &config)
{
	i80c32_device &maincpu(I80C32(config, "maincpu", 11_MHz_XTAL)); // Philips P80C32SBPN (e.g.)
	maincpu.set_addrmap(AS_PROGRAM, &wy150_state::prog_map);
	maincpu.set_addrmap(AS_IO, &wy150_state::ext_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // 8464 or 5564 or similar (e.g. Winbond W2465-70LL) + battery

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(48_MHz_XTAL, 1530, 0, 1200, 523, 0, 416); // 31.372 kHz horizontal
	//m_screen->set_raw(48_MHz_XTAL, 1530, 0, 1188, 402, 0, 338);
	// TBD: WY-160 should have different parameters (has 76 Hz refresh rate rather than 78 Hz)
	m_screen->set_screen_update(FUNC(wy150_state::screen_update));
}


ROM_START(wy150)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("251167-01.bin", 0x00000, 0x10000, CRC(4f425b11) SHA1(e44f54aa98d9f9c668a6ad674ec07e47879fc2a0))

	ROM_REGION(0x20000, "link", 0)
	ROM_LOAD("link_mc3.bin",            0x00000, 0x10000, CRC(9e1d37d9) SHA1(d74c0faf6cf1eb06243607931967cf35a633ac8e))
	ROM_LOAD("link_mc5_xerox-wy30.bin", 0x10000, 0x10000, CRC(1aa00cb4) SHA1(6a7267132fe35c8e07deccd67c0fb4fe5a240c99))
ROM_END

ROM_START(wy120) // b&w
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("wy120_ver1.4.bin", 0x00000, 0x10000, CRC(6de23624) SHA1(ad90087237347662b5ae4fcc8a05d66d76c46a26))
ROM_END

ROM_START(wy160)
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("251167-06.bin", 0x00000, 0x10000, CRC(36e920df) SHA1(8fb7f51b4f47ef63b21d421227d6fef98001e4e9))
ROM_END

ROM_START(wy325) // SCN8032HCCA44, 211009-02, 3x CXK5864CM-70LL, SCN2661BC1A28, 3V battery, color display
	ROM_REGION(0x10000, "program", 0)
	ROM_LOAD("wyse_tech_rev.a_251125-05.bin", 0x00000, 0x10000, CRC(4a327c38) SHA1(061332197d824aa4171ec998e2f286081adcf198)) // M27C512-15XF1
ROM_END

void wy150_state::driver_start()
{
	uint8_t *rom = memregion("program")->base();
	for (offs_t base = 0x00000; base < 0x10000; base += 0x4000)
	{
		std::vector<uint8_t> orig(&rom[base], &rom[base + 0x4000]);

		// Line swap is provided by schematic in WY-120 Maintenance Manual
		for (offs_t offset = 0; offset < 0x4000; offset++)
			rom[base | offset] = bitswap<8>(orig[bitswap<14>(offset, 7, 8, 6, 5, 4, 3, 9, 10, 11, 12, 13, 2, 1, 0)], 3, 4, 2, 5, 1, 6, 0, 7);
	}
}

COMP(1991, wy150, 0, 0, wy150, wy150, wy150_state, empty_init, "Wyse Technology", "WY-150 (v1.0)", MACHINE_IS_SKELETON)
COMP(1992, wy120, 0, 0, wy150, wy150, wy150_state, empty_init, "Wyse Technology", "WY-120 (v1.4)", MACHINE_IS_SKELETON)
COMP(1994, wy160, 0, 0, wy150, wy150, wy150_state, empty_init, "Wyse Technology", "WY-160 (v1.7)", MACHINE_IS_SKELETON)
COMP(1994, wy325, 0, 0, wy150, wy150, wy150_state, empty_init, "Wyse Technology", "WY-325 (v3.2)", MACHINE_IS_SKELETON)
