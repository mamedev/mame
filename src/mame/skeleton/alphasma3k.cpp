// license:BSD-3-Clause
// copyright-holders:
/***************************************************************************************************

AlphaSmart 3000

TODO:
- SW traps asap after writing to uninitialized A0, going off the rails.
- Snippet at PC=400148 is supposed to transfer the rest of the IPL to main RAM,
  but it fails looping at PC=40016c cmpa.w A0, A1 (DragonBall-EZ core bug?)

====================================================================================================

PCB:
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

***************************************************************************************************/

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
		, m_ipl(*this, "ipl")
	{
	}

	void alphasmart3k(machine_config &config);

protected:
	required_device<cpu_device> m_maincpu;
	required_device<hd44780_device> m_lcdc0;
	required_device<hd44780_device> m_lcdc1;
	required_device<ram_device> m_ram;
	required_region_ptr<u16> m_ipl;

	std::unique_ptr<bitmap_ind16> m_tmp_bitmap;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void main_map(address_map &map) ATTR_COLD;
};

void alphasmart3k_state::machine_start()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_ram(0x00000000, m_ram->size() - 1, m_ram->pointer());
}

void alphasmart3k_state::machine_reset()
{
	// TODO: expects specific initialized 68k values, including registers?
	// dc.w 1111 is vector $2c
	memcpy(m_ram->pointer(), memregion("ipl")->base(), 0x100);
}

void alphasmart3k_state::main_map(address_map &map)
{
//  map(0x0000'0000, 0x0003'ffff).ram().share("ram");
	map(0x0040'0000, 0x004f'ffff).rom().region("ipl", 0);
}

static INPUT_PORTS_START( alphasmart3k )
INPUT_PORTS_END



void alphasmart3k_state::alphasmart3k(machine_config &config)
{
	// Basic machine hardware
	MC68EZ328(config, m_maincpu, 16'000'000); // MC68EZ328PU16V, clock unverified
	m_maincpu->set_addrmap(AS_PROGRAM, &alphasmart3k_state::main_map);

	// Values from AlphaSmart 2000, not confirmed for AlphaSmart 3000
	// AlphaSmart 3000 uses a Data Image CM4040 LCD display, LCD is 40x4 according to ref
	KS0066(config, m_lcdc0, 270'000); // TODO: Possibly wrong device type, needs confirmation; clock not measured, datasheet typical clock used
	m_lcdc0->set_default_bios_tag("f05");
	m_lcdc0->set_lcd_size(4, 40);
	KS0066(config, m_lcdc1, 270'000); // TODO: Possibly wrong device type, needs confirmation; clock not measured, datasheet typical clock used
	m_lcdc1->set_default_bios_tag("f05");
	m_lcdc1->set_lcd_size(4, 40);

	RAM(config, RAM_TAG).set_default_size("256K");

	SOFTWARE_LIST(config, "kapps_list").set_original("alphasmart_kapps");
}

// ROM definitions

ROM_START( asma3k )
	ROM_REGION16_BE( 0x100000, "ipl", 0 )
	ROM_LOAD16_WORD( "28f008b3.u1", 0x000000, 0x100000, CRC(73a24834) SHA1(a47e6a6d286feaba4e671a6373632222113f9276) )
ROM_END

} // anonymous namespace

//    YEAR  NAME    PARENT COMPAT MACHINE       INPUT         CLASS               INIT        COMPANY             FULLNAME           FLAGS
COMP( 2000, asma3k, 0,     0,     alphasmart3k, alphasmart3k, alphasmart3k_state, empty_init, "AlphaSmart, Inc.", "AlphaSmart 3000", MACHINE_IS_SKELETON )
