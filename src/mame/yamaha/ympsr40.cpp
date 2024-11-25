// license:BSD-3-Clause
// copyright-holders:AJR
/*******************************************************************************

    Skeleton driver for Yamaha PSR-40 PortaSound keyboard.

*******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"

namespace {

class yamaha_psr40_state : public driver_device
{
public:
	yamaha_psr40_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void psr40(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};


void yamaha_psr40_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x87ff).mirror(0x1800).ram().share("nvram");
	//map(0xa000, 0xa01f).mirror(0x1fe0).rw("ge7", FUNC(ig10771_device::read), FUNC(ig10771_device::write));
	map(0xa01e, 0xa01f).nopr();
	map(0xc000, 0xc003).mirror(0x1ffc).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));
}


static INPUT_PORTS_START(psr40)
INPUT_PORTS_END

void yamaha_psr40_state::psr40(machine_config &config)
{
	Z80(config, m_maincpu, 4'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &yamaha_psr40_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL + battery?

	I8255(config, "ppi");
}

ROM_START(psr40)
	ROM_REGION(0x8000, "program", 0)
	ROM_LOAD("yamaha_xa004a0_6133-7836.ic4", 0x0000, 0x8000, CRC(7ac4a729) SHA1(e792aabe816d8c46bcc683f0e9220ca50c0faaac)) // 28-pin custom-marked ROM
ROM_END

} // anonymous namespace

SYST(1985, psr40, 0, 0, psr40, psr40, yamaha_psr40_state, empty_init, "Yamaha", "PSR-40", MACHINE_IS_SKELETON)
