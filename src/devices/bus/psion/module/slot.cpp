// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Extended Internal Expansion slot emulation

    This port is on Series MC/HC machines only.

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSION_MODULE_SLOT, psion_module_slot_device, "psion_module_slot", "Psion Extended Internal Expansion slot")


//**************************************************************************
//  DEVICE PSION_EXP PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_psion_module_interface - constructor
//-------------------------------------------------

device_psion_module_interface::device_psion_module_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "psionmodule")
{
	m_slot = dynamic_cast<psion_module_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psion_module_slot_device - constructor
//-------------------------------------------------

psion_module_slot_device::psion_module_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_MODULE_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_psion_module_interface>(mconfig, *this)
	, m_intr_cb(*this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_module_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t psion_module_slot_device::data_r()
{
	return m_card ? m_card->data_r() : 0x00;
}

uint8_t psion_module_slot_device::io_r(offs_t offset)
{
	if (m_card)
		return m_card->io_r(offset);
	else
		return 0x00;
}


//-------------------------------------------------
//  write
//-------------------------------------------------

void psion_module_slot_device::data_w(uint16_t data)
{
	if (m_card)
		m_card->data_w(data);
}

void psion_module_slot_device::io_w(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->io_w(offset, data);
}


//-------------------------------------------------
//  SLOT_INTERFACE( psion_module_devices )
//-------------------------------------------------

// slot devices
#include "serpar.h"

void psion_hcmodule_devices(device_slot_interface &device)
{
	device.option_add("serpar", PSION_SERIAL_PARALLEL);         // Psion RS232/Parallel Module
}

void psion_mcmodule_devices(device_slot_interface &device)
{
	device.option_add("serpar", PSION_SERIAL_PARALLEL);         // Psion RS232/Parallel Module
}
