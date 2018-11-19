// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/**********************************************************************

	SGI 4D/20 IP15 workstation skeleton driver

	To Do: Everything

	Memory map:
	1fc00000 - 1fc1ffff      Boot ROM

**********************************************************************/

#include "emu.h"
#include "cpu/mips/mips1.h"

#define LOG_UNKNOWN		(1 << 0)
#define LOG_ALL			(LOG_UNKNOWN)

#define VERBOSE			(0)
#include "logmacro.h"

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
	void mem_map(address_map &map);

	required_device<r3000_device> m_maincpu;
};

void ip15_state::mem_map(address_map &map)
{
	map(0x1fc00000, 0x1fc1ffff).rom().region("user1", 0);
}

static INPUT_PORTS_START( 4d20 )
INPUT_PORTS_END

void ip15_state::ip15(machine_config &config)
{
	R3000(config, m_maincpu, 12500000);
	m_maincpu->set_addrmap(AS_PROGRAM, &ip15_state::mem_map);
}

ROM_START( 4d20 )
	ROM_REGION32_BE( 0x20000, "user1", 0 )
	ROMX_LOAD( "ip15prom.070-040x-008.bin", 0x000000, 0x20000, CRC(7290eb66) SHA1(af4285e8db2a9b44fd0fb8a1df4f92faffe87e45), ROM_GROUPDWORD )
ROM_END

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY                 FULLNAME                                            FLAGS
COMP( 1991, 4d20, 0,      0,      ip15,    4d20,  ip15_state, empty_init, "Silicon Graphics Inc", "4D/20 (Version 4.0 Fri Apr 26 17:12:22 PDT 1991)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
