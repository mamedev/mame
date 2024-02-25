// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ATA Device implementation.

***************************************************************************/

#include "emu.h"
#include "atadev.h"

#include "atapicdr.h"
#include "cp2024.h"
#include "cr589.h"
#include "hdd.h"
#include "px320a.h"
#include "zip100.h"

//-------------------------------------------------
//  device_ata_interface - constructor
//-------------------------------------------------

device_ata_interface::device_ata_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "ata"),
	m_slot(dynamic_cast<ata_slot_device *>(device.owner()))
{
}


//**************************************************************************
//  ATA SLOT DEVICE
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(ATA_SLOT, ata_slot_device, "ata_slot", "ATA Connector")

//-------------------------------------------------
//  ata_slot_device - constructor
//-------------------------------------------------

ata_slot_device::ata_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ATA_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_ata_interface>(mconfig, *this),
	m_irq_handler(*this),
	m_dmarq_handler(*this),
	m_dasp_handler(*this),
	m_pdiag_handler(*this),
	m_dev(nullptr)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void ata_slot_device::device_config_complete()
{
	m_dev = get_card_device();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void ata_slot_device::device_start()
{
}


void ata_devices(device_slot_interface &device)
{
	device.option_add("hdd", IDE_HARDDISK);
	device.option_add("cdrom", ATAPI_CDROM);

	device.option_add("cf", ATA_CF);
	device.option_add("cp2024", CP2024);
	device.option_add("cr589", CR589);
	device.option_add("px320a", PX320A);
	device.option_add("zip100", ZIP100_IDE);
}
