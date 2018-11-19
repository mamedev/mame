// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

	SGI Crimson deskside skeleton driver

	To Do: Everything

	Memory map:
	1fc00000 - 1fc7ffff      Boot ROM

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips3.h"

#define LOG_UNKNOWN		(1 << 0)
#define LOG_ALL			(LOG_UNKNOWN)

#define VERBOSE			(0)
#include "logmacro.h"

class crimson_state : public driver_device
{
public:
	crimson_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void crimson(machine_config &config);

protected:
	void mem_map(address_map &map);

	required_device<r4000be_device> m_maincpu;
};

void crimson_state::mem_map(address_map &map)
{
	map(0x1fc00000, 0x1fc7ffff).rom().region("user1", 0);
}

static INPUT_PORTS_START( crimson )
INPUT_PORTS_END

void crimson_state::crimson(machine_config &config)
{
	R4000BE(config, m_maincpu, 50000000*2);
	m_maincpu->set_icache_size(32768);
	m_maincpu->set_dcache_size(32768);
	m_maincpu->set_addrmap(AS_PROGRAM, &crimson_state::mem_map);
}

ROM_START( crimson )
	ROM_REGION32_BE( 0x80000, "user1", 0 )
	ROMX_LOAD( "ip17prom.070-081x-005.bin", 0x000000, 0x080000, CRC(d62e8c8e) SHA1(b335213ecfd02ca3185b6ba1874a8b76f908c68b), ROM_GROUPDWORD )
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT    CLASS          INIT        COMPANY                 FULLNAME                               FLAGS
COMP( 1992, crimson,  0,      0,      crimson,  crimson, crimson_state, empty_init, "Silicon Graphics Inc", "Crimson (R4000, 100MHz, Ver. 4.0.3)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
