// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for Triple-C stereo compressor.

****************************************************************************/

#include "emu.h"
#include "cpu/dsp563xx/dsp56362.h"

namespace {

class tc_manhattan_state : public driver_device
{
public:
	tc_manhattan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_dsp(*this, "dsp")
		, m_eprom(*this, "eprom")
	{
	}

	void triplec(machine_config &config);

private:
	void p_map(address_map &map) ATTR_COLD;
	void x_map(address_map &map) ATTR_COLD;

	u32 eprom_r(offs_t offset);

	required_device<dsp56362_device> m_dsp;
	required_region_ptr<u8> m_eprom;
};


u32 tc_manhattan_state::eprom_r(offs_t offset)
{
	return m_eprom[offset];
}

void tc_manhattan_state::p_map(address_map &map)
{
	map(0x000c00, 0x0013ff).ram(); // FIXME: actually a configurable extension of internal RAM
	map(0x020000, 0x03ffff).ram().share("sram"); // 3x KM68V1002BJ-10
	map(0xd00000, 0xd3ffff).r(FUNC(tc_manhattan_state::eprom_r));
}

void tc_manhattan_state::x_map(address_map &map)
{
	map(0x020000, 0x03ffff).ram().share("sram");
	map(0x040000, 0x07ffff).r(FUNC(tc_manhattan_state::eprom_r));
}


static INPUT_PORTS_START(triplec)
INPUT_PORTS_END

void tc_manhattan_state::triplec(machine_config &config)
{
	DSP56362(config, m_dsp, 10'000'000); // clock unknown
	m_dsp->set_hard_omr(0b0001); // bytewide external ROM bootstrap
	m_dsp->set_addrmap(dsp56362_device::AS_P, &tc_manhattan_state::p_map);
	m_dsp->set_addrmap(dsp56362_device::AS_X, &tc_manhattan_state::x_map);

	//AT90LS8535(config, "mcu", 12'000'000);
}


/*

TC Electronic Triple-C Stereo

DSP ROM Chip: 39VF020
DSP: Motorola XCB56362PV100

Sticker:
~~~~~~~~~~~~
   M-3 B
 MAIN V1.02
~~~~~~~~~~~~

Main board uses the TC Manhattan architecture, same as G-Major.


Panel MCU: AT90LS8535 in PLCC (not yet dumped!)

Panel MCU Sticker:
~~~~~~~~~~~~~
   M-3 B
 FRONT V1.04
~~~~~~~~~~~~~

*/

ROM_START(triplec)
	ROM_REGION(0x40000, "eprom", 0)
	ROM_LOAD("m-3_b_main_v1.02.ic12", 0x00000, 0x40000, CRC(d06dfc07) SHA1(e0d74a75d8cb107fc05fa93423a5a6681c35edd2))

	ROM_REGION(0x2000, "mcu", 0)
	ROM_LOAD("m-3_b_front_v1.04.bin", 0x0000, 0x2000, NO_DUMP)
ROM_END

} // anonymous namespace

SYST(2000, triplec, 0, 0, triplec, triplec, tc_manhattan_state, empty_init, "TC Electronic", "Triple-C Stereo Channel Multiband Compressor & Envelope", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
