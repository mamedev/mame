// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

Skeleton driver for Qume QVT-70 terminal.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class qvt70_state : public driver_device
{
public:
	qvt70_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void qvt70(machine_config &config);

private:
	void mem_map(address_map &map);
	required_device<cpu_device> m_maincpu;
};

void qvt70_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( qvt70 )
INPUT_PORTS_END

void qvt70_state::qvt70(machine_config &config)
{
	Z80(config, m_maincpu, 2'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &qvt70_state::mem_map);
}

/**************************************************************************************************************

Qume QVT-70.
Chips: Z80, Z80 DART, 5x CXK5864CM-70LL/W242575-70LL, 801000-02, 303489-01, DS1231, Button battery, Beeper
Crystals: unreadable

***************************************************************************************************************/

ROM_START( qvt70 )
	ROM_REGION(0x30000, "maincpu", 0)
	ROM_LOAD( "251513-04_revj.u12", 0x00000, 0x10000, CRC(3960bbd5) SHA1(9db306cef09be21ff43c081ebe11e9b46f617861) )
	ROM_LOAD( "251513-03_revj.u11", 0x10000, 0x20000, CRC(c56796fe) SHA1(afe024ff93d5e75dc18041219d61e1a22fc6d883) )
ROM_END

COMP( 1992, qvt70, 0, 0, qvt70, qvt70, qvt70_state, empty_init, "Qume", "QVT-70", MACHINE_IS_SKELETON )
