// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    ADDS 4000/260

    ASCII/ANSI terminal

    Hardware:
    - P-80C32-16
    - KM622560LP-7L (32k RAM)
    - CY6225LL-70 (32k RAM)
    - LSI Victor 006-9802760 REV B
    - KM622560LP-7L x2 (32k RAM x2)
    - 16 MHz XTAL, 44.976 MHz XTAL

    TODO:
    - Everything

    Notes:
    - Other models in this line: 4000/260C, 4000/260LF, 4000/260LFC
    - Sold under the ADDS brand, but ADDS was part of SunRiver Data Systems
      by then, which became Boundless Technologies.

***************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"


namespace {


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class _4000_260_state : public driver_device
{
public:
	_4000_260_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void _4000_260(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i80c32_device> m_maincpu;

	void mem_map(address_map &map) ATTR_COLD;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void _4000_260_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( _4000_260 )
INPUT_PORTS_END


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

void _4000_260_state::machine_start()
{
}

void _4000_260_state::machine_reset()
{
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void _4000_260_state::_4000_260(machine_config &config)
{
	I80C32(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &_4000_260_state::mem_map);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( 4000_260 )
	ROM_REGION(0x40000, "maincpu", 0)
	// 598-0010669 3.16 SunRiver Data Systems 1995
	ROM_LOAD("4000_260.bin", 0x00000, 0x40000, CRC(b957cd1d) SHA1(1b1185174ba95dca004169e4e1b51b05c8991c43))
ROM_END


} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME      PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY  FULLNAME    FLAGS
COMP( 1995, 4000_260, 0,      0,      _4000_260, _4000_260, _4000_260_state, empty_init, "ADDS",  "4000/260", MACHINE_IS_SKELETON )
