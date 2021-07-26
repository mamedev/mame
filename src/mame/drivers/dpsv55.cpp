// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Sony DPS-V55/V55M effects processor.

    V55 and V55M are identical except for the power supply (120 V, 60 Hz for
    the former, 230 V, 50/60 Hz for the latter).

****************************************************************************/

#include "emu.h"
#include "cpu/f2mc16/mb9061x.h"
#include "machine/nvram.h"

namespace {

class dpsv55_state : public driver_device
{
public:
	dpsv55_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void dpsv55(machine_config &config);

private:
	void mem_map(address_map &map);

	required_device<cpu_device> m_maincpu;
};


void dpsv55_state::mem_map(address_map &map)
{
	map(0xfc0000, 0xfc7fff).mirror(0x18000).ram().share("nvram"); // CS1
	map(0xfe0000, 0xffffff).rom().region("eprom", 0); // CS0
}


static INPUT_PORTS_START(dpsv55)
INPUT_PORTS_END

void dpsv55_state::dpsv55(machine_config &config)
{
	MB90641A(config, m_maincpu, 4_MHz_XTAL); // MB90641APF-G-105BND
	m_maincpu->set_addrmap(AS_PROGRAM, &dpsv55_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CY62256LL-70SNC-T2 + 3V lithium battery + M62021FP-600C reset generator + M5239L voltage detector

	// TODO: LCD unit (LC0801)

	//CXD2707(config, "dsp", 49.152_MHz_XTAL);
}

/*
Sony DPS-V55

Version 1.02

Chip: MX 27C1000DC-90

Sticker:
~~~~~~~~~~~~
 DPS-V55/M
 Ver.1.02
 759-499074
~~~~~~~~~~~~

Multi Processor  DPS-V55
Sony Corporation (c)1998
*/

ROM_START(dpsv55)
	ROM_REGION(0x20000, "eprom", 0)
	ROM_LOAD("dps-v55_m__ver.1.02__759-499-74.ic704", 0x00000, 0x20000, CRC(138c2fe0) SHA1(0916ccb1d7567639b382a19240a56274c5c2fa4a))
ROM_END

} // anonymous namespace

SYST(1998, dpsv55, 0, 0, dpsv55, dpsv55, dpsv55_state, empty_init, "Sony", "DPS-V55 Multi-Effect Processor", MACHINE_IS_SKELETON)
