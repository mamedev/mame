// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for GEM/Baldwin Pianovelle RP electric pianos.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/h8/h83003.h"
#include "machine/intelfsh.h"


namespace {

class gem_rp_state : public driver_device
{
public:
	gem_rp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void rp200(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h83003_device> m_maincpu;
};


void gem_rp_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x200000, 0x23ffff).ram();
}


static INPUT_PORTS_START(rp200)
INPUT_PORTS_END

void gem_rp_state::rp200(machine_config &config)
{
	H83003(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &gem_rp_state::mem_map);

	AMD_29F800B_16BIT(config, "flash");

	//DISP3(config, "disp3mst", 45.1584_MHz_XTAL);
	//DISP3(config, "disp3slv", 45.1584_MHz_XTAL);
}

ROM_START(rp200)
	ROM_REGION(0x100000, "flash", 0)
	ROM_LOAD("gem_rp200_rp220_grp300_am29f800bb_firmware_v1.02.ic3", 0x000000, 0x100000, CRC(4980d8b6) SHA1(8d8c0310b962d422fa1d494eaaf5fe4fd3d20eb5))

	ROM_REGION16_BE(0x200000, "library", 0)
	ROM_LOAD("104014.ic9", 0x000000, 0x200000, NO_DUMP) // HN624316

	ROM_REGION16_BE(0xc00000, "wave", 0)
	ROM_LOAD("104041_wave98.ic13", 0x000000, 0x800000, NO_DUMP) // MX23C6410
	ROM_LOAD("104023_wave3.ic14", 0x800000, 0x400000, NO_DUMP) // 23C2000G
ROM_END

} // anonymous namespace


SYST(1999, rp200, 0, 0, rp200, rp200, gem_rp_state, empty_init, "Generalmusic", "GEM RealPiano RP200", MACHINE_IS_SKELETON)
