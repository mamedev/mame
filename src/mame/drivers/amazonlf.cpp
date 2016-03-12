// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

  uses ADC 'Amazon-LF' SoC, EISC CPU core - similar to crystal system?

*/

#include "emu.h"
#include "cpu/se3208/se3208.h"


class amazonlf_state : public driver_device
{
public:
	amazonlf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
		{ }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;


	virtual void machine_start() override;
	virtual void machine_reset() override;
	UINT32 screen_update_amazonlf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_amazonlf(screen_device &screen, bool state);
};

static ADDRESS_MAP_START( amazonlf_mem, AS_PROGRAM, 32, amazonlf_state )
	AM_RANGE(0x00000000, 0x0007ffff) AM_ROM
ADDRESS_MAP_END

void amazonlf_state::machine_start()
{
}

void amazonlf_state::machine_reset()
{
}

UINT32 amazonlf_state::screen_update_amazonlf(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void amazonlf_state::screen_eof_amazonlf(screen_device &screen, bool state)
{
}


static INPUT_PORTS_START(amazonlf)

INPUT_PORTS_END





static MACHINE_CONFIG_START( amazonlf, amazonlf_state )

	MCFG_CPU_ADD("maincpu", SE3208, 25175000) // ?
	MCFG_CPU_PROGRAM_MAP(amazonlf_mem)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 480)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 479)
	MCFG_SCREEN_UPDATE_DRIVER(amazonlf_state, screen_update_amazonlf)
	MCFG_SCREEN_VBLANK_DRIVER(amazonlf_state, screen_eof_amazonlf)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD_RRRRRGGGGGGBBBBB("palette")
MACHINE_CONFIG_END


ROM_START( crospuzl )
	ROM_REGION( 0x80010, "maincpu", 0 )
	ROM_LOAD("en29lv040a.u5",  0x000000, 0x80010, CRC(d50e8500) SHA1(d681cd18cd0e48854c24291d417d2d6d28fe35c1) )

	ROM_REGION32_LE( 0x8400010, "user1", ROMREGION_ERASEFF ) // Flash
	// mostly empty, but still looks good
	ROM_LOAD("k9f1g08u0a.riser",  0x000000, 0x8400010, CRC(7f3c88c3) SHA1(db3169a7b4caab754e9d911998a2ece13c65ce5b) )
ROM_END


GAME( 200?, crospuzl,        0, amazonlf,  amazonlf, driver_device,         0, ROT0, "<unknown>",          "Cross Puzzle",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
