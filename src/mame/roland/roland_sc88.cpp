// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland SC-88 MIDI sound generator.

****************************************************************************/

#include "emu.h"
#include "cpu/h8500/h8510.h"
//#include "cpu/m6502/m38881.h"
#include "machine/nvram.h"


namespace {

class roland_sc88_state : public driver_device
{
public:
	roland_sc88_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void sc88vl(machine_config &config);

private:
	void main_map(address_map &map) ATTR_COLD;

	required_device<h8510_device> m_maincpu;
};


void roland_sc88_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("progrom", 0);
	map(0x080000, 0x08ffff).ram().share("nvram");
}

static INPUT_PORTS_START(sc88vl)
INPUT_PORTS_END

void roland_sc88_state::sc88vl(machine_config &config)
{
	HD6415108(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &roland_sc88_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // SRM2A256SLM-70 x2 + battery

	//M38881M2(config, "subcpu", 20_MHz_XTAL / 2);
}

ROM_START(sc88vl)
	ROM_REGION16_BE(0x80000, "progrom", 0)
	ROM_LOAD16_WORD_SWAP("roland_sc88_vl-1.04.ic29", 0x00000, 0x80000, CRC(66aa5762) SHA1(3a20f8f8cefd0d5e1edb103046f6fe94bb73ac7a))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("roland-r00232667-m38881m2-150gp.ic23", 0x0000, 0x2000, NO_DUMP)

	ROM_REGION16_LE(0x800000, "waverom", 0)
	ROM_LOAD("roland-r00785356-hn624316fbc25.ic10", 0x000000, 0x200000, NO_DUMP)
	ROM_LOAD("roland-r00785367-hn624316fbc26.ic7",  0x200000, 0x200000, NO_DUMP)
	ROM_LOAD("roland-r00788489-hn624316fbc27.ic4",  0x400000, 0x200000, NO_DUMP)
	ROM_LOAD("roland-r00788490-hn624316fbc28.ic2",  0x600000, 0x200000, NO_DUMP)
ROM_END

} // anonymous namespace


SYST(1995, sc88vl, 0, 0, sc88vl, sc88vl, roland_sc88_state, empty_init, "Roland", "SoundCanvas SC-88VL", MACHINE_IS_SKELETON)
