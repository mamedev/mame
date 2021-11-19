// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Gamebar Catherine Wheel skeleton driver

    Notable parts:
        - ST62T28C6: 8-bit microcontroller from STmicro, ST6 series
        - 3x MM5450N LED drivers
        - 8MHz crystal oscillator
        - 60x red LEDs in a circle
        - 5x 7-segment 1-digit LEDs
        - ULN2003A darlington transistor array

    TODO:
        - Everything

*******************************************************************************/

#include "emu.h"
#include "cpu/st62xx/st62xx.h"

namespace
{

class cwheel_state : public driver_device
{
public:
	cwheel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void cwheel(machine_config &config);

protected:
	required_device <st6228_device> m_maincpu;
};

static INPUT_PORTS_START( cwheel )
INPUT_PORTS_END

void cwheel_state::cwheel(machine_config &config)
{
	ST6228(config, m_maincpu, XTAL(8'000'000));
}

ROM_START( cwheel )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "u4.bin", 0x0000, 0x2000, CRC(2eaab25d) SHA1(c41ad372ec25435cdb6a063b094fd785b19dafd6) )
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT  MACHINE  INPUT   CLASS         INIT        ROT   COMPANY    FULLNAME           FLAGS
GAME( 19??, cwheel, 0,      cwheel,  cwheel, cwheel_state, empty_init, ROT0, "Gamebar", "Catherine Wheel", MACHINE_IS_SKELETON )
