// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg WaveStation vector synthesizer.

****************************************************************************/

#include "emu.h"
#include "cpu/h16/hd641016.h"
#include "cpu/m6502/m3745x.h"
//#include "video/t6963c.h"


namespace {

class korgws_state : public driver_device
{
public:
	korgws_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ksp(*this, "ksp")
	{
	}

	void korgws(machine_config &config);
	void korgwssr(machine_config &config);

private:
	void h16_map(address_map &map) ATTR_COLD;
	void wssr_map(address_map &map) ATTR_COLD;

	required_device<hd641016_device> m_maincpu;
	required_device<m3745x_device> m_ksp;
};


void korgws_state::h16_map(address_map &map)
{
	map(0x000000, 0x000007).rom().region("sysroms", 0); // reset vectors
	map(0xc00000, 0xc3ffff).rom().region("sysroms", 0);
}

void korgws_state::wssr_map(address_map &map)
{
	map(0x000000, 0x000007).rom().region("sysroms", 0); // reset vectors
	map(0xc00000, 0xc7ffff).rom().region("sysroms", 0);
}


static INPUT_PORTS_START(korgws)
INPUT_PORTS_END

void korgws_state::korgws(machine_config &config)
{
	HD641016(config, m_maincpu, 20_MHz_XTAL); // HD641016CP10
	m_maincpu->set_addrmap(AS_PROGRAM, &korgws_state::h16_map);

	M37450(config, m_ksp, 20_MHz_XTAL / 2).set_disable(); // M37450M4-233FP
}

void korgws_state::korgwssr(machine_config &config)
{
	korgws(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &korgws_state::wssr_map);
}

ROM_START(korgwsex)
	ROM_REGION16_BE(0x40000, "sysroms", 0) // original EPROM type is NEC D27C1000A-12
	ROM_LOAD16_BYTE("wsex_319_u18.bin", 0x00000, 0x20000, CRC(623eba63) SHA1(5c5e0842d74a9fefbdea01817325a3349f6f9dd6))
	ROM_LOAD16_BYTE("wsex_319_u19.bin", 0x00001, 0x20000, CRC(c7b00f02) SHA1(196e4e403cbae728ec21d27d7b5762963e559028))

	ROM_REGION(0x2000, "ksp", 0)
	ROM_LOAD("m37450m4-233fp.ic34", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION(0x200000, "exroms", 0)
	ROM_LOAD("ws3p7.bin", 0x000000, 0x80000, CRC(ac2e21e3) SHA1(57cd4701a5ca91a74536d7f7ead4074cfe605607))
	ROM_LOAD("ws3p8.bin", 0x080000, 0x80000, CRC(7480bacf) SHA1(8d6e61c11ba909767413df2cae09259e6d8b1476))
	ROM_LOAD16_BYTE("ws3p9.bin", 0x100000, 0x80000, CRC(80a23b7b) SHA1(10b1831b1c2d5d63dd9f4e74cfcfb73c2862f59e))
	ROM_LOAD16_BYTE("ws4p0.bin", 0x100001, 0x80000, CRC(adf239cf) SHA1(a2236f1b0f63c3f854f8ec8751aeb9e167ea0f47))
ROM_END

ROM_START(korgwsad)
	ROM_REGION16_BE(0x40000, "sysroms", 0)
	ROM_SYSTEM_BIOS(0, "v125", "v1.25 Firmware")
	ROMX_LOAD("wsad_1p25_910205l.ic41", 0x00000, 0x20000, CRC(22e5631e) SHA1(ea51e2f2e9effc3985d547d60b109c40e93d4d33), ROM_BIOS(0) | ROM_SKIP(1)) // 27C1000-15
	ROMX_LOAD("wsad_1p25_910305r.ic40", 0x00001, 0x20000, CRC(6a201cbe) SHA1(ee4194baec787d112795b2f34fb21d752c592179), ROM_BIOS(0) | ROM_SKIP(1)) // MBM27C1000-15z
	ROM_SYSTEM_BIOS(1, "v118", "v1.18 Firmware")
	ROMX_LOAD("wsad_118_ic204", 0x00000, 0x20000, CRC(ee3fd19e) SHA1(726c7c3cc201d3360d6274118c13871f8aa46fbd), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("wsad_118_ic304", 0x00001, 0x20000, CRC(4ad559a6) SHA1(f0db05586e7c7be0ce93edd1e07e83d27b69b661), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION(0x2000, "ksp", 0)
	ROM_LOAD("m37450m4-233fp.bin", 0x0000, 0x2000, NO_DUMP)
ROM_END

ROM_START(korgwssr)
	ROM_REGION16_BE(0x80000, "sysroms", 0)
	ROM_LOAD16_BYTE("920305.ic22", 0x00000, 0x40000, CRC(8307a5da) SHA1(31ce491afb74ac1dfa20ae7320b8d1370fa93698))
	ROM_LOAD16_BYTE("920405.ic23", 0x00001, 0x40000, CRC(2d367253) SHA1(c0f8a104a036dd39d220491cc15713158ab3d62a))

	ROM_REGION(0x2000, "ksp", 0)
	ROM_LOAD("m37450m4-233fp.bin", 0x0000, 0x2000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1992, korgwsex, 0, 0, korgws,   korgws, korgws_state, empty_init, "Korg", "WaveStation EX", MACHINE_IS_SKELETON)
SYST(1991, korgwsad, 0, 0, korgws,   korgws, korgws_state, empty_init, "Korg", "WaveStation A/D", MACHINE_IS_SKELETON)
SYST(1992, korgwssr, 0, 0, korgwssr, korgws, korgws_state, empty_init, "Korg", "WaveStation SR", MACHINE_IS_SKELETON)
