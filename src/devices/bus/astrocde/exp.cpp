// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************

    Bally Astrocade Expansion port

 ***********************************************************************************************************/


#include "emu.h"
#include "exp.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ASTROCADE_EXP_SLOT = &device_creator<astrocade_exp_device>;


device_astrocade_card_interface::device_astrocade_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


device_astrocade_card_interface::~device_astrocade_card_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  astrocade_exp_device - constructor
//-------------------------------------------------
astrocade_exp_device::astrocade_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
						device_t(mconfig, ASTROCADE_EXP_SLOT, "Bally Astrocade expansion", tag, owner, clock, "astrocde_exp", __FILE__),
						device_slot_interface(mconfig, *this),
						m_card_mounted(false), m_card(nullptr)
{
}


//-------------------------------------------------
//  astrocade_exp_device - destructor
//-------------------------------------------------

astrocade_exp_device::~astrocade_exp_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void astrocade_exp_device::device_start()
{
	m_card = dynamic_cast<device_astrocade_card_interface *>(get_card_device());
	if (m_card)
		m_card_mounted = true;
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t astrocade_exp_device::read(address_space &space, offs_t offset, uint8_t mem_mask)
{
	if (m_card)
		return m_card->read(space, offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void astrocade_exp_device::write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (m_card)
		m_card->write(space, offset, data);
}
