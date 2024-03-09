// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

ESS Solo-1/Maestro audio PCI cards

- PCI rev 2.2, APM 1.2, ACPI 1.0, PPMI 1.0;
- ESFM (SoundBlaster clone);
- Spatializer VBX 3-D as DSP core;
- 2x game ports;
- i2s Zoom Video port for MPEG audio;
- Known to have a poor SnR ratio (around -64 dB)

- Required by misc/gammagic.cpp
- Embedded on Chaintech 6ESA-2-E100N (Slot 1, I440EX / PIIX4E)
- Embedded on ASUS ME-99 (Socket 370, SiS620 / SiS5595)

**************************************************************************************************/

#include "emu.h"
#include "ess_maestro.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)

// ES1938 Solo-1
DEFINE_DEVICE_TYPE(ES1946_SOLO1E, es1946_solo1e_device,   "es1946_solo1e",   "ES1938/ES1946/ES1969 Solo-1 Audiodrive")
// ES1969 Solo-1
// ES1948 Maestro-1
// ES1968 Maestro-2
// ES1978 Maestro-2E


es1946_solo1e_device::es1946_solo1e_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	// TODO: subvendor from EEPROM
	set_ids(0x125d1969, 0x01, 0x040100, 0x125d1969);
}

es1946_solo1e_device::es1946_solo1e_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: es1946_solo1e_device(mconfig, ES1946_SOLO1E, tag, owner, clock)
{
}

void es1946_solo1e_device::device_add_mconfig(machine_config &config)
{

}

void es1946_solo1e_device::device_start()
{
	pci_card_device::device_start();

	add_map(64, M_IO, FUNC(es1946_solo1e_device::extended_map));
	add_map(16, M_IO, FUNC(es1946_solo1e_device::sb_map));
	add_map(16, M_IO, FUNC(es1946_solo1e_device::vcbase_map));
	add_map( 4, M_IO, FUNC(es1946_solo1e_device::mpu_map));
	add_map( 4, M_IO, FUNC(es1946_solo1e_device::gameport_map));

	// INTA#
	intr_pin = 1;
}

void es1946_solo1e_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	// ACPI capable, Fast back-to-back, medium DEVSEL#
	status = 0x0290;
	// TODO: min_gnt 0x02, max_lat = 0x18

	remap_cb();
}

uint8_t es1946_solo1e_device::capptr_r()
{
	return 0xc0;
}

void es1946_solo1e_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//  map(0x40, 0x41) Legacy Audio Control
//  map(0x50, 0x53) ES1946 Config
//  map(0x60, 0x61) DDMA Control
	// ACPI
	map(0xc0, 0xc0).lr8(NAME([] () { return 0x01; }));
	map(0xc1, 0xc1).lr8(NAME([] () { return 0x00; })); // NULL pointer
//  map(0xc2, 0xc3) Power Management Capabilities
//  map(0xc4, 0xc5) Power Management Control/Status
}

void es1946_solo1e_device::extended_map(address_map &map)
{
}
void es1946_solo1e_device::sb_map(address_map &map)
{
}

void es1946_solo1e_device::vcbase_map(address_map &map)
{
}

void es1946_solo1e_device::mpu_map(address_map &map)
{
}

void es1946_solo1e_device::gameport_map(address_map &map)
{
}
