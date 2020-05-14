// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg DVP-1 MIDI vocoder.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/upd7810/upd7810.h"
#include "cpu/tms32010/tms32010.h"
#include "machine/nvram.h"

class korgdvp1_state : public driver_device
{
public:
	korgdvp1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_dsp(*this, "dsp%u", 1U)
	{
	}

	void dvp1(machine_config &config);

private:
	void main_map(address_map &map);

	required_device<upd7810_device> m_maincpu;
	required_device_array<tms32010_device, 2> m_dsp;
};


void korgdvp1_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("program", 0);
	map(0x8000, 0x87ff).mirror(0x1800).ram().share("nvram");
}


static INPUT_PORTS_START(dvp1)
INPUT_PORTS_END

void korgdvp1_state::dvp1(machine_config &config)
{
	UPD7810(config, m_maincpu, 12_MHz_XTAL); // µPD7811-161-36 (according to parts list) but with both mode pins pulled up
	m_maincpu->set_addrmap(AS_PROGRAM, &korgdvp1_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + 3V lithium battery

	TMS32010(config, m_dsp[0], 20_MHz_XTAL).set_disable();
	TMS32010(config, m_dsp[1], 20_MHz_XTAL).set_disable();
}

ROM_START(dvp1)
	ROM_REGION(0x8000, "program", 0) // Version: SEP 28, 1985
	ROM_LOAD("850803.ic6", 0x0000, 0x8000, CRC(1170db85) SHA1(4ce773dd22c56982b9493f89dce62111eec596b3)) // MBM27256-25

	ROM_REGION16_LE(0xc00, "dsp1", 0) // 1536 x 16 internal bootloader
	ROM_LOAD("tms320m10nl.ic9", 0x000, 0xc00, NO_DUMP)

	ROM_REGION16_LE(0xc00, "dsp2", 0) // 1536 x 16 internal bootloader (almost certainly identical to DSP 1)
	ROM_LOAD("tms320m10nl.ic8", 0x000, 0xc00, NO_DUMP)
ROM_END

SYST(1985, dvp1, 0, 0, dvp1, dvp1, korgdvp1_state, empty_init, "Korg", "DVP-1 Digital Voice Processor", MACHINE_IS_SKELETON)
