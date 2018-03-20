// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for TeleVideo 965 video display terminal.

TeleVideo 9320 appears to run on similar hardware with a 2681 DUART replacing the ACIAs.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/g65816/g65816.h"
//#include "machine/mos6551.h"
#include "machine/nvram.h"
#include "screen.h"

class tv965_state : public driver_device
{
public:
	tv965_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
	{ }

	void tv965(machine_config &config);
private:
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
};

u32 tv965_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tv965_state::mem_map(address_map &map)
{
	map.global_mask(0x3ffff);
	map(0x00000, 0x01fff).ram().share("nvram");
	map(0x10000, 0x1ffff).rom().region("eprom1", 0);
	map(0x30000, 0x3ffff).rom().region("eprom2", 0);
}

static INPUT_PORTS_START( tv965 )
INPUT_PORTS_END

MACHINE_CONFIG_START(tv965_state::tv965)
	MCFG_CPU_ADD("maincpu", G65816, 44.4528_MHz_XTAL / 10)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_NVRAM_ADD_0FILL("nvram") // CXK5864BP-10L + battery

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(26.9892_MHz_XTAL, 1020, 0, 800, 378, 0, 350)
	//MCFG_SCREEN_RAW_PARAMS(44.4528_MHz_XTAL, 1680, 0, 1320, 378, 0, 350)
	MCFG_SCREEN_UPDATE_DRIVER(tv965_state, screen_update)
MACHINE_CONFIG_END

/**************************************************************************************************************

Televideo TVI-965
Chips: G65SC816P-5, SCN2672TC5N40, 271582-00 (unknown square chip), 2x UM6551A, Beeper, DS1231
Crystals: 44.4528, 26.9892, 3.6864

***************************************************************************************************************/

ROM_START( tv965 )
	ROM_REGION(0x10000, "eprom1", 0)
	ROM_LOAD( "180003-30h.u8", 0x00000, 0x10000, CRC(c7b9ca39) SHA1(1d95a8b0a4ea5caf3fb628c44c7a3567700a0b59) )

	ROM_REGION(0x10000, "eprom2", 0)
	ROM_LOAD( "180003-38h.u9", 0x00000, 0x08000, CRC(30fae408) SHA1(f05bb2a9ce2df60b046733f746d8d8a1eb3ac8bc) )
ROM_END

COMP( 1989, tv965, 0, 0, tv965, tv965, tv965_state, 0, "TeleVideo Systems", "TeleVideo 965", MACHINE_IS_SKELETON )
