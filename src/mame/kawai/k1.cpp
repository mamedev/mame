// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Kawai K1 synthesizer.

***************************************************************************/

#include "emu.h"
#include "mb63h158.h"
#include "cpu/upd78k/upd78k3.h"
#include "machine/nvram.h"


namespace {

class kawai_k1_state : public driver_device
{
public:
	kawai_k1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_mpu(*this, "mpu")
	{
	}

	void k1(machine_config &config);
	void k1m(machine_config &config);

private:
	void k1_map(address_map &map) ATTR_COLD;
	void k1m_map(address_map &map) ATTR_COLD;

	required_device<upd78310_device> m_mpu;
};


void kawai_k1_state::k1m_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("coderom", 0);
	map(0x8000, 0x9fff).ram();
	map(0xa000, 0xbfff).ram().share("toneram");
}

void kawai_k1_state::k1_map(address_map &map)
{
	k1m_map(map);
	map(0xf000, 0xf0ff).mirror(0x700).r("sensor", FUNC(mb63h158_device::read));
}


static INPUT_PORTS_START(k1)
INPUT_PORTS_END

void kawai_k1_state::k1(machine_config &config)
{
	UPD78310(config, m_mpu, 12_MHz_XTAL); // µPD78310G-36
	m_mpu->set_addrmap(AS_PROGRAM, &kawai_k1_state::k1_map);

	NVRAM(config, "toneram", nvram_device::DEFAULT_ALL_0); // LC3564PL-12 + battery

	MB63H158(config, "sensor", 7.2_MHz_XTAL);
}

void kawai_k1_state::k1m(machine_config &config)
{
	k1(config);

	m_mpu->set_addrmap(AS_PROGRAM, &kawai_k1_state::k1m_map);
	config.device_remove("sensor");
}

ROM_START(k1)
	ROM_REGION(0x8000, "coderom", 0)
	//ROM_SYSTEM_BIOS(0, "v1.3", "Version 1.3")
	ROM_LOAD("p104c_e4dp.u20", 0x0000, 0x8000, CRC(5d266846) SHA1(42a2820873817ca05dd085dfb728e8bcadb1342a)) // TC54256AP

	ROM_REGION(0x80000, "waverom", 0)
	ROM_LOAD("p106-m8dw.u8", 0x00000, 0x80000, CRC(b411b848) SHA1(1ea7ca6270b5c128f4e24fa540d205411982e4c1)) // MN234002KAD
ROM_END

ROM_START(k1m)
	ROM_REGION(0x8000, "coderom", 0)
	ROM_SYSTEM_BIOS(0, "v1.5", "Version 1.5")
	ROMX_LOAD("p105e.ic7", 0x0000, 0x8000, CRC(ff560060) SHA1(28d55ec30acbbc1c41d09b7704725f226d1f682c), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.1", "Version 1.1")
	ROMX_LOAD("p105a.ic7", 0x0000, 0x8000, CRC(957e8f55) SHA1(b5db5e619acc0a4af3b85a450ad51ce7f2a21900), ROM_BIOS(1))

	ROM_REGION(0x80000, "waverom", 0)
	ROM_LOAD("waverom_p106-m8dw.ic16", 0x00000, 0x80000, CRC(b411b848) SHA1(1ea7ca6270b5c128f4e24fa540d205411982e4c1))
ROM_END

ROM_START(k1r)
	ROM_REGION(0x8000, "coderom", 0)
	ROM_SYSTEM_BIOS(0, "v1.4", "Version 1.4")
	ROMX_LOAD("p163a.ic7",    0x0000, 0x8000, CRC(5c0e5b4b) SHA1(2348989ddf9398d66cfdff4675dc8e56a5a20d93), ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "v1.3", "Version 1.3")
	ROMX_LOAD("k1r_v1.3.ic7", 0x0000, 0x8000, CRC(25b395b0) SHA1(1e0e1735b7f2c8cb1c9946048c5a7b8e848db2ea), ROM_BIOS(1))

	ROM_REGION(0x80000, "waverom", 0)
	ROM_LOAD("waverom_p106-m8dw.ic16", 0x00000, 0x80000, CRC(b411b848) SHA1(1ea7ca6270b5c128f4e24fa540d205411982e4c1))
ROM_END

ROM_START(k1rii)
	ROM_REGION(0x8000, "coderom", 0)
	//ROM_SYSTEM_BIOS(0, "v1.0", "Version 1.0")
	ROM_LOAD("p205_e4dp.bin", 0x0000, 0x8000, CRC(be15cf1f) SHA1(07a2b7d55cc1f5c49a73e8bc2f50d2f6f0d6c4a8))

	ROM_REGION(0x80000, "waverom", 0)
	ROM_LOAD("p106.bin", 0x00000, 0x80000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1988, k1,  0,  0, k1,  k1, kawai_k1_state, empty_init, "Kawai Musical Instrument Manufacturing", "K1 Digital Multi-Dimensional Synthesizer",         MACHINE_IS_SKELETON)
SYST(1988, k1m, k1, 0, k1m, k1, kawai_k1_state, empty_init, "Kawai Musical Instrument Manufacturing", "K1m Digital Multi-Dimensional Synthesizer Module", MACHINE_IS_SKELETON)
SYST(1988, k1r, k1, 0, k1m, k1, kawai_k1_state, empty_init, "Kawai Musical Instrument Manufacturing", "K1r Digital Multi-Dimensional Synthesizer Module", MACHINE_IS_SKELETON)

SYST(1989, k1rii, 0, 0, k1m, k1, kawai_k1_state, empty_init, "Kawai Musical Instrument Manufacturing", "K1rII Digital Multi-Dimensional Synthesizer Module", MACHINE_IS_SKELETON)
