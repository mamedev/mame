// license:BSD-3-Clause
// copyright-holders:

/*
FDEK (Fujidenshi Kogyo) H8-based medal games

Battle Kids (?) runs on
FDEK 07001A main PCB + FDEK 06001B CPU riser PCB
main components:

HDF2367VF33V main CPU with undumped internal ROM (on riser PCB)
16 MHz XTAL (on riser PCB)
TODO: square chip with XTAL on main PCB?
3 push buttons (reset, last, test)

--

Siren Kids runs on
FDEK 0002AB-2A-2 PCB (note that the 0002AB-2A part is on a sticker, so it may
cover the real PCB model)

HD6412240FA20(H8S/2240) main CPU
20AKSS5MT XTAL
CXK5864BSP-10L SRAM
YMZ280B-F
16CKSS4JI XTAL
3x bank of 8 switches (SW1-SW3)
test push-button
*/


#include "emu.h"

#include "cpu/h8/h8s2245.h"
#include "cpu/h8/h8s2357.h"
#include "sound/ymz280b.h"

#include "speaker.h"


namespace {

class fdek_h8s_state : public driver_device
{
public:
	fdek_h8s_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void fdek_h8s2240(machine_config &config) ATTR_COLD;
	void fdek_h8s2367(machine_config &config) ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void battkids_program_map(address_map &map) ATTR_COLD;
	void sirekids_program_map(address_map &map) ATTR_COLD;
};


void fdek_h8s_state::battkids_program_map(address_map &map)
{
	map(0x000000, 0x05ffff).rom();
}

void fdek_h8s_state::sirekids_program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x000000, 0x01ffff).rom();
	map(0x200000, 0x201fff).ram();
	map(0x400000, 0x400001).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	// map(0x600000, 0x600000).w // ?
	// map(0x600002, 0x600003).r // ?
	// map(0x600004, 0x600004).r // ?
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


void fdek_h8s_state::fdek_h8s2240(machine_config &config)
{
	H8S2241(config, m_maincpu, 20_MHz_XTAL); // TODO: wrong, should be 2240
	m_maincpu->set_addrmap(AS_PROGRAM, &fdek_h8s_state::sirekids_program_map);
	// TODO: various ports read from and written to

	// sound hardware
	SPEAKER(config, "mono").front_center();

	YMZ280B(config, "ymz", 16_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
}

void fdek_h8s_state::fdek_h8s2367(machine_config &config)
{
	H8S2357(config, m_maincpu, 16_MHz_XTAL); // TODO: wrong, should be 2367
	m_maincpu->set_addrmap(AS_PROGRAM, &fdek_h8s_state::battkids_program_map);

	// sound hardware
	SPEAKER(config, "mono").front_center();
}


// believed to be Battle Kids cause it came with its manual
// reference video: https://www.youtube.com/watch?v=Mvth8x2z_H8
ROM_START( battkids )
	ROM_REGION( 0x60000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "internal", 0x00000, 0x60000, NO_DUMP )

	ROM_REGION( 0x400000, "audio_program", 0 )
	// SOUND CONTROLLER PROGRAM GSC2-0000-00.00 DATE  2007/01/25
	ROM_LOAD( "fbk7-0000_00-2.rom1", 0x000000, 0x400000, CRC(71ec966a) SHA1(b21a6fd42084073b5b87ca5d10f4952881c0dfb7) ) //  1xxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

// reference video: https://www.youtube.com/watch?v=gieP-b1uuus
ROM_START( sirekids )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "fbk5-000000-1.ic20", 0x00000, 0x20000, CRC(4c99dae7) SHA1(702b58cbb657718c0be0768afcc6e027bd6cd029) ) // 1xxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "fbk5-000000-2.ic9",  0x000000, 0x100000, CRC(13fc5d64) SHA1(66f80f41171d0c42c646ba20a376472f96cb16fd) )
	ROM_LOAD( "fbk5-000000-3.ic12", 0x100000, 0x100000, CRC(95924aee) SHA1(ff0ceb2a684b8450b6b4f62277ecb8297c120212) )
ROM_END

} // anonymous namespace


GAME( 2006, sirekids, 0, fdek_h8s2240, battkids, fdek_h8s_state, empty_init, ROT0, "FDEK", "Siren Kids",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 2007, battkids, 0, fdek_h8s2367, battkids, fdek_h8s_state, empty_init, ROT0, "FDEK", "Battle Kids", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
