// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for "Spade" by Guan Shing.

    Whatever machine this is (likely some sort of mechanical redemption
    game) clearly consists of more than the dumped sound board. The 8279
    addressed by the code is not at all to be found there.

****************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "speaker.h"


namespace {

class gsspade_state : public driver_device
{
public:
	gsspade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_soundcpu(*this, "soundcpu")
	{
	}

	void gsspade(machine_config &config);

private:
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_soundcpu;
};


void gsspade_state::prog_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("soundcpu", 0);
}

void gsspade_state::ext_map(address_map &map)
{
	map(0x9002, 0x9003).w("psg", FUNC(ay8910_device::address_data_w));
	map(0xb000, 0xb001).rw("kdc", FUNC(i8279_device::read), FUNC(i8279_device::write));
	map(0xc000, 0xc001).w("m3567", FUNC(ym2413_device::write));
}


static INPUT_PORTS_START(gsspade)
	PORT_START("P1")
	// Inputs for testing purposes
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("P1.0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("P1.1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("P1.2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("P1.3")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON5) PORT_NAME("P1.6")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON6) PORT_NAME("P1.7")
INPUT_PORTS_END


void gsspade_state::gsspade(machine_config &config)
{
	I8051(config, m_soundcpu, 10.738635_MHz_XTAL); // Intel/Fujitsu P8051AH
	m_soundcpu->set_addrmap(AS_PROGRAM, &gsspade_state::prog_map);
	m_soundcpu->set_addrmap(AS_IO, &gsspade_state::ext_map);
	m_soundcpu->port_in_cb<1>().set_ioport("P1");

	I8279(config, "kdc", 1'789'772); // ?

	SPEAKER(config, "speaker").front_center();

	ay8910_device &psg(AY8910(config, "psg", 1'789'772)); // File KC89C72 (clock guessed)
	psg.add_route(ALL_OUTPUTS, "speaker", 0.65);

	ym2413_device &m3567(YM2413(config, "m3567", 3.579545_MHz_XTAL)); // M3567
	m3567.add_route(ALL_OUTPUTS, "speaker", 1.0);
}


ROM_START(gsspade)
	ROM_REGION(0x2000, "soundcpu", 0)
	ROM_LOAD("spade-gs-dm-5.u2", 0x0000, 0x2000, CRC(c359201b) SHA1(5e5ac815bcd50f918f9c8b7447bcf6cf9426ae74))
ROM_END

} // anonymous namespace


GAME(199?, gsspade, 0, gsspade, gsspade, gsspade_state, empty_init, ROT0, "Guan Shing", "Spade", MACHINE_NOT_WORKING | MACHINE_MECHANICAL)
