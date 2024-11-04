// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for HD6305-based keyboards by Antonelli.

****************************************************************************/

#include "emu.h"
#include "cpu/m6805/m6805.h"
#include "machine/6850acia.h"

namespace {

class antonelli_hd6305_state : public driver_device
{
public:
	antonelli_hd6305_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void antonelli(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void antonelli_hd6305_state::mem_map(address_map &map)
{
	map(0x0200, 0x0201).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0x1000, 0x1fff).rom().region("eprom", 0x1000);
	map(0x2000, 0x3fff).rom().region("eprom", 0);
}

static INPUT_PORTS_START(antonelli)
INPUT_PORTS_END

void antonelli_hd6305_state::antonelli(machine_config &config)
{
	HD6305Y2(config, m_maincpu, 8'000'000); // type guessed; clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &antonelli_hd6305_state::mem_map);

	ACIA6850(config, "acia");
}

ROM_START(anto2495)
	ROM_REGION(0x2000, "eprom", 0)
	ROM_LOAD("49.bin", 0x0000, 0x2000, CRC(ee474092) SHA1(4f60b174048e29fc299ab28001203b734e3c6592)) // TI TMS 2764JL-25
ROM_END

ROM_START(anto2614)
	ROM_REGION(0x2000, "eprom", 0)
	ROM_LOAD("61.bin", 0x0000, 0x2000, CRC(9357a29b) SHA1(7f580778f1e854e71b51100cc6cd2c50de1a38b3)) // SGS M2764-4F1
ROM_END

} // anonymous namespace

SYST(198?, anto2495, 0, 0, antonelli, antonelli, antonelli_hd6305_state, empty_init, "Antonelli", "Antonelli 2495", MACHINE_IS_SKELETON)
SYST(198?, anto2614, 0, 0, antonelli, antonelli, antonelli_hd6305_state, empty_init, "Antonelli", "Antonelli 2614", MACHINE_IS_SKELETON) // stereo version
