/* Nokia 3310 */

// according to Wikipedia this is based around a MAD2WD1 CPU (based on ARM7TDMI core)

// I can only find update files, I don't know if these can be reconstructed into full ROMs for emulation, or if we need something more??
// if anybody has solid information to aid in the emulation of this (or other phones) please contribute.

#include "emu.h"
#include "cpu/arm7/arm7.h"
#include "cpu/arm7/arm7core.h"


class noki3310_state : public driver_device
{
public:
	noki3310_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	required_device<cpu_device> m_maincpu;
};



static ADDRESS_MAP_START( noki3310_map, AS_PROGRAM, 32, noki3310_state )
	AM_RANGE(0x00000000, 0x000fffff) AM_ROM
ADDRESS_MAP_END


static INPUT_PORTS_START( noki3310 )
INPUT_PORTS_END


static MACHINE_CONFIG_START( noki3310, noki3310_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM7, 50000000) // MAD2WD1 - speed unknown
	MCFG_CPU_PROGRAM_MAP(noki3310_map)

MACHINE_CONFIG_END

ROM_START( noki3310 )
	ROM_REGION(0x0200000, "maincpu", 0 )
	// these 2 are apparently the 6.39 update firmware data
	ROM_LOAD( "NHM5NY06.390",   0x000000, 0x0131161, CRC(5dfb1af7) SHA1(3a8ad82dc239b0cd18be60f537c4d0e07881155d) )
	ROM_REGION(0x0200000, "misc", 0 )
	ROM_LOAD( "NHM5NY06.39I",   0x000000, 0x0090288, CRC(ec214ee4) SHA1(f5b3b9ceaa7280d5246dd70d5696f8f6983122fc) )
ROM_END


GAME( 2000, noki3310, 0,        noki3310,  noki3310, driver_device,  0,  ROT0, "Nokia", "Nokia 3310", MACHINE_NO_SOUND | MACHINE_IS_SKELETON )
