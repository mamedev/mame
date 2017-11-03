// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-03 Skeleton

Convergent Technologies 68k Mightyframe S80

Chips: CPU is a square MC68020RC12B. Also, 3x Z80SCC and 2 undumped proms labelled "7201087B"(@15D) and "7201089B"(@15F)
       The only photo shows just a part of the board, the only crystal is a tiny tubular one next to a OKI M58321 chip.

Manuals: http://mightyframe.blogspot.com.au/p/manuals.html

************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class mightyframe_state : public driver_device
{
public:
	mightyframe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( mightyframe )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, mightyframe_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( mightyframe )
MACHINE_CONFIG_END

ROM_START( mightyframe )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "72-01231.26c", 0x0000, 0x8000, CRC(41faf884) SHA1(d0c6f35394b4006bbe9a3f81b658ded37f41d86f) )
ROM_END

COMP( 1985?, mightyframe, 0, 0, mightyframe, mightyframe, mightyframe_state, 0, "Convergent Technologies", "Mightyframe", MACHINE_IS_SKELETON )
