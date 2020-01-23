// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for MC88100-based NCD X terminals.

****************************************************************************/

#include "emu.h"
#include "cpu/m88000/m88000.h"
#include "screen.h"

class ncd88k_state : public driver_device
{
public:
	ncd88k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{
	}

	void ncd19c(machine_config &config);

private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ncd19c_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

u32 ncd88k_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void ncd88k_state::ncd19c_map(address_map &map)
{
	map(0x00000000, 0x000015fff).rom().region("program", 0);
}

static INPUT_PORTS_START(ncd19c)
INPUT_PORTS_END

void ncd88k_state::ncd19c(machine_config &config)
{
	MC88100(config, m_maincpu, 15'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ncd88k_state::ncd19c_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(125'000'000, 1680, 0, 1280, 1063, 0, 1024); // 74.4 kHz horizontal, 70 Hz vertical
	m_screen->set_screen_update(FUNC(ncd88k_state::screen_update));
}

ROM_START(ncd19c)
	ROM_REGION32_BE(0x16000, "program", 0)
	// These dumps have very strange lengths. The actual ROMs should be standard EEPROM types.
	ROM_LOAD16_BYTE("ncd19c-e.rom", 0x0000, 0xb000, CRC(01e31b42) SHA1(28da6e4465415d00a739742ded7937a144129aad) BAD_DUMP)
	ROM_LOAD16_BYTE("ncd19c-o.rom", 0x0001, 0xb000, CRC(dfd9be7c) SHA1(2e99a325b039f8c3bb89833cd1940e6737b64d79) BAD_DUMP)
ROM_END

COMP(1990, ncd19c, 0, 0, ncd19c, ncd19c, ncd88k_state, empty_init, "Network Computing Devices", "NCD19c", MACHINE_IS_SKELETON)
