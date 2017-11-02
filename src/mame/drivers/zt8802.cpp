// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Ziatech ZT-8802 single-board computer. Advert says: V40 CPU @ 8MHz, 1MB DRAM, 512K EPROM, 3 RS-232 PORTS.

Chips: Maxim MAX249CQH, Exar XT16C452CJPS, Ziatech ICPSMCI-16C49A, 2x GAL22V10D, NEC D70208L-10, 2x ACT15245, 2x ACT11245, ACT11240,
       PALCE 16V??-15JC, 2x CYM1465LPD-120C.

Other: A 3.6volt battery, a tiny crystal, a red LED, and about 2 dozen jumpers.

************************************************************************************************************************************/

#include "emu.h"
//#include "cpu/mcs51/mcs51.h"

class zt8802_state : public driver_device
{
public:
	zt8802_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//		, m_maincpu(*this, "maincpu")
	{ }

protected:
//	required_device<i80c52_device> m_maincpu;
};

static INPUT_PORTS_START( zt8802 )
INPUT_PORTS_END

//static ADDRESS_MAP_START( prg_map, AS_PROGRAM, 8, zt8802_state )
//ADDRESS_MAP_END

static MACHINE_CONFIG_START( zt8802 )
MACHINE_CONFIG_END

ROM_START( zt8802 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c103207-218 a.rom", 0x00000, 0x80000, CRC(fc1c6e99) SHA1(cfbb2f0c9927bac5abc85c12d2b82f7da46cab03) )
ROM_END

COMP( 1994, zt8802, 0, 0, zt8802, zt8802, zt8802_state, 0, "Ziatech", "ZT-8802 SBC", MACHINE_IS_SKELETON )
