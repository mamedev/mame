// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    VideoBrain Accessory Jack emulation

**********************************************************************/

#include "emu.h"
#include "accessory.h"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(VIDEOBRAIN_ACCESSORY_PORT, videobrain_accessory_port_device, "vidbrain_accessory", "VideoBrain accessory jack")



//**************************************************************************
//  DEVICE VIDEOBRAIN_ACCESSORY_INTERFACE
//**************************************************************************

device_videobrain_accessory_interface::device_videobrain_accessory_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "vidbrainacc")
{
	m_port = dynamic_cast<videobrain_accessory_port_device *>(device.owner());
}

void device_videobrain_accessory_interface::videobrain_io_w(uint8_t data)
{
}

uint8_t device_videobrain_accessory_interface::videobrain_io_r()
{
	return 0;
}

void device_videobrain_accessory_interface::videobrain_reset_w(int state)
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

videobrain_accessory_port_device::videobrain_accessory_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VIDEOBRAIN_ACCESSORY_PORT, tag, owner, clock),
	device_single_card_slot_interface<device_videobrain_accessory_interface>(mconfig, *this),
	m_write_ext_int(*this),
	m_card(nullptr)
{
}

void videobrain_accessory_port_device::device_start()
{
	m_card = get_card_device();
}



//-------------------------------------------------
//  SLOT_INTERFACE( videobrain_accessories )
//-------------------------------------------------

#include "expander1.h"

void videobrain_accessories(device_slot_interface &device)
{
	device.option_add("expander1", VIDEOBRAIN_EXPANDER1);
}
