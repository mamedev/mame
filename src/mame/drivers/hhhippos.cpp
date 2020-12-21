// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Skeleton driver for "Hungry Hungy Hippos", by ICE (Innovative Concepts in
    Entertainment, Inc.)

    TODO:
    	- Everything

*******************************************************************************/

#include "emu.h"
#include "cpu/m6805/m68hc05.h"

namespace
{

class hhhippos_state : public driver_device
{
public:
	hhhippos_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void hhhippos(machine_config &config);

protected:
	required_device <m68hc705c8a_device> m_maincpu;
};

static INPUT_PORTS_START( hhhippos )
INPUT_PORTS_END

void hhhippos_state::hhhippos(machine_config &config)
{
	M68HC705C8A(config, m_maincpu, XTAL(2'000'000)); //MC68HC705C8P
}

ROM_START( hhhippos )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "68hc705c8.bin", 0x0000, 0x2000, CRC(5c74bcd7) SHA1(3c30ae38647c8f69f7bbcdbeb35b748c8f4c4cd8) ) // MCU internal ROM

	ROM_REGION( 0x30000, "unsorted", 0 )
	ROM_LOAD( "u119.bin", 0x00000, 0x10000, CRC(77c8bd90) SHA1(e9a044d83f39fb617961f8985bc4bed06a03e07b) )
	ROM_LOAD( "u122.bin", 0x10000, 0x20000, CRC(fc188905) SHA1(7bab8feb1f304c9fe7cde31aff4b40e2db56d525) )

ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT   COMPANY FULLNAME                FLAGS
GAME( 1991, hhhippos, 0,      hhhippos, hhhippos, hhhippos_state, empty_init, ROT0, "ICE",  "Hungry Hungry Hippos", MACHINE_IS_SKELETON_MECHANICAL )
