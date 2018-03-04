// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for AT&T 630 MTG terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68681.h"
#include "screen.h"

class att630_state : public driver_device
{
public:
	att630_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void att630(machine_config &config);
	void mem_map(address_map &map);
private:
	required_device<cpu_device> m_maincpu;
};

u32 att630_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

ADDRESS_MAP_START(att630_state::mem_map)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x200000, 0x20001f) AM_DEVREADWRITE8("duart1", scn2681_device, read, write, 0x00ff)
	AM_RANGE(0x200020, 0x20003f) AM_DEVREADWRITE8("duart2", scn2681_device, read, write, 0x00ff)
	AM_RANGE(0x760000, 0x77ffff) AM_RAM
	AM_RANGE(0x780000, 0x7bffff) AM_RAM
	AM_RANGE(0x7c0000, 0x7fffff) AM_RAM
	AM_RANGE(0xe00000, 0xe03fff) AM_NOP // 0x00ff mask
ADDRESS_MAP_END

static INPUT_PORTS_START( att630 )
INPUT_PORTS_END

MACHINE_CONFIG_START(att630_state::att630)
	MCFG_CPU_ADD("maincpu", M68000, XTAL(40'000'000) / 4) // clock not confirmed
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(87'183'360), 1376, 0, 1024, 1056, 0, 1024)
	MCFG_SCREEN_UPDATE_DRIVER(att630_state, screen_update)

	MCFG_DEVICE_ADD("duart1", SCN2681, XTAL(3'686'400))

	MCFG_DEVICE_ADD("duart2", SCN2681, XTAL(3'686'400))
MACHINE_CONFIG_END


/**************************************************************************************************************

AT&T 630 MTG.
Chips: 2x SCN2681A, AT&T 492F proprietory, blank chip, MC68000P10, MB113F316 (square), MB113F316 (DIL), PAL16R4ACN
Crystals: 40MHz, 87.18336, 3.6864? (hard to read)

***************************************************************************************************************/

ROM_START( att630 )
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD16_BYTE( "460621-1.bin", 0x00000, 0x10000, CRC(136749cd) SHA1(15378c292ddc7384cc69a35de55b69257a9f2a1c) )
	ROM_LOAD16_BYTE( "460620-1.bin", 0x00001, 0x10000, CRC(27ab77f0) SHA1(5ff1d9ee5a69dee308d62c447ee67e1888afab0e) )
	ROM_LOAD16_BYTE( "460623-1.bin", 0x20000, 0x10000, CRC(aeae12fb) SHA1(fa3ce26e4622875aa1dea7cf1bd1df237010ff2b) )
	ROM_LOAD16_BYTE( "460622-1.bin", 0x20001, 0x10000, CRC(c108c1e0) SHA1(ef01349e890b8a4117c01e78d1c23fbd113ba58f) )
ROM_END

COMP( 1987, att630, 0, 0, att630, att630, att630_state, 0, "AT&T", "630 MTG", MACHINE_IS_SKELETON )
