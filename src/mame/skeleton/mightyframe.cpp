// license:BSD-3-Clause
// copyright-holders:
/***********************************************************************************************************************************

2017-11-03 Skeleton

Convergent Technologies 68k Mightyframe S80

Chips: CPU is a square MC68020RC12B. Also, 3x Z80SCC and 2 undumped proms labelled "7201087B"(@15D) and "7201089B"(@15F)
       The only photo shows just a part of the board, the only crystal is a tiny tubular one next to a OKI M58321 chip.

Manuals: http://mightyframe.blogspot.com.au/p/manuals.html

************************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"


namespace {

class mightyframe_state : public driver_device
{
public:
	mightyframe_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//      , m_maincpu(*this, "maincpu")
	{ }

	void mightyframe(machine_config &config);
private:
	void mem_map(address_map &map) ATTR_COLD;
	//  required_device<cpu_device> m_maincpu;
};

void mightyframe_state::mem_map(address_map &map)
{
	map(0x000000, 0x007fff).rom();
}

static INPUT_PORTS_START( mightyframe )
INPUT_PORTS_END

void mightyframe_state::mightyframe(machine_config &config)
{
	m68000_device &maincpu(M68000(config, "maincpu", XTAL(16'000'000))); // no idea of clock
	maincpu.set_addrmap(AS_PROGRAM, &mightyframe_state::mem_map);
}

ROM_START( mightyframe )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "72-01231.26c", 0x0000, 0x8000, CRC(41faf884) SHA1(d0c6f35394b4006bbe9a3f81b658ded37f41d86f) )
ROM_END

} // anonymous namespace


COMP( 1985?, mightyframe, 0, 0, mightyframe, mightyframe, mightyframe_state, empty_init, "Convergent Technologies", "Mightyframe", MACHINE_IS_SKELETON )
