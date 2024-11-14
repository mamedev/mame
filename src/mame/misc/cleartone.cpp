// license:BSD-3-Clause
// copyright-holders:

/*
Bonus Talker by Cleartone Leisure Ltd.

This is a prototype fruit machine. It has a CPU board with a Z80 and a sound board with a MM54104N.
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "sound/digitalk.h"

#include "speaker.h"


namespace {

class cleartone_state : public driver_device
{
public:
	cleartone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_digitalker(*this, "digitalker")
	{}

	void cleartone(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<digitalker_device> m_digitalker;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void cleartone_state::program_map(address_map &map)
{
	map(0x0000, 0x17ff).rom();
	map(0x1800, 0x18ff).ram();
}

void cleartone_state::io_map(address_map &map)
{
}


static INPUT_PORTS_START( cleartone )
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

	// no dips on PCB
INPUT_PORTS_END


void cleartone_state::cleartone(machine_config &config)
{
	Z80(config, m_maincpu, 2_MHz_XTAL); // according to schematics XTAL is 2 MHz but not readable on PCB picture, Z80 is a Z0840004PSC (verified on picture)
	m_maincpu->set_addrmap(AS_PROGRAM, &cleartone_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &cleartone_state::io_map);

	SPEAKER(config, "mono").front_center(); // TODO: possibly some discrete sound effects, too?

	DIGITALKER(config, m_digitalker, 4'000'000); // XTAL isn't readable on PCB picture
	m_digitalker->add_route(ALL_OUTPUTS, "mono", 0.16);
}


ROM_START( bnstlkr )
	ROM_REGION( 0x1800, "maincpu", 0 ) // all 2716s with handwritten labels
	ROM_LOAD( "e1 top.ic13",    0x0000, 0x0800, CRC(4677461e) SHA1(36e4e4513341676204fe8df673b997c0adcdf473) )
	ROM_LOAD( "e2 middle.ic15", 0x0800, 0x0800, CRC(3b8e1e22) SHA1(435baa3cd77c1e63be7a4e6fdaa6645dc15a7e3a) )
	ROM_LOAD( "e3 bottom.ic37", 0x1000, 0x0800, CRC(82d9b633) SHA1(d9e98f095b84d1fc3e3602e74bddd6d49370f5f2) )

	ROM_REGION( 0x4000, "digitalker", 0 ) // all 2716s with handwritten labels
	ROM_LOAD( "bt0", 0x0000, 0x0800, CRC(16706c53) SHA1(0b430685dd38f1e03b40e2225628b648fdc2197c) )
	ROM_LOAD( "bt1", 0x0800, 0x0800, CRC(2b0c6a1f) SHA1(e194ed4f48d681a170b8a25ed449e5e43dab7238) )
	ROM_LOAD( "bt2", 0x1000, 0x0800, CRC(5a740f8f) SHA1(26bf00e190f7818f98f848c887c398eede781924) )
	ROM_LOAD( "bt3", 0x1800, 0x0800, CRC(b0d50dcd) SHA1(2b5a5321b83cd11278b02b03d21fa3935c66f6eb) )
	ROM_LOAD( "bt4", 0x2000, 0x0800, CRC(4383608c) SHA1(4b1491fc08f8ac2f9fd76a98e238c0472f07f89a) )
	ROM_LOAD( "bt5", 0x2800, 0x0800, CRC(28ea9225) SHA1(baf85ab7869dcf6ea38c9f5c705ee5d6aa1f07ae) )
	ROM_LOAD( "bt6", 0x3000, 0x0800, BAD_DUMP CRC(acbc9256) SHA1(e3f5e7b86a01961cced9d128cde0c69db21072be) ) // programmer gave error while dumping
	ROM_LOAD( "bt7", 0x3800, 0x0800, CRC(b88be97f) SHA1(f335140ac7dc4842832dfcd2e3aebabc8d8edc82) )
ROM_END

} // anonymous namespace


GAME( 198?, bnstlkr, 0, cleartone, cleartone, cleartone_state, empty_init, ROT0, "Cleartone Leisure Ltd.", "Bonus Talker", MACHINE_IS_SKELETON_MECHANICAL )
