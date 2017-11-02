// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Transitional Technology Inc., single-board computer. Model number not known, zipfile was named "TTI_10012000.zip"

Chips: NCR 53C90A, Motorola MC68901P, Fujitsu 8464A-10L (8KB static ram), and 14 undumped proms.

Other: LED, 20MHz crystal. Next to the MC68901P is another chip just as large (48 pin DIL), with a huge sticker covering all details.
       Assumed to be a Motorola CPU such as MC68000, MC68008, etc.

************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class tti_state : public driver_device
{
public:
	tti_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> maincpu;
};

static INPUT_PORTS_START( tti )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, tti_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( tti )
MACHINE_CONFIG_END

ROM_START( tti )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "tti_10012000_rev2.3.bin", 0x0000, 0x8000, CRC(95a5bce8) SHA1(46d7c99e37ca5598aec2062dfd9759853a237c14) )
	ROM_LOAD( "tti_10012000_rev1.7.bin", 0x0000, 0x8000, CRC(6660c059) SHA1(05d97009b5b8034dda520f655c73c474da97f822) )
ROM_END

COMP( 1989, tti, 0, 0, tti, tti, tti_state, 0, "Transitional Technology Inc", "Unknown TTI SBC", MACHINE_IS_SKELETON )
