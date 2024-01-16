// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

S3 SonicVibes 86C617 multimedia audio based chipset

First (and only) S3 audio card.

Known subvendor ID cards (from dosdays.co.uk, TODO: subvendor IDs):
- Turtle Beach Daytona - line out jack, 32Kx8 SRAM chip for reverb effects
- Aztech SC128 3D (FCC ID: I38-SN97126) - line out jack, no SRAM chip for reverb effects
- ExpertColor MED6617 - no line out jack, no SRAM chip for reverb effects
- GVC Media Technology card (FCC ID: M4CS0027) - line out jack, 32Kx8 SRAM chip for reverb effects
- Ennyah 3D PCI Sound - appears to just be a rebadged ExpertColor MED6617.

TODO:
- everything;

**************************************************************************************************/

#include "emu.h"
#include "sonicvibes.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(SONICVIBES, sonicvibes_device,   "sonicvibes",   "S3 SonicVibes 86C617 PCI card")



sonicvibes_device::sonicvibes_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	// TODO: return actual subvendor ID
	set_ids(0x5333ca00, 0x00, 0x040100, 0x5333ca00);
}

sonicvibes_device::sonicvibes_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sonicvibes_device(mconfig, SONICVIBES, tag, owner, clock)
{
}

void sonicvibes_device::device_add_mconfig(machine_config &config)
{

}

void sonicvibes_device::device_start()
{
	pci_card_device::device_start();

//	add_map( 16, M_IO, FUNC(sonicvibes_device::games_base_map));
//	add_map( 16, M_IO, FUNC(sonicvibes_device::enhanced_map));
//	add_map(  4, M_IO, FUNC(sonicvibes_device::fm_map));
//	add_map(  4, M_IO, FUNC(sonicvibes_device::midi_map));
//	add_map(  8, M_IO, FUNC(sonicvibes_device::gameport_map));

	// INTA#
	intr_pin = 1;
}

void sonicvibes_device::device_reset()
{
	pci_card_device::device_reset();

	command = 0x0000;
	// medium DEVSEL / User Definable Features
	status = 0x0240;

	remap_cb();
}

void sonicvibes_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
//	map(0x40, 0x43) DMA_A
//	map(0x48, 0x4b) DMA_C
	map(0x50, 0x53).lr32(
		NAME([this] () {
			LOGWARN("Read reserved register $50\n");
			return 0x00010000;
		})
	);
//	map(0x60, 0x63) Wavetable Memory Base
}
