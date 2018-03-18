// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Casio HT-6000

    SD ("Spectrum Dynamic") Synthesizer

    Skeleton driver

***************************************************************************/

#include "emu.h"
#include "cpu/upd7810/upd7811.h"
#include "cpu/mcs48/mcs48.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class ht6000_state : public driver_device
{
public:
	ht6000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void ht6000(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void maincpu_map(address_map &map);
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void ht6000_state::maincpu_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( ht6000 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void ht6000_state::machine_start()
{

}

void ht6000_state::machine_reset()
{

}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

MACHINE_CONFIG_START( ht6000_state::ht6000 )
	MCFG_CPU_ADD("maincpu", UPD7810, 12_MHz_XTAL)
	MCFG_CPU_PROGRAM_MAP(maincpu_map)

	MCFG_CPU_ADD("keycpu", I8049, 10_MHz_XTAL)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( ht6000 )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("m245p_8720ex707.bin", 0x0000, 0x8000, CRC(c3063c07) SHA1(f012add068d7d765bcb701ad372c0bab3302a776))
	ROM_LOAD("m245d_8720ex703.bin", 0x8000, 0x8000, CRC(bc28b60d) SHA1(6f4be2861adea57352f0d52c61e004a5c022854a))

	ROM_REGION(0x800, "keycpu", 0)
	ROM_LOAD("187_8734h7.bin", 0x000, 0x800, CRC(47b47af7) SHA1(8f0515f95dcc6e224a8a59e0c2cd7ddb4796e34e))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT   CLASS         INIT  COMPANY  FULLNAME   FLAGS
CONS( 1987, ht6000, 0,      0,       ht6000,  ht6000, ht6000_state, 0,    "Casio", "HT-6000", MACHINE_IS_SKELETON )
