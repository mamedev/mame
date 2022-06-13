// license:BSD-3-Clause
// copyright-holders:Curt Coder

/*
    
    Luxor X37 prototype

    (Luxor DS90-10 + ABC 1600 video)

*/

#include "emu.h"
#include "softlist_dev.h"
#include "cpu/m68000/m68000.h"
#include "video/abc1600.h"

class x37_state : public driver_device
{
public:
	x37_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void x37(machine_config &config);

private:
	required_device<m68000_base_device> m_maincpu;

	void program_map(address_map &map);
	void cpu_space_map(address_map &map);
};

void x37_state::program_map(address_map &map)
{
	map(0x000000, 0x007fff).rom().region("maincpu", 0);
}

void x37_state::cpu_space_map(address_map &map)
{
	map(0xffff0, 0xfffff).m(m_maincpu, FUNC(m68010_device::autovectors_map));
}

static INPUT_PORTS_START( x37 )
INPUT_PORTS_END

void x37_state::x37(machine_config &config)
{
	// basic machine hardware
	M68010(config, m_maincpu, 10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &x37_state::program_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &x37_state::cpu_space_map);

	// video hardware
	ABC1600_MOVER(config, ABC1600_MOVER_TAG, 0);

	// software list
	SOFTWARE_LIST(config, "flop_list").set_original("x37_flop");
}

ROM_START( x37 )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "x37.rom", 0x0000, 0x8000, CRC(d505e7e7) SHA1(a3ad839e47b1f71c394e5ce28bce199e5e4810d2) )
ROM_END

COMP( 1985, x37, 0,      0,      x37, x37, x37_state, empty_init, "Luxor", "X37 (prototype)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
