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

DEFINE_DEVICE_TYPE(ASTROCADE_EXP_SLOT, astrocade_exp_device, "astrocade_exp", "Bally Astrocade expansion")


device_astrocade_exp_interface::device_astrocade_exp_interface(const machine_config &mconfig, device_t &device) :
	device_interface(device, "astrocadeexp")
{
}


device_astrocade_exp_interface::~device_astrocade_exp_interface()
{
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  astrocade_exp_device - constructor
//-------------------------------------------------
astrocade_exp_device::astrocade_exp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, ASTROCADE_EXP_SLOT, tag, owner, clock),
	device_single_card_slot_interface<device_astrocade_exp_interface>(mconfig, *this),
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
	m_card = get_card_device();
	if (m_card)
		m_card_mounted = true;
}

/*-------------------------------------------------
 read
 -------------------------------------------------*/

uint8_t astrocade_exp_device::read(offs_t offset)
{
	if (m_card)
		return m_card->read(offset);
	else
		return 0xff;
}

uint8_t astrocade_exp_device::read_io(offs_t offset)
{
	if (m_card)
		return m_card->read_io(offset);
	else
		return 0xff;
}

/*-------------------------------------------------
 write
 -------------------------------------------------*/

void astrocade_exp_device::write(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->write(offset, data);
}

void astrocade_exp_device::write_io(offs_t offset, uint8_t data)
{
	if (m_card)
		m_card->write_io(offset, data);
}
