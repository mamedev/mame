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
	m_port = dynamic_cast<sms_3d_port_device *>(device.owner());
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
	device_slot_interface(mconfig, *this),
	m_screen(*this, finder_base::DUMMY_TAG),
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
	save_item(NAME(m_sscope_state));
	m_device = dynamic_cast<device_sms_3d_port_interface *>(get_card_device());
}


WRITE_LINE_MEMBER(sms_3d_port_device::write_sscope)
{
	if (state != m_sscope_state) {
		m_device->update_displayed_range();
		m_sscope_state = state;
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( sms_3d_port_devices )
//-------------------------------------------------

void sms_3d_port_devices(device_slot_interface &device)
{
	device.option_add("3dglass", SMS_3D_GLASSES);
}

