// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI IRIS POWER Series skeleton driver

    To Do: Everything

    Memory map:
    1fc00000 - 1fc1ffff      Boot ROM

    POWER Series models

    Year  Model   Board  CPU    FPU    Clock    I/D Cache
    1988  4D/120  IP5    R2000  R2010  16.7MHz  64KiB/64KiB
          4D/210  IP9    R3000  R3010  25MHz    64KiB/64KiB
          4D/2x0  IP7    R3000  R3010  25MHz    64KiB/64KiB
          4D/3x0  IP7    R3000  R3010  33MHz    64KiB/64KiB
    1991  4D/4x0  IP15   R3000  R3010  40MHz    64KiB/64KiB

    The second digit indicates the number of CPU boards and/or
    CPUs; models with an 'x' support configurations of 1, 2, 4
    or 8 CPU boards.

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips1.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_ALL         (LOG_UNKNOWN)

#define VERBOSE         (0)
#include "logmacro.h"


namespace {

class ip15_state : public driver_device
{
public:
	ip15_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void ip15(machine_config &config);

protected:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<r3000_device> m_maincpu;
};

void ip15_state::mem_map(address_map &map)
{
	map(0x1fc00000, 0x1fc1ffff).rom().region("user1", 0);
}

static INPUT_PORTS_START( ip15 )
INPUT_PORTS_END

void ip15_state::ip15(machine_config &config)
{
	R3000(config, m_maincpu, 40_MHz_XTAL, 65536, 65536);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip15_state::mem_map);
}

ROM_START( 4d410 )
	ROM_REGION32_BE( 0x20000, "user1", 0 )
	ROMX_LOAD( "ip15prom.070-040x-008.bin", 0x000000, 0x20000, CRC(7290eb66) SHA1(af4285e8db2a9b44fd0fb8a1df4f92faffe87e45), ROM_GROUPDWORD )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                 FULLNAME  FLAGS
COMP( 1991, 4d410, 0,      0,      ip15,    ip15,  ip15_state, empty_init, "Silicon Graphics Inc", "4D/410", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
