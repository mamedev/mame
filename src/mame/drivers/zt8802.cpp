// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-02 Skeleton

Ziatech ZT-8802 single-board computer. Advert says: V40 CPU @ 8MHz, 1MB DRAM, 512K EPROM, 3 RS-232 PORTS.

CPU is NEC D70208L-10 (V40) which is a V20 with 8251,8253,8255 included.

Chips: Maxim MAX249CQH, Exar XT16C452CJPS, Ziatech ICPSMCI-16C49A, 2x GAL22V10D, 2x ACT15245, 2x ACT11245,
       ACT11240, PALCE 16V??-15JC, 2x CYM1465LPD-120C (RAM).

Other: A 3.6volt battery, a tiny crystal, a red LED, and about 2 dozen jumpers.

************************************************************************************************************************************/

#include "emu.h"

class zt8802_state : public driver_device
{
public:
	zt8802_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void zt8802(machine_config &config);
private:
	//  required_device<cpu_device> m_maincpu;
};

static INPUT_PORTS_START( zt8802 )
INPUT_PORTS_END

void zt8802_state::zt8802(machine_config &config)
{
}

ROM_START( zt8802 )
	ROM_REGION( 0x80000, "roms", 0 )
	ROM_LOAD( "c103207-218 a.rom", 0x00000, 0x80000, CRC(fc1c6e99) SHA1(cfbb2f0c9927bac5abc85c12d2b82f7da46cab03) )
ROM_END

COMP( 1994, zt8802, 0, 0, zt8802, zt8802, zt8802_state, empty_init, "Ziatech", "ZT-8802 SBC", MACHINE_IS_SKELETON )
