// license:BSD-3-Clause
// copyright-holders:
/**************************************************************************************************

Emagic Audiowerk2 / Audiowerk8 Digital Audio Recording PCI

Audiowerk2 HW:
- SAA7146A (Multimedia PCI Bridge Scaler)
- SAA7367 (Digital audio ADC bitstream conversion)
- 2x TDA1315H (?)
- 28-ish MHz (?)

Back cover box claims:
- 2x analog inputs (-10dBV, unbalanced RCA);
- 2x analog outputs (-10dBV, unbalanced RCA);
- 2x Stereo digital I/O (S/PDIF, RCA);
- Bitstream A/D and D/A converters (128 oversampling);
- Sample Rate 44.1 kHz;
- THD: 0.006% @ 1 kHz, 0 dB signal level;
- dynamic range: 92 dB, input-to-output, A-weighted;
- Frequency response: 20 Hz to 20 kHz, -/+ 0.5 dB, input-to-output;
- PCI busmaster;

SAA7146A is also the basis of several other DVB-based capture cards
cfr. https://admin.pci-ids.ucw.cz/read/PC/1131/7146

**************************************************************************************************/

#include "emu.h"
#include "audiowerk2.h"

#define LOG_WARN      (1U << 1)

#define VERBOSE (LOG_GENERAL | LOG_WARN)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"

#define LOGWARN(...)            LOGMASKED(LOG_WARN, __VA_ARGS__)


DEFINE_DEVICE_TYPE(AUDIOWERK2, audiowerk2_device,   "audiowerk2",   "Emagic Audiowerk2 Digital Audio Recording PCI card")



audiowerk2_device::audiowerk2_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: pci_card_device(mconfig, type, tag, owner, clock)
{
	// TODO: subvendor ID, comes from i2c bus
	set_ids(0x11317146, 0x01, 0x048000, 0x00000000);
}

audiowerk2_device::audiowerk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: audiowerk2_device(mconfig, AUDIOWERK2, tag, owner, clock)
{
}

void audiowerk2_device::device_add_mconfig(machine_config &config)
{

}

void audiowerk2_device::device_start()
{
	pci_card_device::device_start();

	add_map( 0x1000000, M_MEM, FUNC(audiowerk2_device::map));

	// INTA#
	intr_pin = 1;
	// TODO: PCI regs 0x3e/0x3f max_lat = 0x26, min_gnt = 0x0f
}

void audiowerk2_device::device_reset()
{
	pci_card_device::device_reset();

	// TODO: default
	command = 0x0000;
	// fast back-to-back, medium DEVSEL#
	status = 0x0280;

	remap_cb();
}

void audiowerk2_device::config_map(address_map &map)
{
	pci_card_device::config_map(map);
}

void audiowerk2_device::map(address_map &map)
{
	// ...
}
