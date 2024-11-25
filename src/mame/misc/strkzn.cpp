// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Strike Zone (1994)
    Hoop Shot (undumped)

    Redemption games by Purple Star Inc. using infrared light curtains.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/i86/i186.h"


namespace {

class strkzn_state : public driver_device
{
public:
	strkzn_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_lightcpu(*this, "lightcpu")
	{ }

	void strkzn(machine_config &config);

private:
	void light_io(address_map &map) ATTR_COLD;
	void light_mem(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;
	void main_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_lightcpu;
};

void strkzn_state::main_mem(address_map &map)
{
	map(0x0000, 0xdfff).rom().region("maincpu", 0);
	map(0xe000, 0xffff).ram();
}

void strkzn_state::main_io(address_map &map)
{
	map.global_mask(0xff);
}

void strkzn_state::light_mem(address_map &map)
{
	map(0x00000, 0x00fff).ram();
	map(0xf0000, 0xfffff).rom().region("lightcpu", 0);
}

void strkzn_state::light_io(address_map &map)
{
	map(0x0007, 0x0007).nopr();
}

void strkzn_state::strkzn(machine_config &config)
{
	Z80(config, m_maincpu, 4000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &strkzn_state::main_mem);
	m_maincpu->set_addrmap(AS_IO, &strkzn_state::main_io);

	I80188(config, m_lightcpu, 10000000);
	m_lightcpu->set_addrmap(AS_PROGRAM, &strkzn_state::light_mem);
	m_lightcpu->set_addrmap(AS_IO, &strkzn_state::light_io);
}

INPUT_PORTS_START( strkzn )
INPUT_PORTS_END

ROM_START( strkzn )
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD( "strkzn08",  0x00000, 0x10000, CRC(cc217dd6) SHA1(a5e9261c5c3f6d57f34ffd6019227d616f0c59bc) )

	ROM_REGION(0x10000, "lightcpu", 0)
	ROM_LOAD( "strkzn01",  0x00000, 0x10000, CRC(d408582e) SHA1(96a54ebe67db952a77b732f5ab345a94834d0906) )

	ROM_REGION(0x80000, "soundrom", 0) // OKIM6373???
	ROM_LOAD( "strkznu16", 0x00000, 0x80000, CRC(67f7674b) SHA1(451a26da55315fcaccdc02817521c78acdd8eb8a) )
ROM_END

} // anonymous namespace


GAME( 1994, strkzn, 0, strkzn, strkzn, strkzn_state, empty_init, ROT0, "Purple Star", "Strike Zone (Purple Star)", MACHINE_IS_SKELETON_MECHANICAL )
