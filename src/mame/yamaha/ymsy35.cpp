// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Yamaha SY35 AWM/FM vector synthesizer.

*******************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/h8500/h8520.h"
#include "cpu/m6805/m6805.h"

namespace {

class yamaha_sy35_state : public driver_device
{
public:
	yamaha_sy35_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_scancpu(*this, "scancpu")
	{
	}

	void sy35(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8520_device> m_maincpu;
	required_device<cpu_device> m_scancpu;
};

void yamaha_sy35_state::mem_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom().region("program", 0);
	map(0x20000, 0x27fff).mirror(0x8000).ram();
	//map(0x30000, 0x3ffff).rw("card", FUNC(sy35_card_device::read), FUNC(sy35_card_device::write));
	map(0x80000, 0xbffff).rom().region("sound_make", 0);
}

static INPUT_PORTS_START(sy35)
INPUT_PORTS_END

void yamaha_sy35_state::sy35(machine_config &config)
{
	HD6435208(config, m_maincpu, 20_MHz_XTAL); // HD6435208A00P or HD6475208P
	m_maincpu->set_mode(3); // internal ROM disabled
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_sy35_state::mem_map);

	HD6305V0(config, m_scancpu, 8_MHz_XTAL).set_disable(); // HD63B05V0C85P

	//TMC3493PH(config, "gew5a", 12.8_MHz_XTAL);
	//TMC3493PH(config, "gew5b", 12.8_MHz_XTAL);
	//YM3413(config, "ldsp");
}

ROM_START(sy35)
	ROM_REGION(0x20000, "program", 0)
	ROM_LOAD("xk932a0.ic7", 0x00000, 0x20000, CRC(e86f47ac) SHA1(e14e309ba09e0f6de165e9366e199b74d1411f6c)) // 27C010 (second half blank)

	ROM_REGION(0x40000, "sound_make", 0)
	ROM_LOAD("xl401a0.ic6", 0x00000, 0x40000, CRC(614dfe1b) SHA1(7c2edd2bb6157d95b0f54f712fc89666846caead)) // 27C020

	ROM_REGION(0x1000, "scancpu", 0)
	ROM_LOAD("xh257a00.ic2", 0x0000, 0x1000, NO_DUMP)

	ROM_REGION(0x200000, "voice", 0)
	ROM_LOAD("xl429a00.ic8", 0x000000, 0x100000, NO_DUMP)
	ROM_LOAD("xl430a00.ic9", 0x100000, 0x100000, NO_DUMP)
ROM_END

} // anonymous namespace

SYST(1992, sy35, 0, 0, sy35, sy35, yamaha_sy35_state, empty_init, "Yamaha", "SY35 Music Synthesizer", MACHINE_IS_SKELETON)
