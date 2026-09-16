// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

S3 Sonic Vibes 86C617 multimedia audio based chipset

First (and only) S3 audio card.

Known subvendor ID cards (from dosdays.co.uk, TODO: subvendor IDs):
- Turtle Beach Daytona - line out jack, 32Kx8 SRAM chip for reverb effects
- Aztech SC128 3D (FCC ID: I38-SN97126) - line out jack, no SRAM chip for reverb effects
- ExpertColor MED6617 - no line out jack, no SRAM chip for reverb effects
- GVC Media Technology card (FCC ID: M4CS0027) - line out jack, 32Kx8 SRAM chip for reverb effects
- Ennyah 3D PCI Sound - appears to just be a rebadged ExpertColor MED6617.

Notes:
- Phoenix BIOSes (such as the one in pcipc) do not set up PnP properly, specifically mapping all
  the BARs to $fc** ranges in DOS. To fix user needs to preload s3legacy.exe driver, available in
  ibm5170_cdrom:s3drv21

TODO:
- Complete BAR mapping and base device hookup;
- Implement IRQ and DMA routing;
- Verify that DSP is different than SB Pro;
- Optional wavetable ROMs (ibm5170_cdrom:s3drv21 have it)

**************************************************************************************************/

#include "emu.h"
#include "sonicvibes.h"

#include "speaker.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


// Documentation spells "SonicVibes", on-chip markings says "Sonic Vibes" instead
DEFINE_DEVICE_TYPE(SONICVIBES, sonicvibes_device,   "sonicvibes",   "S3 Sonic Vibes 86C617")



sonicvibes_device::sonicvibes_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
	, m_opl3(*this, "opl3")
	, m_joy(*this, "pc_joy")
{
	// TODO: return actual subvendor ID
	set_ids(0x5333ca00, 0x00, 0x040100, 0x5333ca00);
}

// FIXME: subvendor cards eventually need subclassing, relies on above
sonicvibes_device::sonicvibes_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sonicvibes_device(mconfig, SONICVIBES, tag, owner, clock)
{
}

void sonicvibes_device::device_add_mconfig(machine_config &config)
{
	SPEAKER(config, "speaker", 2).front();

	// TODO: (barely visible) only 24'576 xtal on a Turtle Beach PCB, is it really 12-ish MHz?
	YMF262(config, m_opl3, XTAL(14'318'181));
	m_opl3->add_route(0, "speaker", 1.0, 0);
	m_opl3->add_route(1, "speaker", 1.0, 1);
	m_opl3->add_route(2, "speaker", 1.0, 0);
	m_opl3->add_route(3, "speaker", 1.0, 1);

//  DAC_16BIT_R2R(config, m_ldac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC
//  DAC_16BIT_R2R(config, m_rdac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // unknown DAC

	PC_JOY(config, m_joy);

//  MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(sb_device::midi_rx_w));
//  MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

void sonicvibes_device::device_start()
{
	pci_card_device::device_start();

	add_map( 16, M_IO, FUNC(sonicvibes_device::games_legacy_map));
	add_map( 16, M_IO, FUNC(sonicvibes_device::enhanced_map));
	add_map(  4, M_IO, FUNC(sonicvibes_device::fm_map));
	add_map(  4, M_IO, FUNC(sonicvibes_device::midi_map));
	add_map(  8, M_IO, FUNC(sonicvibes_device::gameport_map));

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
//  map(0x40, 0x43) DMA_A
//  map(0x48, 0x4b) DMA_C
	map(0x50, 0x53).lr32(
		NAME([this] () {
			LOGWARN("Read reserved register $50\n");
			return 0x00010000;
		})
	);
//  map(0x60, 0x63) Wavetable Memory Base
}

// 220h
void sonicvibes_device::games_legacy_map(address_map &map)
{
	map(0x00, 0x03).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
//  map(0x04, 0x04) Mixer Register Index (w/o)
//  map(0x05, 0x05) Mixer Register Data
//  map(0x06, 0x06) Reset
	map(0x08, 0x09).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
//  map(0x0a, 0x0a) Input Data (r/o)
//  map(0x0c, 0x0c) Write Data/Command (w) Write Buffer Status (r)
//  map(0x0e, 0x0e) Read Output Buffer Status (r/o)
}

// 530h
void sonicvibes_device::enhanced_map(address_map &map)
{
//  map(0x00, 0x00) CODEC/Mixer Control
//  map(0x01, 0x01) CODEC/Mixer Interrupt Mask
//  map(0x02, 0x02) CODEC/Mixer Status (r/o)

//  map(0x04, 0x04) CODEC/Mixer Index Address
//  map(0x05, 0x05) CODEC/Mixer Index Data
}

// 388h
void sonicvibes_device::fm_map(address_map &map)
{
	map(0x00, 0x03).rw(m_opl3, FUNC(ymf262_device::read), FUNC(ymf262_device::write));
}

// 330h
void sonicvibes_device::midi_map(address_map &map)
{

}

// 200h
void sonicvibes_device::gameport_map(address_map &map)
{
	map(0x00, 0x07).rw(m_joy, FUNC(pc_joy_device::joy_port_r), FUNC(pc_joy_device::joy_port_w));
}
