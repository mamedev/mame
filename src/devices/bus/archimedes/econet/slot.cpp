// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn Archimedes Econet Module

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(ARCHIMEDES_ECONET_SLOT, archimedes_econet_slot_device, "archimedes_econet_slot", "Acorn Archimedes Econet Module slot")



//**************************************************************************
//  DEVICE BBC_MODEM PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_archimedes_econet_interface - constructor
//-------------------------------------------------

device_archimedes_econet_interface::device_archimedes_econet_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "arceconet")
{
	m_slot = dynamic_cast<archimedes_econet_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  archimedes_econet_slot_device - constructor
//-------------------------------------------------

archimedes_econet_slot_device::archimedes_econet_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ARCHIMEDES_ECONET_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_archimedes_econet_interface>(mconfig, *this)
	, m_device(nullptr)
	, m_efiq_handler(*this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void archimedes_econet_slot_device::device_start()
{
	m_device = get_card_device();

	// resolve callbacks
	m_efiq_handler.resolve_safe();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

u8 archimedes_econet_slot_device::read(offs_t offset)
{
	if (m_device)
		return m_device->read(offset & 0x03);
	else
		return 0xff;
}

//-------------------------------------------------
//  write
//-------------------------------------------------

void archimedes_econet_slot_device::write(offs_t offset, u8 data)
{
	if (m_device)
		m_device->write(offset & 0x03, data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( archimedes_econet_devices )
//-------------------------------------------------


// slot devices
#include "econet.h"
#include "midi.h"
#include "rtfmjoy.h"


void archimedes_econet_devices(device_slot_interface &device)
{
	device.option_add("econet", ARC_ECONET);
	device.option_add("midi", ARC_SERIAL_MIDI);
	device.option_add("rtfmjoy", ARC_RTFM_JOY);
	device.option_add("sampler", ARC_SERIAL_SAMPLER);
}
