// license:BSD-3-Clause
// copyright-holders:

/*
FDEK (Fujidenshi Kogyo) H8-based medal games

FDEK 07001A main PCB + FDEK 06001B CPU riser PCB
main components:

HDF2367VF33V main CPU with undumped internal ROM (on riser PCB)
16 MHz XTAL (on riser PCB)
TODO: square chip with XTAL on main PCB?
3 push buttons (reset, last, test)
*/


#include "emu.h"

#include "cpu/h8/h8s2357.h"

#include "speaker.h"


namespace {

class fdek_h8s_state : public driver_device
{
public:
	fdek_h8s_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void fdek_h8s(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void program_map(address_map &map) ATTR_COLD;
};


void fdek_h8s_state::program_map(address_map &map)
{
}


static INPUT_PORTS_START( battkids )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void fdek_h8s_state::fdek_h8s(machine_config &config)
{
	H8S2357(config, m_maincpu, 16_MHz_XTAL); // TODO: wrong, should be 2367
	m_maincpu->set_addrmap(AS_PROGRAM, &fdek_h8s_state::program_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}


// believed to be Battle Kids cause it came with its manual and because of the possible acronym on the ROM label (FBK)
// reference video: https://www.youtube.com/watch?v=Mvth8x2z_H8
ROM_START( battkids )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal", 0x00000, 0x60000, NO_DUMP )

	ROM_REGION( 0x400000, "audio_program", 0 )
	// SOUND CONTROLLER PROGRAM GSC2-0000-00.00 DATE  2007/01/25
	ROM_LOAD( "fbk7-0000_00-2.rom1", 0x000000, 0x400000, CRC(71ec966a) SHA1(b21a6fd42084073b5b87ca5d10f4952881c0dfb7) ) //  1xxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 2007, battkids, 0, fdek_h8s, battkids, fdek_h8s_state, empty_init, ROT0, "FDEK", "Battle Kids", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
