// license:BSD-3-Clause
// copyright-holders:
/************************************************************************************

        AlphaSmart 3000

AlphaSmart 3000 PCB:
            ___________                                                                _____________
           /    ::    |_______________________________________________________________/             \
   _______/ ::  ::                              _________      _________              ________       \_____________
  ||RS232|      ::   28F008B3                  |________|     |________| PCB REV 2.8 |_______|  BATT        | USB |
  ||_____|      ::                 74HC574                                                    CR2032   XTAL |_____|
  |__                  HY62U8200                       HC30M                                          A120I0E     |
   __| SP3223ECA                                                                                                  |
 _|_                     DragonBall EZ                                                                 PDIUSBD11D |
|___|<-Power             MC68EZ328PU16V                                    :)                              _______|
  |                                                                   Rise and Shout                      |
  |                            XTAL                                   the AlphaSmart's                    |
  |__   __      _____                     SW        HC132A             Out!!!                             |
     | |__|    /     | HC132A     HC74A  on/off                                       ____________________|
     |________/      |_______________________________________________________________/

The later AlphaSmart models' firmware can be updated using the Manager application (Windows / Mac) and a USB cable.
Each update comprises two files, the "os" and the "smallos". Those files do not include the full Operating System image.
Two version updaters known:
- System 3 Neo Jul 11 2013, 09:44:53 + OS 3KNeo Small ROM, included with Manager 3.93
- System 3 Neo Jan 27 2010, 13:44:00 + OS 3KNeo Small ROM, included with Manager 3.60

    TODO:
    - Everything

*************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68328.h"
#include "machine/ram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

namespace
{

class alphasmart3k_state : public driver_device
{
public:
	alphasmart3k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lcdc0(*this, "ks0066_0")
		, m_lcdc1(*this, "ks0066_1")
		, m_ram(*this, RAM_TAG)
	{
	}

	void alphasmart3k(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc0;
	required_device<hd44780_device> m_lcdc1;
	required_device<ram_device> m_ram;

	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;
};

static INPUT_PORTS_START( alphasmart3k )
INPUT_PORTS_END

void alphasmart3k_state::alphasmart3k(machine_config &config)
{
	// Basic machine hardware
	MC68328(config, m_maincpu, 16'000'000); // MC68EZ328PU16V, clock unverified

	// Values from AlphaSmart 2000, not confirmed for AlphaSmart 3000
	// AlphaSmart 3000 uses a Data Image CM4040 LCD display
	KS0066_F05(config, m_lcdc0, 0);
	m_lcdc0->set_lcd_size(2, 40);
	KS0066_F05(config, m_lcdc1, 0);
	m_lcdc1->set_lcd_size(2, 40);

	RAM(config, RAM_TAG).set_default_size("256K");

	SOFTWARE_LIST(config, "kapps_list").set_original("alphasmart_kapps");
}

// ROM definitions

ROM_START( asma3k )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "28f008b3.u1", 0x000000, 0x100000, CRC(73a24834) SHA1(a47e6a6d286feaba4e671a6373632222113f9276) )
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT COMPAT MACHINE       INPUT         CLASS               INIT        COMPANY             FULLNAME           FLAGS
COMP( 2000, asma3k, 0,     0,     alphasmart3k, alphasmart3k, alphasmart3k_state, empty_init, "AlphaSmart, Inc.", "AlphaSmart 3000", MACHINE_IS_SKELETON )
