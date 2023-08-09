// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Reduced External Expansion slot emulation

    This port is on Series 3/3a machines only.

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSION_SIBO_SLOT, psion_sibo_slot_device, "psion_sibo_slot", "Psion Reduced External Expansion slot")


//**************************************************************************
//  DEVICE PSION_EXP PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_psion_sibo_interface - constructor
//-------------------------------------------------

device_psion_sibo_interface::device_psion_sibo_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "psionsibo")
{
	m_slot = dynamic_cast<psion_sibo_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psion_sibo_slot_device - constructor
//-------------------------------------------------

psion_sibo_slot_device::psion_sibo_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_SIBO_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_psion_sibo_interface>(mconfig, *this)
	, m_int_cb(*this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_sibo_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t psion_sibo_slot_device::data_r()
{
	return m_card ? m_card->data_r() : 0x00;
}


//-------------------------------------------------
//  write
//-------------------------------------------------

void psion_sibo_slot_device::data_w(uint16_t data)
{
	if (m_card)
		m_card->data_w(data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( psion_sibo_devices )
//-------------------------------------------------

// slot devices
#include "3fax.h"
#include "3link.h"

void psion_sibo_devices(device_slot_interface &device)
{
	device.option_add("fax", PSION_3FAX_MODEM);              // Psion 3-Fax Modem
	device.option_add("parallel", PSION_3LINK_PARALLEL);     // Psion 3-Link Parallel Printer Interface
	device.option_add("serial", PSION_3LINK_SERIAL);         // Psion 3-Link RS232 Serial Interface
}
