// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for mechanical games by Island Design Inc.:

    * Spider Stompin' Deluxe (undumped)
    * Tortoise and the Hare (undumped)
    * Vortex

***************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"
#include "speaker.h"


namespace {

class island_state : public driver_device
{
public:
	island_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
	{
	}

	void vortex(machine_config &config);

private:
	void prog_map(address_map &map) ATTR_COLD;
	void ext_map(address_map &map) ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
};

void island_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0); // TODO: banking
}

void island_state::ext_map(address_map &map)
{
	map(0x2008, 0x200d).noprw();
	map(0x4000, 0x7fff).ram();
}


static INPUT_PORTS_START(vortex)
INPUT_PORTS_END


void island_state::vortex(machine_config &config)
{
	DS80C320(config, m_maincpu, 20_MHz_XTAL); // FIXME: has more registers, faster machine cycles
	m_maincpu->set_addrmap(AS_PROGRAM, &island_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &island_state::ext_map);

	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki, 20_MHz_XTAL / 4, okim6295_device::PIN7_HIGH); // clock & pin 7 not verified
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START(isld_vortex)
	// vortex program w test  version 1.4  u17 = 27c040
	ROM_REGION(0x80000, "program", 0)
	ROM_LOAD("vortex.u17", 0x00000, 0x80000, CRC(4a47626c) SHA1(c11c59ad382f4dffc3062cd434a7efeb9dbe7b18))
	ROM_FILL(0x000d8, 1, 0x00) // workaround for unemulated DS80C320 watchdog timer
	ROM_FILL(0x000d9, 1, 0x00)

	// vortex sound u29 = 27c040  vers. 1.0
	ROM_REGION(0x80000, "oki", 0)
	ROM_LOAD("vortex.u29", 0x00000, 0x80000, CRC(2b0cc5c7) SHA1(ca9426351cec304b29a47ca66da12080269eb6e3))
ROM_END

} // anonymous namespace


GAME(1995, isld_vortex, 0, vortex, vortex, island_state, empty_init, ROT0, "Island Design", "Vortex (Island Design)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_MECHANICAL)
