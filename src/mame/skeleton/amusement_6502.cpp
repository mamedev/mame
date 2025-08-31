// license:BSD-3-Clause
// copyright-holders:

/*
unknown vending machine (?) by Amusement Inc.

Main (?) PCB
on front: AMUSEMENT INC. LOGIC PCB ASSY 58-0003-002 ORG-11 MADE IN TAIWAN R.O.C.
on back: AMUSEMENT INC ORG-11 LOGIC PCB P/N57-0003-002

Main components:
UM6502
UM6116-2
AY-3-8910A
VP1000 (UM5100 compatible)
bank of 4 switches
no XTAL spotted

Plug in PCB
on front: AMUSEMENT INC. ORG-11 PROM-ASSY P/N58-0004-001 MADE IN TAIWAN R.O.C.
on back: AMUSEMENT INC ORG-11 PROM PCB P/N57-0004-001
just 2 ROMs (one for CPU/AY and one for VP1000)

A third PCB came together with the other two, but it isn't clear if they are intended
to work together or if is it for something different

Main components:
TMS5220CNL and its ROM
555

TODO:
- ROM contains a very small program, is this really a complete set or is it missing another PCB?
*/


#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "sound/ay8910.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"

#include "speaker.h"


namespace {

class amusement_6502_state : public driver_device
{
public:
	amusement_6502_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void amusement(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};

void amusement_6502_state::program_map(address_map &map)
{
	map(0x0000, 0x01ff).ram();
	//map(0x2000, 0x2000).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	//map(0x4000, 0x4000).w("ay", FUNC(ay8910_device::address_w));
	map(0xe000, 0xffff).rom().region("maincpu", 0);
}


static INPUT_PORTS_START( amusement )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void amusement_6502_state::amusement(machine_config &config)
{
	M6502(config, m_maincpu, 2'000'000); // TODO: verify clock
	m_maincpu->set_addrmap(AS_PROGRAM, &amusement_6502_state::program_map);
	//m_maincpu->set_periodic_int(FUNC(amusement_6502_state::irq0_line_hold), attotime::from_hz(60));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay(AY8910(config, "ay", 1'000'000)); // TODO: verify clock
	ay.port_a_read_callback().set([this] () { logerror("%s AY port A read\n", machine().describe_context()); return 0; });
	ay.port_b_read_callback().set([this] () { logerror("%s AY port B read\n", machine().describe_context()); return 0; });
	ay.port_a_write_callback().set([this] (uint8_t data) { logerror("%s AY port A write: %02x\n", machine().describe_context(), data); });
	ay.port_b_write_callback().set([this] (uint8_t data) { logerror("%s AY port B write: %02x\n", machine().describe_context(), data); });
	ay.add_route(ALL_OUTPUTS, "mono", 0.60);

	HC55516(config, "cvsd", 0).add_route(ALL_OUTPUTS, "mono", 0.60); // TODO: actually VP1000

	TMS5220C(config, "tms", 640'000).add_route(ALL_OUTPUTS, "mono", 0.60); // TODO: verify clock
}


ROM_START( unkamus )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "music.u16", 0x0000, 0x2000, CRC(4b5274bc) SHA1(5d78ecef66c1cfab2498ec1d8da9b8a1c0ba0042) )

	ROM_REGION( 0x8000, "cvsd", 0 )
	ROM_LOAD( "generic.u15", 0x0000, 0x8000, CRC(dfc81bc3) SHA1(9e072475d5a1f1d97560e9ce14f206af844fcb2e) )

	ROM_REGION( 0x1000, "tms", 0 )
	ROM_LOAD( "u14", 0x0000, 0x1000, CRC(ce38d80f) SHA1(9646e2d964ee4e81a1184795eedeae4bfe40c99d) ) // label unreadable
ROM_END

} // anonymous namespace


SYST( 199?, unkamus, 0, 0, amusement, amusement, amusement_6502_state, empty_init, "Amusement Inc.", "unknown Amusement Inc. vending machine", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
