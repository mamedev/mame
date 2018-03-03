// license:BSD-3-Clause
// copyright-holders:David Haywood

/* Happy Fish (xxx-in-1 multigame)

  Primary system components

  SAMSUNG ARM-based Soc
  2x SAMSUNG Flash ROMs
  + a bunch of RAM

  (todo, fill in exact details)

  progress was previously shown with this, booting to the menu at least
  using (I believe) ghosteo.cpp as a base, but the contributor is no
  longer active, and that driver was never submitted.

*/

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"
#include "screen.h"


class hapyfish_state : public driver_device
{
public:
	hapyfish_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	virtual void video_start() override;
	uint32_t screen_update_hapyfish(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	void hapyfish(machine_config &config);
	void hapyfish_map(address_map &map);
};

ADDRESS_MAP_START(hapyfish_state::hapyfish_map)
	AM_RANGE(0x00000000, 0x00003fff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( hapyfish )
INPUT_PORTS_END

uint32_t hapyfish_state::screen_update_hapyfish(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void hapyfish_state::video_start()
{
}



MACHINE_CONFIG_START(hapyfish_state::hapyfish)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM9, 20000000) // ?? ARM baesd CPU
	MCFG_CPU_PROGRAM_MAP(hapyfish_map)
	MCFG_DEVICE_DISABLE()

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 64*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 56*8-1, 0*8, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(hapyfish_state, screen_update_hapyfish)
MACHINE_CONFIG_END



ROM_START( hapyfsh2 )
	ROM_REGION( 0x04000, "maincpu", ROMREGION_ERASE00 )
	// the SoC copies some data from the flash in order to boot

	ROM_REGION( 0x84000000, "flash1", 0 )
	ROM_LOAD( "flash.u6",        0x00000000, 0x84000000, CRC(3aa364a2) SHA1(fe09f549a937ecaf8f7a859522a6635e272fe714) )

	ROM_REGION( 0x84000000, "flash2", 0 )
	ROM_LOAD( "flash.u28",        0x00000000, 0x84000000, CRC(f00a25cd) SHA1(9c33f8e26b84cea957d9c37fb83a686b948c6834) )
ROM_END

GAME( 201?, hapyfsh2,         0,    hapyfish,    hapyfish, hapyfish_state,    0,       ROT0, "bootleg", "Happy Fish (V2 PCB, 302-in-1)", MACHINE_IS_SKELETON )
