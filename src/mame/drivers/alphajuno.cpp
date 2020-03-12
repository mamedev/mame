// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Roland αJuno/SynthPlus 10 synthesizers.

****************************************************************************/

#include "emu.h"
//#include "bus/midi/midi.h"
#include "cpu/mcs51/mcs51.h"
//#include "machine/nvram.h"

class alphajuno_state : public driver_device
{
public:
	alphajuno_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void ajuno1(machine_config &config);

private:
	void prog_map(address_map &map);
	void ext_map(address_map &map);

	required_device<mcs51_cpu_device> m_maincpu;
};

void alphajuno_state::prog_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("program", 0);
}

void alphajuno_state::ext_map(address_map &map)
{
}

static INPUT_PORTS_START(ajuno1)
INPUT_PORTS_END

static INPUT_PORTS_START(ajuno2)
INPUT_PORTS_END

void alphajuno_state::ajuno1(machine_config &config)
{
	I8032(config, m_maincpu, 12_MHz_XTAL); // P8032AH
	m_maincpu->set_addrmap(AS_PROGRAM, &alphajuno_state::prog_map);
	m_maincpu->set_addrmap(AS_IO, &alphajuno_state::ext_map);

	//NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // TC5517APL + battery

	//MB87123(config, "dco", 12_MHz_XTAL);
}

// Original EPROM labels specify major and minor revisions with punch grids; "U" (update?) tag is separate.
// Version strings may be inconsistent with EPROM labels.
ROM_START(ajuno1)
	ROM_REGION(0x4000, "program", 0)
	//ROM_SYSTEM_BIOS(0, "v26", "Version 2.6")
	ROM_LOAD("u__ju-1_2_6.ic10", 0x0000, 0x4000, CRC(9797fd5b) SHA1(0d2e24f8c5f646279985a34ac8bf7c0b9354d32b)) // M5L27128K-2
ROM_END

ROM_START(ajuno2)
	ROM_REGION(0x4000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v25", "Version 2.5")
	ROMX_LOAD("ju-2_2_5__u.ic24", 0x0000, 0x4000, CRC(13b9e68e) SHA1(28a8207a5cd63ababd61d7a46df102ea7116a898), ROM_BIOS(0)) // NEC D27128D-2
	ROM_SYSTEM_BIOS(1, "v25oled", "Version 2.5 (OLED Display Mod)") // http://wp.visuanetics.nl/oled-display-for-alpha-juno-2/
	ROMX_LOAD("ju2-2_5-modified-for-oled-final.bin", 0x0000, 0x4000, CRC(1bca5bc6) SHA1(22e9c71af4b5f3e185f767740e61e5332c0a979f), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(2, "v24", "Version 2.4")
	ROMX_LOAD("ju-2_2_4.ic24", 0x0000, 0x4000, CRC(bfedda17) SHA1(27eee472befdbc7d7ed0caaf359775d8ff3c836a), ROM_BIOS(2)) // M5M27C128
ROM_END

SYST(1985, ajuno1, 0, 0, ajuno1, ajuno1, alphajuno_state, empty_init, "Roland", "Alpha Juno-1 (JU-1) Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
//SYST(1985, hs10, ajuno1, 0, ajuno1, ajuno1, alphajuno_state, empty_init, "Roland", "SynthPlus 10 (HS-10) Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
SYST(1986, ajuno2, 0, 0, ajuno1, ajuno2, alphajuno_state, empty_init, "Roland", "Alpha Juno-2 (JU-2) Programmable Polyphonic Synthesizer", MACHINE_IS_SKELETON)
