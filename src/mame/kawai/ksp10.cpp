// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Kawai KSP10 digital piano.

****************************************************************************/

#include "emu.h"
#include "cpu/tlcs900/tmp96c141.h"


namespace {

class kawai_ksp10_state : public driver_device
{
public:
	kawai_ksp10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void ksp10(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<tmp96c141_device> m_maincpu;
};


void kawai_ksp10_state::mem_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("firmware", 0);
	map(0x400000, 0x407fff).ram(); // NVRAM?
}


static INPUT_PORTS_START(ksp10)
INPUT_PORTS_END

void kawai_ksp10_state::ksp10(machine_config &config)
{
	TMP96C141(config, m_maincpu, 10'000'000); // clock unknown
	m_maincpu->set_addrmap(AS_PROGRAM, &kawai_ksp10_state::mem_map);
}

ROM_START(ksp10)
	ROM_REGION16_LE(0x80000, "firmware", 0)
	ROM_LOAD("u3_hp041b_e8dp_tms27c240.bin", 0x00000, 0x80000, CRC(b64e7a97) SHA1(bac0f345d0a039a3315883a6ca8eefe659709a26))

	ROM_REGION16_LE(0x40000, "samples", 0)
	ROM_LOAD("u21_hp042a_e7dp_mbm27c2048.bin", 0x00000, 0x40000, CRC(e21b1141) SHA1(181c2beed18da2efa2f0e45cb3233adf6b932127))
ROM_END

} // anonymous namespace


SYST(199?, ksp10, 0, 0, ksp10, ksp10, kawai_ksp10_state, empty_init, "Kawai Musical Instruments Manufacturing", "KSP10 Digital Piano", MACHINE_IS_SKELETON)
