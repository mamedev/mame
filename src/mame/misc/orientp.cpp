// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************

// Features Notes Was from archived websited:
src: Chang yu website

Pearl in the  East 

5 balls per game ... win a bonus if 4 balls flip into the same slot.

When balls flip into certain slots, additional lamps illuminate.

When scoring on all letters of pearl in the east, up to 4 extra lamps randomly illuminate.

Higher bets when scoring on all letters of PEARL IN THE EAST win a higher bonus.

New feature: buy a 6th ball toup the odds.

Oriental Pearl - 1997

5 balls per game and 2 win modes.

1 higher bets mean more bonus lights and more chances to win.

Big bonus for scoring on all letters of "ORIENTAL PEARL".

Electronic ball-checking device ensures where ball lands.

// Hardware info - may not accurate
the dump was from Soccer Santiago II 6 ball pinball

Buttons
K1
K2
K3
K4

ic
u1 kc8279
u17 and u21 nec d8255ac-2
u32 hm6264
??  winbond w27c020? adpcm rom.
u33 winbond w27c512 boot rom.
u39 at89s51 second mcu for protection.

// TODO:
Need hardware info.
Hook up nvram inputs opll and adpcm.
east8 Has undumped mcu and adpcm rom.
Need Layout as and Add segment display as marywu.cpp
*/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "speaker.h"

namespace {

class orientalpearl_state : public driver_device
{
public:
	orientalpearl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{ }

	void orientp(machine_config &config);

private:
	void io_map(address_map &map);
	void program_map(address_map &map);

protected:
	virtual void machine_start() override;

};

static INPUT_PORTS_START( orientp )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
	
	PORT_START("DSW0")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW0:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW0:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW0:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW0:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW0:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW0:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW0:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW0:8")

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")
INPUT_PORTS_END

void orientalpearl_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
}

void orientalpearl_state::io_map(address_map &map)
{
    map(0xfa00, 0xfa01).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
    map(0xfb02, 0xfb03).w("psg", FUNC(ay8910_device::address_data_w));
	map(0xfe00, 0xfe01).w("opll", FUNC(ym2413_device::write));
	
}

void orientalpearl_state::machine_start()
{
}

void orientalpearl_state::orientp(machine_config &config)
{
	/* basic machine hardware */
	i8052_device &maincpu(I8052(config, "maincpu", XTAL(10'738'000)));
	maincpu.set_addrmap(AS_PROGRAM, &orientalpearl_state::program_map);
	maincpu.set_addrmap(AS_IO, &orientalpearl_state::io_map);
	
	/* Keyboard & display interface */
	I8279(config, "kdc", XTAL(10'738'000) / 6); 
	
	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", XTAL(10'738'000) / 6));
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, "oki", XTAL(10'738'000) / 6, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);  // Clock frequency & pin 7 not verified
}

ROM_START( east8 )
	ROM_REGION( 0x10000, "maincpu", 0 ) //  EAST8  v1.05 string
	ROM_LOAD( "w27c512.u33", 0x00000, 0x10000, CRC(8d3d1e91) SHA1(b80907df0878057a1ded8b56225059e06382b9d6) ) // main program

    ROM_REGION( 0x1000, "mcu", ROMREGION_ERASE00 )
    ROM_LOAD( "at89s51.u39", 0x0000, 0x1000, NO_DUMP ) // mcu. protection
	
    ROM_REGION( 0x40000, "oki", ROMREGION_ERASE00 )
    ROM_LOAD( "w27c020.bin", 0x00000, 0x40000, NO_DUMP ) //  oki rom voice

    ROM_END

} // anonymous namespace


//    YEAR  NAME    PARENT   MACHINE   INPUT   STATE         INIT        ROT   COMPANY      FULLNAME                                                FLAGS
GAME( 200?, east8,  0,       orientp,   orientp, orientalpearl_state, empty_init, ROT0, "<unknown>", " Unknown EAST8 (v1.05)",  MACHINE_NOT_WORKING | MACHINE_UNEMULATED_PROTECTION | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK ) // EAST8  v1.05  string
