// license:BSD-3-Clause
// copyright-holders:
/*******************************************************************************

    Skeleton driver for Merit Scorpion darts machines.
    The same PCB is used also on other Merit darts machines, like Regent Darts
    and Pub Time Darts II.

    Hardware overview:
    Main CPU: Dallas DS80C3202-UM or compatible (80C31 on older models)
    Sound: DAC?
    NVRAM: Dallas DS1220Y-120 or compatible
    Other: Dallas DS1232 MicroMonitor
           Dallas DS1204U-3 Electronic Key (not populated)
    OSCs: 12.000 MHz, 3.2768 MHz
    Dips: 2 x 8 dips banks

*******************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "speaker.h"

namespace {

class merits_state : public driver_device
{
public:
	merits_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void scrpiond(machine_config &config);
	void scrpiondold(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void merits_state::mem_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

void merits_state::io_map(address_map &map)
{
	map(0x8000, 0x87ff).ram().share("nvram");
	//map(0x9000, 0x9000).r();
	//map(0xa000, 0xa000).r();
	//map(0xc000, 0xc000).w();
	//map(0xd000, 0xd000).w();
	//map(0xe000, 0xe000).w();
	//map(0xf000, 0xf000).w();
	//map(0xf800, 0xf800).w();
}

static INPUT_PORTS_START(scrpiond)
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

void merits_state::scrpiond(machine_config &config)
{
	DS80C320(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &merits_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &merits_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // DS1220Y
}

void merits_state::scrpiondold(machine_config &config)
{
	I80C31(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &merits_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &merits_state::io_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // DS1220Y
}

ROM_START(scrpiond)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "27c512.u7", 0x00000, 0x10000, CRC(06cdf965) SHA1(4cdac131063fc0dd954eaaee2ae40d5731f83469) )
ROM_END

ROM_START(scrpionda)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "4978-22_u7-r5_c1997_mii.u7", 0x00000, 0x10000, CRC(e647a17e) SHA1(4a7b9e2af3656a1b6f4ffd8c17b68eec5c534776) )
ROM_END

// Old PCB model, i80C31 instead of 80C32. The DS1204U-3 socket is still unpopulated. Other PCB components stays the same.
ROM_START(scrpiondb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "4778-02_u7-r02_c1994_mii.u7", 0x00000, 0x10000, CRC(57a5083d) SHA1(edb94dbb9e040e960c45406b082ede133574351a) )
ROM_END

/* Older PCB (silkcreened "© 1989 Merit Industries"), i80C31 instead of 80C32 and without socket for DS1204U-3.
   The "Solo Challenger" was an upgrade kit, including a replacement EEPROM, a new button, some new art, and a
   complete manual with schematics.
   The standard "Pub Time Darts II Plus 2" machine had a separate PCB for cheat detection with ultrasounds
   (named MIC/ULTRA BOARD):
  ____________________________________________
 | :  RCA CONNECTORS -> (o) (o) (o) (o)  (o)  |
 | : <- CONN J2                               |
 | _________                                  |
 | |_MPQ2484|                                 |
 |                       _________   CONN J9  |
 |                       4116R-001      ···   |
 | _________  _________  _________            |
 | |_TPQ2807| T74LS02B1  |_LM324N_|           |
 |            _________  _________            |
 |          CD74HC4060E  |_LM324N_|           |
 |            _________  _________  _________ |
 |           |_DM74123N| SN74LS08N  SN74LS221N|
 |              CONN J10 -> ····              |
 | :::::::::::::::::::::::::::::::::::::::::: |
 |___________________CONN J1__________________|
*/
ROM_START(pubtimed2ch)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "4378-07_u7_r4b_c1992_mii.u7", 0x00000, 0x08000, CRC(6d5c0634) SHA1(45f4ed2e984f2525a1ed680bbc2a11eab93b0bca) ) // 27256
ROM_END

} // Anonymous namespace

//   YEAR  NAME         PARENT    COMPAT       MACHINE   INPUT         CLASS       INIT  COMPANY  FULLNAME                                                        FLAGS
GAME(1999, scrpiond,    0,        scrpiond,    scrpiond, merits_state, empty_init, ROT0, "Merit", "Scorpion (Jun 15, 1999)",                                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1997, scrpionda,   scrpiond, scrpiond,    scrpiond, merits_state, empty_init, ROT0, "Merit", "Scorpion (Oct 01, 1997)",                                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1993, scrpiondb,   scrpiond, scrpiondold, scrpiond, merits_state, empty_init, ROT0, "Merit", "Scorpion (Dec 24, 1993)",                                      MACHINE_IS_SKELETON_MECHANICAL)
GAME(1992, pubtimed2ch, 0,        scrpiondold, scrpiond, merits_state, empty_init, ROT0, "Merit", "Pub Time Darts II Plus 2 with Solo Challenger (Mar 24, 1992)", MACHINE_IS_SKELETON_MECHANICAL)
