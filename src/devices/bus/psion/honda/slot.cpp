// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Psion Honda Expansion slot emulation

    This port is on Series 3c/3mx/Siena machines only.

    TODO:
    - add RS232 interface handlers

**********************************************************************/

#include "emu.h"
#include "slot.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSION_HONDA_SLOT, psion_honda_slot_device, "psion_honda_slot", "Psion Honda Expansion slot")


//**************************************************************************
//  DEVICE PSION_EXP PORT INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_psion_honda_interface - constructor
//-------------------------------------------------

device_psion_honda_interface::device_psion_honda_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "psionhonda")
{
	m_slot = dynamic_cast<psion_honda_slot_device *>(device.owner());
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psion_honda_slot_device - constructor
//-------------------------------------------------

psion_honda_slot_device::psion_honda_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, PSION_HONDA_SLOT, tag, owner, clock)
	, device_single_card_slot_interface<device_psion_honda_interface>(mconfig, *this)
	, m_rxd_handler(*this)
	, m_dcd_handler(*this)
	, m_dsr_handler(*this)
	, m_cts_handler(*this)
	, m_sdoe_handler(*this)
	, m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_honda_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  read
//-------------------------------------------------

uint8_t psion_honda_slot_device::data_r()
{
	return m_card ? m_card->data_r() : 0x00;
}


//-------------------------------------------------
//  write
//-------------------------------------------------

void psion_honda_slot_device::data_w(uint16_t data)
{
	if (m_card)
		m_card->data_w(data);
}

void psion_honda_slot_device::write_txd(int state)
{
	if (m_card)
		m_card->write_txd(state);
}

void psion_honda_slot_device::write_dtr(int state)
{
	if (m_card)
		m_card->write_dtr(state);
}

void psion_honda_slot_device::write_rts(int state)
{
	if (m_card)
		m_card->write_rts(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( psion_honda_devices )
//-------------------------------------------------

// slot devices
//#include "cyclone.h"
#include "parallel.h"
#include "pclink.h"
#include "ssd.h"

void psion_honda_devices(device_slot_interface &device)
{
	//device.option_add("cyclone", PSION_HONDA_CYCLONE);          // Cyclone Floppy Drive (Honda)
	device.option_add("parallel", PSION_PARALLEL);              // Psion Parallel Printer Link cable
	device.option_add("pclink", PSION_PCLINK);                  // Psion PC Link cable
	device.option_add("ssd", PSION_SIENA_SSD);                  // Psion Siena SSD Drive
}
