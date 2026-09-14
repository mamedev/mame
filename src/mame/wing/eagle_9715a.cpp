// license:BSD-3-Clause
// copyright-holders:

/*
Eagle 9715-A PCB

reference video: https://www.youtube.com/watch?v=D1sWO7WQEIc

Main components:

HD647180X0P6 (internal ROM not used)
24.57600 MHz XTAL
M5M5165P-70 RAM
M62X428 RTC
09DI05 square 144-pin custom
Sanyo LC83020 DSP (?)
Oki M6295
2x bank of 8 switches

TODO:
- everything
*/


#include "emu.h"

#include "cpu/z180/hd647180x.h"
#include "machine/msm6242.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class eagle_9715a_state : public driver_device
{
public:
	eagle_9715a_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void smflight(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void main_program_map(address_map &map) ATTR_COLD;
	void main_io_map(address_map &map) ATTR_COLD;
};


void eagle_9715a_state::main_program_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x0e000, 0x0ffff).ram(); // NVRAM
}

void eagle_9715a_state::main_io_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);

	map(0x00, 0x7f).noprw();
}


static INPUT_PORTS_START( smflight )
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


void eagle_9715a_state::smflight(machine_config &config)
{
	HD647180X(config, m_maincpu, 24.576_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &eagle_9715a_state::main_program_map);
	m_maincpu->set_addrmap(AS_IO, &eagle_9715a_state::main_io_map);

	MSM6242(config, "rtc", 32.768_kHz_XTAL);

	SPEAKER(config, "mono").front_center();

	// LC83020

	OKIM6295(config, "oki", 1'056'000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}


ROM_START( smflight )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "s_magic_flight_jpq_8bet_v1.00.ic18", 0x00000, 0x20000, CRC(e64f1f26) SHA1(0ee40f317cf28aca7c0c1a27e6e02ee63b9cba57) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "s_magic_flight_pcm0_cn9411_v1.xx.ic24", 0x00000, 0x40000, CRC(d907262b) SHA1(055ba1cc975494d48b1dad2afa2e15c7317208ed) )
ROM_END

} // anonymous namespace


GAME( 2003, smflight, 0, smflight, smflight, eagle_9715a_state, empty_init, ROT0, "Eagle", "Super Magic Flight (Japan, v1.0)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
