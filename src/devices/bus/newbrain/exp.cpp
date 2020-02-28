// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Grundy NewBrain Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(NEWBRAIN_EXPANSION_SLOT, newbrain_expansion_slot_device, "newbrain_expansion_slot", "NewBrain expansion port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_newbrain_expansion_slot_interface - constructor
//-------------------------------------------------

device_newbrain_expansion_slot_interface::device_newbrain_expansion_slot_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "newbrainexp")
{
	m_slot = dynamic_cast<newbrain_expansion_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  newbrain_expansion_slot_device - constructor
//-------------------------------------------------

newbrain_expansion_slot_device::newbrain_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, NEWBRAIN_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_newbrain_expansion_slot_interface>(mconfig, *this),
	m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void newbrain_expansion_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void newbrain_expansion_slot_device::device_reset()
{
	if (m_card != nullptr)
	{
		m_card->device().reset();
	}
}


//-------------------------------------------------
//  mreq_r - memory request read
//-------------------------------------------------

uint8_t newbrain_expansion_slot_device::mreq_r(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	if (m_card != nullptr)
	{
		data = m_card->mreq_r(offset, data, romov, exrm, raminh);
	}

	return data;
}


//-------------------------------------------------
//  mreq_w - memory request write
//-------------------------------------------------

void newbrain_expansion_slot_device::mreq_w(offs_t offset, uint8_t data, bool &romov, int &exrm, bool &raminh)
{
	if (m_card != nullptr)
	{
		m_card->mreq_w(offset, data, romov, exrm, raminh);
	}
}


//-------------------------------------------------
//  iorq_r - I/O request read
//-------------------------------------------------

uint8_t newbrain_expansion_slot_device::iorq_r(offs_t offset, uint8_t data, bool &prtov)
{
	if (m_card != nullptr)
	{
		data = m_card->iorq_r(offset, data, prtov);
	}

	return data;
}


//-------------------------------------------------
//  iorq_w - I/O request write
//-------------------------------------------------

void newbrain_expansion_slot_device::iorq_w(offs_t offset, uint8_t data, bool &prtov)
{
	if (m_card != nullptr)
	{
		m_card->iorq_w(offset, data, prtov);
	}
}


//-------------------------------------------------
//  SLOT_INTERFACE( newbrain_expansion_cards )
//-------------------------------------------------

// slot devices
#include "eim.h"
#include "fdc.h"

void newbrain_expansion_cards(device_slot_interface &device)
{
	device.option_add("eim", NEWBRAIN_EIM);
	device.option_add("fdc", NEWBRAIN_FDC);
}
