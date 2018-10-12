// license:BSD-3-Clause
// copyright-holders:David Haywood
/**********************************************************************

        Sony Playstation 'Parallel' slot

**********************************************************************/

#include "emu.h"
#include "parallel.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(PSX_PARALLEL_SLOT, psx_parallel_slot_device, "psx_parallel_slot", "Playstation Parallel Slot")


//**************************************************************************
//  DEVICE SPECTRUM_EXPANSION CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  psx_parallel_interface - constructor
//-------------------------------------------------

psx_parallel_interface::psx_parallel_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device)
{
}


//-------------------------------------------------
//  ~psx_parallel_interface - destructor
//-------------------------------------------------

psx_parallel_interface::~psx_parallel_interface()
{
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  psx_parallel_slot_device - constructor
//-------------------------------------------------

psx_parallel_slot_device::psx_parallel_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PSX_PARALLEL_SLOT, tag, owner, clock),
	device_slot_interface(mconfig, *this),
	m_card(nullptr)
{
}

//-------------------------------------------------
//  expansion_slot_device - destructor
//-------------------------------------------------

psx_parallel_slot_device::~psx_parallel_slot_device()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psx_parallel_slot_device::device_start()
{
	m_card = dynamic_cast<psx_parallel_interface *>(get_card_device());
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psx_parallel_slot_device::device_reset()
{
}

//-------------------------------------------------
//  exp_r
//-------------------------------------------------

READ16_MEMBER(psx_parallel_slot_device::exp_r)
{
	if (m_card)
		return m_card->exp_r(space, offset);
	else
		return 0xff;
}

//-------------------------------------------------
//  exp_w
//-------------------------------------------------

WRITE16_MEMBER(psx_parallel_slot_device::exp_w)
{
	if (m_card)
		m_card->exp_w(space, offset, data);
}

//-------------------------------------------------
//  SLOT_INTERFACE( spectrum_expansion_devices )
//-------------------------------------------------


// slot devices
#include "gamebooster.h"

void psx_parallel_devices(device_slot_interface &device)
{
	device.option_add("gamebooster", PSX_GAMEBOOSTER);
}

