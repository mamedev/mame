// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-29 Skeleton

Motorola AMPS Car Phone.

************************************************************************************************************************************/

#include "emu.h"

class ampscarp_state : public driver_device
{
public:
	ampscarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
//      , m_maincpu(*this, "maincpu")
	{ }

private:
//  required_device<cpu_device> m_maincpu;
};

//static ADDRESS_MAP_START( mem_map, AS_PROGRAM, 8, ampscarp_state )
//ADDRESS_MAP_END

static INPUT_PORTS_START( ampscarp )
INPUT_PORTS_END

static MACHINE_CONFIG_START( ampscarp )
MACHINE_CONFIG_END

ROM_START( ampscarp )
	ROM_REGION( 0x20000, "maincpu", 0 ) // unknown cpu
	ROM_LOAD( "motorola_amps_car_phone_dump.bin", 0x0000, 0x20000, CRC(677ec85e) SHA1(219611b6c4b16461705e2df61d79a0f7ac8f529f) )
ROM_END

COMP( 1998, ampscarp, 0, 0, ampscarp, ampscarp, ampscarp_state, 0, "Motorola", "AMPS Car Phone", MACHINE_IS_SKELETON )
