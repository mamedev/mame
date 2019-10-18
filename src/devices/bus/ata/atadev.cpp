// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    ATA Device implementation.

***************************************************************************/

#include "emu.h"
#include "atadev.h"

#include "atapicdr.h"
#include "idehd.h"

//-------------------------------------------------
//  device_ata_interface - constructor
//-------------------------------------------------

device_ata_interface::device_ata_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_irq_handler(device),
	m_dmarq_handler(device),
	m_dasp_handler(device),
	m_pdiag_handler(device)
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

ata_slot_device::ata_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, ATA_SLOT, tag, owner, clock),
		device_slot_interface(mconfig, *this),
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
	m_dev = dynamic_cast<device_ata_interface *>(get_card_device());
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
}
