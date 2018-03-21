// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio CZ-101

	Digital Synthesizer

    Skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7811.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class cz101_state : public driver_device
{
public:
	cz101_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void cz101(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void maincpu_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void cz101_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( cz101 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void cz101_state::machine_start()
{
}

void cz101_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START( cz101_state::cz101 )
	MCFG_CPU_ADD("maincpu", UPD7810, 10_MHz_XTAL) // actually 7811, but internal ROM disabled
	MCFG_CPU_PROGRAM_MAP(maincpu_map)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( cz101 )
	ROM_REGION(0x1000, "maincpu", 0)
	ROM_LOAD("upd7811.bin", 0x0000, 0x1000, CRC(597ac04a) SHA1(96451a764296eaa22aaad3cba121226dcba865f4))

	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("5f3_s40.bin", 0x0000, 0x8000, CRC(c417bc57) SHA1(2aa5bfb76dc0a56797cf5dd547197816cedfa370))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT   MACHINE  INPUT  CLASS        INIT  COMPANY  FULLNAME  FLAGS
CONS( 1984, cz101, 0,      0,       cz101,   cz101, cz101_state, 0,    "Casio", "CZ-101", MACHINE_IS_SKELETON )
