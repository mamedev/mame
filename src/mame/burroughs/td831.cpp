// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Burroughs TD 831 terminal

    Hardware:
    - M6800
    - RAM: 4k + 4k (expansion)
    - ROM: 8x2k (space for a total of 32 ROMs)
    - EAROM 1400
    - 5x 6821 PIA
    - 3x 6850 ACIA
    - 1843.2 KHz XTAL
    - 80x25 characters display (maximum)
    - 5x7 or 7x9 pixels character size (depending on character generator)

    TODO:
    - Everything

    Notes:
    - Quickly jumps into an invalid location

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "machine/er1400.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class td831_state : public driver_device
{
public:
	td831_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void td831(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<m6800_cpu_device> m_maincpu;

	void mem_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void td831_state::mem_map(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x4000, 0x4fff).ram(); // expansion
	map(0x7f00, 0x7fff).unmaprw(); // pia/acia registers
	map(0x8000, 0xbfff).rom().region("device", 0);
	map(0xc000, 0xffff).rom().region("system", 0);
}


//**************************************************************************
//  INPUT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( td831 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void td831_state::machine_start()
{
}

void td831_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void td831_state::td831(machine_config &config)
{
	M6800(config, m_maincpu, 1000000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &td831_state::mem_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( td831 )
	ROM_REGION(0x4000, "device", 0)
	ROM_LOAD("1.bin", 0x2000, 0x0800, CRC(9d56f6c9) SHA1(ed878441046c94a02f84378ae9a23093e2c97e3e))
	ROM_LOAD("2.bin", 0x2800, 0x0800, CRC(60d79031) SHA1(53015c39e1f8ea2f26a9a68ab72682e22448752b))
	ROM_LOAD("3.bin", 0x3000, 0x0800, CRC(fd459a6f) SHA1(bc6fad318884f86bf7e3edb86cb33bdbb54103e5))
	ROM_LOAD("4.bin", 0x3800, 0x0800, CRC(d7dbab60) SHA1(9893f62aa90b655d9603245abf5aeeb502e71d06))

	ROM_REGION(0x4000, "system", 0)
	ROM_LOAD("5.bin", 0x2000, 0x0800, CRC(1995bd6c) SHA1(69b18306e4edca7b7ef72c169cf8422b2598581a))
	ROM_LOAD("6.bin", 0x2800, 0x0800, CRC(faccd4f8) SHA1(b7ea0e88556f332bc28de675cd95caf9eaf0eca0))
	ROM_LOAD("7.bin", 0x3000, 0x0800, CRC(1412d84a) SHA1(d4f6141322f2020a88ce4c434d9ee6cfa7ac989d))
	ROM_LOAD("8.bin", 0x3800, 0x0800, CRC(1e3473b9) SHA1(85e370a6b1f6b767a3aed750633359fac984043d))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY      FULLNAME  FLAGS
COMP( 1976, td831, 0,      0,      td831,   td831, td831_state, empty_init, "Burroughs", "TD 831", MACHINE_IS_SKELETON )
