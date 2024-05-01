// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_scsiext.c
 *
 * Sharp CZ-6BS1 SCSI-1 controller
 *
 *  Created on: 5/06/2012
 */

#include "emu.h"
#include "x68k_scsiext.h"

#include "bus/nscsi/hd.h"
#include "machine/mb87030.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(X68K_SCSIEXT, x68k_scsiext_device, "x68k_cz6bs1", "Sharp CZ-6BS1 SCSI-1")

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( x68k_cz6bs1 )
	ROM_REGION( 0x10000, "scsiexrom", 0 )
	ROM_LOAD16_WORD_SWAP( "scsiexrom.bin",   0x0000, 0x2000, CRC(7be488de) SHA1(49616c09a8986ffe6a12ad600febe512f7ba8ae4) )
ROM_END

const tiny_rom_entry *x68k_scsiext_device::device_rom_region() const
{
	return ROM_NAME( x68k_cz6bs1 );
}

static void scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

// device machine config
void x68k_scsiext_device::device_add_mconfig(machine_config &config)
{
	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", scsi_devices, "harddisk");
	NSCSI_CONNECTOR(config, "scsi:1", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(8'000'000); // ?
			spc.out_irq_callback().set(*this, FUNC(x68k_scsiext_device::irq_w));
			spc.out_dreq_callback().set(*this, FUNC(x68k_scsiext_device::drq_w));
		});
}

x68k_scsiext_device::x68k_scsiext_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X68K_SCSIEXT, tag, owner, clock)
	, device_x68k_expansion_card_interface(mconfig, *this)
	, m_slot(nullptr)
	, m_spc(*this, "scsi:7:spc")
	, m_rom(*this, "scsiexrom")
{
}

void x68k_scsiext_device::device_start()
{
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());

	m_slot->space().install_rom(0xea0020,0xea1fff, m_rom.target());
	m_slot->space().unmap_write(0xea0020,0xea1fff);
	m_slot->space().install_device(0xea0000, 0xea001f, *m_spc, &mb89352_device::map, 0x00ff00ff);
}

void x68k_scsiext_device::device_reset()
{
}

void x68k_scsiext_device::irq_w(int state)
{
	m_slot->irq2_w(state);  // correct?  Or perhaps selectable?
}

uint8_t x68k_scsiext_device::iack2()
{
	return 0xf6;
}

void x68k_scsiext_device::drq_w(int state)
{
	// TODO
}
