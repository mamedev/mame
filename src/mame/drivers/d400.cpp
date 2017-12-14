// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Data General Dasher 400 series terminals.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/mc68681.h"
#include "machine/x2212.h"

class d400_state : public driver_device
{
public:
	d400_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};

static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, d400_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0x4800, 0x48ff) AM_RAM
	AM_RANGE(0x5000, 0x50ff) AM_RAM
	AM_RANGE(0x6000, 0x6fff) AM_RAM
	AM_RANGE(0x7800, 0x780f) AM_DEVREADWRITE("duart", scn2681_device, read, write)
	AM_RANGE(0x7880, 0x78bf) AM_DEVREADWRITE("novram", x2210_device, read, write)
	AM_RANGE(0x7c00, 0x7c00) AM_WRITENOP
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( d461 )
INPUT_PORTS_END

static MACHINE_CONFIG_START( d461 )
	MCFG_CPU_ADD("maincpu", M6809, 4'000'000)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	MCFG_DEVICE_ADD("novram", X2210, 0)

	MCFG_DEVICE_ADD("duart", SCN2681, XTAL_3_6864MHz)
MACHINE_CONFIG_END



/**************************************************************************************************************

Data General D461.
Chips: SCN2681A, X2210P, 2x HM6116P-2, 2x HM6264P-20, HD68B09EP, CRT9007, 1x 8-sw dip.
Crystals: 3.6864, 59.2920

***************************************************************************************************************/

ROM_START( d461 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "dgc_100_5776-05.bin", 0x0000, 0x8000, CRC(fdce2132) SHA1(82eac1751c31f99d4490505e16af5e7e7a52b310) )
ROM_END

COMP( 1986, d461, 0, 0, d461, d461, d400_state, 0, "Data General", "Dasher D461", MACHINE_IS_SKELETON )
