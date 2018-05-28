// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-29 Skeleton

Motorola Powerstack II. CPU is a PowerPC 604e @ 300MHz.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/powerpc/ppc.h"

class powerstack_state : public driver_device
{
public:
	powerstack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//      , m_maincpu(*this, "maincpu")
	{ }

void powerstack(machine_config &config);
private:
//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( powerstack )
INPUT_PORTS_END

MACHINE_CONFIG_START(powerstack_state::powerstack)
//  MCFG_DEVICE_ADD("maincpu", PPC604, 300'000'000) // PPC604E @ 300MHz
//  MCFG_DEVICE_PROGRAM_MAP(mem_map)
MACHINE_CONFIG_END

ROM_START( powerstk )
	ROM_REGION( 0x80000, "roms", 0 )
	ROM_LOAD( "motorola_powerstack2.bin", 0x0000, 0x80000, CRC(948e8fcd) SHA1(9a8c32b621c98bc33ee525f66747c34d39851685) )
ROM_END

COMP( 1996, powerstk, 0, 0, powerstack, powerstack, powerstack_state, empty_init, "Motorola", "Powerstack II", MACHINE_IS_SKELETON )
