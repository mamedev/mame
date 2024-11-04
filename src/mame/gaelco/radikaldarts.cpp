// license:BSD-3-Clause
// copyright-holders:
/*
    Radikal Darts / Radikal System hardware by Gaelco Darts.

    PC hardware running Linux:
    -Intel Celeron SL6C8 1200/256/100/1.5
    -Chipset: Intel NH82801BA, Intel 82815 Graphics Controller + Realtek RTM560 + IRU3013 + Intel DA82562EM Ethernet
    -IDE hard disk (40GB)
    -128MB RAM (1 x Kingston KVR133X64C3Q/128)

    Additional Gaelco custom riser PCB (REF. 050525) with:
    -IDE connector.
    -DVI connector.
    -AD9288 ADC + Altera Cyclone EP1C3T144C8N FPGA + 2 x ALVCH16374 + PCM1725U + DS90C385AMT

    Note: Similar specs as gaelco/gaelcopc.cpp, including a BIOS with just different ACFG data ...
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class radikaldarts_state : public driver_device
{
public:
	radikaldarts_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void radikaldarts(machine_config &config);


private:
	required_device<cpu_device> m_maincpu;

	void radikaldarts_map(address_map &map) ATTR_COLD;
};

void radikaldarts_state::radikaldarts_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x60000);
	map(0xfff80000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( radikaldarts )
INPUT_PORTS_END


void radikaldarts_state::radikaldarts(machine_config &config)
{
	// Basic machine hardware
	PENTIUM3(config, m_maincpu, 120'000'000); // Celeron SL6C8 1.2 GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &radikaldarts_state::radikaldarts_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( radikaldrt )
	ROM_REGION32_LE(0x80000, "bios", 0)
	ROM_LOAD("sst49lf004b_plcc32.u10", 0x00000, 0x80000, CRC(53ab9628) SHA1(5cd54ecb03e29352d8acd3e2e9be5dfbc4dd4064) ) // BIOS string: "10/16/2002-i815EP-627-6A69RPAVC-00 00"

	DISK_REGION( "ide:0:hdd" ) // Hitachi HTS541040G9AT00
	DISK_IMAGE( "hts541040g9at00_dv7.29.25", 0, BAD_DUMP SHA1(37c987c3f5493cabe9f54786702349029e0fda59) ) // Contains operator and players data
ROM_END

} // Anonymous namespace

GAME( 2011?, radikaldrt, 0, radikaldarts, radikaldarts, radikaldarts_state, empty_init, ROT0, "Gaelco Darts", "Radikal Darts (Diana Version 7.29.25)", MACHINE_IS_SKELETON )
