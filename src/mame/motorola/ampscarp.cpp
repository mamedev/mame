// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-10-29 Skeleton

Motorola AMPS Car Phone.

Nothing is really known about the hardware. The dump contains MC68HC11 code, but has no vector table. It seems likely that
whatever MCU type this uses boots from an internal ROM/PROM/EPROM but can also execute a large bankswitched external program.

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/mc68hc11/mc68hc11.h"


namespace {

class ampscarp_state : public driver_device
{
public:
	ampscarp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ampscarp(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	required_device<cpu_device> m_maincpu;
};

void ampscarp_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( ampscarp )
INPUT_PORTS_END

void ampscarp_state::ampscarp(machine_config &config)
{
	MC68HC11A1(config, m_maincpu, 8'000'000); // type and clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &ampscarp_state::mem_map);
}

ROM_START( ampscarp )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "motorola_amps_car_phone_dump.bin", 0x0000, 0x20000, CRC(677ec85e) SHA1(219611b6c4b16461705e2df61d79a0f7ac8f529f) )
ROM_END

} // anonymous namespace


COMP( 1998, ampscarp, 0, 0, ampscarp, ampscarp, ampscarp_state, empty_init, "Motorola", "AMPS Car Phone", MACHINE_IS_SKELETON )
