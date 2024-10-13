// license:BSD-3-Clause
// copyright-holders:

/*
Skeleton driver for Cirsa 820XXX hardware.
This PCB is an evolution of the 810XXX hardware found in missbamby.cpp. Main differences are there is an i8254
and two dips banks instead of just one (and no service/reset button). Also there is no i8155.
Known games on this hardware:
 ________________________________________________________________________
 | Dumped | Game              | Manufacturer       | Notes              |
 +--------+-------------------+--------------------+--------------------|
 | YES    | Unknown           | Cirsa              | Cirsa PCB 820501 A |
 +-------------------------------------------------+--------------------+
*/

/*
           ________________________________
 __________| | | | | | | | | | | | | | | | |__________
 |         |_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|_|          |
 | ___________________                                |
 | | EMPTY            |                               |
 | |__________________|                               |
 | ___________________                  ____________  |
 | | AY-3-8910        |                 | iP8254    | |
 | |__________________|  __________     |___________| |
 |  _________ _________  |PAT-004_|     ____________  |
 |  |_DIPSx8_||_DIPSx8_| __________     | KM6816-15 | |
 |                       |PAT-024_|     |___________| |
 | ________     ___________________   ______________  |
 | |___?___|    | Intel P8085A     |  | ROM 1C      | |
 | ________     |__________________|  |_____________| |
 | |___?___|      ___                 ______________  |
 | ________      |XTAL   __________   | ROM 1B      | |
 | |___?___|             |_74LS373_|  |_____________| |
 |                                    ______________  |
 |                     BATT           | ROM 1A      | |
 | CIRSA 820501 A                     |_____________| |
 |                                                    |
 |____________________________________________________|

 The Xtal frequency was unreadable.
*/

#include "emu.h"
#include "speaker.h"
#include "cpu/i8085/i8085.h"
#include "machine/pit8253.h"
//#include "machine/nvram.h"
#include "sound/ay8910.h"


namespace {

class cirsa820xxx_state : public driver_device
{
public:
	cirsa820xxx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void cirsa820xxx(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};

void cirsa820xxx_state::prg_map(address_map &map)
{
	map(0x0000, 0x5fff).rom().region("maincpu", 0);
	map(0x6000, 0x67ff).ram();
	map(0x7403, 0x7403).nopw();
	map(0x7800, 0x7801).w("psg", FUNC(ay8910_device::address_data_w));
}

void cirsa820xxx_state::io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x03).rw("pit", FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x14, 0x14).nopr(); // watchdog?
	map(0x19, 0x19).r("psg", FUNC(ay8910_device::data_r));
}


static INPUT_PORTS_START( cirsa820xxx )
	PORT_START("IN0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")
INPUT_PORTS_END


void cirsa820xxx_state::cirsa820xxx(machine_config &config)
{
	I8085A(config, m_maincpu, 6'000'000); // XTAL value was unreadable, guessed for now
	m_maincpu->set_addrmap(AS_PROGRAM, &cirsa820xxx_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &cirsa820xxx_state::io_map);

	PIT8254(config, "pit");

	SPEAKER(config, "mono").front_center();
	ay8910_device &psg(AY8910(config, "psg", 6'000'000 / 4)); // XTAL value was unreadable, guessed for now. Unknown divider
	psg.port_a_read_callback().set_ioport("DSW1");
	psg.port_b_read_callback().set_ioport("DSW2");
	psg.add_route(ALL_OUTPUTS, "mono", 1.0);
}


// Cirsa PCB 820501 A
ROM_START( unk820501 )
	ROM_REGION(0x6000, "maincpu", 0)
	ROM_LOAD( "1b.bin",                     0x0000, 0x2000, CRC(aff9bd07) SHA1(ec38b298f4d7a617326d73e642b47f5c0e974f04) )
	ROM_LOAD( "1a.bin",                     0x2000, 0x2000, CRC(d9eefece) SHA1(3137000fbf70562b300bf4b95c425b3de2dd63c4) )
	ROM_LOAD( "b-1554_unidesa_b-82_1c.bin", 0x4000, 0x2000, CRC(89b3da4a) SHA1(c464fb0e307afd035d091007ec6a428975fb60e4) )

	ROM_REGION(0x104, "plds", 0)
	ROM_LOAD( "pat_004.bin", 0x000, 0x104, NO_DUMP ) // pal164dc
ROM_END

} // Anonymous namespace


GAME( 1982?, unk820501, 0, cirsa820xxx, cirsa820xxx, cirsa820xxx_state, empty_init, ROT0, "Cirsa", "unknown Cirsa slot machine on 820501 A PCB", MACHINE_IS_SKELETON_MECHANICAL )
