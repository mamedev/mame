// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/******************************************************************************

    Skeleton driver for "Twin Basket", by Recreativos Pasqual

    TODO:
    	- Implement ST6225 microcontroller handling based on existing
    	  ST6228 core
        - Everything else (peripherals, artwork, etc.)

*******************************************************************************/

#include "emu.h"
#include "cpu/st62xx/st62xx.h"

namespace
{

class twinbskt_state : public driver_device
{
public:
	twinbskt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void twinbskt(machine_config &config);

protected:
	required_device <st6228_device> m_maincpu;
};

static INPUT_PORTS_START( twinbskt )
INPUT_PORTS_END

void twinbskt_state::twinbskt(machine_config &config)
{
	ST6228(config, m_maincpu, XTAL(8'000'000));
}

ROM_START( twinbskt )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "fab_b-159_b-basket.ic8", 0x0000, 0x1000, CRC(186f1de5) SHA1(d0bc48097e17d9515cec99f86bc92b1399fbeb0b) )
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  MACHINE   INPUT     CLASS           INIT        ROT   COMPANY                FULLNAME       FLAGS
GAME( 1995, twinbskt, 0,      twinbskt, twinbskt, twinbskt_state, empty_init, ROT0, "Recreativos Pasqual", "Twin Basket", MACHINE_IS_SKELETON )
