// license:BSD-3-Clause
// copyright-holders:Enik Land
/**********************************************************************

    Input Port for the Sega 3-D Glasses / SegaScope 3-D Glasses

**********************************************************************/

#include "emu.h"
#include "s3dport.h"

// slot devices
#include "s3dglass.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(SMS_3D_PORT, sms_3d_port_device, "sms_3d_port", "Sega 3-D Glasses Port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_sms_3d_port_interface - constructor
//-------------------------------------------------

device_sms_3d_port_interface::device_sms_3d_port_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "3dport")
{
}


//-------------------------------------------------
//  ~device_sms_3d_port_interface - destructor
//-------------------------------------------------

device_sms_3d_port_interface::~device_sms_3d_port_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  sms_3d_port_device - constructor
//-------------------------------------------------

sms_3d_port_device::sms_3d_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SMS_3D_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_sms_3d_port_interface>(mconfig, *this),
	m_screen(nullptr),
	m_device(nullptr)
{
}


//-------------------------------------------------
//  sms_3d_port_device - destructor
//-------------------------------------------------

sms_3d_port_device::~sms_3d_port_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void sms_3d_port_device::device_start()
{
}


void sms_3d_port_device::device_resolve_objects()
{
	m_device = dynamic_cast<device_sms_3d_port_interface *>(get_card_device());
	if (m_device)
		m_device->set_screen_device(*m_screen);
}


WRITE_LINE_MEMBER(sms_3d_port_device::write_sscope)
{
	if (m_device)
		m_device->write_sscope(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( sms_3d_port_devices )
//-------------------------------------------------

void sms_3d_port_devices(device_slot_interface &device)
{
	device.option_add("3dglass", SMS_3D_GLASSES);
}

