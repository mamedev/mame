// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    SAM Coupe Drive Slot

    32-pin slot

***************************************************************************/

#include "emu.h"
#include "drive.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(SAMCOUPE_DRIVE_PORT, samcoupe_drive_port_device, "samcoupe_drive_port", "SAM Coupe Drive Port")


//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  samcoupe_drive_port_device - constructor
//-------------------------------------------------

samcoupe_drive_port_device::samcoupe_drive_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SAMCOUPE_DRIVE_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_samcoupe_drive_interface>(mconfig, *this),
	m_module(nullptr)
{
}

//-------------------------------------------------
//  samcoupe_drive_port_device - destructor
//-------------------------------------------------

samcoupe_drive_port_device::~samcoupe_drive_port_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void samcoupe_drive_port_device::device_start()
{
	// get inserted module
	m_module = get_card_device();
}

//-------------------------------------------------
//  host to module interface
//-------------------------------------------------

uint8_t samcoupe_drive_port_device::read(offs_t offset)
{
	if (m_module)
		return m_module->read(offset);

	return 0xff;
}

void samcoupe_drive_port_device::write(offs_t offset, uint8_t data)
{
	if (m_module)
		m_module->write(offset, data);
}


//**************************************************************************
//  MODULE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_samcoupe_drive_interface - constructor
//-------------------------------------------------

device_samcoupe_drive_interface::device_samcoupe_drive_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "samcoupedrive")
{
	m_port = dynamic_cast<samcoupe_drive_port_device *>(device.owner());
}

//-------------------------------------------------
//  ~device_samcoupe_drive_interface - destructor
//-------------------------------------------------

device_samcoupe_drive_interface::~device_samcoupe_drive_interface()
{
}
