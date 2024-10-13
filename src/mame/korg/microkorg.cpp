// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Korg microKORG compact synthesizer/vocoder.

    The MS2000 Analog Modeling Synthesizer runs on similar hardware.

****************************************************************************/

#include "emu.h"
#include "cpu/h8/h8s2329.h"
#include "machine/intelfsh.h"


namespace {

class microkorg_state : public driver_device
{
public:
	microkorg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void microkorg(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<h8s2320_device> m_maincpu;
};


void microkorg_state::mem_map(address_map &map)
{
	map(0x000000, 0x0fffff).rw("flash", FUNC(intelfsh16_device::read), FUNC(intelfsh16_device::write));
	map(0x400000, 0x47ffff).ram();
}


static INPUT_PORTS_START(microkorg)
INPUT_PORTS_END

void microkorg_state::microkorg(machine_config &config)
{
	H8S2320(config, m_maincpu, 10_MHz_XTAL); // HD6412320VF25 (or HD6412324SVF25)
	m_maincpu->set_addrmap(AS_PROGRAM, &microkorg_state::mem_map);
	// TODO: serial channel 0 is SPI link to DSP; serial channel 1 is MIDI

	FUJITSU_29LV800B(config, "flash"); // MBM29LV800BA-90PFTN

	//DSP56362(config, "dsp", 12.288_MHz_XTAL / 4); // DSPB56362PV100

	//AK4522(config, "codec", 12.288_MHz_XTAL); // AK4522VF
}

ROM_START(microkorg)
	ROM_REGION16_LE(0x100000, "flash", 0)
	ROM_LOAD("korg_microkorg_v1.03_29lv800b.ic20", 0x000000, 0x100000, CRC(607ada7e) SHA1(4a6e2f4068cac7493484af2a8c1d1db7d8bd7a17))
ROM_END

} // anonymous namespace


SYST(2002, microkorg, 0, 0, microkorg, microkorg, microkorg_state, empty_init, "Korg", "microKORG Synthesizer/Vocoder", MACHINE_IS_SKELETON)
