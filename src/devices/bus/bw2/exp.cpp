// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Bondwell 2 Expansion Port emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(BW2_EXPANSION_SLOT, bw2_expansion_slot_device, "bw2_expansion_slot", "Bondwell 2 expansion port")



//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_bw2_expansion_slot_interface - constructor
//-------------------------------------------------

device_bw2_expansion_slot_interface::device_bw2_expansion_slot_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "bw2exp")
{
	m_slot = dynamic_cast<bw2_expansion_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bw2_expansion_slot_device - constructor
//-------------------------------------------------

bw2_expansion_slot_device::bw2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BW2_EXPANSION_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_bw2_expansion_slot_interface>(mconfig, *this),
	m_memspace(*this, finder_base::DUMMY_TAG, -1),
	m_card(nullptr),
	m_bank(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bw2_expansion_slot_device::device_start()
{
	m_card = get_card_device();
}


//-------------------------------------------------
//  ram_select - select RAM bank
//-------------------------------------------------

void bw2_expansion_slot_device::ram_select(int bank)
{
	m_bank = bank;

	if (m_card != nullptr)
	{
		m_card->ram_select();
	}
}


// -------------------------------------------------
//  rs232_select - select RS-232 port
//-------------------------------------------------

void bw2_expansion_slot_device::rs232_select(int state)
{
	if (m_card != nullptr)
	{
		m_card->rs232_select(state);
	}
}


//-------------------------------------------------
//  slot_r - slot read
//-------------------------------------------------

u8 bw2_expansion_slot_device::slot_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_card != nullptr)
	{
		data &= m_card->slot_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  slot_w - slot write
//-------------------------------------------------

void bw2_expansion_slot_device::slot_w(offs_t offset, u8 data)
{
	if (m_card != nullptr)
	{
		m_card->slot_w(offset, data);
	}
}


//-------------------------------------------------
//  modsel_r - modem read
//-------------------------------------------------

u8 bw2_expansion_slot_device::modsel_r(offs_t offset)
{
	u8 data = 0xff;

	if (m_card != nullptr)
	{
		data &= m_card->modsel_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  modsel_w - modem write
//-------------------------------------------------

void bw2_expansion_slot_device::modsel_w(offs_t offset, u8 data)
{
	if (m_card != nullptr)
	{
		m_card->modsel_w(offset, data);
	}
}



//-------------------------------------------------
//  SLOT_INTERFACE( bw2_expansion_cards )
//-------------------------------------------------

// slot devices
#include "ramcard.h"

void bw2_expansion_cards(device_slot_interface &device)
{
	device.option_add("ramcard", BW2_RAMCARD);
}
