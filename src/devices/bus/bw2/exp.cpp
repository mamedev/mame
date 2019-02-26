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

device_bw2_expansion_slot_interface::device_bw2_expansion_slot_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig,device)
{
	m_slot = dynamic_cast<bw2_expansion_slot_device *>(device.owner());
}


//-------------------------------------------------
//  ~device_bw2_expansion_slot_interface - destructor
//-------------------------------------------------

device_bw2_expansion_slot_interface::~device_bw2_expansion_slot_interface()
{
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  bw2_expansion_slot_device - constructor
//-------------------------------------------------

bw2_expansion_slot_device::bw2_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, BW2_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this), m_cart(nullptr)
{
}


//-------------------------------------------------
//  bw2_expansion_slot_device - destructor
//-------------------------------------------------

bw2_expansion_slot_device::~bw2_expansion_slot_device()
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void bw2_expansion_slot_device::device_start()
{
	m_cart = dynamic_cast<device_bw2_expansion_slot_interface *>(get_card_device());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void bw2_expansion_slot_device::device_reset()
{
	if (m_cart != nullptr)
	{
		m_cart->device().reset();
	}
}


//-------------------------------------------------
//  cd_r - cartridge data read
//-------------------------------------------------

uint8_t bw2_expansion_slot_device::cd_r(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6)
{
	if (m_cart != nullptr)
	{
		data = m_cart->bw2_cd_r(offset, data, ram2, ram3, ram4, ram5, ram6);
	}

	return data;
}


//-------------------------------------------------
//  cd_w - cartridge data write
//-------------------------------------------------

void bw2_expansion_slot_device::cd_w(offs_t offset, uint8_t data, int ram2, int ram3, int ram4, int ram5, int ram6)
{
	if (m_cart != nullptr)
	{
		m_cart->bw2_cd_w(offset, data, ram2, ram3, ram4, ram5, ram6);
	}
}


//-------------------------------------------------
//  slot_r - slot read
//-------------------------------------------------

uint8_t bw2_expansion_slot_device::slot_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cart != nullptr)
	{
		data = m_cart->bw2_slot_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  slot_w - slot write
//-------------------------------------------------

void bw2_expansion_slot_device::slot_w(offs_t offset, uint8_t data)
{
	if (m_cart != nullptr)
	{
		m_cart->bw2_slot_w(offset, data);
	}
}


//-------------------------------------------------
//  modsel_r - modsel read
//-------------------------------------------------

uint8_t bw2_expansion_slot_device::modsel_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_cart != nullptr)
	{
		data = m_cart->bw2_modsel_r(offset);
	}

	return data;
}


//-------------------------------------------------
//  modsel_w - modsel write
//-------------------------------------------------

void bw2_expansion_slot_device::modsel_w(offs_t offset, uint8_t data)
{
	if (m_cart != nullptr)
	{
		m_cart->bw2_modsel_w(offset, data);
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
