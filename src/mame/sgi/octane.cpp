// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

    SGI Octane workstation skeleton driver

    To Do: Everything

    Memory map:
    1fc00000 - 1fc7ffff      Boot ROM

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"

#define LOG_UNKNOWN     (1U << 1)
#define LOG_ALL         (LOG_UNKNOWN)

#define VERBOSE         (0)
#include "logmacro.h"


namespace {

class octane_state : public driver_device
{
public:
	octane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void octane(machine_config &config);

protected:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<r5000be_device> m_maincpu;
};

void octane_state::mem_map(address_map &map)
{
	map(0x1fc00000, 0x1fcfffff).rom().region("user1", 0);
}

static INPUT_PORTS_START( octane )
INPUT_PORTS_END

void octane_state::octane(machine_config &config)
{
	R5000BE(config, m_maincpu, 50000000*4); // NOTE: Wrong - should be R10000BE!
	m_maincpu->set_icache_size(32768);      // Unknown CPU cache size
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &octane_state::mem_map);
}

ROM_START( octane )
	ROM_REGION64_BE( 0x100000, "user1", 0 )
	ROMX_LOAD( "ip30prom.rev4.9.bin", 0x000000, 0x100000, CRC(10bafb52) SHA1(de250875c608add63749d3f9fb81a82cb58c3586), ROM_GROUPDWORD )
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY                 FULLNAME                                 FLAGS
COMP( 1997, octane,   0,      0,      octane,   octane,  octane_state,  empty_init, "Silicon Graphics Inc", "Octane (Version 6.5 Rev 4.9 05/22/03)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
