// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for DEC MicroVAX II ("Mayflower").

*******************************************************************************/

#include "emu.h"

//#include "bus/qbus/qbus.h"
#include "cpu/vax/vax.h"
//#include "machine/dc319.h"
#include "machine/mc146818.h"

namespace {

class uvax2_state : public driver_device
{
public:
	uvax2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void uvax2(machine_config &config) ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void uvax2_state::mem_map(address_map &map)
{
	map(0x00000000, 0x0000ffff).rom().region("bootrom", 0);
}

static INPUT_PORTS_START(uvax2)
INPUT_PORTS_END

void uvax2_state::uvax2(machine_config &config)
{
	DC333(config, m_maincpu, 40_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &uvax2_state::mem_map);

	//DC319(config, "dlart", 614.4_MHz_XTAL);

	MC146818(config, "toyclock", 32.768_kHz_XTAL);
}

ROM_START(uvax2)
	ROM_REGION32_LE(0x10000, "bootrom", 0)
	ROM_SYSTEM_BIOS(0, "ef", "Rev. EF") // Rev. AF is identical?
	ROMX_LOAD("m7606_a1_rev_ef_lm8725_110e6_am27256.bin", 0x0000, 0x8000, CRC(b44e8cab) SHA1(8b74031231248241109b78baa4819659533e1312), ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("m7606_a1_rev_ef_lm8729_111e6_am27256.bin", 0x0001, 0x8000, CRC(44ebb04d) SHA1(031c271ead78cae4c3dfc1125b0ab515843e1911), ROM_SKIP(1) | ROM_BIOS(0))
	ROM_SYSTEM_BIOS(1, "ah", "Rev. AH")
	ROMX_LOAD("m7606_ah_062_110e6_d27256.bin", 0x0000, 0x8000, CRC(653646fa) SHA1(1f900fe5e998d0f8ed4ae63557f1e63c839991c0), ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("m7606_ah_062_111e6_d27256.bin", 0x0001, 0x8000, CRC(725dc43e) SHA1(630a908f7d630a33099ae9c3c3c08df478de941c), ROM_SKIP(1) | ROM_BIOS(1))
ROM_END

} // anonymous namespace

COMP(1984, uvax2, 0, 0, uvax2, uvax2, uvax2_state, empty_init, "Digital Equipment Corporation", "MicroVAX II", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
