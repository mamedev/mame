// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

	SGI O2 workstation skeleton driver

	To Do: Everything

	Memory map:
	1fc00000 - 1fc7ffff      Boot ROM

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"

#define ENABLE_ENTRY_GFX	(1)

#define LOG_UNKNOWN		(1 << 0)
#define LOG_ALL			(LOG_UNKNOWN)

#define VERBOSE			(LOG_UNKNOWN)
#include "logmacro.h"

class o2_state : public driver_device
{
public:
	o2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void o2(machine_config &config);

protected:
	void mem_map(address_map &map);

	required_device<rm7000be_device> m_maincpu;
};

void o2_state::mem_map(address_map &map)
{
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
}

static INPUT_PORTS_START( o2 )
INPUT_PORTS_END

void o2_state::o2(machine_config &config)
{
	RM7000BE(config, m_maincpu, 50000000*6);	// Unknown CPU speed
	m_maincpu->set_icache_size(32768);			// Unknown cache size
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &o2_state::mem_map);
}

ROM_START( o2 )
	ROM_REGION32_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip32prom.rev4.18.bin", 0x000000, 0x080000, CRC(02b3c53d) SHA1(f2cfa7246d67f88fe5490e40dac6c04b1deb4d28), ROM_GROUPDWORD )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY                 FULLNAME             FLAGS
COMP( 1996, o2,       0,      0,      o2,       o2,      o2_state,      empty_init, "Silicon Graphics Inc", "O2 (version 4.18)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
