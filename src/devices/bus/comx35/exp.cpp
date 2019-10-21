// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    COMX-35 Expansion Slot emulation

**********************************************************************/

#include "emu.h"
#include "exp.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(COMX_EXPANSION_SLOT, comx_expansion_slot_device, "comx_expansion_slot", "COMX-35 expansion slot")



//**************************************************************************
//  DEVICE COMX_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_comx_expansion_card_interface - constructor
//-------------------------------------------------

device_comx_expansion_card_interface::device_comx_expansion_card_interface(const machine_config &mconfig, device_t &device) :
	device_slot_card_interface(mconfig, device),
	m_ds(1)
{
	m_slot = dynamic_cast<comx_expansion_slot_device *>(device.owner());
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  comx_expansion_slot_device - constructor
//-------------------------------------------------

comx_expansion_slot_device::comx_expansion_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, COMX_EXPANSION_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_write_irq(*this), m_card(nullptr)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void comx_expansion_slot_device::device_start()
{
	m_card = dynamic_cast<device_comx_expansion_card_interface *>(get_card_device());

	// resolve callbacks
	m_write_irq.resolve_safe();
}


//-------------------------------------------------
//  mrd_r - memory read
//-------------------------------------------------

uint8_t comx_expansion_slot_device::mrd_r(offs_t offset, int *extrom)
{
	uint8_t data = 0;

	if (m_card != nullptr)
		data = m_card->comx_mrd_r(offset, extrom);

	return data;
}


//-------------------------------------------------
//  mwr_w - memory write
//-------------------------------------------------

void comx_expansion_slot_device::mwr_w(offs_t offset, uint8_t data)
{
	if (m_card != nullptr)
		m_card->comx_mwr_w(offset, data);
}


//-------------------------------------------------
//  io_r - I/O read
//-------------------------------------------------

uint8_t comx_expansion_slot_device::io_r(offs_t offset)
{
	uint8_t data = 0;

	if (m_card != nullptr)
		data = m_card->comx_io_r(offset);

	return data;
}


//-------------------------------------------------
//  sout_w - I/O write
//-------------------------------------------------

void comx_expansion_slot_device::io_w(offs_t offset, uint8_t data)
{
	if (m_card != nullptr)
		m_card->comx_io_w(offset, data);
}


//-------------------------------------------------
//  ds_w - device select write
//-------------------------------------------------

WRITE_LINE_MEMBER(comx_expansion_slot_device::ds_w)
{
	if (m_card != nullptr)
		m_card->comx_ds_w(state);
}


//-------------------------------------------------
//  q_w - Q write
//-------------------------------------------------

WRITE_LINE_MEMBER(comx_expansion_slot_device::q_w)
{
	if (m_card != nullptr)
		m_card->comx_q_w(state);
}


//-------------------------------------------------
//  ef4_r - EF4 poll
//-------------------------------------------------

READ_LINE_MEMBER(comx_expansion_slot_device::ef4_r)
{
	int state = CLEAR_LINE;

	if (m_card != nullptr)
		state = m_card->comx_ef4_r();

	return state;
}


//-------------------------------------------------
//  sc_w - state code/N0-N2 write
//-------------------------------------------------

WRITE8_MEMBER(comx_expansion_slot_device::sc_w)
{
	if (m_card != nullptr)
		m_card->comx_sc_w(offset, data);
}


//-------------------------------------------------
//  tpb_w - TPB write
//-------------------------------------------------

WRITE_LINE_MEMBER(comx_expansion_slot_device::tpb_w)
{
	if (m_card != nullptr)
		m_card->comx_tpb_w(state);
}


//-------------------------------------------------
//  SLOT_INTERFACE( comx_expansion_cards )
//-------------------------------------------------

// slot devices
#include "clm.h"
#include "eprom.h"
#include "expbox.h"
#include "fdc.h"
#include "joycard.h"
#include "printer.h"
#include "ram.h"
#include "thermal.h"

void comx_expansion_cards(device_slot_interface &device)
{
	device.option_add("eb", COMX_EB);
	device.option_add("fd", COMX_FD);
	device.option_add("clm", COMX_CLM);
	device.option_add("ram", COMX_RAM);
	device.option_add("joy", COMX_JOY);
	device.option_add("prn", COMX_PRN);
	device.option_add("thm", COMX_THM);
	device.option_add("epr", COMX_EPR);
}
