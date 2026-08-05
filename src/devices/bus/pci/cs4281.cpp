// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cs4281.h"

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info

#include "logmacro.h"


DEFINE_DEVICE_TYPE(CS4281, cs4281_device,   "cs4281",   "Cirrus Logic Crystal CS4281 \"SoundFusion\" sound card")


cs4281_device::cs4281_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
}

cs4281_device::cs4281_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cs4281_device(mconfig, CS4281, tag, owner, clock)
{
	// Subsystem ID actually r/w from optional EEPROM (and PCI register $fc)
	set_ids(0x10136005, 0x01, 0x040100, 0x10134281);
}

void cs4281_device::device_add_mconfig(machine_config &config)
{

}

void cs4281_device::device_start()
{
	pci_card_device::device_start();

	add_map( 4*1024,  M_MEM, FUNC(cs4281_device::io_map));
	add_map( 64*1024, M_MEM, FUNC(cs4281_device::mmio_map));

	// INTA#
	intr_pin = 1;

	minimum_grant = 0x04;
	maximum_latency = 0x18;
}

void cs4281_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	command_mask = 0x46;
	status = 0x0210;

	remap_cb();
}


u8 cs4281_device::capptr_r()
{
	return 0x40;
}


void cs4281_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
	// Power Management v1.0, PME# (D0 up to D3hot), D2 and D1 support, Device Specific Init (bit 5)
	map(0x40, 0x43).lr32(NAME([] () { return 0x7f21'0001; }));
//  map(0x44, 0x47) PMCSR

//  map(0xe0, 0xe3) Configuration Write Protect (CWPR)
//  map(0xe4, 0xe7) <reserved>
//  map(0xe8, 0xeb) GPIO Pin Interface (GPIOR)
//  map(0xec, 0xef) Serial Port Power Management & Control (SMPC)
//  map(0xf0, 0xf3) Configuration Load (CFLR)
//  map(0xf4, 0xf7) BIOS and ISA IRQ
//  map(0xf8, 0xfb) <reserved>
//  map(0xfc, 0xff) Subsystem ID write once
}

void cs4281_device::io_map(address_map &map)
{
}

void cs4281_device::mmio_map(address_map &map)
{
}
