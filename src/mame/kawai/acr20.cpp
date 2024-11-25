// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Kawai ACR-20 Digital Accompaniment Center.

****************************************************************************/

#include "emu.h"
#include "cpu/tlcs900/tmp96c141.h"

namespace {

class acr20_state : public driver_device
{
public:
	acr20_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void acr20(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<tmp96c141_device> m_maincpu;
};


void acr20_state::mem_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("firmware", 0);
	map(0x400000, 0x4017ff).ram();
	map(0x402000, 0x402067).ram();
	map(0x800000, 0x800000).lr8(NAME([] { return 0x80; }));
	map(0x880000, 0x880043).nopw();
	map(0x89fe00, 0x89ffff).ram();
}


static INPUT_PORTS_START(acr20)
INPUT_PORTS_END

void acr20_state::acr20(machine_config &config)
{
	TMP96C141(config, m_maincpu, 10'000'000); // clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &acr20_state::mem_map);
}

ROM_START(acr20)
	ROM_REGION16_LE(0x20000, "firmware", 0)
	ROM_LOAD("kp148a.bin", 0x00000, 0x20000, CRC(a73ca17b) SHA1(e0baf24a33bead76af08b1653e3034f4b5b1c888)) // TMS27C210A
	// ROM also contains strings for DRP-10 Digital Recorder Playback
ROM_END

} // anonymous namespace

SYST(199?, acr20, 0, 0, acr20, acr20, acr20_state, empty_init, "Kawai Musical Instruments Manufacturing", "ACR-20 Digital Accompaniment Center", MACHINE_IS_SKELETON)
